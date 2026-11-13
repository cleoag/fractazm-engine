#!/usr/bin/env python3
"""Find external formulas that use .m3f [CONSTANTS] and compare that with current C++ registration metadata.

This helps detect formulas where positive PVar constants are likely not modeled faithfully yet.
"""

from __future__ import annotations

import re
from pathlib import Path


PAT = re.compile(
    r'RegisterExternalFormula\(\{\s*FORMULA_\w+,\s*"([^"]+)",\s*(-?\d+),\s*(\d+),\s*'
    r'([\d.]+),\s*([\d.]+),\s*([\d.]+),\s*\w+,\s*&\w+,\s*(\d+),\s*\{([^}]*)\}'
)


def count_constants(m3f_path: Path) -> int:
    text = m3f_path.read_text(errors="replace").splitlines()
    in_sec = False
    count = 0
    for line in text:
        s = line.strip()
        if s == "[CONSTANTS]":
            in_sec = True
            continue
        if in_sec and s.startswith("["):
            break
        if in_sec and s and not s.startswith(";"):
            count += 1
    return count


def load_registrations() -> dict[str, dict]:
    regs: dict[str, dict] = {}
    files = [Path("src/core/external_formula_table.cpp")] + sorted(
        Path("src/core/generated").glob("external_formula_table_*.cpp")
    )
    for f in files:
        txt = f.read_text(errors="replace")
        for m in PAT.finditer(txt):
            regs[m.group(1)] = {
                "deopt": int(m.group(2)),
                "param_count": int(m.group(3)),
                "opt_count": int(m.group(7)),
            }
    return regs


def main() -> None:
    regs = load_registrations()
    rows = []
    for p in sorted(Path("mb3d/M3Formulas").glob("*.m3f")):
        txt = p.read_text(errors="replace")
        if "[CONSTANTS]" not in txt:
            continue
        name = p.stem
        if name not in regs:
            continue
        c = count_constants(p)
        rows.append((name, c, regs[name]["param_count"], regs[name]["opt_count"], regs[name]["deopt"]))

    print("formula\tconstants\tparam_count\topt_count\tdeoption")
    for row in sorted(rows, key=lambda r: (r[3], -r[1], r[0])):
        print("\t".join(str(x) for x in row))


if __name__ == "__main__":
    main()
