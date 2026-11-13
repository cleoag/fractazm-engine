#!/usr/bin/env python3
"""Source-linked ordinary-formula inventory; NOT a GPU/ARM readiness certificate.

No compilation, rendering, runtime profiling, Monte Carlo or JIT evaluation.
Run from any directory: python comparison/tools/audit_backend_readiness.py
"""

import argparse
import csv
import hashlib
import json
import re
from collections import Counter, defaultdict
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_CSV = "comparison/results/external-complete-20261013/formula-status-ordinary-v1.csv"
DEFAULT_OUTPUT = "comparison/results/backend-readiness-20261018.json"
EXCLUDED = {"JITTrigExample", "JITBenesiPineTree", "JIT_IQ_Bulb"}
TOKEN = re.compile(r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*[\s\S]*?\*/')
INCLUDE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.M)
CONSTANT = re.compile(r'\bconstexpr\s+unsigned(?:\s+int)?\s+(FORMULA_\w+)\s*=\s*(\d+)\s*;')
FEATURES = {
    "long_double": re.compile(r"\blong\s+double\b"),
    "map_callback_or_sampling": re.compile(
        r"\b(?:TLMSfunction|cpu_map_\w+|PMapFunc2?|execute_formula_with_(?:direct|sphere)_map)\b"
    ),
    "mutable_extra_state_marker": re.compile(
        r"\bparams\s*\[\s*(?:FORMULA_STATE_[XYZW]_SLOT|WRITE_J[1234]_SLOT|"
        r"BFIRSTIT_SLOT|DIFS_(?:VARYSCALE|ROLD|COMMIT_XYZ)_SLOT|MAX_FORMULA_PARAMS\s*-\s*1)"
        r"\s*\]\s*(?:[+*/-]=|=(?!=)|\+\+|--)|"
        r"\bstruct\s+ExternalFormula(?:Uses(?:Sse01|KaliState|XYZW?State)|"
        r"Writes(?:WState|PersistentCLanes)|AlwaysWritesJulia(?:XYZ|W))\s*<[^>]+>"
        r"\s*\{[^}]*\bvalue\s*=\s*true\b|"
        r"\bint32_t\s*&\s*dest\s*=\s*it->nHybrid\["
    ),
}


def strip_comments(source):
    """Preserve strings, character positions and line numbers; ignore comments."""
    return TOKEN.sub(lambda m: re.sub(r"[^\n]", " ", m[0])
                     if m[0].startswith(("//", "/*")) else m[0], source)


def masked_strings(source):
    return TOKEN.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), source)


def closing(source, start):
    pairs = {"(": ")", "{": "}", "[": "]"}
    stack = [pairs[source[start]]]
    for index in range(start + 1, len(source)):
        char = source[index]
        if char in pairs:
            stack.append(pairs[char])
        elif char in ")}]":
            if not stack or char != stack.pop():
                raise ValueError("Unbalanced C++ delimiters")
            if not stack:
                return index
    raise ValueError("Unclosed C++ delimiter")


def fields(source):
    masked = masked_strings(source)
    result, start, index = [], 0, 0
    while index < len(masked):
        if masked[index] in "({[":
            index = closing(masked, index)
        elif masked[index] == ",":
            result.append(source[start:index].strip())
            start = index + 1
        index += 1
    result.append(source[start:].strip())
    return result


def line_at(source, index):
    return source.count("\n", 0, index) + 1


def ratio(count, total, unit):
    return {"numerator": count, "denominator": total, "unit": unit,
            "percent": round(100 * count / total, 4) if total else None}


class Audit:
    def __init__(self, root):
        self.root = root.resolve()
        self.cache = {}
        self.formulas = self.root / "external_formulas/Formulas"

    def relative(self, path):
        return path.resolve().relative_to(self.root).as_posix()

    def read(self, path):
        path = path.resolve()
        if path not in self.cache:
            data = path.read_bytes()
            source = data.decode("utf-8-sig")
            self.cache[path] = (source, strip_comments(source), hashlib.sha256(data).hexdigest())
        return self.cache[path][1]

    def ref(self, path, index, symbol=None):
        result = {"path": self.relative(path), "line": line_at(self.read(path), index)}
        if symbol:
            result["symbol"] = symbol
        return result

    def resolve_include(self, parent, name):
        for path in (parent.parent / name, self.formulas / name, self.root / "src" / name):
            if path.is_file():
                return path.resolve()
        return None

    def function(self, path, name):
        source = self.read(path)
        pattern = r"\b" + re.escape(name) + r"\s*\([^;{}]*\)\s*\{"
        matches = list(re.finditer(pattern, source))
        if len(matches) != 1:
            raise ValueError(f"Expected one definition of {name} in {path}; got {len(matches)}")
        match = matches[0]
        end = closing(masked_strings(source), match.end() - 1)
        return path, match.start(), source[match.start():end + 1]

    def registrations(self):
        main = self.root / "src/core/external_formula_table.cpp"
        main_block = self.function(main, "RegisterExternalFormulaTable")
        paths = [(main, main_block)]
        # Only generated registration functions actually called by the entry point.
        calls = sorted(set(re.findall(r"\b(RegisterExternalFormulas_\w+)\s*\(\s*\)", main_block[2])))
        for call in calls:
            category = call.removeprefix("RegisterExternalFormulas_")
            path = self.root / "src/core/generated" / ("external_formula_table_" + category + ".cpp")
            paths.append((path, self.function(path, call)))
        result = defaultdict(list)
        for path, (_, registration_offset, registration_code) in paths:
            source = self.read(path)
            include_constants = defaultdict(list)
            for inc in INCLUDE.finditer(source):
                if not inc[1].endswith(".inc") or Path(inc[1]).stem in EXCLUDED:
                    continue
                incpath = self.resolve_include(path, inc[1])
                if incpath is None:
                    raise ValueError(f"Missing formula include: {path}: {inc[1]}")
                for const in CONSTANT.finditer(self.read(incpath)):
                    include_constants[const[1]].append((incpath, int(const[2]), const.start(), inc.start()))
            wrappers = defaultdict(list)
            for match in re.finditer(r"\bDEFINE_EXTERNAL_FORMULA_WRAPPER(?:_EX)?\s*\(", source):
                end = closing(masked_strings(source), match.end() - 1)
                args = fields(source[match.end():end])
                wrappers[args[0]].append((match.start(), args))
            for match in re.finditer(r"\bRegisterExternalFormula\s*\(\s*\{", registration_code):
                end = closing(masked_strings(registration_code), match.end() - 1)
                args = fields(registration_code[match.end():end])
                if len(args) < 13 or not re.fullmatch(r'"[^"\\]+"', args[1]):
                    raise ValueError(f"Unsupported registration at {path}:{line_at(source, registration_offset + match.start())}")
                name = args[1][1:-1]
                if name in EXCLUDED:
                    continue
                record = {"name": name, "token": args[0], "wrapper": args[8],
                          "de_option": int(args[2]), "map_value_mask": int(args[12], 0),
                          "registration": self.ref(path, registration_offset + match.start()), "path": path,
                          "candidates": include_constants[args[0]], "wrapper_candidates": wrappers[args[0]]}
                result[name].append(record)
        return result

    def asm_procedures(self):
        path = self.root / "src/core/win64_de_x87_helpers.asm"
        source = re.sub(r";[^\n]*", lambda m: " " * len(m[0]), self.read(path))
        result = {}
        for match in re.finditer(r"^(\w+)\s+PROC\b", source, re.M | re.I):
            end = re.search(r"^" + re.escape(match[1]) + r"\s+ENDP\b", source[match.end():], re.M | re.I)
            if end is None:
                raise ValueError(f"Missing ENDP for {match[1]}")
            body = source[match.end():match.end() + end.start()]
            # Native assembly need not contain x87 (some helpers use only SSE).
            x87 = re.search(r"^\s*(?:fld\w*|fst\w*|fadd\w*|fsub\w*|fmul\w*|fdiv\w*|"
                            r"fcom\w*|fucom\w*|fsqrt|fsin|fcos|fsincos|fpatan|fyl2x\w*|"
                            r"f2xm1|fscale|frndint|fprem\w*|fchs|fabs|fxch|fwait)\b", body, re.M | re.I)
            result[match[1]] = (self.ref(path, match.start(), match[1]), bool(x87))
        return result

    def scan_closure(self, path):
        blocks, unknown, visited = [], [], set()

        def visit(current):
            if current in visited:
                return
            visited.add(current)
            source = self.read(current)
            blocks.append((current, 0, source))
            for inc in INCLUDE.finditer(source):
                dependency = self.resolve_include(current, inc[1])
                if dependency is None:
                    unknown.append(self.ref(current, inc.start(), inc[1]))
                elif dependency.is_relative_to(self.formulas) or dependency.name.startswith("formula_"):
                    visit(dependency)
                else:
                    # Do not attribute an entire shared engine header to each formula.
                    unknown.append(self.ref(current, inc.start(), inc[1]))

        visit(path)
        return blocks, unknown

    def map_formula(self, record):
        token, wrapper, path = record["token"], record["wrapper"], record["path"]
        if wrapper == "ExternWrap_" + token:
            if len(record["candidates"]) != 1 or len(record["wrapper_candidates"]) != 1:
                return None, [], "non_unique_include_constant_or_wrapper"
            inc, numeric_id, const_offset, include_offset = record["candidates"][0]
            wrap_offset, args = record["wrapper_candidates"][0]
            if int(args[1]) != record["de_option"]:
                return None, [], "wrapper_de_option_mismatch"
            impls = list(re.finditer(r"\bexecute_formula(?:_dIFS(?:_transform)?|_with_direct_map|_with_sphere_map)?"
                                    r"\s*<\s*" + re.escape(token) + r"\s*>\s*\([^;{}]*\)\s*\{", self.read(inc)))
            if not impls:
                return None, [], "include_has_no_matching_template_specialization"
            mapping = {"kind": "included_template", "numeric_id": numeric_id,
                       "include": self.ref(path, include_offset), "constant": self.ref(inc, const_offset, token),
                       "wrapper": self.ref(path, wrap_offset),
                       "implementation": self.ref(inc, impls[0].start()),
                       "wrapper_arguments": args}
            if 10353 <= numeric_id <= 10357:
                shared = self.root / "src/core/external_formulas.h"
                guard = re.search(r"if constexpr\(\(FORMULA_ID\)>=10353 && \(FORMULA_ID\)<=10357\)", self.read(shared))
                if guard is None:
                    return None, [], "conditional_dispatch_guard_changed"
                block = self.function(shared, "ExecuteConditionalFormula")
                mapping.update(kind="conditional_engine_dispatch", declared_template=mapping["implementation"],
                               implementation=self.ref(shared, block[1], "ExecuteConditionalFormula"),
                               dispatch_guard=self.ref(shared, guard.start()))
                return mapping, [block], []
            blocks, unknown = self.scan_closure(inc)
            return mapping, blocks, unknown
        if wrapper in {"nativeBenesi1", "nativeBenesi2", "nativeBenesi3"}:
            block = self.function(path, wrapper)
            blocks = [block]
            for helper in ("benesiTransform", "nativeOption"):
                if not re.search(r"\b" + helper + r"\s*\(", block[2]):
                    return None, [], "native_helper_call_changed"
                blocks.append(self.function(path, helper))
            ids = [int(m[2]) for m in CONSTANT.finditer(self.read(path)) if m[1] == token]
            if len(ids) != 1:
                return None, [], "non_unique_native_constant"
            return {"kind": "native_cpp", "numeric_id": ids[0],
                    "implementation": self.ref(path, block[1], wrapper)}, blocks, []
        return None, [], "unknown_wrapper"

    def features(self, blocks, asm):
        findings = {name: [] for name in ["native_x87_reference", "native_asm_reference", *FEATURES]}
        for path, offset, source in blocks:
            code = masked_strings(source)
            for name, pattern in FEATURES.items():
                match = pattern.search(code)
                if match:
                    findings[name].append(self.ref(path, offset + match.start()))
            for symbol in sorted(set(re.findall(r"\b\w+\b", code)) & asm.keys()):
                match = re.search(r"\b" + symbol + r"\b", code)
                ref = self.ref(path, offset + match.start(), symbol)
                ref["assembly_definition"] = asm[symbol][0]
                findings["native_asm_reference"].append(ref)
                if asm[symbol][1]:
                    findings["native_x87_reference"].append(ref)
        return {name: {"status": "observed" if refs else "not_observed_in_scanned_sources",
                       "evidence": refs} for name, refs in findings.items()}

    def report(self, csv_path):
        self.read(csv_path)
        with csv_path.open(encoding="utf-8-sig", newline="") as stream:
            rows = list(csv.DictReader(stream))
        names = [row["formula"] for row in rows]
        if len(set(names)) != len(names):
            raise ValueError("Duplicate formula names in input CSV")
        ordinary = [row for row in rows if row["formula"] not in EXCLUDED]
        if len(ordinary) != 457 or set(names) & EXCLUDED != EXCLUDED:
            raise ValueError("Expected historical inventory of 460 names minus the three deferred names = 457")
        registrations, asm = self.registrations(), self.asm_procedures()
        inventory = []
        for row in sorted(ordinary, key=lambda r: (r["formula"].casefold(), r["formula"])):
            count = int(row["downloaded_unique_scenes"])
            if count < 0:
                raise ValueError("Negative downloaded scene count")
            entry = {"formula": row["formula"], "downloaded_unique_scenes": count}
            records = registrations[row["formula"]]
            if len(records) != 1:
                entry.update(mapping_status="unknown", reason="missing_or_duplicate_registration", features="unknown")
            else:
                record = records[0]
                entry.update(registration=record["registration"], de_option=record["de_option"],
                             map_value_mask=record["map_value_mask"], formula_token=record["token"])
                mapping, blocks, unknown = self.map_formula(record)
                if mapping is None:
                    entry.update(mapping_status="unknown", reason=unknown, features="unknown")
                else:
                    entry.update(mapping_status="mapped", source_mapping=mapping,
                                 scanned_sources=[self.ref(p, offset) for p, offset, _ in blocks],
                                 unresolved_or_out_of_scope_includes=unknown, features=self.features(blocks, asm))
            inventory.append(entry)
        mapped = sum(e["mapping_status"] == "mapped" for e in inventory)
        total = len(inventory)
        incidences = sum(e["downloaded_unique_scenes"] for e in inventory)
        top = sorted(inventory, key=lambda e: (-e["downloaded_unique_scenes"], e["formula"].casefold(), e["formula"]))[:15]
        feature_counts = {}
        for name in ["native_x87_reference", "native_asm_reference", *FEATURES]:
            statuses = Counter(e["features"][name]["status"] if e["mapping_status"] == "mapped" else "unknown" for e in inventory)
            feature_counts[name] = {"observed": ratio(statuses["observed"], total, "active ordinary formulas"),
                                    "not_observed_in_scanned_sources": statuses["not_observed_in_scanned_sources"],
                                    "unknown_unmapped": statuses["unknown"]}
        return {
            "schema_version": 1,
            "purpose": "Source-linked static inventory for backend planning; not backend readiness or parity certification",
            "scope": {"input_csv": self.relative(csv_path), "frequency_column": "downloaded_unique_scenes",
                      "input_rows": len(rows), "excluded_names": sorted(EXCLUDED), "active_ordinary_formulas": total,
                      "monte_carlo": "excluded", "jit": "deferred_and_excluded", "compiled_or_rendered": False},
            "method": [
                "Exact CSV name -> RegisterExternalFormula initializer -> wrapper token -> matching FORMULA_ID constant in a directly included .inc -> template specialization.",
                "Native Benesi maps to exact native functions and their two verified local helper definitions. Five conditional IDs map to the actual engine dispatch instead of the declared template.",
                "Features scan comment/string-masked primary sources and explicitly included formula helpers. Generic wrapper/engine headers and implicit helper calls are not transitively scanned.",
                "Assembly references are exact identifiers joined to PROC definitions; x87 means the referenced PROC contains an x87 mnemonic. Reference presence is not proof the branch executes.",
                "Both preprocessor branches and every branch in a shared helper are scanned, without template or macro evaluation. Shared helper observations may overapproximate a specific instantiation.",
                "Mutable extra-state markers are named-slot assignments or enabled state/write traits (plus conditional hybrid-state access). Numeric/aliased writes may remain unrecognized.",
                "not_observed_in_scanned_sources is not evidence of absence. Transitive dependencies, numerical portability, runtime coverage, GPU/ARM capability and performance remain unknown.",
                "Downloaded counts are imported historical corpus counts, not measured runtime costs. Their sum counts formula-scene incidences, NOT distinct scenes; no scene-coverage percentage is inferred.",
            ],
            "unknown": ["gpu_readiness", "arm_readiness", "portable_fallback_numeric_parity", "runtime_feature_reachability",
                        "complete_transitive_call_graph", "performance", "distinct_downloaded_scene_union"],
            "summary": {"mapped": ratio(mapped, total, "active ordinary formulas"), "unmapped": total - mapped,
                        "source_kinds": dict(sorted(Counter(e["source_mapping"]["kind"] for e in inventory if e["mapping_status"] == "mapped").items())),
                        "formulas_seen_in_downloads": ratio(sum(e["downloaded_unique_scenes"] > 0 for e in inventory), total, "active ordinary formulas"),
                        "sum_formula_scene_incidences": incidences,
                        "top15_incidence_share": ratio(sum(e["downloaded_unique_scenes"] for e in top), incidences, "formula-scene incidences, not distinct scenes"),
                        "static_features": feature_counts},
            "top15_by_downloaded_unique_scenes": [
                {"formula": e["formula"], "downloaded_unique_scenes": e["downloaded_unique_scenes"],
                 "implementation": e.get("source_mapping", {}).get("implementation", "unknown"),
                 "observed_static_features": [k for k, v in e["features"].items() if v["status"] == "observed"]
                 if isinstance(e["features"], dict) else "unknown"} for e in top],
            "formulas": inventory,
            "source_manifest": [{"path": self.relative(p), "sha256": self.cache[p][2]} for p in sorted(self.cache)],
        }


def self_test():
    source = 'A, "name, // still text", {1,2}, fn(3,4), /* ignored */ 0'
    parsed = fields(strip_comments(source))
    assert len(parsed) == 5 and parsed[1] == '"name, // still text"' and parsed[-1] == "0"
    clean = masked_strings(strip_comments('// long double\n"long double"; /* long double */ double x;'))
    assert not FEATURES["long_double"].search(clean)
    assert FEATURES["mutable_extra_state_marker"].search("params[BFIRSTIT_SLOT] = 1;")
    assert not FEATURES["mutable_extra_state_marker"].search("if (params[BFIRSTIT_SLOT] == 1) {}")
    assert ratio(0, 0, "items")["percent"] is None
    print("Parser/feature self-tests passed")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--csv", type=Path, default=ROOT / DEFAULT_CSV)
    parser.add_argument("--output", type=Path, default=ROOT / DEFAULT_OUTPUT)
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        self_test()
        return
    audit = Audit(ROOT)
    audit.read(Path(__file__))
    report = audit.report(args.csv.resolve())
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8", newline="\n")
    print(json.dumps({"output": str(args.output), "summary": report["summary"],
                      "top15": [(e["formula"], e["downloaded_unique_scenes"]) for e in report["top15_by_downloaded_unique_scenes"]]}, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
