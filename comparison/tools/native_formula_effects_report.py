#!/usr/bin/env python3
"""Compact per-formula evidence from static effects and device poison probes.

Joins effects.json (native_formula_effects.py) with poison.json
(native_formula_poison.py). Each external formula gets the class under the
driver of every per-formula scene that probed it, the device step/orbit-entry
outcome and which of its retained (driver-uninitialized) lanes changed the
device result. Nothing here qualifies a formula, chain or scene.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys

import native_formula_effects as fx

SCHEMA = "mb3d.native-formula-effects-evidence.v1"


def key(driver, julia):
    return driver + ("/julia" if julia else "")


def build(effects, poison):
    formulas = {f["id"]: f for f in effects["formulas"]}
    homes = {}
    for scene in poison["results"]:
        if scene["status"] != "COMPLETE":
            continue
        for slot in scene["slots"]:
            homes.setdefault(slot["id"], []).append((scene, slot))
    skipped, loaded = {}, {}
    for scene in poison["results"]:
        stem = Path(scene["scene"]).stem
        if scene["status"] != "COMPLETE":
            skipped[stem] = scene["error"]
        else:
            loaded[stem] = [slot["id"] for slot in scene["slots"]]
    rows, counts = [], {}
    for ident, f in sorted(formulas.items()):
        row = {"id": ident, "name": f["name"], "family": f["family"], "static": f["effects"]["status"]}
        if f["effects"].get("opaque"):
            row["static"] = "OPAQUE"
        row["probes"] = []
        for scene, slot in homes.get(ident, []):
            driver = key(scene["driver"], scene["julia"])
            cls = f["classes"][driver]
            init = fx.DRIVERS[(scene["driver"], scene["julia"])]["init"]
            retained = sorted(set(f["effects"].get("consumed", [])) - init - fx.LIVE_INPUTS, key=fx.LANES.index)
            control = slot.get("control", {})
            probe = {"scene": Path(scene["scene"]).stem, "slot": slot["slot"], "driver": driver,
                     "class": cls["class"], "step": slot["status"], "orbit_entry": scene["orbit"]["status"]}
            for name in ("bound", "formula_class", "driver_carried", "invariant_unless_written", "carried",
                         "mutated_live", "imprecise"):
                if cls.get(name):
                    probe[name] = cls[name]
            if retained and "observed" in control:
                probe["retained_observed"] = [lane for lane in retained if lane in control["observed"]]
            if control.get("unobserved"):
                probe["consumed_unobserved"] = control["unobserved"]
            row["probes"].append(probe)
            bucket = counts.setdefault(driver, {})
            evidence = ("device" if slot["status"] == "PASS" and scene["orbit"]["status"] == "PASS"
                        else f'step:{slot["status"]}/orbit:{scene["orbit"]["status"]}')
            bucket.setdefault(cls["class"], {}).setdefault(evidence, 0)
            bucket[cls["class"]][evidence] += 1
        if not row["probes"]:
            row["untested"] = untested_reason(f["name"], skipped, loaded)
        rows.append(row)
    return rows, counts, skipped


def untested_reason(name, skipped, loaded):
    """Why no per-formula scene probed this formula (scene stems are extall_<family>_<name>)."""
    wanted = name.replace(" ", "_").replace("-", "_")
    for stem in sorted(set(skipped) | set(loaded)):
        if stem.startswith("extall_") and stem.split("_", 2)[2] == wanted:
            if stem in skipped:
                return f"{stem}: {skipped[stem]}"
            return f"{stem}: scene loads formula IDs {loaded[stem]}"
    return "no extall scene"


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--effects", type=Path, required=True)
    parser.add_argument("--poison", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True, help="formulas.json to write")
    args = parser.parse_args(argv)
    effects, poison = json.loads(args.effects.read_text()), json.loads(args.poison.read_text())
    if poison["effects_sha256"] != fx.sha256(args.effects):
        sys.exit("poison.json was produced from a different effects.json")
    rows, counts, skipped = build(effects, poison)
    untested = {r["id"]: r["untested"] for r in rows if not r["probes"]}
    result = {
        "schema": SCHEMA,
        "status": "STATIC_EFFECTS_WITH_DEVICE_PROBES_UNQUALIFIED",
        "scope": "Per-formula single-step effects and orbit-entry initialization on finite probe points of the "
                 "per-formula extall scenes; no chain, pixel-independence, scene or admission claim.",
        "effects_sha256": poison["effects_sha256"],
        "poison_sha256": fx.sha256(args.poison),
        "library_sha256": poison["library_sha256"],
        "probe_sha256": poison["probe_sha256"],
        "effects_tool_sha256": effects["tool_sha256"],
        "poison_tool_sha256": poison["tool_sha256"],
        "report_tool_sha256": fx.sha256(Path(__file__)),
        "toolchain": effects["toolchain"],
        "devices": poison["devices"],
        "static_summary": effects["summary"],
        "audit_contradictions": effects["audit_contradictions"],
        "audit_unconfirmed": effects["audit_unconfirmed"],
        "home_driver_classes": counts,
        "device_untested": untested,
        "preparation_failed_scenes": skipped,
        "formulas": rows,
        "internal_formulas": [{"id": f["id"], "name": f["name"],
                               **{k: f["effects"].get(k) for k in ("status", "consumed", "written", "imprecise")}}
                              for f in effects.get("internal_formulas", [])],
    }
    args.output.write_text(json.dumps(result, indent=1, sort_keys=True) + "\n")
    print(json.dumps({k: result[k] for k in ("home_driver_classes", "device_untested")}, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
