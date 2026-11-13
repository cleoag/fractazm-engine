#!/usr/bin/env python3
"""Compiler-derived NativeFormulaState effects of lowered external formulas.

Each lowered external formula is compiled through a byte-identical copy of the
production wrapper with that formula's fixed ABI metadata. The Metal frontend
inlines the wrapper/body and air-opt removes dead state. Loads and stores that
survive on the input/output state buffers are the consumed and written lanes;
an identity copy from input to the same output lane is neither. Any pointer use
the parser does not understand makes the formula opaque: every lane counts as
consumed and written.

The per-driver classes combine these lanes with the orbit driver's entry
initialization. They are static scheduling evidence only: no formula, chain or
scene is qualified, and production admission is unchanged. Device poison
probes and ordered/parallel full-frame comparisons remain separate gates.
"""
from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
NATIVE = Path("src/render/native_metal")
SCHEMA = "mb3d.native-formula-effects.v1"
UNROLL_THRESHOLD = 4000
STATE_BYTES = 188

# NativeFormulaState in formula_abi.h; every lane is four bytes.
STATE_FIELDS = (
    ("xyz", 4), ("c", 4), ("j", 4), ("original_j", 4), ("derivative", 4), ("sse", 4),
    ("rout", 1), ("rold", 1), ("rstop", 1), ("rstop_d", 1), ("otrap", 1), ("vary_scale", 1),
    ("dfree1", 1), ("dfree2", 1), ("smooth", 1), ("iteration", 1), ("first", 1), ("tmp", 1),
    ("max_iterations", 1), ("hybrid_counts", 6), ("julia", 1), ("inside", 1), ("calc_smooth", 1),
    ("current_slot", 1),
)
FIELD_OFFSETS, LANES = [], []
for _name, _count in STATE_FIELDS:
    FIELD_OFFSETS.append(len(LANES) * 4)
    LANES.extend(f"{_name}[{i}]" if _count > 1 else _name for i in range(_count))
assert len(LANES) * 4 == STATE_BYTES
ALL_LANES = frozenset(LANES)


def lanes(*names):
    result = set()
    for name in names:
        expanded = [lane for lane in LANES if lane == name or lane.startswith(name + "[")]
        if not expanded:
            raise ValueError("Unknown state lane: " + name)
        result.update(expanded)
    return frozenset(result)


def byte_lanes(begin, size):
    return {LANES[i] for i in range(max(0, begin // 4), min(len(LANES), (begin + size + 3) // 4))}


# native_step after every formula step: the FP32 range check reads xyz, rout and
# vary_scale; a fault record reads iteration (orbit_driver.metal.h).
STEP_READS = frozenset({"xyz[0]", "xyz[1]", "xyz[2]", "xyz[3]", "rout", "vary_scale", "iteration"})
# Scene/task constants: set once per logical lane and never reset per orbit.
LIVE_INPUTS = lanes("original_j", "hybrid_counts", "julia", "inside")
# Set by native_distance_controlled before every DE query.
QUERY_INIT = lanes("c[0]", "c[1]", "c[2]", "max_iterations", "rstop", "rstop_d", "calc_smooth")


def driver_variants():
    """Lanes definitely initialized before a formula step, and lanes the driver may write.

    Source: orbit_driver.metal.h native_orbit_passes entry and stores before
    native_step, native_step's current_slot and native_distance_controlled's
    query setup. Ordinary callers retain Vary; only dIFS initializes it. Only
    4D assigns j.w, including in Julia mode. Interpolation retains tmp.
    Ordinary return SSE stores are possible writes, not entry initialization:
    a body may consume the preceding query's value before those stores occur.
    These single-formula variants do not prove a combined caller or a later
    body's conditional first-write; per-body may-write masks cannot do that.
    """
    variants = {}
    specs = (
        ("Hybrid3D", dict(four=False, analytic=False, interpolated=False, difs=False, mode=0)),
        ("Hybrid3DAnalytic", dict(four=False, analytic=True, interpolated=False, difs=False, mode=0)),
        ("Hybrid3DAnalytic/xyz-derivative", dict(four=False, analytic=True, interpolated=False, difs=False, mode=32)),
        ("Hybrid4D", dict(four=True, analytic=False, interpolated=False, difs=False, mode=0)),
        ("Hybrid4DAnalytic", dict(four=True, analytic=True, interpolated=False, difs=False, mode=0)),
        ("Hybrid4DAnalytic/xyz-derivative", dict(four=True, analytic=True, interpolated=False, difs=False, mode=32)),
        ("Interpolated3D", dict(four=False, analytic=False, interpolated=True, difs=False, mode=0)),
        ("Interpolated3DAnalytic", dict(four=False, analytic=True, interpolated=True, difs=False, mode=0)),
        ("Interpolated4D", dict(four=True, analytic=False, interpolated=True, difs=False, mode=0)),
        ("Interpolated4DAnalytic", dict(four=True, analytic=True, interpolated=True, difs=False, mode=0)),
        ("HybridDifs", dict(four=False, analytic=False, interpolated=False, difs=True, mode=0, no_init=False)),
        ("HybridDifsNoInitialization", dict(four=False, analytic=False, interpolated=False, difs=True, mode=0, no_init=True)),
    )
    for name, v in specs:
        for julia in (False, True):
            init = set(QUERY_INIT) | lanes("first", "iteration", "rout", "otrap", "current_slot", "rold")
            writes = lanes("xyz[0]", "xyz[1]", "xyz[2]", "first", "iteration", "rout", "otrap", "current_slot", "rold")
            if v["four"] or v["interpolated"]:
                writes |= lanes("xyz[3]")  # 4D rotation/norm and interpolation blend
            if not v.get("no_init"):
                init |= lanes("xyz[0]", "xyz[1]", "xyz[2]", "j[0]", "j[1]", "j[2]")
                writes |= lanes("j[0]", "j[1]", "j[2]")
                # Hybrid3D analytic XYZ-derivative mode retains the incoming w lane.
                if v["four"] or not (v["analytic"] and v["mode"] == 32 and not v["interpolated"]):
                    init |= lanes("xyz[3]")
                    writes |= lanes("xyz[3]")
                if v["four"]:
                    init |= lanes("j[3]")
                    writes |= lanes("j[3]")
            if v["difs"]:
                init |= lanes("vary_scale")
                writes |= lanes("vary_scale")
            if v["analytic"]:
                if v["four"]:
                    init |= lanes("derivative[0]")
                    writes |= lanes("derivative[0]")
                    if v["mode"] == 32:
                        init |= lanes("derivative[1]", "derivative[2]")
                        writes |= lanes("derivative[1]", "derivative[2]")
                elif not v["interpolated"] and v["mode"] == 32:
                    init |= lanes("derivative[0]", "derivative[1]", "derivative[2]")
                    writes |= lanes("derivative[0]", "derivative[1]", "derivative[2]")
            if name == "Hybrid3D" or v["interpolated"]:
                init |= lanes("sse")
            if not v["difs"]:
                # Includes conditional analytic return transport; do not infer
                # that SSE was initialized before the first callback.
                writes |= lanes("sse")
            if v["interpolated"] and v["four"] and v["analytic"]:
                writes |= lanes("derivative[0]")
            if v["difs"]:
                init |= lanes("dfree1", "tmp") if v.get("no_init") else lanes("dfree1")
                writes |= lanes("dfree1", "dfree2", "tmp", "smooth")
            else:
                if not v["interpolated"]:
                    init |= lanes("tmp")
                    writes |= lanes("tmp")
                writes |= lanes("smooth")
            variants[(name, julia)] = {"init": frozenset(init), "writes": frozenset(writes)}
    return variants


DRIVERS = driver_variants()
# dIFS drivers consume retained lanes when no iteration improves the 65535
# entry minimum (orbit_driver.metal.h: minimum_iteration=s.tmp, otrap=s.dfree2);
# the no-initialization variant resets tmp but still returns the carried dfree2.
DRIVER_CARRIED_READS = {"HybridDifs": lanes("tmp", "dfree2"), "HybridDifsNoInitialization": lanes("dfree2")}


def classify(effects, driver, julia=False):
    """A: consumes only per-orbit/query initialized lanes or immutable live inputs.
    B: also consumes lanes nobody in a single-formula chain writes (they keep the
       logical lane's initial value); another chain member could still write them.
    C: consumes a retained lane this formula or its driver may write, or mutates
       a live input. U: opaque or failed analysis, or an imprecise lane decides
       the class; "bound" is then the class with every imprecise lane consumed.
    A driver that itself reads carried lanes makes every formula at least C;
    "formula_class" then keeps the formula-only class.
    """
    result = formula_class(effects, driver, julia)
    carried = sorted(DRIVER_CARRIED_READS.get(driver, ()), key=LANES.index)
    if not carried:
        return result
    if result["class"] in ("A", "B"):
        return {**result, "class": "C", "formula_class": result["class"], "driver_carried": carried}
    return {**result, "driver_carried": carried}


def formula_class(effects, driver, julia=False):
    if effects.get("status") != "ANALYZED" or effects.get("opaque"):
        return {"class": "U", "carried": sorted(ALL_LANES)}
    variant = DRIVERS[(driver, julia)]
    consumed, written = set(effects["consumed"]) | STEP_READS, set(effects["written"])
    retained = consumed - variant["init"] - LIVE_INPUTS
    mutated_live = written & LIVE_INPUTS
    carried = retained & (written | variant["writes"])
    # Whole-region fallbacks are conservative, not evidence of real carry.
    imprecise = (retained | mutated_live) & set(effects.get("imprecise", ()))
    if imprecise:
        worst = formula_class({**effects, "consumed": sorted(consumed | imprecise), "imprecise": []}, driver, julia)
        return {"class": "U", "imprecise": sorted(imprecise, key=LANES.index), "bound": worst["class"]}
    if mutated_live or carried:
        return {"class": "C", "carried": sorted(carried), "mutated_live": sorted(mutated_live)}
    if retained:
        return {"class": "B", "invariant_unless_written": sorted(retained)}
    return {"class": "A"}


# --- LLVM IR (typed-pointer AIR) analysis -----------------------------------

DEFINE = re.compile(r"^define\s.*@effect_probe\((.*)\)\s.*\{\s*$")
LABEL = re.compile(r"^([-\w.$]+):(?:\s|$)")
PHI = re.compile(r"phi\s+i(?:32|64)\s+(.*)$")
INCREMENT = re.compile(r"add\s+(?:nuw\s+)?(?:nsw\s+)?i(?:32|64)\s+(%[-\w.$]+),\s*1$")
COMPARE = re.compile(r"icmp\s+(eq|ne|ult|slt)\s+i(?:32|64)\s+(%[-\w.$]+),\s*(-?\d+)$")
BRANCH = re.compile(r"br\s+i1\s+(%[-\w.$]+),\s*label\s+(%[-\w.$]+),\s*label\s+(%[-\w.$]+)")
EXTEND = re.compile(r"(?:zext|sext)\s+i(?:32|64)\s+(%[-\w.$]+)\s+to\s+i64$")
CHOICE = re.compile(r"select\s+i1\s+%[-\w.$]+,\s*i(?:32|64)\s+(\d+),\s*i(?:32|64)\s+(\d+)$")
# Instructions a float value passes through without deciding anything:
# arithmetic, conversions, phi, select (a float is never its i1 condition) and
# pure FP intrinsics.
PURE_FLOW = re.compile(r"(?:fmul|fadd|fsub|fdiv|fneg|fpext|fptrunc|phi|select)\b|"
                       r"(?:tail\s+)?call\s+(?:\w+\s+)*float\s+@(?:llvm\.(?:fma|fmuladd|fabs)|air\.(?:fma|fabs))\.f32\(")
ASSIGN = re.compile(r"^\s*(%[-\w.$]+)\s*=\s*(.*)$")
GEP = re.compile(r"getelementptr\s+(?:inbounds\s+)?(.+?),\s*(.+?)\s+(%[-\w.$]+)((?:\s*,\s*[^,]+)*)\s*(?:,\s*!.*)?$")
VALUE = re.compile(r"%[-\w.$]+")
TYPE_SIZE = {"float": 4, "i32": 4, "i64": 8, "double": 8, "i16": 2, "i8": 1, "half": 2, "i1": 1}


def type_size(text):
    text = text.strip()
    if text in TYPE_SIZE:
        return TYPE_SIZE[text]
    m = re.fullmatch(r"<(\d+)\s+x\s+(\w+)>", text)
    if m and m.group(2) in TYPE_SIZE:
        return int(m.group(1)) * TYPE_SIZE[m.group(2)]
    m = re.fullmatch(r"\[(\d+)\s+x\s+(.+)\]", text)
    if m:
        inner = type_size(m.group(2))
        return int(m.group(1)) * inner if inner else None
    return None


def split_operands(text):
    parts, depth, start = [], 0, 0
    for i, ch in enumerate(text):
        if ch in "([{<":
            depth += 1
        elif ch in ")]}>":
            depth -= 1
        elif ch == "," and not depth:
            parts.append(text[start:i].strip()); start = i + 1
    parts.append(text[start:].strip())
    return [p for p in parts if p]


def kernel_body(ir):
    lines, inside, header = [], False, None
    for line in ir.splitlines():
        if not inside:
            m = DEFINE.match(line)
            if m:
                inside, header = True, m.group(1)
            continue
        if line.startswith("}"):
            return header, lines
        lines.append(line)
    raise ValueError("effect_probe kernel not found in IR")


def parse_type(text):
    """Layout of an IR type: (size, align, children) where children are (offset, type)."""
    text = text.strip()
    if "NativeFormulaState" in text and not text.endswith("*"):
        children = []
        for (name, count), offset in zip(STATE_FIELDS, FIELD_OFFSETS):
            children.append((offset, (count * 4, 4, [(i * 4, (4, 4, None)) for i in range(count)] if count > 1 else None)))
        return STATE_BYTES, 4, children
    if text in TYPE_SIZE:
        return TYPE_SIZE[text], TYPE_SIZE[text], None
    m = re.fullmatch(r"([<\[])\s*(\d+)\s+x\s+(.+?)\s*[>\]]", text)
    if m:
        element = parse_type(m.group(3))
        if element is None:
            return None
        size, align, _ = element
        return int(m.group(2)) * size, align, [(i * size, element) for i in range(int(m.group(2)))]
    if text.startswith("{") and text.endswith("}"):
        children, offset, align = [], 0, 1
        for field in split_operands(text[1:-1]):
            element = parse_type(field)
            if element is None:
                return None
            offset = (offset + element[1] - 1) // element[1] * element[1]
            children.append((offset, element)); offset += element[0]; align = max(align, element[1])
        return (offset + align - 1) // align * align, align, children
    return None


def gep_offset(source_type, indices):
    """Byte offset of a constant GEP, or None for dynamic indices and unknown types."""
    values = []
    for index in indices:
        m = re.fullmatch(r"(i\d+)\s+(-?\d+)", index.strip())
        if not m:
            return None
        values.append(int(m.group(2)))
    layout = parse_type(source_type)
    if layout is None or not values:
        return None if layout is None else 0
    offset = values[0] * layout[0]
    for value in values[1:]:
        children = layout[2]
        if children is None or not 0 <= value < len(children):
            return None
        child_offset, layout = children[value]
        offset += child_offset
    return offset


def analyze_ir(ir):
    """Return consumed/written state lanes for the effect_probe kernel IR.

    in/out are the kernel's input and output NativeFormulaState buffers. A value
    loaded from an input lane and stored unchanged to the same output lane (also
    through private allocas and memcpy) is an identity copy. Other uses of input
    values consume their lanes; other output stores write them. A direct
    input-to-output memcpy/memmove at the same offset is likewise an identity
    copy. Unrecognised use of a state pointer is opaque; unresolved
    private-memory flows are reported as imprecise, consuming/writing their
    whole regions.
    """
    header, body = kernel_body(ir)
    args = [VALUE.findall(arg)[-1] for arg in split_operands(header)]
    if len(args) < 2:
        raise ValueError("effect_probe signature is not recognised")
    pointers = {args[0]: ("in", 0), args[1]: ("out", 0)}
    # Private pointer -> (alloca root, byte offset, or ("range", first, last)
    # element offsets of a counted loop index, or None).
    slots = {}
    instructions, opaque = [], []
    state_loads, state_stores, slot_loads, slot_stores, copies = {}, [], {}, [], []
    defs, phis, branches, block = {}, {}, {}, "%entry"

    def last_value(text):
        found = VALUE.findall(text)
        return found[-1] if found else None

    def counted_range(value):
        """[first, end) of an index that is one of two constants, or a loop
        index that starts at a constant and steps by one until it equals a
        constant end; None for any other shape."""
        extended = EXTEND.fullmatch(defs.get(value, ""))
        value = extended.group(1) if extended else value
        choice = CHOICE.fullmatch(defs.get(value, ""))
        if choice:
            a, b = int(choice.group(1)), int(choice.group(2))
            return min(a, b), max(a, b) + 1
        if value not in phis or len(phis[value][1]) != 2:
            return None
        header, incoming = phis[value]
        starts = [int(v) for v, _ in incoming if re.fullmatch(r"-?\d+", v)]
        steps = [(v, pred) for v, pred in incoming if v.startswith("%")]
        if len(starts) != 1 or len(steps) != 1:
            return None
        step, latch = steps[0]
        increment = INCREMENT.fullmatch(defs.get(step, ""))
        branch = branches.get(latch)
        if not increment or increment.group(1) != value or not branch:
            return None
        compare = COMPARE.fullmatch(defs.get(branch[0], ""))
        if not compare or compare.group(2) != step:
            return None
        # The back edge must be taken only while the stepped index is below end.
        back = branch[2] if compare.group(1) == "eq" else branch[1]
        exit_ = branch[1] if compare.group(1) == "eq" else branch[2]
        first, end = starts[0], int(compare.group(3))
        if back != header or exit_ == header or not 0 <= first < end:
            return None
        return first, end

    def range_offsets(g):
        """Element offsets of a GEP whose only dynamic index is its last one."""
        indices = split_operands(g.group(4))
        dynamic = re.fullmatch(r"(i\d+)\s+(%[-\w.$]+)", indices[-1].strip()) if indices else None
        bounds = counted_range(dynamic.group(2)) if dynamic else None
        if not bounds:
            return None
        def at(k):
            return gep_offset(g.group(1), indices[:-1] + [f"{dynamic.group(1)} {k}"])
        first, last = at(bounds[0]), at(bounds[1] - 1)
        if first is None or last is None:
            return None
        if bounds[1] - 1 > bounds[0]:
            second = at(bounds[0] + 1)
            if second is None or last - first != (bounds[1] - 1 - bounds[0]) * (second - first):
                return None
        return first, last

    # Control flow first: a loop's exit branch follows the accesses it bounds.
    for line in body:
        m = ASSIGN.match(line)
        name, rhs = (m.group(1), m.group(2).strip()) if m else (None, line.strip())
        label = LABEL.match(rhs) if not m else None
        if label:
            block = "%" + label.group(1)
        elif name:
            defs[name] = rhs.split(", !")[0].strip()
            phi = PHI.match(rhs)
            if phi:
                phis[name] = (block, [tuple(p.strip() for p in pair.split(","))
                                      for pair in re.findall(r"\[\s*([^\]]+?)\s*\]", phi.group(1))])
        elif BRANCH.match(rhs):
            branches[block] = BRANCH.match(rhs).groups()

    for line in body:
        m = ASSIGN.match(line)
        name, rhs = (m.group(1), m.group(2).strip()) if m else (None, line.strip())
        instructions.append((name, rhs, line.strip()))
        if not rhs or rhs.endswith(":") or rhs.startswith(";") or (not m and LABEL.match(rhs)):
            continue
        if name and rhs.startswith("alloca"):
            slots[name] = (name, 0)
        elif name and rhs.startswith("getelementptr"):
            g = GEP.match(rhs)
            base = g.group(3) if g else None
            delta = gep_offset(g.group(1), split_operands(g.group(4))) if g else None
            if base in pointers:
                if delta is None:
                    opaque.append(line.strip())
                else:
                    pointers[name] = (pointers[base][0], pointers[base][1] + delta)
            elif base in slots:
                root, offset = slots[base]
                if delta is None and isinstance(offset, int):
                    span = range_offsets(g)
                    slots[name] = (root, ("range", offset + span[0], offset + span[1]) if span else None)
                else:
                    slots[name] = (root, offset + delta if isinstance(offset, int) and delta is not None else None)
        elif name and rhs.startswith(("bitcast", "addrspacecast")):
            source = last_value(rhs.split(" to ")[0])
            if source in pointers:
                pointers[name] = pointers[source]
            elif source in slots:
                slots[name] = slots[source]
        elif name and rhs.startswith("load"):
            operands = split_operands(rhs[len("load"):].replace("volatile", "", 1))
            pointer = last_value(operands[1]) if len(operands) > 1 else None
            size = type_size(operands[0])
            if pointer in pointers:
                if size is None:
                    opaque.append(line.strip())
                else:
                    state_loads[name] = (pointers[pointer], size)
            elif pointer in slots:
                slot_loads[name] = (*slots[pointer], size, len(instructions) - 1)
        elif rhs.startswith("store"):
            operands = split_operands(rhs[len("store"):].replace("volatile", "", 1))
            typed_value, pointer = operands[0], last_value(operands[1]) if len(operands) > 1 else None
            value = typed_value.rsplit(" ", 1)[-1]
            size = type_size(typed_value.rsplit(" ", 1)[0])
            if pointer in pointers:
                state_stores.append((pointers[pointer], size, value, line.strip()))
            elif pointer in slots:
                slot_stores.append((*slots[pointer], size, value, len(instructions) - 1))
        elif "@llvm.memcpy" in rhs or "@llvm.memmove" in rhs or "@llvm.memset" in rhs:
            operands = split_operands(rhs[rhs.index("(") + 1:rhs.rindex(")")])
            size = re.fullmatch(r"i64\s+(\d+)", operands[2]) if len(operands) > 2 else None
            size = int(size.group(1)) if size else None
            dst = last_value(operands[0])
            src = None if "@llvm.memset" in rhs else last_value(operands[1])
            copies.append((dst, src, size, line.strip(), len(instructions) - 1))

    def uses(value):
        pattern = re.compile(re.escape(value) + r"(?![-\w.$])")
        return [(n, r) for n, r, _ in instructions if n != value and pattern.search(r)]

    consumed, written, imprecise = set(), set(), set()
    # Lanes consumed other than through a loaded value (state copies, private
    # region flows); such a lane is never self-contained.
    indirect = set()

    def lanes_of(offset, size):
        return byte_lanes(offset, size or 0)

    # Private alloca groups as 4-byte lanes.
    groups = {}

    def group(root):
        return groups.setdefault(root, {"loads": [], "stores": [], "copy_in": [], "copy_out": [],
                                        "range_loads": [], "range_stores": [], "other": False})

    def spanned(first, last, size):
        """Lanes a counted access may touch; only scalar lane accesses are resolved."""
        if size != 4 or first % 4 or last % 4:
            return None
        return range(first // 4, last // 4 + 1)

    for name, (root, offset, size, at) in slot_loads.items():
        if isinstance(offset, tuple):
            group(root)["range_loads"].append((name, spanned(offset[1], offset[2], size), at))
        else:
            group(root)["loads"].append((name, offset, size, at))
    for root, offset, size, value, at in slot_stores:
        if isinstance(offset, tuple):
            group(root)["range_stores"].append((spanned(offset[1], offset[2], size), value, at))
        else:
            group(root)["stores"].append((offset, size, value, at))
    for dst, src, size, line, at in copies:
        fill = split_operands(line[line.index("(") + 1:line.rindex(")")]) if "@llvm.memset" in line else None
        if fill is not None and dst in slots and isinstance(slots[dst][1], int) and size is not None and \
                re.fullmatch(r"i8\s+-?\d+", fill[1].strip()):
            # A constant fill: a new value in every lane, never a state source.
            group(slots[dst][0])["stores"].append((slots[dst][1], size, "<memset>", at))
        elif dst in pointers and src in slots:
            root, offset = slots[src]
            group(root)["copy_out"].append((pointers[dst], size, offset, line, at))
        elif dst in slots and src in pointers:
            root, offset = slots[dst]
            group(root)["copy_in"].append((pointers[src], size, offset, at))
        elif dst in pointers and src in pointers and size is not None:
            # A direct input-to-output state copy; same offsets are identity lanes.
            (dst_base, dst_offset), (src_base, src_offset) = pointers[dst], pointers[src]
            if dst_base != "out" or src_base != "in":
                opaque.append(line)
            elif dst_offset != src_offset:
                written |= lanes_of(dst_offset, size)
                consumed |= lanes_of(src_offset, size); indirect |= lanes_of(src_offset, size)
        elif dst in pointers or src in pointers:
            if dst in pointers and src is None and pointers[dst][0] == "out":
                written |= lanes_of(pointers[dst][1], size)
            else:
                opaque.append(line)
        else:
            for ref in (dst, src):
                if ref in slots:
                    group(slots[ref][0])["other"] = True
    # Escapes of private pointers (calls, selects, phis, dynamic offsets).
    for name, rhs, _ in instructions:
        if not rhs or rhs.startswith((";", "load", "getelementptr", "bitcast", "addrspacecast", "alloca")):
            continue
        if any(k in rhs for k in ("@llvm.memcpy", "@llvm.memmove", "@llvm.memset", "@llvm.lifetime")):
            continue
        refs = [r for r in VALUE.findall(rhs) if r in slots]
        if rhs.startswith("store"):
            operands = split_operands(rhs[len("store"):])
            refs = [r for r in VALUE.findall(operands[0]) if r in slots]  # pointer stored as a value
        for ref in refs:
            group(slots[ref][0])["other"] = True
    for g in groups.values():
        records = ([o for _, o, _, _ in g["loads"]] + [o for o, _, _, _ in g["stores"]] +
                   [o for _, _, o, _ in g["copy_in"]] + [o for _, _, o, _, _ in g["copy_out"]])
        sizes = [s for _, _, s, _ in g["loads"]] + [s for _, s, _, _ in g["stores"]]
        if any(o is None or not isinstance(o, int) or o % 4 for o in records) or any(s is None or s % 4 for s in sizes) or \
                any(span is None for _, span, _ in g["range_loads"] + g["range_stores"]):
            g["other"] = True

    # Resolve aliases (values equal to input bytes) to a fixed point.
    alias = {name: region for name, region in state_loads.items() if region[0][0] == "in"}

    def source(value):
        return alias.get(value)

    # Reaching definitions per private lane over the basic-block graph. A
    # constant store, fill or copy replaces a lane's definitions; a counted
    # store may leave the earlier ones. A lane with no definition contributes
    # nothing, as in the flow-insensitive maps this refines.
    block_of, successors, current, order = [], {"%entry": set()}, "%entry", ["%entry"]
    for name, rhs, _ in instructions:
        label = LABEL.match(rhs) if name is None and rhs else None
        if label:
            current = "%" + label.group(1); order.append(current); successors.setdefault(current, set())
        block_of.append(current)
        if rhs.startswith(("br ", "switch ")):
            successors[current].update(re.findall(r"label\s+(%[-\w.$]+)", rhs))
        elif rhs.startswith(("indirectbr", "callbr", "invoke")):
            for g in groups.values():
                g["other"] = True
    predecessors = {b: set() for b in order}
    for b, targets in successors.items():
        for t in targets:
            predecessors.setdefault(t, set()).add(b)

    def definitions(g):
        events = [(at, [(offset // 4 + k, ("s", value, k)) for k in range(size // 4)], True)
                  for offset, size, value, at in g["stores"]]
        events += [(at, [(offset // 4 + k, ("c", in_offset // 4 + k if base == "in" else None))
                         for k in range((size or 0) // 4)], True)
                   for (base, in_offset), size, offset, at in g["copy_in"]]
        events += [(at, [(lane, ("s", value, 0)) for lane in span], False) for span, value, at in g["range_stores"]]
        return sorted(events, key=lambda e: e[0])

    def flow(g):
        if "flow" not in g:
            events, entry = definitions(g), {}
            def apply(state, at_most=None, block=None):
                state = dict(state)
                for at, lanes_defs, kills in events:
                    if block_of[at] != block or (at_most is not None and at >= at_most):
                        continue
                    for lane, d in lanes_defs:
                        state[lane] = frozenset([d]) if kills else state.get(lane, frozenset()) | {d}
                return state
            ins, outs = {b: {} for b in order}, {b: None for b in order}
            changed = True
            while changed:
                changed = False
                for b in order:
                    merged = {}
                    for p in predecessors.get(b, ()):
                        for lane, ds in (outs.get(p) or {}).items():
                            merged[lane] = merged.get(lane, frozenset()) | ds
                    if merged != ins[b] or outs[b] is None:
                        ins[b], outs[b], changed = merged, apply(merged, block=b), True
            g["flow"] = (ins, apply)
        return g["flow"]

    def lane_map(g, at):
        ins, apply = flow(g)
        stored, copied = {}, {}
        for lane, ds in apply(ins[block_of[at]], at, block_of[at]).items():
            for d in ds:
                if d[0] == "s":
                    stored.setdefault(lane, []).append((d[1], d[2]))
                else:
                    copied.setdefault(lane, []).append(d[1])
        return stored, copied

    for _ in range(8):
        changed = False
        for g in groups.values():
            if g["other"]:
                continue
            for name, offset, size, at in g["loads"]:
                if name in alias:
                    continue
                stored, copied = lane_map(g, at)
                cover = range(offset // 4, offset // 4 + size // 4)
                if all(l not in stored and len(copied.get(l, ())) == 1 and copied[l][0] is not None for l in cover):
                    first = copied[offset // 4][0]
                    if all(copied[l][0] == first + i for i, l in enumerate(cover)):
                        alias[name] = (("in", first * 4), size); changed = True
                elif all(l not in copied for l in cover):
                    entries = [stored.get(l, []) for l in cover]
                    if all(len(e) == 1 for e in entries) and len({e[0][0] for e in entries}) == 1:
                        value = entries[0][0][0]
                        lanes_k = [e[0][1] for e in entries]
                        src = source(value)
                        if src and lanes_k == list(range(lanes_k[0], lanes_k[0] + len(lanes_k))):
                            alias[name] = (("in", src[0][1] + lanes_k[0] * 4), size); changed = True
        if not changed:
            break

    # Group flows: copy_out lanes and loads that remain unresolved.
    for g in groups.values():
        if g["other"]:
            for (base, offset), size, _, _ in g["copy_in"]:
                consumed.update(lanes_of(offset, size)); imprecise.update(lanes_of(offset, size))
            for value in [v for _, _, v, _ in g["stores"]] + [v for _, v, _ in g["range_stores"]]:
                if source(value):
                    region = source(value)
                    consumed.update(lanes_of(region[0][1], region[1])); imprecise.update(lanes_of(region[0][1], region[1]))
            for (base, offset), size, _, line, _ in g["copy_out"]:
                written.update(lanes_of(offset, size)); imprecise.update(lanes_of(offset, size))
            continue
        unresolved = [(range(offset // 4, offset // 4 + size // 4), at)
                      for name, offset, size, at in g["loads"] if name not in alias]
        for cover, at in unresolved + [(span, at) for _, span, at in g["range_loads"]]:
            stored, copied = lane_map(g, at)
            for l in cover:
                for in_lane in copied.get(l, ()):
                    if in_lane is not None:
                        consumed.add(LANES[in_lane]); imprecise.add(LANES[in_lane])
                for value, k in stored.get(l, ()):
                    src = source(value)
                    if src:
                        lane = src[0][1] // 4 + k
                        consumed.add(LANES[lane]); imprecise.add(LANES[lane])
        for (base, out_offset), size, offset, line, at in g["copy_out"]:
            if base != "out":
                opaque.append(line); continue
            stored, copied = lane_map(g, at)
            for k in range((size or 0) // 4):
                l, out_lane = offset // 4 + k, out_offset // 4 + k
                sources = list(copied.get(l, ()))
                for value, vk in stored.get(l, ()):
                    src = source(value)
                    sources.append(src[0][1] // 4 + vk if src else "new")
                if sources == [out_lane]:
                    continue
                written.add(LANES[out_lane])
                for src in sources:
                    if isinstance(src, int) and src != out_lane:
                        consumed.add(LANES[src]); indirect.add(LANES[src])

    # Direct output stores.
    def identity(value, offset, size):
        src = source(value)
        return src is not None and src == (("in", offset), size)

    for (base, offset), size, value, line in state_stores:
        if base != "out" or size is None:
            opaque.append(line)
        elif not identity(value, offset, size):
            written |= lanes_of(offset, size)

    # Input values: uses besides identity stores and private parking consume them.
    for value, ((base, offset), size) in alias.items():
        for _, rhs in uses(value):
            if rhs.startswith("store"):
                operands = split_operands(rhs[len("store"):])
                pointer = last_value(operands[1]) if len(operands) > 1 else None
                stored = operands[0].rsplit(" ", 1)[-1] == value
                if stored and pointer in pointers and pointers[pointer] == ("out", offset):
                    continue
                if stored and pointer in slots and not groups[slots[pointer][0]]["other"]:
                    continue
            consumed |= lanes_of(offset, size)
            break

    # Self-contained lanes: every value read from the lane reaches only that
    # lane's own output store, through arithmetic, phi and select data operands.
    # No compare, branch, call or other store sees it, so a carried value of
    # such a lane changes no other lane, record or control flow.
    def contained(value, offset):
        work, seen, reached = [value], {value}, False
        while work:
            v = work.pop()
            for name, rhs in uses(v):
                if rhs.startswith("store"):
                    operands = split_operands(rhs[len("store"):])
                    pointer = last_value(operands[1]) if len(operands) > 1 else None
                    if (operands[0].rsplit(" ", 1)[-1] != v or type_size(operands[0].rsplit(" ", 1)[0]) != 4 or
                            pointers.get(pointer) != ("out", offset)):
                        return False
                    reached = True
                    continue
                if not name or not PURE_FLOW.match(rhs):
                    return False
                if name not in seen:
                    seen.add(name); work.append(name)
        return reached

    readers = {}
    for value, ((base, offset), size) in alias.items():
        for lane in lanes_of(offset, size):
            readers.setdefault(lane, []).append((value, offset, size))
    self_only = {lane for lane in consumed - imprecise - indirect
                 if readers.get(lane) and all(contained(value, offset) for value, offset, _ in readers[lane])}

    # Any other use of a state pointer (call argument, phi/select, ptrtoint, dynamic GEP).
    for name, rhs, line in instructions:
        if not rhs or rhs.startswith((";", "store", "load", "getelementptr", "bitcast", "addrspacecast")):
            continue
        if any(k in rhs for k in ("@llvm.memcpy", "@llvm.memmove", "@llvm.memset", "@llvm.lifetime")):
            continue
        if any(re.search(re.escape(p) + r"(?![-\w.$])", rhs) for p in pointers):
            opaque.append(line)
    return {"consumed": sorted(consumed, key=LANES.index), "written": sorted(written, key=LANES.index),
            "imprecise": sorted(imprecise, key=LANES.index), "self_only": sorted(self_only, key=LANES.index),
            "opaque": bool(opaque), "opaque_instructions": opaque[:8]}


# --- Inputs and compilation -------------------------------------------------

def sha256(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def slot_param_count(row):
    """The count formula_host.cpp uploads, not ExternalFormulaInfo.paramCount.

    It is the CPU FormulaParamMap length (inventory parameter_types), or two PVar
    doubles for the map-less _BenesiT bodies. With fewer slot parameters the
    wrapper shifts Julia/C into a[0..], as Benesi1Pow2 (map count 0) relies on.
    """
    return 2 if 95001 <= row["id"] <= 95003 else len(row["parameter_types"])


def load_entries(root):
    inventory = json.loads((root / NATIVE / "generated/inventory.json").read_text())
    metadata = {}
    for m in re.finditer(r"case (\d+)u: slot\.traits=(\d+)u; slot\.wrapper_flags=(\d+)u; slot\.difs_style=(\d+)u;",
                         (root / NATIVE / "generated/formula_metadata.h").read_text()):
        metadata[int(m.group(1))] = tuple(int(m.group(i)) for i in range(2, 5))
    calls = {}
    for m in re.finditer(r"case (\d+)u: if\([^)]*\) \{ (result=[^;]+); return true; \}",
                         (root / NATIVE / "generated/formula_dispatch.metal.h").read_text()):
        calls[int(m.group(1))] = m.group(2)
    entries = []
    for row in inventory["formulas"]:
        traits, flags, difs = metadata.get(row["id"], (None, None, None))
        entries.append({"id": row["id"], "name": row["name"], "family": row["family"], "source": row["source"],
                        "de_option": row["de_option"], "param_count": slot_param_count(row),
                        "declared_param_count": row["param_count"],
                        "traits": traits, "wrapper_flags": flags, "difs_style": difs,
                        "call": calls.get(row["id"]),
                        # Conditional iteration counters are implemented by the driver itself.
                        "generated": row.get("generated_path") or str(NATIVE / "internal_formulas.metal.h")})
    return entries


def internal_entries(root):
    """The ten public internal formulas, named by the CPU registry."""
    source = (root / "src/core/formula_registry.cpp").read_text()
    block = re.search(r"InternFormulaNames = \{\{(.*?)\}\};", source, re.S)
    names = re.findall(r'"([^"]*)"', block.group(1)) if block else []
    if len(names) != 10:
        raise ValueError("Unexpected internal formula registry")
    return [{"id": i, "name": name, "family": "internal", "source": "internal", "namespace": "internal",
             "generated": str(NATIVE / "internal_formulas.metal.h")} for i, name in enumerate(names)]


DISPATCH_STUB = """#pragma once
// Effect-analysis stub: exactly one lowered formula, no function constants.
#include "src/render/native_metal/formula_math.metal.h"
#include MB3D_EFFECT_HEADER
namespace mb3d_native {
inline bool native_external_raw(uint id,FormulaVec3 p,FormulaParams a,thread float& de,thread float& vs,thread FormulaVec3& result) {
    if(id!=MB3D_EFFECT_ID)return false;
    MB3D_EFFECT_CALL;
    return true;
}
}
"""
KERNEL = """#define MB3D_METAL_NATIVE_FLOAT 1
#include "shadow/formula_wrapper.metal.h"
using namespace mb3d_native;
[[host_name("effect_probe")]] kernel void effect_probe(device const NativeFormulaState* in [[buffer(0)]],
    device NativeFormulaState* out [[buffer(1)]],constant NativeFormulaSlot& slot [[buffer(2)]],
    device const NativeMapDescriptor* maps [[buffer(3)]],device const float* pixels [[buffer(4)]],
    constant uint2& counts [[buffer(5)]],device NativeFormulaFault* fault_out [[buffer(6)]],device uint* ok [[buffer(7)]],
    constant uint3& limits [[buffer(8)]]) {
    NativeFormulaState s=in[0];
    NativeResources resources{maps,pixels,counts.x,counts.y,nullptr};
    // Work counters stay runtime values: constant limits let the compiler fold
    // FormulaWorkBudget loops into their exhaustion path and drop the body.
    NativeFormulaFault fault{};fault.work=limits.x;fault.limit=limits.y;
    const bool completed=native_formula_step_body<true,MB3D_EFFECT_COUNT,MB3D_EFFECT_TRAITS,MB3D_EFFECT_OPTION,MB3D_EFFECT_FLAGS>(
        s,slot,resources,fault,limits.z);
    out[0]=s;fault_out[0]=fault;ok[0]=completed;
}
"""
CONDITIONAL_KERNEL = """#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/internal_formulas.metal.h"
using namespace mb3d_native;
[[host_name("effect_probe")]] kernel void effect_probe(device const NativeFormulaState* in [[buffer(0)]],
    device NativeFormulaState* out [[buffer(1)]],constant NativeFormulaSlot& slot [[buffer(2)]],device uint* ok [[buffer(3)]]) {
    NativeFormulaState s=in[0];
    const bool completed=native_conditional_step(s,slot);
    out[0]=s;ok[0]=completed;
}
"""


# Internal formulas run directly from native_step: no external wrapper or guard.
# The id test lets the optimizer fold the body switch to one case; selector
# function constants stay undefined, so both their paths remain.
INTERNAL_KERNEL = """#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/internal_formulas.metal.h"
using namespace mb3d_native;
[[host_name("effect_probe")]] kernel void effect_probe(device const NativeFormulaState* in [[buffer(0)]],
    device NativeFormulaState* out [[buffer(1)]],constant NativeFormulaSlot& slot [[buffer(2)]],
    device const NativeMapDescriptor* maps [[buffer(3)]],device const float* pixels [[buffer(4)]],
    constant uint2& counts [[buffer(5)]],device NativeFormulaFault* fault_out [[buffer(6)]],device uint* ok [[buffer(7)]]) {
    NativeFormulaState s=in[0];
    NativeResources resources{maps,pixels,counts.x,counts.y,nullptr};
    NativeFormulaFault fault{};bool completed=false;
    if(slot.id==MB3D_EFFECT_ID)completed=native_internal_formula_step(s,slot,resources,fault);
    out[0]=s;fault_out[0]=fault;ok[0]=completed;
}
"""


def prepare_shadow(root, work):
    shadow = work / "shadow"
    (shadow / "generated").mkdir(parents=True, exist_ok=True)
    for relative in ('formula_wrapper.metal.h', 'generated/formula_result_contracts.metal.h'):
        source, target = root / NATIVE / relative, shadow / relative
        shutil.copyfile(source, target)
        if sha256(target) != sha256(source):
            raise RuntimeError('Shadow contract is not byte-identical: ' + relative)
    (shadow / "generated/formula_dispatch.metal.h").write_text(DISPATCH_STUB)
    (work / "effect_kernel.metal").write_text(KERNEL)
    (work / "conditional_kernel.metal").write_text(CONDITIONAL_KERNEL)
    (work / "internal_kernel.metal").write_text(INTERNAL_KERNEL)


def compile_entry(root, work, entry, keep):
    ident, internal = entry["id"], entry.get("namespace") == "internal"
    stem = f"internal_{ident}" if internal else f"formula_{ident}"
    raw, optimized = work / f"{stem}.ll", work / f"{stem}.opt.ll"
    conditional = not internal and 10353 <= ident <= 10357
    base = ["xcrun", "-sdk", "macosx", "metal", "-std=metal3.1", "-fno-fast-math", "-ffp-contract=fast", "-O2",
            "-mllvm", "-inline-threshold=100000", "-Wno-unused-variable",
            "-I", str(root), "-I", str(root / "comparison/metal_exact"), "-I", str(work)]
    if internal:
        command = base + [f"-DMB3D_EFFECT_ID={ident}u", "-S", "-emit-llvm", str(work / "internal_kernel.metal"),
                          "-o", str(raw)]
    elif conditional:
        command = base + ["-S", "-emit-llvm", str(work / "conditional_kernel.metal"), "-o", str(raw)]
    else:
        if entry["call"] is None or entry["traits"] is None:
            return {"status": "NOT_DISPATCHED"}
        command = base + [f'-DMB3D_EFFECT_HEADER="{entry["generated"]}"', f"-DMB3D_EFFECT_ID={ident}u",
                          f"-DMB3D_EFFECT_CALL={entry['call']}", f"-DMB3D_EFFECT_COUNT={entry['param_count']}u",
                          f"-DMB3D_EFFECT_TRAITS={entry['traits']}u", f"-DMB3D_EFFECT_OPTION={entry['de_option']}",
                          f"-DMB3D_EFFECT_FLAGS={entry['wrapper_flags']}u",
                          "-S", "-emit-llvm", str(work / "effect_kernel.metal"), "-o", str(raw)]
    frontend = subprocess.run(command, capture_output=True, text=True)
    if frontend.returncode:
        return {"status": "COMPILE_FAILED", "stderr": frontend.stderr[-2000:]}
    # Full unrolling of the wrapper's constant-count parameter copy lets SROA
    # promote FormulaParams, so its parked state lanes fold to their values.
    opt = subprocess.run(["xcrun", "air-opt", "-passes=default<O2>", f"-unroll-threshold={UNROLL_THRESHOLD}",
                          "-S", str(raw), "-o", str(optimized)],
                         capture_output=True, text=True)
    if opt.returncode:
        return {"status": "OPTIMIZE_FAILED", "stderr": opt.stderr[-2000:]}
    ir = optimized.read_text()
    try:
        effects = analyze_ir(ir)
    except ValueError as error:
        return {"status": "PARSE_FAILED", "error": str(error)}
    # The temporary directory reaches ModuleID/source_filename; hash it out.
    stable = ir.replace(str(work), "<work>").encode()
    entry_name = ("native_internal_formula_step" if internal else
                  "native_conditional_step" if conditional else "native_formula_step_body")
    result = {"status": "ANALYZED", "entry": entry_name,
              "ir_sha256": hashlib.sha256(stable).hexdigest(), **effects}
    if not keep:
        raw.unlink(missing_ok=True); optimized.unlink(missing_ok=True)
    return result


# --- Audited production knowledge -------------------------------------------
# IDs the existing hand audits treat as pixel-independent under a driver
# (pixel_independence.h, pixel_state_contract.h). Class C contradicts an audit;
# class U leaves it unconfirmed by this analysis.
AUDITED = {
    "Hybrid3DAnalytic": (10005, 7227, 7060, 10186, 1509, 7076, 10037, 7007, 8024, 8900, 10014,
                         429, 2153, 3778, 9526, 10006, 10030, 10269, 6030, 3676, 10010, 10171),
    "Hybrid3D": (429, 2153, 3778, 9526, 10006, 10030, 10269, 6030, 9452, 7046),
    "Hybrid4DAnalytic": (7060, 10324, 10343, 10350, 5823),
    "Hybrid4D": (9401,),
}


def toolchain():
    version = subprocess.run(["xcrun", "-sdk", "macosx", "metal", "--version"], capture_output=True, text=True)
    return version.stdout.strip().splitlines()[0] if version.stdout else "unknown"


def run(root, output, jobs, ids, keep):
    entries = load_entries(root)
    internals = internal_entries(root)
    if ids:
        entries = [e for e in entries if e["id"] in ids]
        internals = []
    math_path = root / NATIVE / "formula_math.metal.h"
    math_sha256 = sha256(math_path)
    translate_add_path = root / NATIVE / "translate_add.metal.h"
    translate_add_sha256 = sha256(translate_add_path)
    output.mkdir(parents=True, exist_ok=False)
    work = Path(tempfile.mkdtemp(prefix="formula-effects-", dir=output)) if keep else Path(tempfile.mkdtemp(prefix="mb3d-formula-effects-"))
    try:
        prepare_shadow(root, work)
        with ThreadPoolExecutor(max_workers=jobs) as pool:
            results = list(pool.map(lambda e: compile_entry(root, work, e, keep), entries + internals))
        if sha256(math_path) != math_sha256:
            raise RuntimeError("Formula math changed during effect analysis")
        if sha256(translate_add_path) != translate_add_sha256:
            raise RuntimeError("Translate add helper changed during effect analysis")
    finally:
        if not keep:
            shutil.rmtree(work, ignore_errors=True)
    formulas, summary = [], {}
    internal_formulas = [{**entry, "generated_sha256": sha256(root / entry["generated"]), "effects": effects,
                          "classes": {driver + ("/julia" if julia else ""): classify(effects, driver, julia)
                                      for driver, julia in DRIVERS}}
                         for entry, effects in zip(internals, results[len(entries):])]
    for entry, effects in zip(entries, results):
        classes = {}
        for driver, julia in DRIVERS:
            key = driver + ("/julia" if julia else "")
            classes[key] = classify(effects, driver, julia)
            bucket = summary.setdefault(key, {"A": 0, "B": 0, "C": 0, "U": 0})
            bucket[classes[key]["class"]] += 1
        formulas.append({**{k: v for k, v in entry.items() if k != "call"},
                         "generated_sha256": sha256(root / entry["generated"]), "effects": effects, "classes": classes})
    by_id = {f["id"]: f for f in formulas}
    audits = {name: [{"driver": driver, "id": ident, "class": by_id[ident]["classes"][driver]}
                     for driver, idents in AUDITED.items() for ident in idents
                     if ident in by_id and by_id[ident]["classes"][driver]["class"] == cls]
              for name, cls in (("audit_contradictions", "C"), ("audit_unconfirmed", "U"))}
    result = {
        "schema": SCHEMA,
        "status": "STATIC_EFFECTS_UNQUALIFIED",
        "scope": "Compiler-derived wrapper/body state lanes per lowered external formula; no device execution, "
                 "admission change or qualification.",
        "toolchain": toolchain(),
        "wrapper_sha256": sha256(root / NATIVE / "formula_wrapper.metal.h"),
        "math_sha256": math_sha256,
        "translate_add_sha256": translate_add_sha256,
        "result_contract_sha256": sha256(root / NATIVE / "generated/formula_result_contracts.metal.h"),
        "metadata_sha256": sha256(root / NATIVE / "generated/formula_metadata.h"),
        "inventory_sha256": sha256(root / NATIVE / "generated/inventory.json"),
        "orbit_driver_sha256": sha256(root / NATIVE / "orbit_driver.metal.h"),
        "tool_sha256": sha256(Path(__file__)),
        "lanes": LANES,
        "drivers": {driver + ("/julia" if julia else ""): {"init": sorted(v["init"], key=LANES.index),
                                                           "writes": sorted(v["writes"], key=LANES.index)}
                    for (driver, julia), v in DRIVERS.items()},
        "driver_carried_reads": {k: sorted(v, key=LANES.index) for k, v in DRIVER_CARRIED_READS.items()},
        "analysis_status": {s: sum(f["effects"]["status"] == s for f in formulas)
                            for s in sorted({f["effects"]["status"] for f in formulas})},
        "opaque": sum(bool(f["effects"].get("opaque")) for f in formulas),
        "summary": summary,
        **audits,
        "formulas": formulas,
        "internal_formulas": internal_formulas,
    }
    (output / "effects.json").write_text(json.dumps(result, indent=1, sort_keys=True) + "\n")
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--output", type=Path, required=True, help="New output directory")
    parser.add_argument("--jobs", type=int, default=6)
    parser.add_argument("--id", type=int, action="append", default=[], help="Analyze only these formula IDs")
    parser.add_argument("--keep-ir", action="store_true", help="Keep optimized IR under the output directory")
    args = parser.parse_args(argv)
    result = run(ROOT, args.output.resolve(), args.jobs, set(args.id), args.keep_ir)
    print(json.dumps({k: result[k] for k in ("status", "analysis_status", "opaque", "audit_contradictions", "audit_unconfirmed")}, indent=1))
    print(json.dumps(result["summary"], indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
