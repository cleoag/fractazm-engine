#!/usr/bin/env python3
"""Chain state classes and ordered/fresh device query differentials.

For each scene the Native host prepares, the enabled slots' compiler-derived
effects (native_formula_effects.py) are combined under the program's driver.
A lane that a member consumes and the driver does not initialize is retained.
It is carried when any member or the driver writes it. The production
primary-pixel policies are evaluated on the same program words by compiling
pixel_independence.h and pixel_state_contract.h unchanged.

The device differential runs native_distance through mb3d_native_formula_probe
on fixed point sequences twice. The ordered run carries the logical lane from
query to query, as persistent primary tasks do. The fresh run starts every
query from native_initial_state, as independent pixel tasks do. Records must
match bit for bit. A match is sampled evidence for these points, not a
pixel-independence proof. A mismatch shows a real order dependence. Nothing
here changes admission.
"""
from __future__ import annotations

import argparse
from collections import Counter
from concurrent.futures import ThreadPoolExecutor
import json
import math
from pathlib import Path
import random
import struct
import subprocess
import sys

import native_formula_effects as fx
import native_formula_poison as px

ROOT = fx.ROOT
SCHEMA = "mb3d.native-formula-chains.v1"
NATIVE = ROOT / "src/render/native_metal"
PROGRAM_WORDS = 2048
SLOT_BASE, SLOT_WORDS = 56, 332
LANE = px.LANE
POLICY_SOURCE = r"""
#include "pixel_independence.h"
#include "pixel_state_contract.h"
#include <cstdio>
#include <fstream>
int main(int argc,char** argv){
    if(argc!=2)return 2;
    mb3d_native::NativeFormulaProgram p{};
    std::ifstream in(argv[1],std::ios::binary);in.read(reinterpret_cast<char*>(&p),sizeof(p));
    if(!in||sizeof(p)!=8192)return 2;
    const auto s=mb3d_native::AnalyzePrimaryPixelState(p);
    std::printf("{\"independent_primary_pixels\":%s,\"contract_known\":%s,\"contract_independent\":%s,"
        "\"production_chain\":%u}\n",mb3d_native::IndependentPrimaryPixels(p)?"true":"false",
        s.known?"true":"false",s.independent?"true":"false",s.production_chain);
}
"""


# formula_host.cpp PrepareNativeFormulaProgram: requires_persistent_lane, the
# request's legacy_formula_persistent. IndependentPrimaryPixels clears it.
HOST_PERSISTENCE_RULE = (
    "out.requires_persistent_lane=four||p.driver>=HybridDifs||",
    "(p.driver==Hybrid3DAnalytic&&(p.de_option&0x38)==32);",
    "out.requires_persistent_lane|=slot.enabled&&slot.namespace_id==ExternalFormula;",
    "if(IndependentPrimaryPixels(p))out.requires_persistent_lane=false;",
)


def legacy_persistent(words, independent_primary_pixels):
    """The host's legacy primary persistence before any state contract."""
    driver, de_option = words[1], struct.unpack("<i", struct.pack("<I", words[6]))[0]
    four = driver in (2, 3, 6, 7)
    persistent = four or driver >= 8 or (driver == 1 and (de_option & 0x38) == 32)
    for n in range(6):
        b = SLOT_BASE + n * SLOT_WORDS
        persistent |= bool(words[b + 9]) and words[b + 1] == 1
    return persistent and not independent_primary_pixels


def policy_label(production):
    """Primary ownership GeneralExecutionPolicy selects.

    An independent state contract replaces only the legacy persistent order.
    """
    if production["independent_primary_pixels"]:
        return "whitelist"
    if not production["legacy_formula_persistent"]:
        return "legacy-pixels"
    if production["production_chain"]:
        return f"contract{production['production_chain']}"
    return "state-pixels" if production["contract_independent"] else "ordered"


def policy_record(policy, path, words):
    done = subprocess.run([str(policy), str(path)], capture_output=True, text=True)
    record = json.loads(done.stdout)
    return {**record, "legacy_formula_persistent": legacy_persistent(words, record["independent_primary_pixels"])}


def f32(value):
    return struct.unpack("<f", struct.pack("<f", value))[0]


def word(value):
    return struct.unpack("<I", struct.pack("<f", value))[0]


def number(bits):
    return struct.unpack("<f", struct.pack("<I", bits))[0]


def signed(bits):
    return struct.unpack("<i", struct.pack("<I", bits))[0]


def program(words):
    """NativeFormulaProgram fields used here (formula_abi.h word offsets)."""
    slots = []
    for i in range(6):
        b = SLOT_BASE + i * SLOT_WORDS
        slots.append({"namespace": words[b + 1], "id": words[b + 2], "de_option": signed(words[b + 3]),
                      "param_count": words[b + 4], "traits": words[b + 5], "wrapper_flags": words[b + 6],
                      "difs_style": words[b + 7], "enabled": words[b + 9]})
    return {"driver": words[1], "start_slot": words[3], "repeat_slot": words[4], "end_slot": words[5],
            "de_option": signed(words[6]), "max_iterations": words[7], "julia": words[8], "inside": words[9],
            "calc_smooth": words[10], "radius_stop": words[16], "stop_scale": words[22],
            "julia_value": words[24:28], "hybrid_counts": words[50:56], "slots": slots}


def initial_state(info, point, stop):
    """native_initial_state(point, stop, program) as 47 state words."""
    s = [0] * px.STATE_WORDS
    for k in range(3):
        s[LANE[f"c[{k}]"]] = word(point[k])
    s[LANE["max_iterations"]] = info["max_iterations"]
    s[LANE["rstop"]] = info["radius_stop"]
    s[LANE["rstop_d"]] = s[LANE["rold"]] = word(f32(stop) * number(info["stop_scale"]))
    s[LANE["julia"]], s[LANE["inside"]], s[LANE["calc_smooth"]] = info["julia"], info["inside"], info["calc_smooth"]
    for k in range(4):
        s[LANE[f"j[{k}]"]] = s[LANE[f"original_j[{k}]"]] = info["julia_value"][k]
    s[LANE["derivative[0]"]] = word(1.0)
    for k in range(6):
        s[LANE[f"hybrid_counts[{k}]"]] = info["hybrid_counts"][k]
    return s


def query(lane, point, stop, info):
    """The lane as native_distance receives it: only the query point and stop differ."""
    s = list(lane)
    for k in range(3):
        s[LANE[f"c[{k}]"]] = word(point[k])
    s[LANE["rstop_d"]] = word(f32(stop) * number(info["stop_scale"]))
    return s


def unit(rng):
    while True:
        v = [rng.uniform(-1, 1) for _ in range(3)]
        n = math.sqrt(sum(x * x for x in v))
        if 1e-3 < n <= 1:
            return [x / n for x in v]


def scouts(seed, count=4096, smallest=0.02, largest=20.0):
    """Log-uniform radius, uniform direction FP32 points for locating the fractal."""
    rng = random.Random(seed)
    return [[f32(math.exp(rng.uniform(math.log(smallest), math.log(largest))) * x) for x in unit(rng)]
            for _ in range(count)]


def anchors(points, rows, count, seed):
    """Non-faulted scouts with the most iterations, sampled from the deepest quarter."""
    usable = [(r[LANE["iteration"]], i) for i, r in enumerate(rows) if not r[px.STATE_WORDS] and r[px.STATE_WORDS + 9]]
    if not usable:
        return []
    usable.sort(reverse=True)
    deepest = usable[:max(count, len(usable) // 4)]
    rng = random.Random(seed)
    return [points[i] for _, i in (rng.sample(deepest, count) if len(deepest) >= count else
                                   [rng.choice(deepest) for _ in range(count)])]


def sequences(starts, ends, steps):
    """Fixed FP32 segments between anchor points, as a march crossing the fractal would sample."""
    return [[[f32(a + (b - a) * t / (steps - 1)) for a, b in zip(start, end)] for t in range(steps)]
            for start, end in zip(starts, ends)]


def effects_index(data):
    index = {(1, f["id"]): f for f in data["formulas"]}
    index.update({(0, f["id"]): f for f in data.get("internal_formulas", [])})
    return index


def chain(info, index):
    """Union of enabled slot effects classified under the program's driver."""
    name, julia = px.driver_variant({"driver": info["driver"], "de_option": info["de_option"], "julia": info["julia"]})
    members, mismatch, consumed, written, imprecise, status = [], [], set(), set(), set(), "ANALYZED"
    for n, s in enumerate(info["slots"]):
        if not s["enabled"]:
            continue
        entry = index.get((s["namespace"], s["id"]))
        members.append({"slot": n, "namespace": "internal" if s["namespace"] == 0 else "external", "id": s["id"],
                        "name": entry["name"] if entry else None})
        if entry is None or entry["effects"].get("status") != "ANALYZED" or entry["effects"].get("opaque"):
            status = "UNANALYZED"
            continue
        if s["namespace"] == 1:
            fields = ("de_option", "param_count", "traits", "wrapper_flags", "difs_style")
            if any(s[k] != entry[k] for k in fields):
                mismatch.append({"slot": n, "id": s["id"], "program": [s[k] for k in fields],
                                 "effects": [entry[k] for k in fields]})
        e = entry["effects"]
        consumed |= set(e["consumed"]); written |= set(e["written"]); imprecise |= set(e.get("imprecise", ()))
    if mismatch:
        status = "METADATA_MISMATCH"
    union = {"status": status, "opaque": False, "consumed": sorted(consumed), "written": sorted(written),
             "imprecise": sorted(imprecise)}
    return {"driver": name, "julia": julia, "members": members, "metadata_mismatch": mismatch,
            "consumed": sorted(consumed | imprecise, key=fx.LANES.index),
            "written": sorted(written, key=fx.LANES.index), **fx.classify(union, name, julia)}


QUERY_SETS = fx.lanes("c[0]", "c[1]", "c[2]", "max_iterations", "rstop", "rstop_d", "calc_smooth")


def history_lanes(chain_result, driver, julia):
    """Lanes a query result may not depend on: all but query-set lanes, live inputs and
    retained chain reads. Driver-initialized lanes stay included; entry must overwrite them."""
    init = fx.DRIVERS[(driver, julia)]["init"]
    keep = QUERY_SETS | fx.LIVE_INPUTS | set(chain_result.get("consumed", ())) - init
    return sorted(fx.ALL_LANES - keep, key=fx.LANES.index)


def adversarial_failures(rows, groups):
    """[fresh, nan, random] triples: outputs must match fresh unless a lane only passed its own poison through."""
    failures = []
    for g in range(groups):
        i0, o0 = rows[g * 3]["input_words"], rows[g * 3]["result_words"]
        for k in (1, 2):
            iv, ov = rows[g * 3 + k]["input_words"], rows[g * 3 + k]["result_words"]
            bad = [cx_name(n) for n in range(len(o0)) if ov[n] != o0[n] and
                   not (n < px.STATE_WORDS and ov[n] == iv[n] and o0[n] == i0[n])]
            if bad:
                failures.append({"point": g, "variant": k, "words": bad[:16]})
    return failures


def cx_name(n):
    return (fx.LANES + [f"fault[{i}]" for i in range(8)] + ["distance", "completed", "escaped", "reserved"])[n]


def carried_variant(state, lanes):
    """Plausible leftovers of an earlier query: minimum iteration 7 and saved trap 0.5."""
    s = list(state)
    if "tmp" in lanes:
        s[LANE["tmp"]] = 7
    if "dfree2" in lanes:
        s[LANE["dfree2"]] = word(0.5)
    return s


def compare(ordered, fresh):
    """Lane/word names where an ordered query record differs from its fresh twin."""
    names = fx.LANES + [f"fault[{i}]" for i in range(8)] + ["distance", "completed", "escaped", "reserved"]
    return [names[i] for i, (a, b) in enumerate(zip(ordered, fresh)) if a != b]


def build_policy(output):
    source, binary = output / "policy.cpp", output / "policy"
    source.write_text(POLICY_SOURCE)
    done = subprocess.run(["xcrun", "clang++", "-std=c++20", "-O1", "-I", str(NATIVE), "-I", str(ROOT / "src"),
                           str(source), "-o", str(binary)], capture_output=True, text=True)
    if done.returncode:
        raise RuntimeError("policy helper failed: " + done.stderr[-1000:])
    return binary


def probe(args, scene, root, output, states=None):
    command = [str(args.build / "mb3d_native_formula_probe"), "--scene", str(scene),
               "--library", str(args.build / "metal_native_scene.metallib"), "--output", str(output),
               "--mode", "distance", "--repeats", "2", "--resource-root", str(root)]
    if states is not None:
        output.with_suffix(".states").write_bytes(px.pack(states))
        command += ["--states", str(output.with_suffix(".states"))]
    done = subprocess.run(command, capture_output=True, text=True, timeout=600)
    report = json.loads(output.read_text()) if output.is_file() else None
    return report, done.stderr.strip()[-300:]


def prepare(args, scene, out):
    """First resource root that prepares the scene, with the default-probe report."""
    error = ""
    for n, root in enumerate(args.resource_root):
        report, error = probe(args, scene, root, out / f"default{n}.json")
        if report:
            return root, report, None
        if "map is unavailable" not in error:
            break
    return None, None, error


def differential(args, scene, root, info, out, seed, chain_effects):
    stop = args.stop
    points = scouts(seed)
    scout, error = probe(args, scene, root, out / "scout.json", [initial_state(info, p, stop) for p in points])
    if not scout:
        return {"status": "PROBE_FAILED", "error": error}
    chosen = anchors(points, [r["result_words"] for r in scout["rows"]], 2 * args.rays, seed)
    if not chosen:
        return {"status": "NO_USABLE_POINTS"}
    paths = sequences(chosen[:args.rays], chosen[args.rays:], args.steps)
    fresh_states = [initial_state(info, p, stop) for path in paths for p in path]
    fresh, error = probe(args, scene, root, out / "fresh.json", fresh_states)
    if not fresh:
        return {"status": "PROBE_FAILED", "error": error}
    fresh_rows = [r["result_words"] for r in fresh["rows"]]
    lanes = {r: initial_state(info, (0.0, 0.0, 0.0), stop) for r in range(args.rays)}
    diffs, lanes_seen, compared, stopped, result_diffs = [], Counter(), 0, 0, 0
    for t in range(args.steps):
        live = sorted(lanes)
        if not live:
            break
        states = [query(lanes[r], paths[r][t], stop, info) for r in live]
        ordered, error = probe(args, scene, root, out / f"ordered{t:02d}.json", states)
        if not ordered:
            return {"status": "PROBE_FAILED", "step": t, "error": error}
        for r, row in zip(live, ordered["rows"]):
            result = row["result_words"]
            names = compare(result, fresh_rows[r * args.steps + t])
            compared += 1
            if names:
                diffs.append({"ray": r, "step": t, "words": names[:16]})
                lanes_seen.update(names)
                result_diffs += any(n not in fx.LANES for n in names)
            # A faulted query leaves the renderer's lane unchanged and stops the task.
            if result[px.STATE_WORDS] or not result[px.STATE_WORDS + 9]:
                del lanes[r]; stopped += 1
            else:
                lanes[r] = result[:px.STATE_WORDS]
    fresh_faults = sum(1 for r in fresh_rows if r[px.STATE_WORDS])
    # Adversarial history: every lane outside query/driver/live/retained reads poisoned.
    chain_result = chain_effects
    poison = history_lanes(chain_result, chain_result["driver"], chain_result["julia"])
    triples = [v for i, s in enumerate(fresh_states) for v in px.variants(s, poison, seed + 7 * i)]
    adversarial = None
    for part in range(0, len(triples), 3 * 1360):
        report, error = probe(args, scene, root, out / f"adversarial{part // 4080:02d}.json", triples[part:part + 3 * 1360])
        if not report:
            return {"status": "PROBE_FAILED", "error": error, "phase": "adversarial"}
        found = adversarial_failures(report["rows"], len(report["rows"]) // 3)
        adversarial = (adversarial or []) + found
    carry = None
    if chain_effects.get("driver_carried"):
        # Previous-query leftovers in the driver-carried lanes, on far points (no
        # iteration can beat the 65535 dIFS entry minimum) and on the near samples.
        rng = random.Random(seed + 13)
        far = [[f32(math.exp(rng.uniform(math.log(1e5), math.log(1e6))) * x) for x in unit(rng)] for _ in range(680)]
        base = [initial_state(info, p, stop) for p in far] + fresh_states[:680]
        rows = [v for s in base for v in (s, carried_variant(s, chain_effects["driver_carried"]))]
        report, error = probe(args, scene, root, out / "driver_carry.json", rows)
        if not report:
            return {"status": "PROBE_FAILED", "error": error, "phase": "driver_carry"}
        results = [r["result_words"] for r in report["rows"]]
        differences = [compare(results[2 * i + 1], results[2 * i]) for i in range(len(base))]
        words = Counter(n for d in differences for n in d)
        carry = {"lanes": chain_effects["driver_carried"], "far_points": 680,
                 "far_differences": sum(bool(d) for d in differences[:680]), "near_points": len(base) - 680,
                 "near_differences": sum(bool(d) for d in differences[680:]), "words": dict(words.most_common())}
    control = None
    if args.rank_control and info.get("rank") in args.rank_control:
        # Harness control: retained reads poisoned as well must change some record.
        lanes = sorted(fx.ALL_LANES - QUERY_SETS - fx.LIVE_INPUTS, key=fx.LANES.index)
        triples = [v for i, s in enumerate(fresh_states[:1360]) for v in px.variants(s, lanes, seed + 11 * i)]
        report, error = probe(args, scene, root, out / "control.json", triples)
        if not report:
            return {"status": "PROBE_FAILED", "error": error, "phase": "control"}
        found = adversarial_failures(report["rows"], len(report["rows"]) // 3)
        control = {"status": "DETECTED" if found else "NOT_DETECTED", "failures": len(found), "examples": found[:3]}
    return {"status": "MISMATCH" if diffs else "MATCH", "queries": compared, "mismatches": len(diffs),
            "retained_poison_control": control, "driver_carry": carry,
            "adversarial": {"status": "FAIL" if adversarial else "PASS", "poisoned_lanes": poison,
                            "failures": len(adversarial), "examples": adversarial[:6]},
            "distance_or_fault_mismatches": result_diffs,
            "first_step_mismatches": sum(d["step"] == 0 for d in diffs), "ordered_stopped": stopped,
            "fresh_faulted": fresh_faults, "escaped_fraction": round(sum(r[57] for r in fresh_rows) / len(fresh_rows), 3),
            "max_iteration_fraction": round(sum(r[LANE["iteration"]] == info["max_iterations"]
                                                for r in fresh_rows) / len(fresh_rows), 3),
            "deepest_iteration": max(signed(r[LANE["iteration"]]) for r in fresh_rows),
            "max_iterations": info["max_iterations"],
            "mismatch_words": dict(lanes_seen.most_common()), "examples": diffs[:6]}


def scene_result(args, policy, index, row):
    scene = ROOT / row["scene"]
    out = args.output / "raw" / f"{row['rank']:02d}"
    out.mkdir(parents=True, exist_ok=False)
    result = {"rank": row["rank"], "scene": row["scene"], "admission": row["admission"]}
    root, report, error = prepare(args, scene, out)
    if report is None:
        return {**result, "status": "PREPARATION_FAILED", "error": error}
    words = report["program_words"]
    (out / "program.bin").write_bytes(struct.pack(f"<{PROGRAM_WORDS}I", *words))
    info = {**program(words), "rank": row["rank"]}
    result.update(status="COMPLETE", resource_root=str(Path(root).relative_to(ROOT)), device=report["device"],
                  production=policy_record(policy, out / "program.bin", words), chain=chain(info, index))
    seed = int(fx.sha256(scene)[:8], 16)
    result["device_differential"] = differential(args, scene, root, info, out, seed, result["chain"])
    return result


def class_policy_device(results):
    table = Counter()
    for r in results:
        if r["status"] == "COMPLETE":
            table[f"{r['chain']['class']}|{policy_label(r['production'])}|{r['device_differential']['status']}"] += 1
    return dict(sorted(table.items()))


def policy_sources():
    return {name: fx.sha256(NATIVE / name) for name in
            ("pixel_independence.h", "pixel_state_contract.h", "soft_shadow_state_contract.h")}


def reclassify(directory):
    """Recompute production policy records from saved programs; device results are kept."""
    source = directory / "chains.json"
    summary = json.loads(source.read_text())
    work = directory / "reclassify"
    work.mkdir(exist_ok=True)
    policy = build_policy(work)
    for r in summary["results"]:
        if r["status"] == "COMPLETE":
            path = directory / "raw" / f"{r['rank']:02d}" / "program.bin"
            words = struct.unpack(f"<{PROGRAM_WORDS}I", path.read_bytes())
            r["production"] = policy_record(policy, path, words)
    summary.update(class_policy_device=class_policy_device(summary["results"]), tool_sha256=fx.sha256(Path(__file__)),
                   policy_sources_sha256=policy_sources(), reclassified_from_sha256=fx.sha256(source))
    out = directory / "chains-reclassified.json"
    out.write_text(json.dumps(summary, indent=1, sort_keys=True) + "\n")
    print(json.dumps(summary["class_policy_device"], indent=1))
    return 0


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--reclassify", type=Path, help="Recompute policy records of an existing output directory")
    parser.add_argument("--effects", type=Path)
    parser.add_argument("--queue", type=Path, default=ROOT / "comparison/scene-queue.json")
    parser.add_argument("--build", type=Path, default=ROOT / "build-native-arm64-fast")
    parser.add_argument("--resource-root", type=Path, action="append", default=[],
                        help="Tried in order while a scene's formula map is unavailable")
    parser.add_argument("--output", type=Path, help="New output directory")
    parser.add_argument("--rays", type=int, default=64)
    parser.add_argument("--steps", type=int, default=24)
    parser.add_argument("--stop", type=float, default=0.001)
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--rank", type=int, action="append", default=[])
    parser.add_argument("--rank-control", type=int, action="append", default=[],
                        help="Also poison retained reads for this scene; some record must change")
    args = parser.parse_args(argv)
    if args.reclassify:
        return reclassify(args.reclassify.resolve())
    if not args.effects or not args.resource_root or not args.output:
        parser.error("--effects, --resource-root and --output are required for a device run")
    if args.rays * args.steps > 4096 or args.steps < 2:
        parser.error("rays*steps must fit one 4096-state probe corpus and steps must be at least 2")
    args.build, args.output = args.build.resolve(), args.output.resolve()
    args.resource_root = [r.resolve() for r in args.resource_root]
    args.output.mkdir(parents=True, exist_ok=False)
    policy = build_policy(args.output)
    data = json.loads(args.effects.read_text())
    index = effects_index(data)
    rows = json.loads(args.queue.read_text())["rows"]
    if args.rank:
        rows = [r for r in rows if r["rank"] in args.rank]
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        results = list(pool.map(lambda r: scene_result(args, policy, index, r), rows))
    summary = {
        "schema": SCHEMA,
        "status": "CHAIN_CLASSES_AND_DEVICE_DIFFERENTIALS_UNQUALIFIED",
        "scope": "Single-program static chain classes and sampled ordered/fresh native_distance records; "
                 "no full frame, normals, secondary entries, admission change or qualification.",
        "effects_sha256": fx.sha256(args.effects),
        "queue_sha256": fx.sha256(args.queue),
        "library_sha256": fx.sha256(args.build / "metal_native_scene.metallib"),
        "probe_sha256": fx.sha256(args.build / "mb3d_native_formula_probe"),
        "policy_sources_sha256": policy_sources(),
        "tool_sha256": fx.sha256(Path(__file__)),
        "sampling": {"rays": args.rays, "steps": args.steps, "stop": args.stop, "scouts": 4096},
        "control_ranks": args.rank_control,
        "scenes": len(results),
        "preparation_failed": sum(r["status"] != "COMPLETE" for r in results),
        "class_policy_device": class_policy_device(results),
        "results": results,
    }
    (args.output / "chains.json").write_text(json.dumps(summary, indent=1, sort_keys=True) + "\n")
    print(json.dumps({k: v for k, v in summary.items() if k != "results"}, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
