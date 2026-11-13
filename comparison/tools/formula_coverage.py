"""Inventory external registry, reference assets, scene use and measured results.

Registration and scene presence are coverage, never a parity certificate.
"""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

from filter_builtin_scenes import parse_scene


def inventory(root: Path, results: Path | None) -> dict:
    records: dict[str, dict] = {}

    def record(name):
        return records.setdefault(name.casefold(), {
            "name": name, "assets": [], "translations": [], "registry": [],
            "scenes": [], "measurements": [],
        })

    for path in sorted((root / "mb3d/M3Formulas").glob("*.m3f")):
        record(path.stem)["assets"].append(path.relative_to(root).as_posix())
    for path in sorted((root / "external_formulas/Formulas").rglob("*.inc")):
        text = path.read_text(encoding="utf-8", errors="replace")
        match = re.search(r'\.name\s*=\s*"([^"]+)"', text)
        record(match[1] if match else path.stem)["translations"].append(path.relative_to(root).as_posix())
    tables = [root / "src/core/external_formula_table.cpp"]
    tables += sorted((root / "src/core/generated").glob("external_formula_table_*.cpp"))
    for path in tables:
        for match in re.finditer(r'RegisterExternalFormula\(\{\s*(FORMULA_\w+),\s*"([^"]+)"', path.read_text(encoding="utf-8", errors="replace")):
            record(match[2])["registry"].append({"id": match[1], "source": path.relative_to(root).as_posix()})

    by_scene: dict[str, set[str]] = {}
    unreadable = []
    scene_count = 0
    for folder in sorted((root / "comparison").glob("scenes*")):
        if not folder.is_dir():
            continue
        for path in sorted(folder.glob("*.m3p")):
            scene_count += 1
            scene = parse_scene(path)
            if scene.error:
                unreadable.append({"scene": path.relative_to(root).as_posix(), "error": scene.error})
                continue
            names = {slot.customName for slot in scene.formulas if slot.active and not slot.is_builtin}
            by_scene.setdefault(path.stem, set()).update(names)
            for name in names:
                record(name)["scenes"].append(path.relative_to(root).as_posix())

    if results:
        for path in sorted(results.glob("*.json")):
            data = json.loads(path.read_text(encoding="utf-8", errors="replace"))
            if not isinstance(data, dict):
                continue
            for result in data.get("results", []):
                for name in by_scene.get(result["scene"], set()):
                    record(name)["measurements"].append({
                        "scene": result["scene"], "status": result["status"],
                        "source": path.relative_to(root).as_posix(),
                    })

    formulas = sorted(records.values(), key=lambda item: item["name"].casefold())
    for item in formulas:
        item["support"] = "registered" if item["registry"] else "unregistered"
        item["coverage"] = "measured" if item["measurements"] else "scene-only" if item["scenes"] else "uncovered"
    return {
        "scope": "Static inventory; measurements apply only to the listed scenes and recorded builds.",
        "summary": {
            "assets": sum(len(item["assets"]) for item in formulas),
            "translations": sum(len(item["translations"]) for item in formulas),
            "registered_names": sum(bool(item["registry"]) for item in formulas),
            "registered_without_scene": sum(bool(item["registry"]) and not item["scenes"] for item in formulas),
            "registered_with_measurement": sum(bool(item["registry"]) and bool(item["measurements"]) for item in formulas),
            "reference_assets_unregistered": sum(bool(item["assets"]) and not item["registry"] for item in formulas),
            "scenes_scanned": scene_count,
        },
        "unreadable_scenes": unreadable, "formulas": formulas,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--results", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    report = inventory(args.root.resolve(), args.results.resolve() if args.results else None)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report["summary"]))


if __name__ == "__main__":
    main()
