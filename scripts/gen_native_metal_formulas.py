#!/usr/bin/env python3
"""Lower the ordinary static registry to offline Native FP32 Metal sources.

This is a source generator, not an execution engine or runtime compiler. ARM
preprocessor branches are selected before token lowering; CPU-only branches are
never substituted because they happen to be easier to compile. Generated source
is explicitly UNQUALIFIED until the independent device/scene gates admit it.
Unknown constructs remain in the inventory as blocked, never successful stubs.
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
FORMULAS = ROOT / "external_formulas/Formulas"
TABLES = [ROOT / "src/core/external_formula_table.cpp", *sorted((ROOT / "src/core/generated").glob("external_formula_table_*.cpp"))]
TRAITS = (
    "ExternalFormulaMultipliesAnalyticDE", "ExternalFormulaUsesXYZState",
    "ExternalFormulaUsesKaliState", "ExternalFormulaUsesSse01", "ExternalFormulaUsesXYZWState",
    "ExternalFormulaUsesWrittenCForSameIterationAdd", "ExternalFormulaAlwaysWritesJuliaXYZ",
    "ExternalFormulaAlwaysWritesJuliaW", "ExternalFormulaReadsRout", "ExternalFormulaWritesWState",
    "ExternalFormulaWritesPersistentCLanes", "ExternalFormulaConsumesPersistentCLanes",
    "ExternalFormulaAppendsPersistentCLanes", "ExternalFormulaAppendsJuliaMode", "ExternalDifsUsesSphereMap",
)
TOKEN = re.compile(r'(?P<space>\s+)|(?P<comment>//[^\n]*|/\*.*?\*/)|(?P<string>"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\')|(?P<number>(?:0[xX][0-9a-fA-F]+(?:\.[0-9a-fA-F]*)?(?:[pP][+-]?\d+)?|(?:\d+\.\d*|\.\d+|\d+)(?:[eE][+-]?\d+)?)[fFlLuU]*)|(?P<name>[A-Za-z_]\w*)|(?P<operator>::|->|<<=|>>=|<<|>>|&&|\|\||<=|>=|==|!=|\+\+|--|\+=|-=|\*=|/=|&=|\|=|\^=|.)', re.S)


def digest(data):
    return hashlib.sha256(data if isinstance(data, bytes) else data.encode()).hexdigest()


def write_if_changed(path,data):
    if not path.is_file() or path.read_text()!=data:path.write_text(data)


def tokens(source):
    result = []
    cursor = 0
    for match in TOKEN.finditer(source):
        if match.start() != cursor:
            raise ValueError("Unrecognized source token")
        cursor = match.end()
        if match.lastgroup not in ("space", "comment"):
            result.append(match.group())
    if cursor != len(source):
        raise ValueError("Unrecognized source suffix")
    return result


def matching(values, begin, left="{", right="}"):
    depth = 0
    for i in range(begin, len(values)):
        if values[i] == left:
            depth += 1
        elif values[i] == right:
            depth -= 1
            if not depth:
                return i
    raise ValueError("Unbalanced " + left)


def split_fields(values):
    fields, start, depth = [], 0, 0
    for i, value in enumerate(values):
        if value in ("{", "(", "["):
            depth += 1
        elif value in ("}", ")", "]"):
            depth -= 1
        elif value == "," and not depth:
            fields.append("".join(values[start:i])); start = i + 1
    fields.append("".join(values[start:]))
    return fields


def split_token_args(values):
    parts=[];start=0;depth=0
    for i,value in enumerate(values):
        if value in ("(","[","{"):depth+=1
        elif value in (")","]","}"):depth-=1
        elif value=="," and not depth:parts.append(values[start:i]);start=i+1
    parts.append(values[start:]);return parts


@dataclass
class Entry:
    id: int
    symbol: str
    name: str
    source: str | None
    family: str
    de_option: int
    param_count: int
    adds_c: int = 1
    difs_style: int = 0
    parameter_types: tuple[int, ...] = ()


def registry():
    includes, constants, wrappers, rows, parameter_maps = {}, {}, {}, [], {}
    for table in TABLES:
        source = table.read_text()
        for include in re.findall(r'#include\s+"([^"\n]+\.inc)"', source):
            # Deferred entries are not opened, lowered or in denominators.
            if any(word in include.lower() for word in ("jit", "montecarlo", "halton")):
                continue
            path = FORMULAS / include
            if not path.is_file():
                continue
            content = path.read_text()
            for symbol, number in re.findall(r'constexpr\s+unsigned(?:\s+int)?\s+(FORMULA_\w+)\s*=\s*(\d+)', content):
                constants[symbol] = int(number); includes[symbol] = include
        for symbol, number in re.findall(r'constexpr\s+unsigned(?:\s+int)?\s+(FORMULA_\w+)\s*=\s*(\d+)', source):
            constants[symbol] = int(number)
        for extended, arguments in re.findall(r'DEFINE_EXTERNAL_FORMULA_WRAPPER(_EX)?\(([^)]*)\)', source):
            args = [part.strip() for part in arguments.split(",")]
            if len(args) in (3, 5):
                wrappers[args[0]] = (int(args[1]), int(args[3]) if extended else 1, int(args[4]) if extended else 0)
        values = tokens(source)
        for i, token in enumerate(values):
            if token == "FormulaParamMap" and values[i+2:i+4] == ["=", "{"]:
                end=matching(values,i+3)
                fields=split_token_args(values[i+4:end])
                if len(fields)!=2:raise ValueError("Unexpected formula parameter map")
                count=int(fields[0][0],0)
                cells=split_token_args(fields[1][1:-1])[:count]
                kinds=tuple(int(split_token_args(cell[1:-1])[1][0],0) for cell in cells)
                if len(kinds)!=count:raise ValueError("Incomplete formula parameter map")
                parameter_maps[values[i+1]]=kinds
            if token == "RegisterExternalFormula" and values[i+1:i+3] == ["(", "{"]:
                end = matching(values, i+2)
                fields = split_fields(values[i+3:end])
                name = json.loads(fields[1])
                if any(word in name.lower() for word in ("jit", "montecarlo", "halton")):
                    continue
                rows.append(fields)
    entries = []
    for fields in rows:
        symbol, name = fields[0], json.loads(fields[1])
        if symbol not in constants:
            raise ValueError("Unresolved ordinary registry id " + symbol)
        source = includes.get(symbol)
        de, adds, style = wrappers.get(symbol, (int(fields[2]), 1, 0))
        family = Path(source).parent.name if source else "native"
        kinds=parameter_maps.get(fields[9].lstrip("&"),())
        entries.append(Entry(constants[symbol], symbol, name, source, family, de, int(fields[3]), adds, style,kinds))
    if len(entries) != 457 or len({entry.id for entry in entries}) != len(entries) or len({entry.name for entry in entries}) != len(entries):
        raise ValueError("Ordinary registry inventory must contain 457 unique named entries")
    return sorted(entries, key=lambda entry: (entry.family, entry.id))


def expand_local(path, seen, provenance):
    path = path.resolve()
    if path in seen:
        return ""
    seen.add(path)
    source = path.read_text()
    provenance[str(path.relative_to(ROOT))] = digest(source)
    result = []
    for line in source.splitlines():
        include = re.match(r'\s*#\s*include\s+([<"])([^>"]+)[>"]', line)
        if include:
            if include[1] == "<":
                # Formula includes only declarations for memcpy; helpers replace
                # these explicitly, never include the host standard library.
                if include[2] not in ("cstring", "cmath", "cstdint", "limits"):
                    raise ValueError("Unrecognized system include: " + include[2])
                continue
            candidates = (path.parent / include[2], ROOT / "src" / include[2], ROOT / "src/core" / include[2])
            child = next((p for p in candidates if p.is_file()), None)
            if child is None or not child.resolve().is_relative_to(ROOT):
                raise ValueError("Unresolved local helper: " + include[2])
            result.append(expand_local(child, seen, provenance))
        elif not re.match(r'\s*#\s*pragma\s+once', line):
            result.append(line)
    return "\n".join(result) + "\n"


def select_arm(source, compiler):
    # Preprocess only: -E cannot produce object code, an executable or a library.
    command = [compiler, "-E", "-P", "-x", "c++", "-std=c++20", "-nostdinc", "-undef",
               "-DMB3D_SOFTWARE_EXT80=1", "-D__aarch64__=1", "-"]
    result = subprocess.run(command, input=source, text=True, capture_output=True)
    if result.returncode:
        raise ValueError("ARM preprocessing failed: " + result.stderr.strip())
    return result.stdout


def remove_metadata(values, symbol):
    traits, result, i = 0, [], 0
    while i < len(values):
        if values[i:i+4] == ["template", "<", ">", "struct"] and values[i+4] in TRAITS:
            begin = values.index("{", i); end = matching(values, begin)
            if "true" in values[begin:end]:
                traits |= 1 << TRAITS.index(values[i+4])
            i = end + 1 + (end+1 < len(values) and values[end+1] == ";")
            continue
        if values[i] == "FormulaInfo":
            if result and result[-1] == "constant": result.pop()
            begin = values.index("{", i); end = matching(values, begin)
            i = end + 1 + (end+1 < len(values) and values[end+1] == ";")
            continue
        result.append(values[i]); i += 1
    return result, traits


def lower_canonical_norms(values):
    """Scale only sqrt(a*a+b*b+c*c), with side-effect-free identifier leaves.

    Parentheses and association do not change the recognized sum. Calls,
    subscripts, members, coefficients and unequal factors are left untouched.
    This is an explicit Native FP32 range adaptation, not algebraic rewriting
    of general formula expressions.
    """
    def unparen(v):
        while len(v)>1 and v[0]=="(" and matching(v,0,"(",")")==len(v)-1:
            v=v[1:-1]
        return v
    def squares(v):
        v=unparen(v); depth=0
        for i,t in enumerate(v):
            if t=="(":depth+=1
            elif t==")":depth-=1
            elif t=="+" and depth==0:
                left,right=squares(v[:i]),squares(v[i+1:])
                return None if left is None or right is None else left+right
        if len(v)==3 and v[1]=="*" and v[0]==v[2] and re.fullmatch(r'[A-Za-z_]\w*',v[0]):
            return [v[0]]
        return None
    out=[];i=0
    while i<len(values):
        start=i
        if values[i:i+2]==["metal","::"]:i+=2
        if i<len(values) and values[i]=="sqrt" and values[i+1:i+2]==["("] and not(start and values[start-1] in ("::",".","->")):
            end=matching(values,i+1,"(",")")
            args=squares(values[i+2:end])
            if args is not None and len(args)==3:
                out += ["native_formula_length3","(",args[0],",",args[1],",",args[2],")"]
                i=end+1;continue
        i=start;out.append(values[i]);i+=1
    return out


def lower(values):
    # Operate on lexical tokens, never comments/strings or preprocessor branches.
    replacements = {"double": "float", "cpu_double2": "float2", "cpu_double3": "FormulaVec3", "cpu_double4": "float4",
                    "CPUVec3": "FormulaVec3", "int32_t": "int", "uint32_t": "uint", "int64_t": "long", "uint64_t": "ulong",
                    "int16_t": "short", "uint16_t": "ushort", "uint8_t": "uchar", "constant": "thread", "half": "formula_half",
                    "fabsl":"fabs", "nearbyint":"rint", "rintl":"rint", "atan2l":"atan2", "truncl":"trunc", "exp2l":"exp2", "ldexpl":"ldexp"}
    result, i = [], 0
    while i < len(values):
        value = values[i]
        if value == "UINT64_C" and values[i+1:i+2] == ["("]:
            end=matching(values,i+1,"(",")")
            if end!=i+3:raise ValueError("Nonliteral UINT64_C payload")
            result.append(values[i+2].rstrip("uUlL")+"UL");i=end+1;continue
        if value in ("E", "Extended80", "X87Real") and values[i+1:i+4] == ["::", "fromBits", "("]:
            result.extend(["numeric", "::", "fromBits", "("]);i+=4;continue
        if re.fullmatch(r'[A-Za-z_]\w*',value) and values[i+1:i+5] == [".", "signExponent", "(", ")"]:
            result.extend(["numeric", "::", "signExponent", "(", value, ")"]);i+=5;continue
        if values[i:i+3] == ["long", "double", "("] or values[i:i+2] == ["long", "double"]:
            result.append("float"); i += 2; continue
        if values[i:i+3] == ["mb3d", "::", "numeric"]:
            result.append("numeric"); i += 3; continue
        if values[i:i+4] == ["mb3d", "::", "NativePrecisionEnabled", "("]:
            end = matching(values, i+3, "(", ")"); result.append("true"); i = end+1; continue
        if values[i:i+2] == ["std", "::"]:
            # numeric_limits/bit_cast/memcpy require named helper lowering below.
            if values[i+2] in ("numeric_limits", "bit_cast", "memcpy", "swap", "pair"):
                result.extend(["native_std", "::"])
            else:
                result.extend(["metal", "::"])
            i += 2; continue
        if re.fullmatch(r'(?:\d+\.\d*|\.\d+|\d+[eE][+-]?\d+|\d+\.\d*[eE][+-]?\d+|0[xX][0-9a-fA-F]+(?:\.[0-9a-fA-F]*)?[pP][+-]?\d+)[fFlL]?', value):
            result.append(value.rstrip("fFlL") + "f")
        else:
            result.append(replacements.get(value, value))
        i += 1
    result=lower_canonical_norms(result)
    # Lower the source's round-to-nearest integer member on a full postfix
    # expression; do not strip the parentheses or change it to truncation.
    j=0
    while j<len(result)-3:
        if result[j:j+4]==[".","integer","(",")"]:
            start=j-1
            if result[start]==")":
                depth=1;start-=1
                while start>=0:
                    if result[start]==")":depth+=1
                    elif result[start]=="(":depth-=1
                    if depth==0:break
                    start-=1
            expression=result[start:j]
            result=result[:start]+["x87_fistp_int32","(",*expression,")"]+result[j+4:]
            j=start+len(expression)+3
        else:j+=1
    # Metal pointers and references must name their storage address space.
    # Formula state and parameter scratch are all per-invocation thread values.
    types = {"float", "float2", "float3", "float4", "FormulaVec3", "int", "uint", "long", "ulong", "short", "ushort", "uchar",
             "RotMatrix3", "RotMatrix4", "FormulaWorkBudget", "Extended80", "X87Real", "auto"}
    types.update(result[i+1] for i in range(len(result)-2) if result[i] == "using" and result[i+2] == "=")
    qualified, i = [], 0
    while i < len(result):
        if result[i] in types and i+1 < len(result) and result[i+1] in ("*", "&"):
            start = len(qualified)
            while start and qualified[start-1] in ("const", "volatile"):
                start -= 1
            if not (start and qualified[start-1] in ("thread", "constant", "device", "threadgroup")):
                qualified.insert(start, "thread")
        qualified.append(result[i]); i += 1
    final, depth = [], 0
    for value in qualified:
        if value == "constexpr" and depth == 0:
            final.append("constant")
        final.append(value)
        if value == "{":depth+=1
        elif value == "}":depth-=1
    return final


def emit_text(values):
    result = " ".join(values)
    return result.replace(" ;", ";\n").replace(" {", " {\n").replace(" }", "\n}\n") + "\n"


def lower_lambdas(values):
    """Replace source closures with explicit local functors (MSL has no lambdas).

    Capture names are selected from declarations visible before the lambda and
    actually referenced by its body. decltype preserves pointer/array qualifiers;
    captures by reference stay references to the original per-thread storage.
    """
    serial = 0
    while True:
        begin = next((i for i in range(2,len(values)-2) if values[i]=="[" and values[i-1]=="="
                      and values[i-3]=="auto"), None)
        if begin is None:return values
        end_capture = matching(values,begin,"[","]")
        arg_begin = end_capture+1
        if values[arg_begin]=="(":
            arg_end=matching(values,arg_begin,"(",")");args=values[arg_begin+1:arg_end];body_begin=arg_end+1
        else:args=[];body_begin=arg_begin
        mutable=False
        if values[body_begin]=="mutable":mutable=True;body_begin+=1
        return_type=["auto"]
        if values[body_begin]=="->":
            type_end=values.index("{",body_begin);return_type=values[body_begin+1:type_end];body_begin=type_end
        if values[body_begin]!="{":raise ValueError("Unsupported lambda declarator")
        body_end=matching(values,body_begin);body=values[body_begin+1:body_end]
        prefix=visible_function_prefix(values[:begin-3])
        type_names={"float","float2","float3","float4","FormulaVec3","FormulaParams","int","uint","long","ulong","short","ushort","auto","bool","Extended80","X87Real","RotMatrix3","RotMatrix4"}
        type_names.update(prefix[i+1] for i in range(len(prefix)-2) if prefix[i]=="using" and prefix[i+2]=="=")
        declared=declaration_names(prefix,type_names)
        locals_=declaration_names(args,type_names)|declaration_names(body,type_names)
        used={v for i,v in enumerate(body) if i==0 or body[i-1] not in (".","::")}
        capture_spec=values[begin+1:end_capture]
        captures=sorted((declared&used)-locals_-type_names) if capture_spec in (["&"],["="]) else [v for v in capture_spec if v not in ("&",",")]
        if any(not re.fullmatch(r'[A-Za-z_]\w*',v) for v in captures):raise ValueError("Unsupported initialized lambda capture")
        name=f"NativeClosure{serial}";serial+=1
        fields=[]
        for capture in captures:
            fields+=(["thread"] if capture_spec==["&"] else [])+["decltype","(",capture,")"]+(["&"] if capture_spec==["&"] else [])+[capture,";"]
        closure=["struct",name,"{",*fields,*return_type,"operator","(",")","(",*args,")"]
        if not mutable:closure.append("const")
        closure += ["{",*body,"}","}",";"]
        start=begin-3
        if values[start]!="auto":raise ValueError("Lambda binding must use auto")
        if start and values[start-1]=="const":start-=1
        initialization=[]
        for capture in captures:
            if initialization:initialization.append(",")
            initialization.append(capture)
        values=values[:start]+closure+values[start:begin]+[name,"{",*initialization,"}"]+values[body_end+1:]


def declaration_names(seq,type_names):
    """Lex declarations, including comma and direct-initialized declarators."""
    names=set()
    for i,value in enumerate(seq):
        if value not in type_names:continue
        j=i+1
        while j<len(seq) and seq[j] in ("const","volatile","*","&"):j+=1
        if j>=len(seq) or not re.fullmatch(r'[A-Za-z_]\w*',seq[j]) or seq[j] in type_names:continue
        if j+1>=len(seq) or seq[j+1] not in ("=","(","[",",",";",")","{"):continue
        names.add(seq[j]);j+=1
        while j<len(seq):
            if seq[j] in ("(","[","{"):
                pairs={"(":")","[":"]","{":"}"}
                try:j=matching(seq,j,seq[j],pairs[seq[j]])+1
                except ValueError:break
                continue
            if seq[j] in (";",")","}"):break
            if seq[j]=="," and j+2<len(seq) and re.fullmatch(r'[A-Za-z_]\w*',seq[j+1]) and seq[j+1] not in type_names and seq[j+2] in ("=","(","[",",",";",")"):
                names.add(seq[j+1])
            j+=1
    return names


def visible_function_prefix(seq):
    """Discard previous functions and closed scopes before capture analysis."""
    braces=[];parens=[];starts=[]
    for i,value in enumerate(seq):
        if value=="(":parens.append(i)
        elif value==")" and parens:starts.append((i,parens.pop()))
        elif value=="{":
            previous=next((a for b,a in reversed(starts) if b==i-1),None)
            function=previous is not None and previous>0 and seq[previous-1] not in ("if","for","while","switch","catch")
            braces.append((i,previous if function else None))
        elif value=="}" and braces:braces.pop()
    function_start=next((p for _,p in reversed(braces) if p is not None),0)
    visible=[];i=function_start
    active={a for a,_ in braces}
    while i<len(seq):
        if seq[i]=="{" and i not in active:
            i=matching(seq,i)+1
        else:visible.append(seq[i]);i+=1
    return visible


def blocked_constructs(values):
    text = " ".join(values)
    reasons = []
    checks = {
        "map_resource_lowering_pending": r'\bTLMSfunction\b|\bcpu_map_(?:direct|sphere)_sample\b',
        "binary64_alias_layout_lowering_pending": r'\breinterpret_cast\b',
        "scalar_extended_methods_pending": r'\b(?:E|Extended80|X87Real)\s*::|\.\s*(?:significand|signExponent|toDouble|toFloat)\b',
        "conditional_engine_entry": r'\bExecuteConditionalFormula\b',
    }
    for reason, expression in checks.items():
        if re.search(expression, text): reasons.append(reason)
    return reasons


def replace_tokens(values,old,new):
    old=tokens(old);new=tokens(new);out=[];i=0
    while i<len(values):
        if values[i:i+len(old)]==old:out+=new;i+=len(old)
        else:out.append(values[i]);i+=1
    return out


def lower_map_bindings(values,sphere):
    # A source callback becomes a static GPU sampling function with explicit
    # uploaded resources. No function pointer or CPU callback survives.
    out=[];i=0
    while i<len(values):
        if values[i]=="TLMSfunction":out.append("bool");i+=1;continue
        if values[i]=="nullptr":out.append("false");i+=1;continue
        if values[i] in ("cpu_map_direct_sample","cpu_map_sphere_sample") and values[i+1]=="(":
            out += ["native_formula_map_"+("direct" if "direct" in values[i] else "sphere"),"(","params",","];i+=2;continue
        if values[i:i+2]==["map","("]:
            end=matching(values,i+1,"(",")")
            out += ["native_formula_map","(","params",",",*values[i+2:end],",","true" if sphere else "false",")"];i=end+1;continue
        out.append(values[i]);i+=1
    # Three cylinder/torus variants intentionally alias two Single values in
    # one Double scratch lane. Keep their raw words separate from FP32 numeric
    # storage; channel3 still observes the decoded packed Double, as in CPU.
    if "storeSingle" in out:
        out=replace_tokens(out,"float s [ 11 ]", "ulong native_s8_bits=0; float s[11]")
        out=replace_tokens(out,"native_std :: memcpy ( reinterpret_cast < unsigned char * > ( s + 5 ) + offset , & f , 4 )",
            "native_store_packed_single(native_s8_bits,s[8],offset,f)")
        out=replace_tokens(out,"s [ 8 ] = float ( axis ) ;", "s[8]=float(axis); native_s8_bits=mb3d_metal_exact::native_encode_double(s[8]);")
        out=replace_tokens(out,"native_std :: memcpy ( & storedDistance , reinterpret_cast < unsigned char * > ( s + 5 ) + 24 , 4 )",
            "storedDistance=as_type<float>(uint(native_s8_bits))")
        out=replace_tokens(out,"native_std :: memcpy ( & storedRadius , reinterpret_cast < unsigned char * > ( s + 5 ) + 28 , 4 )",
            "storedRadius=as_type<float>(uint(native_s8_bits>>32))")
    return out


def lower_context(values):
    """Carry resources, fault/work bounds and original parameter bits explicitly.

    FormulaParams remains an indexed FP32 scratch view. It does not fetch CPU
    data at runtime; all descriptors and original payload words are GPU buffers.
    """
    out=[];i=0
    while i<len(values):
        if values[i]=="incrementFormulaCounter" and i and values[i-1]=="int" and values[i+1]=="(":
            close=matching(values,i+1,"(",")")
            if values[close+1]!="{":raise ValueError("Unexpected counter helper declaration")
            end=matching(values,close+1)
            out += ["incrementFormulaCounter","(","FormulaParams","params",")","{","return","native_increment_formula_counter","(","params",")",";","}"]
            i=end+1;continue
        if values[i:i+4]==["params","[","BFIRSTIT_SLOT","]"]:
            if values[i+4:i+5]==["+="]:
                end=values.index(";",i+5)
                if len(values[i+5:end])!=1 or values[i+5] not in ("1","1.","1.f","1.0","1.0f"):
                    raise ValueError("Unrecognized integer formula counter increment")
                out += ["native_increment_formula_counter","(","params",")"];i=end;continue
            # bFirstIt is an int32 lane even when the CPU wrapper transports it
            # through a binary64 scratch cell. Do not round it through FP32.
            out += ["native_formula_counter","(","params",")"];i+=4;continue
        if values[i:i+4]==["thread","float","*","params"]:
            out += ["FormulaParams","params"];i+=4;continue
        if values[i:i+4]==["thread","float","*","values"]:
            out += ["FormulaParams","values"];i+=4;continue
        if values[i:i+5]==["thread","const","float","*","params"]:
            out += ["FormulaParams","params"];i+=5;continue
        if values[i]=="FormulaWorkBudget" and i+2<len(values) and values[i+2]=="(":
            out += values[i:i+3]+["params",","];i+=3;continue
        if i+5<len(values) and values[i+1:i+6]==[".","beforeStep","(",")",";"]:
            out += ["if","(","!",values[i],".","beforeStep","(",")",")","return","NAN",";"];i+=6;continue
        # Preserve literal binary64 payload reads from uploaded parameters. An
        # arithmetic FP32 value uses the boundary encoder instead.
        if values[i:i+7]==["native_std","::","bit_cast","<","ulong",">","("] and values[i+7:i+9]==["params","["]:
            close=matching(values,i+8,"[","]")
            if values[close+1]==")":
                out += ["params",".","bits","(",*values[i+9:close],")"];i=close+2;continue
        if values[i:i+3]==["native_std","::","memcpy"] and values[i+3]=="(":
            end=matching(values,i+3,"(",")");args=split_token_args(values[i+4:end])
            if len(args)==3 and len(args[0])==2 and args[0][0]=="&" and args[1][:3]==["&","params","["]:
                out += [args[0][1],"=","params",".","bits","(",*args[1][3:-1],")"];i=end+1;continue
        out.append(values[i]);i+=1
    return out


def lower_typed_seed_reads(values,entry):
    """Preserve the two random modifiers' declared int32 seed words.

    Only the audited bounded read-only seed loop is admitted here. Ordinary
    floating parameters and mutable scratch reads keep Native FP32 semantics.
    """
    variable={10019:"plane",10320:"axis"}.get(entry.id)
    if variable is None:return values
    if entry.parameter_types[3:6]!=(2,2,2):
        raise ValueError("Random seed reads must have three declared int32 parameters")
    loop=tokens(f'for(int {variable}=0;{variable}<3;++{variable})')
    if not any(values[i:i+len(loop)]==loop for i in range(len(values))):
        raise ValueError("Random seed loop bounds changed")
    for i in range(len(values)-2):
        if values[i:i+2]==["params","["]:
            close=matching(values,i+1,"[","]")
            if values[close+1:close+2] and values[close+1] in ("=","+=","-=","*=","/=","++","--"):
                raise ValueError("Typed seed lowering requires read-only formula parameter scratch")
    old=tokens(f'static_cast<int>(params[3+{variable}])')
    if sum(values[i:i+len(old)]==old for i in range(len(values)))!=1:
        raise ValueError("Random seed int32 conversion shape changed")
    return replace_tokens(values,' '.join(old),f'native_int32_parameter(params,3+{variable})')


def lower_riemann_native(values,entry):
    # Keep CPU/Compatible source and every finite original Native result.
    # Only Native output invalidated by trig cancellation uses the bounded
    # homogeneous inverse stereographic reconstruction.
    if entry.id!=7100:return values
    return replace_tokens(values,
        'return FormulaVec3(float((nx+nx)+E(params[2])),float(ny+E(params[3])),float((nz+nz)+E(params[4])));',
        'const FormulaVec3 original(float((nx+nx)+E(params[2])),float(ny+E(params[3])),float((nz+nz)+E(params[4]))); '
        'return FormulaVec3(mb3d_native_riemann::reconstruct(float3(original),angleA,angleB,radius,float3(params[2],params[3],params[4]), all(isfinite(float4(x,y,z,projection))) && all(isfinite(float4(a,b,a+a,b+b))) && all(isfinite(float2(a*a-1.f,b*b-1.f)))).value);')


# Box bodies whose precomputed Scale/MinR^2 (params[1]) is consumed only by
# the inner-sphere branch. Its MinR^2 (params[2]) can underflow in FP32.
BOX_UNORDERED_SCALE={
    1509:'!(rr>=E(params[2])) ? E(params[1])',
    10046:'!(rr>=params[2]) ? E(params[1])',
    10049:'!(rr>=params[2]) ? E(params[1])',
    10186:'!(rr>=E(params[2])) ? E(params[1])',
    10187:'!(rr>=E(params[2])) ? E(params[1])',
    10324:'!(rr>=params[2]) ? params[1]',
}
BOX_ORDERED_SCALE={8024:('ScaleOverMinR2','minR2'),10173:('ScaleMinR2','MinR2')}


def lower_aboxmod1_inner_scale(values,entry):
    if entry.id in BOX_UNORDERED_SCALE:
        old=BOX_UNORDERED_SCALE[entry.id]
        new=('native_box_inner_unordered(rr,params) ? native_aboxmod1_inner_scale(params)' if entry.id==10324
             else 'native_box_inner_unordered(float(rr),params) ? E(native_aboxmod1_inner_scale(params))')
        pattern=tokens(old)
        if sum(values[i:i+len(pattern)]==pattern for i in range(len(values)))!=1:
            raise ValueError('Box unordered deferred scale source contract changed')
        return replace_tokens(values,old,new)
    if entry.id not in BOX_ORDERED_SCALE:return values
    scale,minimum=BOX_ORDERED_SCALE[entry.id]
    # Exact patterns make a changed source body fail closed rather than
    # accidentally checking the scale eagerly or changing another formula.
    replacements=[(f'const float {scale} = params[1];',''),
                  (f'const float {minimum} = params[2];',''),
                  (f'rr < {minimum}','native_aboxmod1_inner(rr,params)'),
                  (f'm = {scale};','m = native_aboxmod1_inner_scale(params);')]
    for old,new in replacements:
        pattern=tokens(old)
        if sum(values[i:i+len(pattern)]==pattern for i in range(len(values)))!=1:
            raise ValueError('ABoxMod1 deferred scale source contract changed')
        values=replace_tokens(values,old,new)
    return values


# These original M3F bodies update coordinates and Julia constants independently.
# Their CPU adapter returns coordinates-C for the wrapper's later +C. Lowering
# that adapter literally to FP32 loses small coordinates and signed zero. Keep
# the CPU ABI unchanged; explicitly pair the direct body with a GPU result
# contract that suppresses only this redundant wrapper addition.
DIRECT_COORDINATE_RESULTS = {
    10001: ('FORMULA__TRANSLATEC4D', '_TranslateC4d', 'Ads/_TranslateC4d.inc', 'Ads', -1, 8, 544),
    9525: ('FORMULA__SCALEC4D', '_ScaleC4d', '4Da/_ScaleC4d.inc', '4Da', 5, 2, 0),
}


def lower_direct_coordinate_result(values, entry, traits):
    expected = DIRECT_COORDINATE_RESULTS.get(entry.id)
    if expected is None:
        return values, False
    descriptor = (entry.symbol, entry.name, entry.source, entry.family,
                  entry.de_option, entry.param_count, traits)
    if descriptor != expected or entry.adds_c != 0 or entry.difs_style != 0:
        raise ValueError('Direct-coordinate descriptor contract changed')
    if entry.id == 10001:
        replacements = [
            ('return FormulaVec3(p.x+x_add-cx_out,p.y+y_add-cy_out,p.z+z_add-cz_out);',
             'return FormulaVec3(p.x+x_add,p.y+y_add,p.z+z_add);', 1)]
    else:
        # Both the template and the alternative direct function are emitted.
        replacements = [
            ('p*Scale-FormulaVec3(params[2],params[3],params[4])', 'p*Scale', 2),
            ('de=de*Scale-params[5];', 'de=de*Scale;', 2)]
    for old, new, count in replacements:
        pattern = tokens(old)
        if sum(values[i:i+len(pattern)] == pattern for i in range(len(values))) != count:
            raise ValueError('Direct-coordinate source contract changed')
        values = replace_tokens(values, old, new)
    return values, True


# Original _reciprocalX3b/_reciprocalY3b CODE squares the coordinate at
# 0x23, then multiplies by the second multiplier at 0x25. The CPU adapter's
# left-associated multiplier*value*value changes Native FP32 rounding.
# Preserve that original order in the common body; do not disable contraction
# for other expressions or change either CPU arithmetic profile.
RECIPROCAL_SQUARE_FIRST = {
    4812: ('FORMULA__RECIPROCALX3B', '_reciprocalX3b', 'Ads/_reciprocalX3b.inc',
           'Ads', -1, 6, 1, 0, (0, 0, 0, 0, 0, 0), 0),
    5823: ('FORMULA__RECIPROCALY3B', '_reciprocalY3b', 'Ads/_reciprocalY3b.inc',
           'Ads', -1, 6, 1, 0, (0, 0, 0, 0, 0, 0), 0),
}


def lower_reciprocal_square_first(values, entry, traits):
    expected = RECIPROCAL_SQUARE_FIRST.get(entry.id)
    if expected is None:
        return values
    descriptor = (entry.symbol, entry.name, entry.source, entry.family,
                  entry.de_option, entry.param_count, entry.adds_c,
                  entry.difs_style, entry.parameter_types, traits)
    if descriptor != expected:
        raise ValueError('Reciprocal square-first descriptor contract changed')
    old = 'result += params[3] - 1.0f / (abs(params[5]*value*value)+params[2]);'
    new = 'result += params[3] - 1.0f / (abs((value*value)*params[5])+params[2]);'
    pattern = tokens(old)
    if sum(values[i:i+len(pattern)] == pattern for i in range(len(values))) != 1:
        raise ValueError('Reciprocal square-first source contract changed')
    return replace_tokens(values, old, new)


GROUP02_INTEGER_SEMANTICS = {
    7010: ('FORMULA__SINY', '_SinY', 'Ads/_SinY.inc',
           'Ads', -1, 5, 1, 0, (2, 1, 1, 1, 1), 512),
    7065: ('FORMULA__POLYFOLDING', '_PolyFolding', 'Ads/_PolyFolding.inc',
           'Ads', -1, 7, 1, 0, (0, 0, 0, 0, 0, 0, 0), 0),
    7076: ('FORMULA__POLYFOLDSYM', '_PolyFold-sym', 'Ads/_PolyFold-sym.inc',
           'Ads', -1, 7, 1, 0, (0, 0, 0, 0, 0, 0, 0), 0),
    7081: ('FORMULA__NGON', '_ngon', 'Ads/_ngon.inc',
           'Ads', -1, 8, 1, 0, (0, 0, 0, 0, 0, 2, 0, 0), 0),
}


def lower_group02_integer_semantics(values, entry, traits):
    """Keep audited original integer operations out of FP32 parameter casts.

    This is limited to the four reviewed bodies. FRNDINT retains a real
    sector; FISTP/FILD reloads an int32 sector. Neither changes CPU profiles.
    """
    expected = GROUP02_INTEGER_SEMANTICS.get(entry.id)
    if expected is None:
        return values
    descriptor = (entry.symbol, entry.name, entry.source, entry.family,
                  entry.de_option, entry.param_count, entry.adds_c,
                  entry.difs_style, entry.parameter_types, traits)
    if descriptor != expected:
        raise ValueError('Group02 integer descriptor contract changed')
    replacements = []
    typed = {7010: (0, 'Index'), 7081: (5, 'No_yfold')}.get(entry.id)
    if typed:
        index, name = typed
        access = tokens(f'params[{index}]')
        if sum(values[i:i+len(access)] == access for i in range(len(values))) != 1:
            raise ValueError('Group02 immutable typed parameter contract changed')
        replacements.append((f'const int {name}=int(params[{index}]);',
                             f'const int {name}=native_int32_parameter(params,{index});'))
    if entry.id == 7065:
        replacements.append(('float sector=round(combined/TWOPI*Order);',
                             'float sector=rint(combined/TWOPI*Order);'))
    if entry.id in (7076, 7081):
        replacements.append(('float sector=rint(sector_value); int sector_i=int(sector);',
                             'int sector_i=0; float sector=native_fistp_sector(sector_value,sector_i);'))
    for old, new in replacements:
        pattern = tokens(old)
        if sum(values[i:i+len(pattern)] == pattern for i in range(len(values))) != 1:
            raise ValueError('Group02 integer source contract changed')
        values = replace_tokens(values, old, new)
    return values


GROUP03_ORIGINAL_SEMANTICS = {
    8481: ('FORMULA__ROTATEDFOLDING', '_RotatedFolding', 'Ads/_RotatedFolding.inc',
           'Ads', -1, 10, 1, 0, (1, 1, 1, 1, 1, 1, 1, 1, 1, 0), 0),
    10006: ('FORMULA__MAPTRANSLATE', '_MapTranslate', 'Ads/_MapTranslate.inc',
            'Ads', -1, 7, 1, 0, (2, 1, 1, 1, 1, 1, 1), 0),
}


def lower_group03_original_semantics(values, entry, traits):
    """Preserve the two independently diagnosed original operations.

    RotatedFolding subtracts from x and then applies FCHS, including -0.
    MapTranslate loads a raw int32 selector, never a narrowed FP32 value.
    The source/ABI patterns are deliberately limited to these two bodies.
    """
    expected = GROUP03_ORIGINAL_SEMANTICS.get(entry.id)
    if expected is None:
        return values
    descriptor = (entry.symbol, entry.name, entry.source, entry.family,
                  entry.de_option, entry.param_count, entry.adds_c,
                  entry.difs_style, entry.parameter_types, traits)
    if descriptor != expected:
        raise ValueError('Group03 original-operation descriptor contract changed')
    def occurrences(text):
        pattern = tokens(text)
        return sum(values[i:i+len(pattern)] == pattern for i in range(len(values)))
    if entry.id == 8481:
        old = 'r0=fabs(r0+fold)-fabs(r0-fold)-r0;'
        if occurrences(old) != 1:
            raise ValueError('Group03 original FCHS source contract changed')
        return replace_tokens(values, old, 'r0=-(r0-(fabs(r0+fold)-fabs(r0-fold)));')
    calls = ('native_formula_map_sphere(params,p.x,p.y,p.z,int(params[0]))',
             'native_formula_map(params,&sample,&input,int(params[0]),false)')
    if occurrences('params[0]') != 2 or any(occurrences(call) != 1 for call in calls):
        raise ValueError('Group03 immutable map selector source contract changed')
    return replace_tokens(values, 'int(params[0])', 'native_int32_parameter(params,0)')


GROUP04_ORIGINAL_SEMANTICS = {
    7071: ('FORMULA_MSLTOESYM2', 'MsltoeSym2', '3D/MsltoeSym2.inc',
           '3D', 0, 1, 1, 0, (0,), 0),
    7085: ('FORMULA_BENESI3POW2', 'Benesi3Pow2', '3D/Benesi3Pow2.inc',
           '3D', 0, 0, 1, 0, (), 0),
    10269: ('FORMULA_TGLADTETRA', 'TgladTetra', '3D/TgladTetra.inc',
            '3D', 0, 9, 1, 0, (2, 2, 0, 0, 0, 0, 0, 0, 0), 0),
    10352: ('FORMULA_KALILINCOMBSSE2SM', 'KaliLinCombSSE2sm', '3D/KaliLinCombSSE2sm.inc',
            '3D', 0, 3, 1, 0, (2, 0, 0), 4),
}


def lower_group04_original_semantics(values, entry, traits):
    """Restore independently decoded original state, sign and integer reads.

    Msltoe reads the caller's saved Rout. Benesi inspects Jx's sign byte,
    including negative zero. Tglad and Kali read typed integers before masks
    or shifts. The CPU adapter descriptor and both CPU modes stay unchanged.
    """
    expected = GROUP04_ORIGINAL_SEMANTICS.get(entry.id)
    if expected is None:
        return values
    descriptor = (entry.symbol, entry.name, entry.source, entry.family,
                  entry.de_option, entry.param_count, entry.adds_c,
                  entry.difs_style, entry.parameter_types, traits)
    if descriptor != expected:
        raise ValueError('Group04 original-operation descriptor contract changed')
    replacements = {
        7071: [('float r2=x*x+y*y+z*z;', 'float r2=params[FORMULA_ROUT_SLOT];')],
        7085: [('rho < x && Cx >= 0.0f', 'rho < x && !signbit(Cx)')],
        10269: [('int(params[0])', 'native_int32_parameter(params,0)'),
                ('int(params[1])', 'native_int32_parameter(params,1)')],
        10352: [('static_cast<uint>(static_cast<int>(params[0]))',
                 'as_type<uint>(native_int32_parameter(params,0))')],
    }[entry.id]
    # Typed source slots are immutable and read exactly once in these bodies.
    for index in ({10269: (0, 1), 10352: (0,)}.get(entry.id, ())):
        pattern = tokens(f'params[{index}]')
        if sum(values[i:i+len(pattern)] == pattern for i in range(len(values))) != 1:
            raise ValueError('Group04 immutable integer read contract changed')
    for old, new in replacements:
        pattern = tokens(old)
        if sum(values[i:i+len(pattern)] == pattern for i in range(len(values))) != 1:
            raise ValueError('Group04 original-operation source contract changed')
        values = replace_tokens(values, old, new)
    return values


def lower_translate_source_add(values, entry, traits):
    """Preserve original RN32 additions even when hardware flushes subnormals."""
    if entry.id != 8065:
        return values
    descriptor = (entry.symbol, entry.name, entry.source, entry.family,
                  entry.de_option, entry.param_count, entry.adds_c,
                  entry.difs_style, entry.parameter_types, traits)
    expected = ('FORMULA__TRANSLATE', '_Translate', 'Ads/_Translate.inc', 'Ads',
                -1, 3, 1, 0, (0, 0, 0), 0)
    if descriptor != expected:
        raise ValueError('Translate original-add descriptor contract changed')
    original = ROOT / 'mb3d/M3Formulas/_Translate.m3f'
    if digest(original.read_bytes()) != '3acdcb61881b9007cd1a66a33887d8a13ae7541bd3120472b582e2ac9a09b706':
        raise ValueError('Translate original CODE contract changed')
    replacements = (
        ('return FormulaVec3(p.x+X_add,p.y+Y_add,p.z+Z_add);',
         'return FormulaVec3(native_translate_add(p.x,X_add),native_translate_add(p.y,Y_add),native_translate_add(p.z,Z_add));'),
        ('return FormulaVec3(p.x+params[0],p.y+params[1],p.z+params[2]);',
         'return FormulaVec3(native_translate_add(p.x,params[0]),native_translate_add(p.y,params[1]),native_translate_add(p.z,params[2]));'),
    )
    for old, new in replacements:
        pattern = tokens(old)
        if sum(values[i:i+len(pattern)] == pattern for i in range(len(values))) != 1:
            raise ValueError('Translate original-add source contract changed')
        values = replace_tokens(values, old, new)
    return values


def emit_group04_source_contract(values, entry):
    text = emit_text(values)
    if entry.id != 10269:
        return text
    # Original Tglad instructions keep the cubic and complex products separate.
    # Exact-rational replay of the frozen AIR proves that contracting them
    # causes the independently pinned interior-case differences. Scope this
    # rule to this function: keep hardware FP32 and other formulas' policy.
    signature = emit_text(tokens('template<> FormulaVec3 execute_formula<FORMULA_TGLADTETRA>'
                                 '(FormulaVec3 p,FormulaParams params,thread float& de){')).strip()
    if text.count(signature) != 1:
        raise ValueError('Group04 Tglad arithmetic scope contract changed')
    return text.replace(signature, signature+'\n#pragma clang fp contract(off)', 1)


def emit_surf_source_contract(text, entry, traits):
    """Keep the original Surf products/adds separate; never fit device targets."""
    if entry.id != 1509:
        return text
    descriptor = (entry.symbol, entry.name, entry.source, entry.family,
                  entry.de_option, entry.param_count, entry.adds_c,
                  entry.difs_style, entry.parameter_types, traits)
    expected = ('FORMULA_AMAZING_SURF', 'Amazing Surf', '3Da/Amazing_Surf.inc',
                '3Da', 11, 15, 1, 0, (0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 2), 0)
    if descriptor != expected:
        raise ValueError('Surf source-order descriptor contract changed')
    original = ROOT / 'mb3d/M3Formulas/Amazing Surf.m3f'
    if digest(original.read_bytes()) != '19f5cfa2cdd127a34851f7312c0226078aea675ca9d17e6079b46b9ec70be03d':
        raise ValueError('Surf original CODE contract changed')
    if digest(text) != '55d626a5c879ffe6ab8052d1a6f08c3c7b96e7a6f5d16f15334bcc5a2b6fce67':
        raise ValueError('Surf reviewed operation sequence changed')
    signature = emit_text(tokens('template<> FormulaVec3 execute_formula<FORMULA_AMAZING_SURF>'
                                 '(FormulaVec3 p,FormulaParams params,thread float& de){')).strip()
    if text.count(signature) != 1:
        raise ValueError('Surf arithmetic scope contract changed')
    return text.replace(signature, signature+'\n#pragma clang fp contract(off)', 1)


def generate(output, report_path, compiler):
    entries = registry()
    output.mkdir(parents=True, exist_ok=True)
    records, families, provenance = [], {}, {}
    source_traits={}
    for table in TABLES:
        provenance[str(table.relative_to(ROOT))] = digest(table.read_bytes())
        source=table.read_text()
        for index,trait in enumerate(TRAITS):
            for symbol,body in re.findall(r'\b'+trait+r'\s*<\s*(FORMULA_\w+)\s*>\s*\{([^}]+)\}',source):
                if re.search(r'\btrue\b',body):source_traits[symbol]=source_traits.get(symbol,0)|(1<<index)
    provenance['scripts/gen_native_metal_formulas.py']=digest(Path(__file__).read_bytes())
    for entry in entries:
        row = dict(vars(entry), status="blocked", traits=0, reasons=[], qualification="not_run")
        if 10353 <= entry.id <= 10357:
            row["status"]="driver_implemented_unqualified"
            row["implementation"]="src/render/native_metal/internal_formulas.metal.h:native_conditional_step"
        elif not entry.source:
            source=tokens(TABLES[0].read_text())
            begin=source.index("benesiTransform")-2
            body=source.index("{",begin);end=matching(source,body)
            selected=source[begin:end+1]
            lowered=lower_lambdas(lower(selected))
            lowered += tokens(f'template<> FormulaVec3 execute_formula<{entry.id}>(FormulaVec3 p, FormulaParams params, thread float& de) {{ benesiTransform(p.x,p.y,p.z,{entry.id-95000},params[0],params[1]); return p; }}')
            destination=output/f'formula_{entry.id}.metal.h'
            write_if_changed(destination,'// Generated ordinary Benesi transform; Native FP32, unqualified.\n#pragma once\nnamespace mb3d_native { namespace formula_'+str(entry.id)+' {\n'
                'template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);\n'+emit_text(lowered)+'}}\n')
            row.update(status="emitted_uncompiled",selected_arm_sha256=digest(emit_text(selected)),lowered_sha256=digest(emit_text(lowered)),generated_path='src/render/native_metal/generated/'+destination.name,generated_sha256=digest(destination.read_bytes()))
            families.setdefault("native",[]).append(entry)
        else:
            try:
                expanded = expand_local(FORMULAS / entry.source, set(), provenance)
                selected = select_arm(expanded, compiler)
                values, row["traits"] = remove_metadata(tokens(selected), entry.symbol)
                row["traits"] |= source_traits.get(entry.symbol,0)
                lowered = lower_context(lower(values))
                lowered, direct_coordinates = lower_direct_coordinate_result(lowered, entry, row["traits"])
                if direct_coordinates:row["result_contract"] = "direct_coordinates"
                lowered = lower_reciprocal_square_first(lowered, entry, row["traits"])
                lowered = lower_typed_seed_reads(lowered,entry)
                lowered = lower_group02_integer_semantics(lowered, entry, row["traits"])
                lowered = lower_riemann_native(lowered,entry)
                lowered = lower_aboxmod1_inner_scale(lowered,entry)
                lowered = lower_lambdas(lower_map_bindings(lowered,bool(row["traits"]&(1<<14))))
                lowered = lower_group03_original_semantics(lowered, entry, row["traits"])
                lowered = lower_group04_original_semantics(lowered, entry, row["traits"])
                lowered = lower_translate_source_add(lowered, entry, row["traits"])
                if entry.id == 8065:
                    row["arithmetic_contract"] = "source_rn32_add_exact_scale24"
                    provenance['mb3d/M3Formulas/_Translate.m3f'] = digest((ROOT/'mb3d/M3Formulas/_Translate.m3f').read_bytes())
                if entry.id == 7071:
                    row["caller_state_contract"] = "saved_rout"
                if entry.id == 10269:
                    row["arithmetic_contract"] = "source_ordered_fp32"
                row["reasons"] = blocked_constructs(lowered)
                row["selected_arm_sha256"] = digest(selected)
                row["numeric_helpers"] = sorted(set(re.findall(r'\bnumeric\s*::\s*(\w+)', " ".join(lowered))))
                lowered_text = emit_group04_source_contract(lowered, entry)
                lowered_text = emit_surf_source_contract(lowered_text, entry, row["traits"])
                if entry.id == 1509:
                    row["arithmetic_contract"] = "source_ordered_fp32"
                    provenance['mb3d/M3Formulas/Amazing Surf.m3f'] = digest((ROOT/'mb3d/M3Formulas/Amazing Surf.m3f').read_bytes())
                row["lowered_sha256"] = digest(lowered_text)
                # Retain every lowered candidate for review; blocked candidates
                # are never included by the active dispatch headers.
                name = f"formula_{entry.id}.metal.h"
                destination = output / name
                prefix = f"// Generated from {entry.source}; ARM branch, Native FP32.\n// Qualification: NOT RUN. Do not count source emission as GPU support.\n"
                write_if_changed(destination,prefix + "#pragma once\nnamespace mb3d_native { namespace formula_" + str(entry.id) + " {\n" +
                    "template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);\n" +
                    "template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);\n" +
                    "template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);\n" +
                    "template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }\n" +
                    "template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }\n" +
                    lowered_text + "}}\n")
                row["generated_path"] = 'src/render/native_metal/generated/'+destination.name
                row["generated_sha256"] = digest(destination.read_bytes())
                if not row["reasons"]:
                    row["status"] = "emitted_uncompiled"
                    families.setdefault(entry.family, []).append(entry)
            except (OSError, ValueError) as error:
                # A changed audited contract must stop generation, not drop
                # its body while retaining an old generated result contract.
                if (entry.id in (8065, 1509) or entry.id in DIRECT_COORDINATE_RESULTS or
                        entry.id in RECIPROCAL_SQUARE_FIRST or
                        entry.id in GROUP02_INTEGER_SEMANTICS or
                        entry.id in GROUP03_ORIGINAL_SEMANTICS or
                        entry.id in GROUP04_ORIGINAL_SEMANTICS):
                    raise
                row["reasons"] = [str(error)]
        records.append(row)
    for family, items in sorted(families.items()):
        write_if_changed(output / ("family_" + family + ".metal.h"),"// Generated ordinary formula family; not qualified.\n#pragma once\n" +
            "\n".join(f'#include "formula_{entry.id}.metal.h"' for entry in items) + "\n")
    active = [row for row in records if row["status"] == "emitted_uncompiled"]
    metadata = ["// Generated device admission metadata. Emission is not qualification.", "#pragma once", '#include "../formula_abi.h"',
                "namespace mb3d_native {", "inline bool configure_native_external_slot(NativeFormulaSlot& slot) {", "switch(slot.id) {"]
    for row in records:
        if row["status"] not in ("emitted_uncompiled","driver_implemented_unqualified"):continue
        metadata.append(f'case {row["id"]}u: slot.traits={row["traits"]}u; slot.wrapper_flags={row["adds_c"]}u; slot.difs_style={row["difs_style"]}u; return true;')
    metadata += ["default: return false;", "}}", "}"]
    write_if_changed(output / "formula_metadata.h","\n".join(metadata)+"\n")
    # Bodies that return extendedNaN emulate the x87 indefinite of an
    # out-of-range FSIN/FCOS/FPTAN. That NaN is the formula's result in every
    # precision, never an FP32 overflow.
    indefinite = sorted(row["id"] for row in active
                        if "extendedNaN" in (output / Path(row["generated_path"]).name).read_text())
    write_if_changed(output / "formula_indefinite.metal.h", "\n".join(
        ["// Generated: formulas whose bodies return the x87 indefinite NaN (extendedNaN).", "#pragma once",
         "namespace mb3d_native {", "inline bool native_formula_x87_indefinite(uint id) {", "switch(id) {"] +
        [f"case {i}u:" for i in indefinite] + (["return true;"] if indefinite else []) +
        ["default: return false;", "}}", "}"]) + "\n")
    dispatch = ["// Generated raw formula dispatch; all bodies require device qualification.", "#pragma once", '#include "../formula_math.metal.h"']
    dispatch += [f'#include "family_{family}.metal.h"' for family in sorted(families)]
    direct_ids = [row["id"] for row in active if row.get("result_contract") == "direct_coordinates"]
    result_contract = ["// Generated only from successful checked body lowerings.", "#pragma once",
                       "namespace mb3d_native {", "inline bool native_external_returns_coordinates(uint id) {", "switch(id) {"]
    result_contract += [f"case {id}u:" for id in sorted(direct_ids)]
    if direct_ids:result_contract.append("return true;")
    result_contract += ["default: return false;", "}}", "}"]
    # A checked original-CODE lowering may require caller state absent from
    # the inherited CPU descriptor. Bind that input by the audited identity;
    # do not overwrite reserved/parameter lanes of unrelated formulas.
    caller_rout_rows = [row for row in active if row.get("caller_state_contract") == "saved_rout"]
    caller_contract = ["namespace mb3d_native {",
                       "inline bool native_external_reads_caller_rout(uint id,uint count,uint traits,int option,uint flags) {",
                       "switch(id) {"]
    for row in sorted(caller_rout_rows,key=lambda r:r["id"]):
        caller_contract.append(f'case {row["id"]}u: return count=={row["param_count"]}u && '
                               f'traits=={row["traits"]}u && option=={row["de_option"]} && flags=={row["adds_c"]}u;')
    caller_contract += ["default: return false;", "}}", "}"]
    write_if_changed(output / "formula_result_contracts.metal.h", "\n".join(result_contract+caller_contract)+"\n")
    dispatch += ["namespace mb3d_native {", "inline bool native_external_raw(uint id,FormulaVec3 p,FormulaParams a,thread float& de,thread float& vs,thread FormulaVec3& result) {", "switch(id) {"]
    for row in active:
        prefix = f'formula_{row["id"]}::'
        if row["de_option"] == 20 and row["difs_style"] == 0:
            call = f'{prefix}execute_formula_dIFS<{row["id"]}>(p,a,vs,de)'
        elif row["de_option"] == 21 and row["difs_style"] == 0:
            call = f'{prefix}execute_formula_dIFS_transform<{row["id"]}>(p,a,vs)'
        else:
            call = f'{prefix}execute_formula<{row["id"]}>(p,a,de)'
        guard=" || ".join(f'native_formula_id{i}=={row["id"]}u' for i in range(6))
        dispatch.append(f'case {row["id"]}u: if({guard}) {{ result={call}; return true; }} return false;')
    dispatch += ["default: return false;", "}}", "}"]
    write_if_changed(output / "formula_dispatch.metal.h","\n".join(dispatch)+"\n")
    inventory = {"schema": "mb3d.native-metal-formula-generation.v1", "status": "SOURCE_GENERATED_UNQUALIFIED",
                 "arithmetic": "hardware_float32", "ordinary_registry_count": len(entries),
                 "emitted_uncompiled": sum(row["status"] == "emitted_uncompiled" for row in records),
                 "driver_implemented_unqualified": sum(row["status"] == "driver_implemented_unqualified" for row in records),
                 "blocked": sum(row["status"] == "blocked" for row in records), "device_qualified": 0,
                 "preprocessor": {"executable": compiler, "mode": "preprocess_only", "defines": ["MB3D_SOFTWARE_EXT80=1", "__aarch64__=1"]},
                 "source_sha256": provenance, "traits": {name: 1 << i for i, name in enumerate(TRAITS)}, "formulas": records}
    data = json.dumps(inventory, indent=2, sort_keys=True) + "\n"
    write_if_changed(output / "inventory.json",data)
    if report_path:
        report_path.parent.mkdir(parents=True, exist_ok=True); write_if_changed(report_path,data)
    return inventory


def check_generated(output,compiler):
    with tempfile.TemporaryDirectory(prefix="mb3d-native-metal-check-") as name:
        expected=Path(name);inventory=generate(expected,None,compiler)
        names={p.name for p in expected.iterdir() if p.is_file()}
        # native_formula_effects_header.py writes and checks the state masks.
        others={"formula_state_effects.h","formula_state_effects.sources.tsv"}
        actual={p.name for p in output.iterdir() if p.is_file() and p.name not in others} if output.is_dir() else set()
        mismatch=sorted(names^actual)
        mismatch += sorted(n for n in names&actual if (expected/n).read_bytes()!=(output/n).read_bytes())
        if mismatch:raise ValueError("Stale generated Native Metal sources: "+", ".join(mismatch[:20]))
        return dict(inventory,check="CURRENT",verified_files=len(names))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output-dir", type=Path, default=ROOT / "src/render/native_metal/generated")
    parser.add_argument("--report", type=Path)
    parser.add_argument("--preprocessor", default="/usr/bin/clang")
    parser.add_argument("--check",action="store_true",help="Regenerate in a temporary directory and compare exact bytes without rewriting outputs")
    args = parser.parse_args()
    if args.check and args.report:parser.error("--check does not write --report")
    try:
        inventory=check_generated(args.output_dir.resolve(),args.preprocessor) if args.check else generate(args.output_dir.resolve(), args.report.resolve() if args.report else None,args.preprocessor)
    except (OSError,ValueError) as error:parser.exit(1,str(error)+"\n")
    print(json.dumps({key:inventory[key] for key in ("status","ordinary_registry_count","emitted_uncompiled","driver_implemented_unqualified","blocked","device_qualified","check","verified_files") if key in inventory}))


if __name__ == "__main__":
    main()
