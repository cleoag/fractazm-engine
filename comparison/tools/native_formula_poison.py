#!/usr/bin/env python3
"""Device poison probes for compiler-derived Native formula effects.

For every enabled external slot of each per-formula scene, the real Native
library executes one formula step on three state variants: the probe's point
state and two copies whose lanes outside the static consumed set carry
different poison words. Consumed outputs, faults and work must be bit-identical,
and lanes outside the static write set must keep their own input words. A
driver-entry probe poisons the lanes native_run_orbit initializes and requires
identical orbit records.

This checks the static effect model on the device for finite point states. It
is not formula qualification, a pixel-independence proof or a scene result.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import json
from pathlib import Path
import random
import struct
import subprocess
import sys

import native_formula_effects as fx

ROOT = fx.ROOT
SCHEMA = "mb3d.native-formula-poison.v1"
STATE_WORDS = len(fx.LANES)
RESULT_WORDS = 59
DRIVER_NAMES = ("Hybrid3D", "Hybrid3DAnalytic", "Hybrid4D", "Hybrid4DAnalytic", "Interpolated3D",
                "Interpolated3DAnalytic", "Interpolated4D", "Interpolated4DAnalytic", "HybridDifs",
                "HybridDifsNoInitialization")
LANE = {name: i for i, name in enumerate(fx.LANES)}
# native_step's reads (FP32 range check and fault record); it records the slot.
STEP_READS = fx.STEP_READS
STEP_WRITES = frozenset({"current_slot"})


def poison_words(seed):
    """Two deterministic poison patterns: tagged quiet NaNs and seeded random words."""
    nan = [0x7FC00000 | (0x5A00 + lane) for lane in range(STATE_WORDS)]
    rng = random.Random(seed)
    return nan, [rng.getrandbits(32) for _ in range(STATE_WORDS)]


def variants(base, poison_lanes, seed):
    rows = [list(base)]
    for pattern in poison_words(seed):
        row = list(base)
        for lane in poison_lanes:
            row[LANE[lane]] = pattern[LANE[lane]]
        rows.append(row)
    return rows


def nudge(word):
    """A finite, clearly different word: x*1.5+0.75 for finite floats, 0.5 otherwise."""
    value = struct.unpack("<f", struct.pack("<I", word))[0]
    value = value * 1.5 + 0.75 if value == value and abs(value) < 1e30 else 0.5
    return struct.unpack("<I", struct.pack("<f", value))[0]


def lane_variants(base, lanes):
    """One row per (lane, pattern): the lane alone nudged, then the lane alone as tagged NaN."""
    nan = poison_words(0)[0]
    rows = []
    for lane in lanes:
        for word in (nudge(base[LANE[lane]]), nan[LANE[lane]]):
            row = list(base)
            row[LANE[lane]] = word
            rows.append(row)
    return rows


def observed_lanes(base_row, rows, lanes):
    """Lanes whose single-lane change altered any result word other than its own pass-through."""
    seen = set()
    base = base_row["result_words"]
    for i, lane in enumerate(lanes):
        for r in rows[i * 2:i * 2 + 2]:
            out, word = r["result_words"], LANE[lane]
            if any(a != b for k, (a, b) in enumerate(zip(base, out)) if k != word) or \
                    out[word] not in (base[word], r["input_words"][word]):
                seen.add(lane)
    return seen


def contained_violations(base_row, rows, lanes, contained):
    """A self-contained lane's nudge or NaN may change only its own result word."""
    base, found = base_row["result_words"], []
    for i, lane in enumerate(lanes):
        if lane not in contained:
            continue
        for k, r in enumerate(rows[i * 2:i * 2 + 2]):
            changed = [fx.LANES[w] if w < STATE_WORDS else f"word{w}"
                       for w, (a, b) in enumerate(zip(base, r["result_words"])) if a != b and w != LANE[lane]]
            if changed:
                found.append({"lane": lane, "variant": ("nudge", "nan")[k], "changed": changed})
    return found


def pack(rows):
    return b"".join(struct.pack("<%dI" % STATE_WORDS, *row) for row in rows)


def program_info(words):
    slots = []
    for i in range(6):
        b = 56 + i * 332
        slots.append({"namespace": words[b + 1], "id": words[b + 2], "enabled": words[b + 9]})
    de_option = struct.unpack("<i", struct.pack("<I", words[6]))[0]
    return {"driver": words[1], "de_option": de_option, "julia": words[8], "slots": slots}


def driver_variant(info):
    name = DRIVER_NAMES[info["driver"]]
    if name in ("Hybrid3DAnalytic", "Hybrid4DAnalytic") and (info["de_option"] & 0x38) == 32:
        name += "/xyz-derivative"
    return name, bool(info["julia"])


def orbit_entry_lanes(name, julia):
    """Lanes native_run_orbit itself writes before any read (no query/live inputs)."""
    init = fx.DRIVERS[(name, julia)]["init"]
    return sorted(init - fx.QUERY_INIT - fx.LIVE_INPUTS - {"current_slot", "rold"} -
                  ({"tmp"} if name == "HybridDifs" else set()), key=fx.LANES.index)


def run_probe(args, output, scene, mode, slot, states=None, root=None):
    command = [str(args.build / "mb3d_native_formula_probe"), "--scene", str(scene),
               "--library", str(args.build / "metal_native_scene.metallib"), "--output", str(output),
               "--mode", mode, "--slot", str(slot), "--repeats", "2", "--resource-root",
               str(root or args.resource_root[0])]
    if states is not None:
        output.with_suffix(".states").write_bytes(pack(states))
        command += ["--states", str(output.with_suffix(".states"))]
    done = subprocess.run(command, capture_output=True, text=True, timeout=600)
    record = {"command": command[1:], "returncode": done.returncode, "stderr": done.stderr.strip()[-500:]}
    if output.is_file():
        record["report"] = json.loads(output.read_text())
    return record


def compare_step(rows, effects, groups):
    """rows: probe rows in [base, nan, random] groups. Returns failures."""
    written = set(effects["written"]) | STEP_WRITES
    consumed = set(effects["consumed"]) | STEP_READS
    failures = []
    for g in range(groups):
        (i0, o0), *others = [(rows[g * 3 + k]["input_words"], rows[g * 3 + k]["result_words"]) for k in range(3)]
        for k, (iv, ov) in enumerate(others, 1):
            if ov[STATE_WORDS:] != o0[STATE_WORDS:]:
                failures.append({"point": g, "variant": k, "field": "fault/distance/completed",
                                 "expected": o0[STATE_WORDS:], "observed": ov[STATE_WORDS:]})
                continue
            for lane, name in enumerate(fx.LANES):
                if name in written:
                    ok = ov[lane] == o0[lane] or (name not in consumed and ov[lane] == iv[lane] and o0[lane] == i0[lane])
                else:
                    ok = ov[lane] == iv[lane]
                if not ok:
                    failures.append({"point": g, "variant": k, "lane": name, "input": iv[lane],
                                     "expected": o0[lane], "observed": ov[lane]})
    return failures


def compare_orbit(rows, groups):
    failures = []
    for g in range(groups):
        base = rows[g * 3]["result_words"]
        for k in (1, 2):
            observed = rows[g * 3 + k]["result_words"]
            if observed != base:
                diff = [fx.LANES[i] if i < STATE_WORDS else f"word{i}" for i, (a, b) in enumerate(zip(base, observed)) if a != b]
                failures.append({"point": g, "variant": k, "differences": diff[:12]})
    return failures


def probe_scene(args, effects, scene):
    out = args.output / "raw" / scene.stem
    out.mkdir(parents=True, exist_ok=False)
    # Later resource roots are tried only while a formula map is unavailable.
    for n, root in enumerate(args.resource_root):
        first = run_probe(args, out / f"default{n}.json", scene, "step", 0, root=root)
        if "report" in first or "map is unavailable" not in first["stderr"]:
            break
    result = {"scene": str(scene.relative_to(ROOT)), "scene_sha256": fx.sha256(scene), "slots": [], "orbit": None,
              "resource_root": str(root.relative_to(ROOT)) if root.is_relative_to(ROOT) else str(root)}
    if "report" not in first:
        result["status"] = "PREPARATION_FAILED"
        result["error"] = first["stderr"]
        return result
    report = first["report"]
    info = program_info(report["program_words"])
    bases = [row["input_words"] for row in report["rows"]]
    result.update(driver=driver_variant(info)[0], julia=bool(info["julia"]), point_count=len(bases),
                  device=report["device"])
    seed = int(fx.sha256(scene)[:8], 16)
    for slot, s in enumerate(info["slots"]):
        if not s["enabled"] or s["namespace"] != 1:
            continue
        entry = effects.get(s["id"])
        row = {"slot": slot, "id": s["id"], "name": entry["name"] if entry else None}
        if entry is None or entry["effects"].get("status") != "ANALYZED" or entry["effects"].get("opaque"):
            row["status"] = "NO_STATIC_EFFECTS"
            result["slots"].append(row); continue
        e = entry["effects"]
        poison = sorted(fx.ALL_LANES - set(e["consumed"]) - STEP_READS, key=fx.LANES.index)
        states = [v for i, base in enumerate(bases) for v in variants(base, poison, seed + slot * 7919 + i)]
        probe = run_probe(args, out / f"step-slot{slot}.json", scene, "step", slot, states, root)
        if "report" not in probe:
            row.update(status="PROBE_FAILED", error=probe["stderr"])
        else:
            failures = compare_step(probe["report"]["rows"], e, len(bases))
            faults = sum(r["result_words"][STATE_WORDS] != 0 for r in probe["report"]["rows"][::3])
            row.update(status="FAIL" if failures else "PASS", poisoned_lanes=len(poison),
                       baseline_faulted_points=faults, failures=failures[:10])
            # Positive control: each statically consumed lane alone, same process and slot.
            lanes = sorted(e["consumed"], key=fx.LANES.index)
            states = [r for base in bases for r in [base] + lane_variants(base, lanes)]
            control = run_probe(args, out / f"control-slot{slot}.json", scene, "step", slot, states, root)
            if "report" not in control:
                row["control"] = {"status": "PROBE_FAILED", "error": control["stderr"]}
            else:
                rows, width, seen = control["report"]["rows"], 1 + 2 * len(lanes), set()
                # native_step itself reads STEP_READS, so only other lanes can stay contained.
                contained, violations = set(e.get("self_only", ())) - STEP_READS, []
                for g in range(len(bases)):
                    group = rows[g * width + 1:(g + 1) * width]
                    seen |= observed_lanes(rows[g * width], group, lanes)
                    violations += [{"point": g, **v} for v in contained_violations(rows[g * width], group, lanes, contained)]
                row["control"] = {"status": "COMPLETE", "observed": sorted(seen, key=fx.LANES.index),
                                  "unobserved": [lane for lane in lanes if lane not in seen],
                                  "contained": sorted(contained, key=fx.LANES.index)}
                if violations:
                    row.update(status="FAIL", failures=(row["failures"] + violations)[:10])
        result["slots"].append(row)
    name, julia = driver_variant(info)
    lanes = orbit_entry_lanes(name, julia)
    states = [v for i, base in enumerate(bases) for v in variants(base, lanes, seed + 104729 + i)]
    probe = run_probe(args, out / "orbit.json", scene, "orbit", 0, states, root)
    if "report" not in probe:
        result["orbit"] = {"status": "PROBE_FAILED", "error": probe["stderr"]}
    else:
        failures = compare_orbit(probe["report"]["rows"], len(bases))
        faults = sum(r["result_words"][STATE_WORDS] != 0 for r in probe["report"]["rows"][::3])
        result["orbit"] = {"status": "FAIL" if failures else "PASS", "driver": name, "julia": julia,
                           "poisoned_lanes": lanes, "baseline_faulted_points": faults, "failures": failures[:10]}
    result["status"] = "COMPLETE"
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--effects", type=Path, required=True, help="effects.json from native_formula_effects.py")
    parser.add_argument("--build", type=Path, default=ROOT / "build-native-arm64-fast")
    parser.add_argument("--scenes", default="comparison/scenes/extall_*.m3p")
    parser.add_argument("--resource-root", type=Path, action="append",
                        help="Repeatable; later roots are tried while a formula map is unavailable "
                             "(default comparison/fixtures)")
    parser.add_argument("--output", type=Path, required=True, help="New output directory")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--limit", type=int, default=0)
    args = parser.parse_args(argv)
    args.build, args.output = args.build.resolve(), args.output.resolve()
    args.resource_root = [r.resolve() for r in (args.resource_root or [ROOT / "comparison/fixtures"])]
    args.output.mkdir(parents=True, exist_ok=False)
    data = json.loads(args.effects.read_text())
    effects = {f["id"]: f for f in data["formulas"]}
    scenes = sorted(ROOT.glob(args.scenes))
    if args.limit:
        scenes = scenes[:args.limit]
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        results = list(pool.map(lambda scene: probe_scene(args, effects, scene), scenes))
    tested, passed, failed = {}, set(), set()
    for r in results:
        for s in r.get("slots", []):
            tested.setdefault(s["id"], []).append(s["status"])
    for ident, states in tested.items():
        (failed if "FAIL" in states else passed if "PASS" in states else set()).add(ident)
    orbit = [r["orbit"]["status"] for r in results if r.get("orbit")]
    summary = {
        "schema": SCHEMA,
        "status": "DEVICE_EFFECT_PROBES_UNQUALIFIED",
        "scope": "Single-step and orbit-entry poison probes on finite point states; not formula, chain or scene qualification.",
        "effects_sha256": fx.sha256(args.effects),
        "library_sha256": fx.sha256(args.build / "metal_native_scene.metallib"),
        "probe_sha256": fx.sha256(args.build / "mb3d_native_formula_probe"),
        "tool_sha256": fx.sha256(Path(__file__)),
        "devices": sorted({r["device"] for r in results if "device" in r}),
        "scenes": len(results),
        "preparation_failed": sum(r["status"] == "PREPARATION_FAILED" for r in results),
        "formulas_with_step_pass": len(passed - failed),
        "formulas_with_step_fail": sorted(failed),
        "formulas_untested": sorted(set(effects) - set(tested)),
        "formulas_without_pass": sorted(i for i, states in tested.items() if "PASS" not in states),
        "orbit_entry": {s: orbit.count(s) for s in sorted(set(orbit))},
        "results": results,
    }
    (args.output / "poison.json").write_text(json.dumps(summary, indent=1, sort_keys=True) + "\n")
    print(json.dumps({k: len(v) if isinstance(v, list) and k.startswith("formulas_") else v
                      for k, v in summary.items() if k != "results"}, indent=1))
    return 1 if failed or "FAIL" in orbit else 0


if __name__ == "__main__":
    sys.exit(main())
