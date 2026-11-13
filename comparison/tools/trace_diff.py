"""Compare Pascal and C++ parity trace files, report first divergence.

See docs/PARITY_TRACE.md for the contract.

Usage:
    python trace_diff.py --pascal pascal.trace --cpp cpp.trace [--schema schema.json]
                         [--mode bits|text] [--context N]

Exit codes:
    0   identical through all stages
    1   divergence found (first-divergence reported)
    2   infrastructure error (missing file, schema violation, canary failure)
"""
from __future__ import annotations

import argparse
import json
import math
import re
import struct
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Optional

LINE_RE = re.compile(r"^stage=(\S+)\s+x=(-?\d+)\s+y=(-?\d+)\s*(.*)$")


@dataclass
class TraceLine:
    raw: str
    stage: str
    x: int
    y: int
    fields: dict[str, str] = field(default_factory=dict)

    @property
    def checksum(self) -> Optional[str]:
        return self.fields.get("checksum")


def parse_line(raw: str, line_no: int, side: str) -> TraceLine:
    raw = raw.rstrip("\r\n")
    m = LINE_RE.match(raw)
    if not m:
        raise ValueError(f"{side} line {line_no}: unparseable: {raw!r}")
    stage, x, y, rest = m.group(1), int(m.group(2)), int(m.group(3)), m.group(4)
    fields: dict[str, str] = {}
    if rest:
        for tok in rest.split(" "):
            if not tok:
                continue
            if "=" not in tok:
                raise ValueError(f"{side} line {line_no}: token without '=': {tok!r}")
            k, v = tok.split("=", 1)
            if k in fields or k in {"stage", "x", "y"}:
                raise ValueError(f"{side} line {line_no}: duplicate/reserved key {k!r}")
            fields[k] = v
    return TraceLine(raw=raw, stage=stage, x=x, y=y, fields=fields)


def load_trace(path: Path, side: str) -> list[TraceLine]:
    if not path.exists():
        raise FileNotFoundError(f"{side} trace not found: {path}")
    out: list[TraceLine] = []
    with path.open("rb") as fh:
        for i, raw in enumerate(fh, 1):
            text = raw.decode("utf-8", errors="replace")
            if not text.strip():
                continue
            out.append(parse_line(text, i, side))
    return out


def schema_validate(trace: list[TraceLine], schema: dict, side: str) -> list[str]:
    issues: list[str] = []
    known_stages = set(schema.get("stages", {}).keys())
    if not known_stages:
        return issues  # schema is empty / permissive
    for i, line in enumerate(trace, 1):
        prefix = f"{side} line {i} ({line.stage})"
        if line.stage not in known_stages:
            issues.append(f"{side} line {i}: unknown stage {line.stage!r}")
            continue
        if not re.fullmatch(r"0x[0-9A-F]{16}", line.checksum or ""):
            issues.append(f"{prefix}: missing/invalid checksum")
        payload = {k: v for k, v in line.fields.items() if k != "checksum"}
        if not payload:  # Explicit checksum-only checkpoint, never field-wise evidence.
            continue
        expected = []
        hash_lines = []
        valid = True
        for spec in schema["stages"][line.stage]["fields"]:
            name, kind = spec["name"], spec["type"]
            expected.append(name)
            value = payload.get(name)
            ok = value is not None
            if kind.startswith("double"):
                expected.append(name + "_bits")
                bits = payload.get(name + "_bits", "")
                ok = ok and bool(re.fullmatch(r"0x[0-9A-F]{16}", bits))
                try:
                    float(value)
                except (TypeError, ValueError):
                    ok = False
                hash_lines += [f"{name}={bits}", f"{name}_bits={bits}"]
            else:
                if kind == "int":
                    ok = ok and bool(re.fullmatch(r"-?(?:0|[1-9][0-9]*)", value))
                elif kind == "bool_neg1":
                    ok = value in {"0", "-1"}
                elif kind == "hex64":
                    ok = ok and bool(re.fullmatch(r"0x[0-9A-F]{16}", value))
                elif kind == "string":
                    ok = ok and bool(re.fullmatch(r"[!-~]+", value))
                else:
                    ok = False
                hash_lines.append(f"{name}={value}")
            if not ok:
                valid = False
                issues.append(f"{prefix}: missing/invalid {kind} field {name}")
        if list(payload) != expected:
            valid = False
            issues.append(f"{prefix}: payload fields/order differ from schema")
        if valid:
            checksum = 0xCBF29CE484222325
            for byte in "\n".join(hash_lines).encode("ascii"):
                checksum = ((checksum ^ byte) * 0x100000001B3) & ((1 << 64) - 1)
            if line.checksum != f"0x{checksum:016X}":
                issues.append(f"{prefix}: payload checksum is invalid")
    return issues


def check_canary(pascal: list[TraceLine], cpp: list[TraceLine]) -> Optional[str]:
    if not pascal or pascal[0].stage != "_canary":
        return "pascal: first stage is not _canary"
    if not cpp or cpp[0].stage != "_canary":
        return "cpp: first stage is not _canary"
    p = pascal[0].checksum
    c = cpp[0].checksum
    if p is None or c is None:
        return f"_canary missing checksum field (pascal={p}, cpp={c})"
    if p != c:
        return (f"_canary checksums disagree (pascal={p}, cpp={c}) â€” "
                "serialization or math drift detected; inspect emitters and floating-point state")
    return None


def ulp_distance(hex_a: str, hex_b: str) -> Optional[int]:
    try:
        a = int(hex_a, 16)
        b = int(hex_b, 16)
    except (ValueError, TypeError):
        return None
    if not (0 <= a < 1 << 64 and 0 <= b < 1 << 64):
        return None
    if a == b:
        return 0
    fa, fb = (struct.unpack('>d', n.to_bytes(8, 'big'))[0] for n in (a, b))
    if not math.isfinite(fa) or not math.isfinite(fb):
        return None  # NaNs/infinities only match identical bits, never nearby finite values.
    sign = 1 << 63
    def ordered(n):
        return sign - (n & (sign - 1)) if n & sign else sign + n
    return abs(ordered(a) - ordered(b))


def diff_traces(pascal: list[TraceLine], cpp: list[TraceLine],
                mode: str, context: int, ulp_tol: int) -> tuple[int, str]:
    n = min(len(pascal), len(cpp))
    for i in range(n):
        p, c = pascal[i], cpp[i]
        if p.stage != c.stage:
            return (i, f"stage name mismatch: pascal={p.stage} cpp={c.stage}")
        if (p.x, p.y) != (c.x, c.y):
            return (i, f"pixel coords mismatch: pascal=({p.x},{p.y}) cpp=({c.x},{c.y})")
        if mode == "bits":
            if p.checksum is None or c.checksum is None:
                return (i, f"missing checksum at stage={p.stage}; bitwise comparison is invalid")
            if p.checksum != c.checksum:
                return (i, f"checksum mismatch at stage={p.stage}: "
                           f"pascal={p.checksum} cpp={c.checksum}")
        elif mode == "tolerance":
            if p.stage == "_canary":
                if p.checksum != c.checksum:
                    return (i, "canary checksum differs")
                continue
            pk, ck = set(p.fields) - {"checksum"}, set(c.fields) - {"checksum"}
            if not pk or not ck:
                return (i, "tolerance mode requires full detail payload on both sides")
            if pk != ck:
                return (i, f"payload field sets differ: pascal-only={pk-ck}, cpp-only={ck-pk}")
            # Compare every *_bits field with ULP distance <= ulp_tol.
            # Checksums will diverge even at 1 ULP; use detail-mode payload.
            for key in p.fields:
                if not key.endswith("_bits") or key == "checksum":
                    continue
                cval = c.fields.get(key)
                if cval is None:
                    return (i, f"field missing on cpp at stage={p.stage}: {key}")
                dist = ulp_distance(p.fields[key], cval)
                if dist is None or dist > ulp_tol:
                    return (i, f"field {key} at stage={p.stage} exceeds "
                               f"tolerance ({dist} ULP > {ulp_tol}): "
                               f"pascal={p.fields[key]} cpp={cval}")
            # Also check int fields match exactly
            for key in p.fields:
                if key.endswith("_bits") or key == "checksum":
                    continue
                cval = c.fields.get(key)
                if cval is None or cval != p.fields[key]:
                    # Int fields like ItResultI must match exactly
                    if key + "_bits" not in p.fields:
                        return (i, f"int/bool field {key} at stage={p.stage} "
                                   f"differs: pascal={p.fields[key]} cpp={cval}")
        else:  # text
            if p.raw != c.raw:
                return (i, f"line mismatch at stage={p.stage}")
    if len(pascal) != len(cpp):
        return (n, f"trace length mismatch: pascal={len(pascal)} cpp={len(cpp)}")
    return (-1, "")


def format_first_divergence(idx: int, reason: str,
                            pascal: list[TraceLine], cpp: list[TraceLine],
                            context: int) -> str:
    lines = ["=== FIRST DIVERGENCE ===", reason, ""]
    lo = max(0, idx - context)
    hi = min(max(len(pascal), len(cpp)), idx + context + 1)
    lines.append("pascal                                                  | cpp")
    for i in range(lo, hi):
        p = pascal[i].raw if i < len(pascal) else "<EOF>"
        c = cpp[i].raw if i < len(cpp) else "<EOF>"
        marker = ">>>" if i == idx else "   "
        lines.append(f"{marker} {p}")
        lines.append(f"    {c}")
        lines.append("")
    return "\n".join(lines)


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--pascal", type=Path, required=True)
    p.add_argument("--cpp", type=Path, required=True)
    p.add_argument("--schema", type=Path,
                   default=Path(__file__).parent / "parity_trace_schema.json")
    p.add_argument("--mode", choices=["bits", "text", "tolerance"], default="bits")
    p.add_argument("--ulp", type=int, default=4,
                   help="max ULP distance per field in tolerance mode (default 4)")
    p.add_argument("--context", type=int, default=2,
                   help="lines of context around first divergence")
    p.add_argument("--report", type=Path, default=None)
    args = p.parse_args()
    if args.ulp < 0:
        p.error("--ulp must be nonnegative")

    try:
        pascal = load_trace(args.pascal, "pascal")
        cpp = load_trace(args.cpp, "cpp")
    except (FileNotFoundError, ValueError) as e:
        print(f"error: {e}", file=sys.stderr)
        return 2

    schema: dict = {}
    if not args.schema.is_file():
        print(f"error: schema not found: {args.schema}", file=sys.stderr)
        return 2
    if args.schema.exists():
        try:
            schema = json.loads(args.schema.read_text())
        except json.JSONDecodeError as e:
            print(f"error: schema parse failed: {e}", file=sys.stderr)
            return 2

    print(f"pascal: {len(pascal)} lines")
    print(f"cpp:    {len(cpp)} lines")
    print(f"mode:   {args.mode}")

    canary_err = check_canary(pascal, cpp)
    if canary_err is not None:
        msg = f"CANARY FAIL: {canary_err}"
        print(msg, file=sys.stderr)
        if args.report:
            args.report.write_text(msg + "\n")
        return 2
    print(f"CANARY OK: checksum={pascal[0].checksum}")

    if schema:
        issues = schema_validate(pascal, schema, "pascal") + \
                 schema_validate(cpp, schema, "cpp")
        if issues:
            print("SCHEMA ISSUES:", file=sys.stderr)
            for issue in issues:
                print(f"  {issue}", file=sys.stderr)
            return 2

    idx, reason = diff_traces(pascal, cpp, args.mode, args.context, args.ulp)
    if idx < 0:
        print("IDENTICAL through all stages.")
        if args.report:
            args.report.write_text("IDENTICAL\n")
        return 0

    report = format_first_divergence(idx, reason, pascal, cpp, args.context)
    print(report)
    if args.report:
        args.report.write_text(report + "\n")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
