#!/usr/bin/env python3
"""Generate bulk external formula registration .cpp files.

Scans all .inc files in external_formulas/Formulas/, reads matching .m3f files
for option types, computes PVar byte offsets, and generates per-category .cpp files.

Usage:
    python3 scripts/gen_bulk_formula_table.py \
        --formulas-dir external_formulas/Formulas \
        --m3f-dir mb3d/M3Formulas \
        --existing-table src/core/external_formula_table.cpp \
        --output-dir src/core/generated
"""

import argparse
import os
import re
import struct
import sys
from pathlib import Path
from dataclasses import dataclass, field
from typing import Optional

# ============================================================================
# Constants
# ============================================================================

CATEGORIES = ['3D', '3Da', '4D', '4Da', 'Ads', 'dIFS', 'dIFSa']

# Formulas whose external-code contract is "no generic wrapper +C".
# Pascal calls their machine code directly with no engine-side Julia add after
# the call. In generated C++ wrappers the closest equivalent is ADDS_C=1
# (suppress wrapper-side post-call +C), even if the formula body itself does
# not read explicit J1/J2/J3 params.
NO_WRAPPER_C_FORMULAS = {
    "TgladTetra", "TgladTetra2", "TgladDihed", "TgladDihed2",  # Overflow bypasses internal C.
    "MsltoeFoldQuat", "Barnsley4Dc", "Barnsley4Dnc",  # Original native bodies include Julia C.
    "MandelView", "Riemann",  # Original internal C, before Double stores.
    "Mandel4DBiC", "Magnet4D",  # Native bodies include Julia C.
    "_Folding2", "_SphereFolding2",  # Pure transforms with no Julia add.
    "KaliDucks", "Kalisets1",  # C is added inside the original x87 body.
    "Fuzzy",  # Original consumes Rout and adds C internally.
    "IQ-bulb", "IQ-bulb old", "IQ_PineTree",  # Original internal C.
    "ABoxVS_icen1",  # Original internal C and center operations.
    "ABoxMapSFold",  # Internal C after map-driven sphere fold.
    "IQ_NormBulb",  # Internal C addition before x87 stores.
    "ABoxMod2",  # C addition before original x87 Double stores.
    "ABoxVSWavy",  # Original x87 C addition occurs before final Double stores.
    "TetraFolding Pow2",  # C is inside the original body, except negative radius.
    "AmazingBox2",  # Julia addition is inside the original x87 body.
    "ABoxPlatinum",  # Native body adds C before final Double stores.
    "MsltoeSym4", "TalisTpx",  # Original native bodies include Julia C.
    "KaliLinCombSSE2sm",  # Adds Julia C to its persistent complex lanes.
    "_MapTranslate",  # Original map transform never adds Julia C.
    "Aexion1",  # Native quadray iteration includes persistent Julia constants.
    "ATetraVS",  # Native fold includes Julia addition before the final Double stores.
    "General",  # Applies signed/absolute Julia C before final Double stores.
    "ABoxPlatinumB",  # Original applies Julia C before final Double stores.
    "ABoxSphereOffset4d",  # Adds C before sphere offsets; preserves signed Deriv1.
    "BulboxP-2",  # Adds Julia C before restoring the center offsets.
    "Mandalex",  # Adds Julia C before its second linear pull.
    "Mandalay-KIFS",  # Native helper retains scale-plus-C extended arithmetic.
    "Riemann2",  # Original stores xyz without adding Julia C.
    "_hyc3Dinv",  # Pure inversion transform; no Julia C in original code.
    "_RotatePlaneOnIts", "_RotateXYonIts", "_RotateXYonItsP",
    "_RotateRnd", "_TranslateRnd",
    "_Transform",  # Writes C conditionally; never adds C to its orbit output.
    "OctahedronIFS", "OctaFullIFS",  # Original code applies fixed offsets, no Julia C.
    "ABoxVaryScale2",  # Body already adds all three Julia components.
    "ABoxVS_icen2",  # Body selects either Julia or inversion-center addition.
    "_DonutTransform", "_HexaTransform", "_juliax2", "_FoldingCTetra3d", "_IntPowRotateC",
    "_GenCWarp", "_vanderpol", "_vanderpol2",
    # Original x86 code modifies xyz/w without adding Julia C.
    "_BT1_4D_Transform1", "_BT1_4D_clampXYZ", "_sqr_4D", "_asdamTrans",
    "_LogX", "_LogY", "_LogZ", "_SinhX", "_SinY", "_FoldingTetra3d",
    "_sphereXinv", "_sphereYinv", "_sphereZinv",
    "_reciprocalX3b", "_reciprocalY3b", "_reciprocalZ3b",
    # Original x86 point oracle: no implicit Julia addition (scene-points audit).
    '_boxtiling',
    '_BPolygonFromCircle',
    '_BPolygonToCircle',
    '_BPolyhedronFromSphere',
    '_BPolyhedronToSphere',
    '_BRotateFromMag',
    '_BRotateToMag',
    '_BStellahedron_F_Sphere',
    '_BT1_Transform1',
    '_BT2_Transform2',
    '_BT3_Transform3',
    '_BT4_Transform4',
    '_BT5b_Transform5b',
    '_CondItcountR',
    '_dynamic',
    '_dynamicfast',
    '_Fold45double',
    '_Fold45single',
    '_FoldingIcosa',
    '_FoldingInfy',
    '_FoldingTetraKaleid',
    '_FoldingVar',
    '_gnarl2D',
    '_gnarl2D_2',
    '_gnarl2D_2fast',
    '_gnarl2Dfast',
    '_gnarl3D',
    '_gnarl3Dfast',
    '_helispiral',
    '_hopalm1',
    '_hopalm2',
    '_hopalong',
    '_invbisp',
    '_kamtor',
    '_kamtorcut',
    '_LinCombineCxyz',
    '_PartlyJuliaR',
    '_lorenz',
    '_rossler',
    '_cosmartin',
    '_Rotate4d',
    '_SinApprox',
    '_PlusSinApp',
    '_LinCombineXYZ',
    '_MaxClipping',
    '_NeoSqrX',
    '_NeoSqrY',
    '_NeoSqrZ',
    '_ngon',
    '_ngon2',
    '_planeFold',
    '_poincare',
    '_PolyFold-sym',
    '_PolyFolding',
    '_rabinovich-f',
    '_recFold',
    '_reciprocalX3',
    '_reciprocalY3',
    '_reciprocalZ3',
    '_RotateBack',
    '_RotatedAbs',
    '_RotatedFolding',
    '_SinhY',
    '_SinhZ',
    '_sqr_3D',
    '_toruspinv1',
    '_toruspinv2',
    '_toruspinv3',
    '_totorical2',
    '_YplusSinZ2',

    "_QuadrayTransform2",
    "_Abs4d",
    "_conj",
    "_conj2D",
    "_FlipXY",
    "_FlipXZ",
    "_FlipYZ",
    "_FlipXZc",
    "_FlipYZc",
    "_FoldingOct",
    "_HopSqrtX",
    "_HopSqrtY",
    "_HopSqrtZ",
    "_invcylindrical",
    "_invspherical",
    "_invtorical",
    "_invtorical2",
    "_JuliaSets",
    "_reciprocalX",
    "_skew",
    "_tocylindrical",
    "_tospherical",
    "_totorical",
    "_updateC_4d",
    "_YplusSinZ",

    "_BenesiSpheric",
    "_BenesiUnSpheric",
    "ABoxSphereOffset4d",
    "ABoxVaryScale4d",
    "Aex-TorusPow",
    "Benesi2Pow6",
    "Beth1032",
    "CosinePow8",
    "BT2Pine",
    "HalfOct4a",
    "HalfOct4b",
    "Ikenagabulb",
    "_Scaling",
    "TalisTpx",
    "Quintic",
    "MsltoeSym2",
    "Ruckerbulb",
    "ASurfMod1",
    "DodecahedronIFS",
    "DodecahedronIFSex",
    "CrossBiFold",
    "CantorIFS",
    "koch_cube",
    "koch_oct",
    "koch_surf",
    "Menger3",
    "MengerHyper",
    "MengerIFSplus",
    "MengerKoch",
    "MengerKochV2",
    "MengerSmt",
    "HalfOctIFS",
    "IcosahedronIFS",
    "JCube3",
    "OctKoch",
    "Menger4",
    "Menger4ex",
    "MixPinski4",
    "MixPinski4ex",
    "_PartlyJulia",
    "_IFS_tess",
    "_quadist",
    "_Rpow3",
    "_SinePow2",
    "_sphereinv",
    "_SquareR",
    "_SphereFolding1",
    "_e_3Dtess",
    "_MobiusTpx",
    "Octahedron4",
    "PG-bulb",
    "ManowarBulb",
    "ABoxVSShapes",
    "SierpHilbert",
    "SierpinskiFull",
    "Sierpinski4",
    "Sierpinski4ex",
    "Sierpinski3alt",
    "_Atan4D",
    "_Exp4D",
    "_Log4D",
    "_Rpow3_4d",
    "_Scalingplus",
    "_Sin4Dplus",
    "_SphereInvC4d",
    "_Tanh4D",
    "_conj4D",
    "_coordvspow",
    "_julibrot",
    "_setw",
    "Riemann2",
    "_DEComb_ISph_Plane",
}

# IDs 90002-90013 are internal formula dupes — skip them
SKIP_ID_MIN = 90002
SKIP_ID_MAX = 90013

# .m3f option type prefix → type code
M3F_OPTION_MAP = {
    'double': 0,
    'single': 1,
    'integer': 2,
    'doubleangle': 3,
    'singleangle': 4,
    '3doubleangles': 5,
    '3singleangles': 6,
    'boxscale': 7,
    'folding': 8,
    'dsquare': 9,
    'novariable': 10,
    'folding16': 11,
    '6singleangles': 12,
    'drecipro': 13,
    '2doubles': 14,
    'dsqrreci': 15,
    '2singles': 16,
    '4singles': 17,
    '3scalesangles': 18,
    'scalesrot': 19,
    '2integer': 20,
    'sreci2': 21,
    'dreci2': 22,
}

# Option type → byte size consumed in PVar buffer
# (used for PVar offset computation)
OPTION_TYPE_SIZES = {
    0: 8,    # DOUBLE
    1: 4,    # SINGLE
    2: 4,    # INTEGER
    3: 16,   # DOUBLEANGLE (sin + cos, 2 doubles)
    4: 8,    # SINGLEANGLE (sin + cos, 2 floats)
    5: 72,   # 3DOUBLEANGLES (3x3 matrix of doubles)
    6: 36,   # 3SINGLEANGLES (3x3 matrix of floats)
    7: 16,   # BOXSCALE (Scale/MinR^2 + MinR^2, 2 doubles)
    8: 40,   # FOLDING (4 doubles + 8-byte ptr)
    9: 8,    # DSQUARE
    10: 0,   # NOVARIABLE
    11: 32,  # FOLDING16 (4 doubles)
    12: 64,  # 6SINGLEANGLES (4x4 matrix of floats)
    13: 8,   # DRECIPRO
    14: 16,  # 2DOUBLES (1 double stored twice)
    15: 8,   # DSQRRECI
    16: 8,   # 2SINGLES (1 float stored twice)
    17: 16,  # 4SINGLES (1 float stored 4x)
    18: 36,  # 3SCALESANGLES (scale + 3x3 matrix of floats)
    19: 8,   # SCALESROT (2 floats)
    20: 8,   # 2INTEGER (1 int stored twice)
    21: 8,   # SRECI2 (2 floats)
    22: 16,  # DRECI2 (2 doubles)
}


@dataclass
class FormulaEntry:
    """Parsed formula info from .inc + .m3f files."""
    inc_path: str           # relative path from formulas-dir (e.g., "3D/CosinePow2.inc")
    category: str           # e.g., "3D"
    inc_basename: str       # e.g., "CosinePow2"
    formula_id: int
    formula_id_name: str    # e.g., "FORMULA_COSINEPOW2"
    formula_name: str       # human name for registration (e.g., "CosinePow2")
    de_option: int = 0
    de_scale: float = 1.0
    ade_scale: float = 1.0
    si_power: float = 0.0
    rstop: float = 100.0
    option_types: list = field(default_factory=list)   # list of int type codes
    map_value_mask: int = 0                            # expanded value slots that are MAP refs
    param_entries: list = field(default_factory=list)   # list of (offset, value_type)
    const_entries: list = field(default_factory=list)   # list of (value_type, raw_bits)
    uses_difs_template: bool = False     # True if specializes execute_formula_dIFS
    uses_difs_transform: bool = False    # True if specializes execute_formula_dIFS_transform
    adds_c_internally: bool = True       # True if .inc code adds C (J1/J2/J3) itself


# ============================================================================
# .inc file parsing
# ============================================================================

def parse_inc_file(inc_path: str) -> Optional[tuple]:
    """Extract formula ID constant name and value from .inc file.
    Returns (id_name, id_value) or None if not found.
    """
    with open(inc_path, 'r', errors='replace') as f:
        content = f.read()

    # Match: constexpr unsigned int FORMULA_XXX = NNN;
    m = re.search(r'constexpr\s+unsigned\s+int\s+(FORMULA_\w+)\s*=\s*(\d+)\s*;', content)
    if not m:
        return None

    id_name = m.group(1)
    id_value = int(m.group(2))

    # Check template type
    uses_difs = 'execute_formula_dIFS<' in content and 'execute_formula_dIFS_transform<' not in content.split('execute_formula_dIFS<')[0]
    # More precise check
    uses_difs = bool(re.search(r'execute_formula_dIFS<' + id_name + r'>', content))
    uses_difs_transform = bool(re.search(r'execute_formula_dIFS_transform<' + id_name + r'>', content))

    # Detect whether formula adds C (Julia constants J1/J2/J3) internally.
    # Strip comments from code to avoid false positives from comment text.
    code_only = re.sub(r'//[^\n]*', '', content)   # strip // comments
    code_only = re.sub(r'/\*.*?\*/', '', code_only, flags=re.DOTALL)  # strip /* */ comments
    # Check if code reads C from params (e.g., params[N] assigned to Cx/Cy/Cz)
    # or directly references J1/J2/J3 from PIt3D
    # Pattern 1: reads C from params into Cx/Cy/Cz variables
    reads_c_from_params = bool(re.search(r'(Cx|Cy|Cz|C_x|C_y|C_z)\s*=\s*params\s*\[', code_only))
    # Pattern 2: adds params[N] directly inline (+ params[N])
    adds_params_inline = bool(re.search(r'\+\s*params\s*\[\s*\d+\s*\]', code_only))
    # Pattern 3: direct J1/J2/J3 from PIt3D
    uses_j_direct = bool(re.search(r'PIt3D->J[123]', code_only))
    adds_c = reads_c_from_params or adds_params_inline or uses_j_direct

    return id_name, id_value, uses_difs, uses_difs_transform, adds_c


def extract_formula_name_from_inc(inc_path: str) -> str:
    """Extract human-readable formula name from .inc metadata or filename."""
    with open(inc_path, 'r', errors='replace') as f:
        content = f.read()

    # Try to find .name = "XXX" in FormulaInfo
    m = re.search(r'\.name\s*=\s*"([^"]+)"', content)
    if m:
        return m.group(1)

    # Fallback to filename without extension
    return Path(inc_path).stem


# ============================================================================
# .m3f file parsing
# ============================================================================

def find_m3f_file(m3f_dir: str, formula_name: str) -> Optional[str]:
    """Find the .m3f file matching a formula name.
    Tries exact match first, then underscore→space mapping.
    """
    # Try exact match
    path = os.path.join(m3f_dir, formula_name + '.m3f')
    if os.path.exists(path):
        return path

    # Try underscore → space
    alt_name = formula_name.replace('_', ' ')
    path = os.path.join(m3f_dir, alt_name + '.m3f')
    if os.path.exists(path):
        return path

    # Try case-insensitive search
    try:
        m3f_files = os.listdir(m3f_dir)
    except OSError:
        return None
    for f in m3f_files:
        if f.lower() == (formula_name + '.m3f').lower():
            return os.path.join(m3f_dir, f)
        alt = alt_name + '.m3f'
        if f.lower() == alt.lower():
            return os.path.join(m3f_dir, f)

    return None


def parse_m3f_options(m3f_path: str) -> dict:
    """Parse [OPTIONS] and [CONSTANTS] sections from .m3f file.
    Returns dict with keys: de_option, de_scale, ade_scale, si_power, rstop, option_types, const_entries.
    """
    result = {
        'de_option': 0,
        'de_scale': 1.0,
        'ade_scale': 1.0,
        'si_power': 0.0,
        'rstop': 100.0,
        'option_types': [],
        'option_defaults': [],
        'map_value_mask': 0,
        'const_entries': [],
    }

    with open(m3f_path, 'r', errors='replace') as f:
        lines = f.readlines()

    def parse_const_line(raw_line: str):
        line = raw_line.strip()
        if not line or '=' not in line:
            return None
        key, _, val = line.partition('=')
        key = key.strip()
        # DivUtils.StrLastWord keeps a trailing semicolon; invalid floats become zero.
        val = val.rsplit('=', 1)[-1].strip().split(' ')[-1]
        type_name = key
        if type_name.startswith('.'):
            type_name = type_name[1:]
        type_name = type_name.split()[0].lower() if type_name.split() else ''

        try:
            def parse_int(text: str) -> int:
                text = text.strip()
                if text.startswith('$'):
                    return int(text[1:], 16)
                if text.startswith('-$'):
                    return -int(text[2:], 16)
                return int(text, 0)

            def parse_int64_bits(text: str) -> int:
                text = text.strip()
                if text.startswith('$'):
                    return int(text[1:], 16) & 0xFFFFFFFFFFFFFFFF
                if text.startswith('-$'):
                    return ((-int(text[2:], 16)) + (1 << 64)) & 0xFFFFFFFFFFFFFFFF
                value = int(text, 0)
                return value & 0xFFFFFFFFFFFFFFFF

            def pascal_float(text):
                try: return float(text.replace(',', '.'))
                except ValueError: return 0.0

            if type_name == 'double':
                bits = struct.unpack('<Q', struct.pack('<d', pascal_float(val)))[0]
                return (0, bits)
            if type_name == 'single':
                low = struct.unpack('<I', struct.pack('<f', pascal_float(val)))[0]
                return (1, low)
            if type_name == 'integer':
                # CustomFormulas.pas matches the first five characters against
                # DOUBLINTINT64SINGLE. INTEG is absent: Pascal skips this line
                # without advancing the constant slot (_IFS_tess uses it).
                return None
            if type_name == 'int64':
                bits = parse_int64_bits(val)
                return (3, bits)
        except (ValueError, OverflowError) as exc:
            print(f"  WARNING: Failed to parse constant '{line}' in {m3f_path}: {exc}",
                  file=sys.stderr)
            return None

        print(f"  WARNING: Unknown constant type '{type_name}' in {m3f_path}",
              file=sys.stderr)
        return None

    in_options = False
    in_constants = False
    value_index = 0
    for line in lines:
        line = line.strip()
        if line == '[OPTIONS]':
            in_options = True
            in_constants = False
            continue
        if line == '[CONSTANTS]':
            in_options = False
            in_constants = True
            continue
        if line.startswith('[') and line.endswith(']') and line not in ('[OPTIONS]', '[CONSTANTS]'):
            in_options = False
            in_constants = False
            continue

        if in_constants:
            parsed_const = parse_const_line(line)
            if parsed_const is not None:
                result['const_entries'].append(parsed_const)
            continue

        if not in_options:
            continue

        # Parse key=value
        if '=' not in line:
            # Lines like .SSE2 or .Version = N (no option type)
            if line.startswith('.SSE2') or line.startswith('.FPU') or line.startswith('.x87'):
                continue
            if line.startswith('.Version'):
                continue
            if line.startswith('.ADEscale'):
                continue
            continue

        key, _, val = line.partition('=')
        key = key.strip()
        val = val.strip()

        if key.lower() == '.deoption':
            result['de_option'] = int(val)
        elif key.lower() == '.descale':
            result['de_scale'] = float(val)
        elif key.lower() in ('.sipower', '.sipow'):
            result['si_power'] = float(val)
        elif key.lower() == '.rstop':
            result['rstop'] = float(val)
        elif key.lower() == '.adescale':
            result['ade_scale'] = float(val)
        elif key.startswith('.Version'):
            pass
        elif key.startswith('.'):
            # This is an option type line like ".Double Scale = 2"
            # The format is: .TypeName ParamName = DefaultValue
            # We need to extract the type name
            # Remove leading dot
            opt_str = key[1:]
            opt_parts = opt_str.split()
            opt_type_name = opt_parts[0].lower() if opt_parts else ''
            opt_label = opt_str[len(opt_parts[0]):].strip() if opt_parts else ''

            if opt_type_name in M3F_OPTION_MAP:
                opt_type = M3F_OPTION_MAP[opt_type_name]
                result['option_types'].append(opt_type)
                if opt_type == 2 and 'MAP' in opt_label.upper() and value_index < 16:
                    result['map_value_mask'] |= (1 << value_index)
                # Scene values are angles, not the expanded matrix storage.
                count = {5: 3, 6: 3, 12: 6, 18: 4, 19: 2}.get(opt_type, 1)
                words = val.rstrip(';').split()
                default = float(words[-1])
                values = [default] * count
                if opt_type == 18:
                    values = [default, 0.0, 0.0, 0.0]
                elif opt_type == 19:
                    values = [float(words[0]), default]
                result['option_defaults'].extend(values)
                value_index += count
            elif opt_type_name in ('sse2', 'fpu', 'x87'):
                pass  # Not an option type
            else:
                print(f"  WARNING: Unknown option type '{opt_type_name}' in {m3f_path}",
                      file=sys.stderr)

    return result


# ============================================================================
# PVar offset computation
# ============================================================================

def compute_pvar_offsets(option_types: list) -> list:
    """Compute PVar byte offsets for each param entry.
    Returns list of (byte_offset, value_type) tuples.
    byte_offset is negative (e.g., -0x10).
    value_type: 0=double(8B), 1=float(4B), 2=int32(4B).
    """
    entries = []
    offset = -8  # start after 0.5 sentinel at PVar[-8]

    for opt_type in option_types:
        if opt_type == 0:  # DOUBLE
            offset -= 8
            entries.append((offset, 0))
        elif opt_type == 1:  # SINGLE
            offset -= 4
            entries.append((offset, 1))
        elif opt_type == 2:  # INTEGER
            offset -= 4
            entries.append((offset, 2))
        elif opt_type == 3:  # DOUBLEANGLE
            offset -= 8
            entries.append((offset, 0))  # sin
            offset -= 8
            entries.append((offset, 0))  # cos
        elif opt_type == 4:  # SINGLEANGLE
            offset -= 4
            entries.append((offset, 1))
            offset -= 4
            entries.append((offset, 1))
        elif opt_type == 5:  # 3DOUBLEANGLES (9 doubles = 3x3 matrix)
            for _ in range(9):
                offset -= 8
                entries.append((offset, 0))
        elif opt_type == 6:  # 3SINGLEANGLES (9 floats = 3x3 matrix)
            for _ in range(9):
                offset -= 4
                entries.append((offset, 1))
        elif opt_type == 7:  # BOXSCALE (Scale/MinR^2, MinR^2 — 2 doubles)
            offset -= 8
            entries.append((offset, 0))
            offset -= 8
            entries.append((offset, 0))
        elif opt_type == 8:  # FOLDING (4 doubles + 8-byte function ptr)
            for _ in range(4):
                offset -= 8
                entries.append((offset, 0))
            offset -= 8  # skip function pointer
        elif opt_type == 9:  # DSQUARE
            offset -= 8
            entries.append((offset, 0))
        elif opt_type == 10:  # NOVARIABLE
            pass  # no storage
        elif opt_type == 11:  # FOLDING16 (4 doubles)
            for _ in range(4):
                offset -= 8
                entries.append((offset, 0))
        elif opt_type == 12:  # 6SINGLEANGLES (16 floats = 4x4 matrix)
            for _ in range(16):
                offset -= 4
                entries.append((offset, 1))
        elif opt_type == 13:  # DRECIPRO
            offset -= 8
            entries.append((offset, 0))
        elif opt_type == 14:  # 2DOUBLES (1 double stored twice)
            # Pascal FillCustomVBufWithVars writes the first copy at the higher
            # address and the second copy at the lower address. The external
            # formulas and asm comments consistently read the lower duplicate
            # slot ([edi-0x20], [edi-0x30], ...), not the first write.
            offset -= 16
            entries.append((offset, 0))
        elif opt_type == 15:  # DSQRRECI
            offset -= 8
            entries.append((offset, 0))
        elif opt_type == 16:  # 2SINGLES (1 float stored twice)
            offset -= 4
            entries.append((offset, 1))
            offset -= 4  # skip duplicate
        elif opt_type == 17:  # 4SINGLES (1 float stored 4x)
            offset -= 4
            entries.append((offset, 1))
            offset -= 12  # skip 3 duplicates
        elif opt_type == 18:  # 3SCALESANGLES (scale + 3x3 matrix of floats = 9 floats)
            for _ in range(9):
                offset -= 4
                entries.append((offset, 1))
        elif opt_type == 19:  # SCALESROT (2 floats)
            offset -= 4
            entries.append((offset, 1))
            offset -= 4
            entries.append((offset, 1))
        elif opt_type == 20:  # 2INTEGER (1 int stored twice)
            offset -= 4
            entries.append((offset, 2))
            offset -= 4  # skip duplicate
        elif opt_type == 21:  # SRECI2 (2 floats)
            offset -= 4
            entries.append((offset, 1))
            offset -= 4
            entries.append((offset, 1))
        elif opt_type == 22:  # DRECI2 (2 doubles)
            offset -= 8
            entries.append((offset, 0))
            offset -= 8
            entries.append((offset, 0))
        else:
            print(f"  WARNING: Unknown option type code {opt_type}", file=sys.stderr)

    return entries


def append_constant_offsets(entries: list, const_entries: list) -> list:
    """Append positive +N*8 constant slots after negative option entries.

    Pascal stores .m3f [CONSTANTS] starting at PVar[+0], advancing by 8 bytes per slot
    regardless of actual type width. Formulas that were ported to params[] expect
    option-derived params first, then constant slots in ascending positive-offset order.
    """
    result = list(entries)
    for idx, (value_type, _raw_bits) in enumerate(const_entries):
        result.append((idx * 8, value_type))
    return result


# ============================================================================
# Extract already-registered formula IDs from existing table
# ============================================================================

def get_existing_ids(table_path: str) -> set:
    """Parse external_formula_table.cpp to find already-registered formula IDs."""
    ids = set()
    if not os.path.exists(table_path):
        return ids

    with open(table_path, 'r') as f:
        content = f.read()

    # Match DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_XXX, ...)
    for m in re.finditer(r'DEFINE_EXTERNAL_FORMULA_WRAPPER\s*\(\s*(FORMULA_\w+)', content):
        # Find the constexpr ID value from the included .inc files
        pass

    # Better: match RegisterExternalFormula calls and extract the ID constant name,
    # then find the ID value from the constexpr lines in .inc files
    # But simpler: just extract the formula names from the #include lines
    included_incs = set()
    for m in re.finditer(r'#include\s+"(\w+/\w+\.inc)"', content):
        included_incs.add(m.group(1))

    # Also match includes with subdirectories and special chars
    for m in re.finditer(r'#include\s+"([^"]+\.inc)"', content):
        included_incs.add(m.group(1))

    return included_incs


def get_existing_formula_id_names(table_path: str) -> set:
    """Get set of FORMULA_XXX constant names from DEFINE_EXTERNAL_FORMULA_WRAPPER calls."""
    names = set()
    if not os.path.exists(table_path):
        return names
    with open(table_path, 'r') as f:
        for line in f:
            m = re.search(r'DEFINE_EXTERNAL_FORMULA_WRAPPER\s*\(\s*(FORMULA_\w+)', line)
            if m:
                names.add(m.group(1))
    return names


# ============================================================================
# C++ code generation
# ============================================================================

def sanitize_map_name(formula_id_name: str) -> str:
    """Convert FORMULA_XXX to a map variable name like map_XXX."""
    # Remove FORMULA_ prefix
    short = formula_id_name.replace('FORMULA_', '')
    return f'map_{short}'


def format_offset(offset: int) -> str:
    """Format a byte offset as hex string."""
    if offset < 0:
        return f'-0x{abs(offset):02X}'
    return f'0x{offset:02X}'


def format_param_map(entry: FormulaEntry) -> str:
    """Generate FormulaParamMap definition."""
    map_name = sanitize_map_name(entry.formula_id_name)
    count = len(entry.param_entries)

    if count == 0:
        return f'static const FormulaParamMap {map_name} = {{ 0, {{}} }};'

    entries_str = ', '.join(
        f'{{ {format_offset(off)}, {vtype} }}'
        for off, vtype in entry.param_entries
    )

    # Wrap long lines
    if count > 4:
        lines = []
        for i in range(0, count, 4):
            chunk = entry.param_entries[i:i+4]
            line = ', '.join(f'{{ {format_offset(off)}, {vtype} }}' for off, vtype in chunk)
            lines.append(f'        {line}')
        entries_str = ',\n'.join(lines)
        return (f'static const FormulaParamMap {map_name} = {{\n'
                f'    {count}, {{\n{entries_str}\n    }}\n}};')
    else:
        return f'static const FormulaParamMap {map_name} = {{ {count}, {{ {entries_str} }} }};'


def get_wrapper_de_option(entry: FormulaEntry) -> int:
    """Determine what DEoption to pass to DEFINE_EXTERNAL_FORMULA_WRAPPER.

    Always pass through the actual DEoption from the .m3f metadata.
    The wrapper macro now handles all combinations via DIFS_STYLE parameter.
    """
    return entry.de_option


def get_difs_style(entry: FormulaEntry) -> int:
    """Determine DIFS_STYLE for the wrapper macro.

    0 = formula specializes execute_formula_dIFS<> or execute_formula_dIFS_transform<>
    1 = formula uses standard execute_formula<> but is a dIFS formula (DEoption=20/21)
        The wrapper will call execute_formula<> and route de output appropriately.
    """
    if entry.de_option == 20 and not entry.uses_difs_template:
        return 1
    if entry.de_option == 21 and not entry.uses_difs_transform:
        return 1
    return 0


def get_adds_c_flag(entry: FormulaEntry) -> int:
    """Determine ADDS_C for wrapper generation.

    Pascal external dIFS formulas (DEoption 20/21) do not receive the generic
    post-call Julia +C application from the engine wrapper. Their transform
    semantics are entirely encoded inside the formula body and its own params
    (for example X_add/Y_add/Z_add, VaryScale, Rout).

    The old hand-registered dIFS formulas in src/core/external_formula_table.cpp
    also used the default wrapper path, which implies ADDS_C=1.

    So for generated dIFS/dIFSa formulas we force ADDS_C=1 regardless of the
    generic "reads params inline" heuristic that is otherwise used for 3D/4D
    external formulas.
    """
    if entry.de_option in (20, 21):
        return 1
    normalized_name = entry.formula_name.strip()
    # In DEFINE_EXTERNAL_FORMULA_WRAPPER_EX:
    #   ADDS_C=0 => wrapper adds C after the formula call
    #   ADDS_C=1 => formula already handles C or must not receive wrapper-side +C
    # Formulas listed here require wrapper-side +C suppression.
    if normalized_name in NO_WRAPPER_C_FORMULAS or entry.formula_id_name == "FORMULA__SPHEREINV":
        return 1
    return 1 if entry.adds_c_internally else 0


def format_option_types_array(option_types: list) -> str:
    """Format option types as C++ array initializer."""
    if not option_types:
        return '{}'
    return '{' + ', '.join(str(t) for t in option_types) + '}'


def format_const_array(entry: FormulaEntry) -> tuple[str, str]:
    """Generate static FormulaConstEntry array if formula has [CONSTANTS]."""
    if not entry.const_entries:
        return '', 'nullptr'

    array_name = f'consts_{entry.formula_id_name.replace("FORMULA_", "")}'
    lines = [f'static const FormulaConstEntry {array_name}[] = {{']
    for value_type, raw_bits in entry.const_entries:
        lines.append(f'    {{ {value_type}, UINT64_C(0x{raw_bits:016X}) }},')
    lines.append('};')
    return '\n'.join(lines), array_name


def generate_category_cpp(category: str, entries: list, formulas_dir: str) -> str:
    """Generate a complete .cpp file for one category."""
    lines = []
    lines.append(f'// external_formula_table_{category}.cpp — Auto-generated bulk formula registrations')
    lines.append(f'// Category: {category} ({len(entries)} formulas)')
    lines.append(f'//')
    lines.append(f'// DO NOT EDIT — regenerate with: python3 scripts/gen_bulk_formula_table.py')
    lines.append('')
    lines.append('#include "core/external_formulas.h"')
    lines.append('#include "core/lightmap.h"')
    lines.append('#include <cmath>')
    lines.append('#include <cstring>')
    lines.append('#include <cstdint>')
    lines.append('#include <cfloat>')
    lines.append('')
    # NOTE: 'constant' must be empty (not 'const') so that .inc files that use
    # 'constant double* params' in their execute_formula<> specializations match
    # the template declaration in external_formulas.h which uses 'double* params'.
    lines.append('#define constant')
    lines.append('')
    lines.append('using std::abs;')
    lines.append('using std::sqrt;')
    lines.append('using std::sin;')
    lines.append('using std::cos;')
    lines.append('using std::atan2;')
    lines.append('using std::asin;')
    lines.append('using std::acos;')
    lines.append('using std::log;')
    lines.append('using std::exp;')
    lines.append('using std::pow;')
    lines.append('using std::sinh;')
    lines.append('using std::cosh;')
    lines.append('using std::tanh;')
    lines.append('using std::max;')
    lines.append('using std::min;')
    lines.append('using std::rint;')
    lines.append('')
    lines.append('namespace mb3d {')
    lines.append('')
    lines.append('// FormulaInfo struct (used by .inc metadata blocks)')
    lines.append('struct FormulaInfo {')
    lines.append('    unsigned int id;')
    lines.append('    const char* name;')
    lines.append('    int param_count;')
    lines.append('    int complexity;')
    lines.append('    bool uses_transcendental;')
    lines.append('};')
    lines.append('')
    # Compatibility types and operators needed by .inc files
    lines.append('// ============================================================================')
    lines.append('// Compatibility types and operators for .inc formula files')
    lines.append('// ============================================================================')
    lines.append('')
    lines.append('// cpu_double2 — packed double pair (GenIntPower, KaliLinCombSSE2sm, etc.)')
    lines.append('struct cpu_double2 {')
    lines.append('    double x, y;')
    lines.append('    cpu_double2() : x(0.0), y(0.0) {}')
    lines.append('    cpu_double2(double v) : x(v), y(v) {}')
    lines.append('    cpu_double2(double x_, double y_) : x(x_), y(y_) {}')
    lines.append('    cpu_double2 operator+(const cpu_double2& b) const { return {x + b.x, y + b.y}; }')
    lines.append('    cpu_double2 operator-(const cpu_double2& b) const { return {x - b.x, y - b.y}; }')
    lines.append('    cpu_double2 operator*(const cpu_double2& b) const { return {x * b.x, y * b.y}; }')
    lines.append('    cpu_double2 operator*(double s) const { return {x * s, y * s}; }')
    lines.append('    cpu_double2 operator/(double s) const { return {x / s, y / s}; }')
    lines.append('    cpu_double2& operator+=(const cpu_double2& b) { x += b.x; y += b.y; return *this; }')
    lines.append('    cpu_double2& operator-=(const cpu_double2& b) { x -= b.x; y -= b.y; return *this; }')
    lines.append('    cpu_double2& operator*=(double s) { x *= s; y *= s; return *this; }')
    lines.append('    cpu_double2& operator/=(double s) { x /= s; y /= s; return *this; }')
    lines.append('};')
    lines.append('inline cpu_double2 operator*(double s, const cpu_double2& v) { return {s * v.x, s * v.y}; }')
    lines.append('')
    lines.append('// cpu_double3 operators (the struct is in external_formulas.h but lacks operators)')
    lines.append('inline cpu_double3 operator+(const cpu_double3& a, const cpu_double3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }')
    lines.append('inline cpu_double3 operator-(const cpu_double3& a, const cpu_double3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }')
    lines.append('inline cpu_double3 operator*(const cpu_double3& a, double s) { return {a.x * s, a.y * s, a.z * s}; }')
    lines.append('inline cpu_double3 operator*(double s, const cpu_double3& a) { return {s * a.x, s * a.y, s * a.z}; }')
    lines.append('inline cpu_double3 operator/(const cpu_double3& a, double s) { return {a.x / s, a.y / s, a.z / s}; }')
    lines.append('inline cpu_double3 operator-(const cpu_double3& a) { return {-a.x, -a.y, -a.z}; }')
    lines.append('')
    lines.append('// length functions')
    lines.append('inline double length(cpu_double2 v) { return std::sqrt(v.x * v.x + v.y * v.y); }')
    lines.append('inline double length(cpu_double3 v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }')
    lines.append('')
    lines.append('// rsqrt — reciprocal square root')
    lines.append('inline double rsqrt(double v) { return 1.0 / std::sqrt(v); }')
    lines.append('')
    lines.append('// M_PI_2_F — pi/2 as double constant (used by General.inc, _SinApprox.inc)')
    lines.append('#ifndef M_PI_2_F')
    lines.append('#define M_PI_2_F 1.5707963267948966')
    lines.append('#endif')
    lines.append('')
    lines.append('// sincos_of_power_theta — compute sin/cos of (power * atan2(num, den))')
    lines.append('// Used by General.inc. Full angle-doubling approach omitted for bulk gen.')
    lines.append('inline void sincos_of_power_theta(double num, double den, double power,')
    lines.append('                                   double& sin_out, double& cos_out) {')
    lines.append('    double angle = std::atan2(num, den) * power;')
    lines.append('    sin_out = std::sin(angle);')
    lines.append('    cos_out = std::cos(angle);')
    lines.append('}')
    lines.append('')
    lines.append('// cpu_double4 — packed 4 doubles (used by 4Da Sierpinski4ex, MixPinski4ex, etc.)')
    lines.append('struct cpu_double4 {')
    lines.append('    double x, y, z, w;')
    lines.append('    cpu_double4() : x(0), y(0), z(0), w(0) {}')
    lines.append('    cpu_double4(double v) : x(v), y(v), z(v), w(v) {}')
    lines.append('    cpu_double4(double x_, double y_, double z_, double w_) : x(x_), y(y_), z(z_), w(w_) {}')
    lines.append('    cpu_double4 operator+(const cpu_double4& b) const { return {x+b.x, y+b.y, z+b.z, w+b.w}; }')
    lines.append('    cpu_double4 operator-(const cpu_double4& b) const { return {x-b.x, y-b.y, z-b.z, w-b.w}; }')
    lines.append('    cpu_double4 operator*(double s) const { return {x*s, y*s, z*s, w*s}; }')
    lines.append('    cpu_double4 operator/(double s) const { return {x/s, y/s, z/s, w/s}; }')
    lines.append('    cpu_double4 operator-() const { return {-x, -y, -z, -w}; }')
    lines.append('};')
    lines.append('inline cpu_double4 operator*(double s, const cpu_double4& v) { return {s*v.x, s*v.y, s*v.z, s*v.w}; }')
    lines.append('inline double dot(cpu_double4 a, cpu_double4 b) { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }')
    lines.append('inline double length(cpu_double4 v) { return std::sqrt(dot(v, v)); }')
    lines.append('')
    lines.append('// RotMatrix4 * cpu_double4 — 4x4 matrix-vector multiply')
    lines.append('inline cpu_double4 operator*(const RotMatrix4& M, const cpu_double4& v) {')
    lines.append('    return cpu_double4(')
    lines.append('        M.m[0][0]*v.x + M.m[0][1]*v.y + M.m[0][2]*v.z + M.m[0][3]*v.w,')
    lines.append('        M.m[1][0]*v.x + M.m[1][1]*v.y + M.m[1][2]*v.z + M.m[1][3]*v.w,')
    lines.append('        M.m[2][0]*v.x + M.m[2][1]*v.y + M.m[2][2]*v.z + M.m[2][3]*v.w,')
    lines.append('        M.m[3][0]*v.x + M.m[3][1]*v.y + M.m[3][2]*v.z + M.m[3][3]*v.w);')
    lines.append('}')
    lines.append('')
    lines.append('// cpu_bitcast_to_int — reinterpret float bits as int')
    lines.append('inline int cpu_bitcast_to_int(double v) {')
    lines.append('    float f = static_cast<float>(v);')
    lines.append('    int32_t i; std::memcpy(&i, &f, 4); return i;')
    lines.append('}')
    lines.append('')
    lines.append('// CPUVec3 — alias for height map sampling results')
    lines.append('using CPUVec3 = cpu_double3;')
    lines.append('')
    lines.append('// Height map sampling helpers backed by the shared Pascal-style lightmap runtime.')
    lines.append('inline CPUVec3 cpu_map_direct_sample(double u, double v, int map_nr) {')
    lines.append('    TVec3D in = {u, v, 0.0};')
    lines.append('    TVec3D out = {};')
    lines.append('    GetMapPixelDirectXYspline(out, &in, map_nr);')
    lines.append('    return CPUVec3(out[0], out[1], out[2]);')
    lines.append('}')
    lines.append('inline CPUVec3 cpu_map_sphere_sample(double x, double y, double z, int map_nr) {')
    lines.append('    TVec3D in = {x, y, z};')
    lines.append('    TVec3D out = {};')
    lines.append('    GetMapPixelSphereSpline(out, &in, map_nr);')
    lines.append('    return CPUVec3(out[0], out[1], out[2]);')
    lines.append('}')
    lines.append('')
    lines.append('// RotMatrix3Ext — extends RotMatrix3 with .col0/.col1/.col2 column access')
    lines.append('// Many .inc files use RM.col0.x, RM.col1.y, etc. (column-major access).')
    lines.append('// RotMatrix3.m is row-major: m[row][col]. Column j = {m[0][j], m[1][j], m[2][j]}.')
    lines.append('struct RMCol { double x, y, z; };')
    lines.append('struct RotMatrix3Ext : RotMatrix3 {')
    lines.append('    RMCol col0, col1, col2;')
    lines.append('    RotMatrix3Ext() : RotMatrix3{}, col0{}, col1{}, col2{} {}')
    lines.append('    RotMatrix3Ext(const RotMatrix3& r) : RotMatrix3(r),')
    lines.append('        col0{r.m[0][0], r.m[1][0], r.m[2][0]},')
    lines.append('        col1{r.m[0][1], r.m[1][1], r.m[2][1]},')
    lines.append('        col2{r.m[0][2], r.m[1][2], r.m[2][2]} {}')
    lines.append('};')
    lines.append('// Shadow buildRotMatrix3/rotMatrix3FromFloats to return RotMatrix3Ext')
    lines.append('inline RotMatrix3Ext buildRotMatrix3Ext(double ax, double ay, double az) {')
    lines.append('    return RotMatrix3Ext(buildRotMatrix3(ax, ay, az));')
    lines.append('}')
    lines.append('#define buildRotMatrix3 buildRotMatrix3Ext')
    lines.append('inline RotMatrix3Ext rotMatrix3FromFloatsExt(const double* p, int i) {')
    lines.append('    RotMatrix3 r = {}; r.m[0][0]=p[i]; r.m[0][1]=p[i+1]; r.m[0][2]=p[i+2];')
    lines.append('    r.m[1][0]=p[i+3]; r.m[1][1]=p[i+4]; r.m[1][2]=p[i+5];')
    lines.append('    r.m[2][0]=p[i+6]; r.m[2][1]=p[i+7]; r.m[2][2]=p[i+8];')
    lines.append('    return RotMatrix3Ext(r);')
    lines.append('}')
    lines.append('#define rotMatrix3FromFloats rotMatrix3FromFloatsExt')
    lines.append('// Redefine RotMatrix3 so .inc files that declare "RotMatrix3 Rot" get the Ext version')
    lines.append('#define RotMatrix3 RotMatrix3Ext')
    lines.append('')
    lines.append('// Undefine system macros that conflict with .inc variable names')
    lines.append('#ifdef OVERFLOW')
    lines.append('#undef OVERFLOW')
    lines.append('#endif')
    lines.append('')
    lines.append('// sign() function (returns -1, 0, or 1)')
    lines.append('inline double sign(double v) { return (v > 0.0) ? 1.0 : ((v < 0.0) ? -1.0 : 0.0); }')
    lines.append('')
    lines.append('// Integer clamp to avoid ambiguity with std::clamp in C++17')
    lines.append('inline int clamp(int v, int lo, int hi) { return std::max(lo, std::min(v, hi)); }')
    lines.append('')
    lines.append('// uint typedef (used by _RotateRnd.inc)')
    lines.append('typedef unsigned int uint;')
    lines.append('')
    lines.append('// dot product for RMCol (column of RotMatrix3Ext)')
    lines.append('inline double dot(RMCol a, cpu_double3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }')
    lines.append('inline double dot(cpu_double3 a, cpu_double3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }')
    lines.append('')
    lines.append('// sincos shim — returns sin(angle) and writes cos(angle) to second arg')
    lines.append('// Matches .inc usage: sin_val = sincos(angle, cos_val)')
    lines.append('inline double sincos(double angle, double& cos_out) {')
    lines.append('    cos_out = std::cos(angle);')
    lines.append('    return std::sin(angle);')
    lines.append('}')
    lines.append('')

    # buildRotMatrix4d alias (needed by Sierpinski4, _Rotate4d, etc.)
    lines.append('// Alias for formulas that call buildRotMatrix4d instead of buildRotMatrix4')
    lines.append('inline RotMatrix4 buildRotMatrix4d(double a1, double a2, double a3,')
    lines.append('                                    double a4, double a5, double a6) {')
    lines.append('    return buildRotMatrix4(a1, a2, a3, a4, a5, a6);')
    lines.append('}')
    lines.append('')

    # Include all .inc files for this category
    lines.append('// ============================================================================')
    lines.append(f'// Include {category} formula .inc files')
    lines.append('// ============================================================================')
    lines.append('')
    for entry in entries:
        lines.append(f'#include "{entry.inc_path}"')
    lines.append('')

    # Generate FormulaParamMap definitions
    lines.append('// ============================================================================')
    lines.append(f'// PVar offset maps ({category})')
    lines.append('// ============================================================================')
    lines.append('')
    for entry in entries:
        lines.append(format_param_map(entry))
        const_def, _ = format_const_array(entry)
        if const_def:
            lines.append(const_def)
        wrapper_de = get_wrapper_de_option(entry)
        map_name = sanitize_map_name(entry.formula_id_name)
        adds_c = get_adds_c_flag(entry)
        difs_style = get_difs_style(entry)
        # Plain wrapper macro defaults to ADDS_C=1 (formula adds C internally).
        # Use _EX macro when ADDS_C=0 (wrapper must add C) or non-default DIFS_STYLE.
        if adds_c == 1 and difs_style == 0:
            lines.append(f'DEFINE_EXTERNAL_FORMULA_WRAPPER({entry.formula_id_name}, {wrapper_de}, {map_name})')
        else:
            lines.append(f'DEFINE_EXTERNAL_FORMULA_WRAPPER_EX({entry.formula_id_name}, {wrapper_de}, {map_name}, {adds_c}, {difs_style})')
        lines.append('')

    # Generate registration function
    func_name = f'RegisterExternalFormulas_{category}'
    lines.append('// ============================================================================')
    lines.append(f'// Registration: {func_name}()')
    lines.append('// ============================================================================')
    lines.append('')
    lines.append(f'void {func_name}() {{')

    for entry in entries:
        map_name = sanitize_map_name(entry.formula_id_name)
        opts_str = format_option_types_array(entry.option_types)
        param_count = len(entry.param_entries)
        opt_count = len(entry.option_types)
        const_count = len(entry.const_entries)
        _, const_array_name = format_const_array(entry)

        lines.append(f'    RegisterExternalFormula({{')
        lines.append(f'        {entry.formula_id_name}, "{entry.formula_name}", '
                     f'{entry.de_option}, {param_count},')
        lines.append(f'        {entry.de_scale}, {entry.ade_scale}, {entry.si_power}, {entry.rstop},')
        lines.append(f'        ExternWrap_{entry.formula_id_name}, &{map_name},')
        lines.append(f'        {opt_count}, {opts_str},')
        lines.append(f'        0x{entry.map_value_mask:04X},')
        lines.append(f'        {const_count}, {const_array_name}')
        lines.append(f'    }});')
        lines.append('')

    lines.append('}')
    lines.append('')
    lines.append('} // namespace mb3d')
    lines.append('')
    lines.append('#undef constant')
    lines.append('#undef buildRotMatrix3')
    lines.append('#undef rotMatrix3FromFloats')
    lines.append('#undef RotMatrix3')
    lines.append('#undef M_PI_2_F')
    lines.append('')

    return '\n'.join(lines)


# ============================================================================
# Main
# ============================================================================

def generate_defaults_header(m3f_dir, output_dir):
    """Freeze the Pascal option defaults, including compound value expansion."""
    lines = ['// Generated by scripts/gen_bulk_formula_table.py; do not edit.',
             '#pragma once', 'namespace mb3d {',
             'struct ExternalOptionDefaults { const char* name; int count; double values[16]; };',
             'inline constexpr ExternalOptionDefaults kExternalOptionDefaults[] = {']
    for path in sorted(Path(m3f_dir).glob('*.m3f'), key=lambda p: p.name.lower()):
        values = parse_m3f_options(str(path))['option_defaults'][:16]
        name = path.stem.upper().replace('\\', '\\\\').replace('"', '\\"')
        numbers = ', '.join(format(v, '.17g') for v in values)
        lines.append('    {"%s", %d, {%s}},' % (name, len(values), numbers))
    lines += ['};', '} // namespace mb3d', '']
    Path(output_dir, 'external_formula_defaults.h').write_text('\n'.join(lines), encoding='utf-8')


def main():
    parser = argparse.ArgumentParser(description='Generate bulk external formula registration code')
    parser.add_argument('--formulas-dir', required=True, help='Path to external_formulas/Formulas/')
    parser.add_argument('--m3f-dir', required=True, help='Path to mb3d/M3Formulas/')
    parser.add_argument('--existing-table', required=True, help='Path to existing external_formula_table.cpp')
    parser.add_argument('--output-dir', required=True, help='Output directory for generated .cpp files')
    args = parser.parse_args()

    formulas_dir = args.formulas_dir
    m3f_dir = args.m3f_dir
    existing_table = args.existing_table
    output_dir = args.output_dir

    os.makedirs(output_dir, exist_ok=True)
    generate_defaults_header(m3f_dir, output_dir)

    # Get already-registered includes from existing table
    existing_includes = get_existing_ids(existing_table)
    existing_id_names = get_existing_formula_id_names(existing_table)
    print(f"Already registered: {len(existing_id_names)} formulas")
    for name in sorted(existing_id_names):
        print(f"  {name}")

    # Scan all .inc files
    all_entries = {cat: [] for cat in CATEGORIES}
    skip_count = 0
    existing_count = 0
    no_m3f_count = 0
    total_count = 0

    for category in CATEGORIES:
        cat_dir = os.path.join(formulas_dir, category)
        if not os.path.isdir(cat_dir):
            print(f"WARNING: Category directory not found: {cat_dir}", file=sys.stderr)
            continue

        inc_files = sorted(f for f in os.listdir(cat_dir) if f.endswith('.inc'))

        for inc_file in inc_files:
            inc_path_full = os.path.join(cat_dir, inc_file)
            inc_path_rel = f'{category}/{inc_file}'
            inc_basename = inc_file[:-4]  # remove .inc

            # Parse .inc file for formula ID
            parsed = parse_inc_file(inc_path_full)
            if parsed is None:
                print(f"  SKIP (no ID): {inc_path_rel}", file=sys.stderr)
                continue

            id_name, id_value, uses_difs, uses_difs_transform, adds_c = parsed
            total_count += 1

            # Skip internal dupes (IDs 90002-90013)
            if SKIP_ID_MIN <= id_value <= SKIP_ID_MAX:
                skip_count += 1
                print(f"  SKIP (internal dupe {id_value}): {inc_path_rel}")
                continue

            # Skip already registered
            if id_name in existing_id_names:
                existing_count += 1
                print(f"  SKIP (existing): {inc_path_rel} ({id_name})")
                continue

            # Also skip if include path matches
            if inc_path_rel in existing_includes:
                existing_count += 1
                print(f"  SKIP (existing include): {inc_path_rel}")
                continue

            # Extract human name
            formula_name = extract_formula_name_from_inc(inc_path_full)

            # Find matching .m3f file
            m3f_path = find_m3f_file(m3f_dir, inc_basename)
            if m3f_path is None:
                # Try the formula_name from the .inc file
                m3f_path = find_m3f_file(m3f_dir, formula_name)

            if m3f_path is None:
                no_m3f_count += 1
                print(f"  WARNING: No .m3f found for {inc_path_rel} (name={formula_name})",
                      file=sys.stderr)
                # Create entry with empty options (no PVar params)
                entry = FormulaEntry(
                    inc_path=inc_path_rel,
                    category=category,
                    inc_basename=inc_basename,
                    formula_id=id_value,
                    formula_id_name=id_name,
                    formula_name=formula_name,
                    uses_difs_template=uses_difs,
                    uses_difs_transform=uses_difs_transform,
                    adds_c_internally=adds_c,
                )
                all_entries[category].append(entry)
                continue

            # Parse .m3f options
            m3f_opts = parse_m3f_options(m3f_path)

            # Compute PVar offsets: negative option storage first, then +N*8 constants
            option_param_entries = compute_pvar_offsets(m3f_opts['option_types'])
            param_entries = append_constant_offsets(option_param_entries, m3f_opts['const_entries'])
            if formula_name.strip() in ('KaliLinCombSSE2sm', 'MengerKochV2', 'Riemann'):
                param_entries.append((-8, 0))  # Original output normalization coefficient.
            if formula_name.strip() == 'ABoxVS_icen2':
                # The original nonzero C-swap branch loads a qword overlapping
                # the integer selector at PVar-0x54, rather than Julia J3.
                param_entries.append((-0x58, 0))

            entry = FormulaEntry(
                inc_path=inc_path_rel,
                category=category,
                inc_basename=inc_basename,
                formula_id=id_value,
                formula_id_name=id_name,
                formula_name=formula_name,
                de_option=m3f_opts['de_option'],
                de_scale=m3f_opts['de_scale'],
                ade_scale=m3f_opts['ade_scale'],
                si_power=m3f_opts['si_power'],
                rstop=m3f_opts['rstop'],
                option_types=m3f_opts['option_types'],
                map_value_mask=m3f_opts['map_value_mask'],
                param_entries=param_entries,
                const_entries=m3f_opts['const_entries'],
                uses_difs_template=uses_difs,
                uses_difs_transform=uses_difs_transform,
                adds_c_internally=adds_c,
            )
            all_entries[category].append(entry)

    # Generate .cpp files per category
    print(f"\n{'='*60}")
    print(f"Generation summary:")
    print(f"  Total .inc files scanned: {total_count}")
    print(f"  Skipped (internal dupes): {skip_count}")
    print(f"  Skipped (already registered): {existing_count}")
    print(f"  Missing .m3f (empty params): {no_m3f_count}")
    print(f"{'='*60}")

    total_generated = 0
    for category in CATEGORIES:
        entries = all_entries[category]
        if not entries:
            print(f"  {category}: 0 formulas (skipped)")
            continue

        cpp_content = generate_category_cpp(category, entries, formulas_dir)
        out_path = os.path.join(output_dir, f'external_formula_table_{category}.cpp')
        with open(out_path, 'w', encoding='utf-8') as f:
            f.write(cpp_content)

        print(f"  {category}: {len(entries)} formulas → {out_path}")
        total_generated += len(entries)

    print(f"\n  TOTAL generated: {total_generated} formulas in {sum(1 for c in CATEGORIES if all_entries[c])} files")

    # Generate forward declarations header
    header_path = os.path.join(output_dir, 'external_formula_table_generated.h')
    with open(header_path, 'w', encoding='utf-8') as f:
        f.write('// external_formula_table_generated.h — Auto-generated\n')
        f.write('// Forward declarations for bulk formula registration functions\n')
        f.write('#pragma once\n\n')
        f.write('namespace mb3d {\n\n')
        for category in CATEGORIES:
            if all_entries[category]:
                f.write(f'void RegisterExternalFormulas_{category}();\n')
        f.write('\n} // namespace mb3d\n')

    print(f"\n  Header: {header_path}")


if __name__ == '__main__':
    main()
