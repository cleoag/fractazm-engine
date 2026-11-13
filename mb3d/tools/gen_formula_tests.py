#!/usr/bin/env python3
"""Generate synthetic .m3p test files for all built-in MB3D formulas.

Uses M3Parameter/spineJulia.m3p as a template and binary-patches formula
fields to create one test scene per formula type.

Binary layout (1976 bytes total):
  TMandHeader10    [0..839]   - main parameters
  THeaderCustomAddon [840..1975] - formula configuration

Key offsets:
  92   : RStop (Double, 8 bytes) - bailout radius
  190  : bIsJulia (Byte) - 0=Mandelbrot, 1=Julia
  848  : Formulas[0].iItCount (Integer, 4 bytes)
  852  : Formulas[0].iFnr (Integer, 4 bytes) - formula ID
  856  : Formulas[0].iOptionCount (Integer, 4 bytes)
  860  : Formulas[0].CustomFname (32 bytes, null-padded)
  908  : Formulas[0].dOptionValue[0] (Double, 8 bytes)
  1036 : Formulas[1].iItCount (start of formula slot 1)
"""

import os
import struct
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(SCRIPT_DIR)
TEMPLATE = os.path.join(PROJECT_DIR, "M3Parameter", "spineJulia.m3p")
OUTPUT_DIR = os.path.join(PROJECT_DIR, "M3Parameter_test")

# Offsets within the .m3p binary
OFF_ITERATIONS = 12     # Integer - max iterations
OFF_XMID = 36           # Double - camera X center
OFF_YMID = 44           # Double - camera Y center
OFF_ZMID = 52           # Double - camera Z center
OFF_ZOOM = 84           # Double - zoom level
OFF_RSTOP = 92          # Double - bailout radius
OFF_BISJULIA = 190      # Byte - Julia mode flag
OFF_JX = 191            # Double - Julia X constant
OFF_JY = 199            # Double - Julia Y constant
OFF_JZ = 207            # Double - Julia Z constant
OFF_JW = 215            # Double - Julia W constant
OFF_F0_ITCOUNT = 848    # Integer - iteration count for formula 0
OFF_F0_IFNR = 852       # Integer - formula ID
OFF_F0_OPTCOUNT = 856   # Integer - option count
OFF_F0_NAME = 860       # 32 bytes - formula name (null-padded)
OFF_F0_OPTVAL0 = 908    # Double - first option value (e.g., power)
OFF_F1_ITCOUNT = 1036   # Integer - iteration count for formula 1 (=0 to disable)

# Formula definitions: (filename, iFnr, optVal0, RStop, name, optCount, camera_override)
# camera_override: None to keep template's Julia camera, or dict with Mandelbrot-mode camera params
MANDELBROT_CAM = {"isJulia": 0, "zoom": 0.5, "center": (0.0, 0.0, 0.0), "iters": 40}
AMAZINGBOX_CAM = {"isJulia": 0, "zoom": 0.15, "center": (0.0, 0.0, 0.0), "iters": 60}
BULBOX_CAM     = {"isJulia": 0, "zoom": 0.15, "center": (0.0, 0.0, 0.0), "iters": 60}

FORMULAS = [
    ("formula_0_IntPow2.m3p",    0, 2.0,  16.0,    "Integer Power",    2, None),
    ("formula_0_IntPow3.m3p",    0, 3.0,  16.0,    "Integer Power",    2, None),
    ("formula_0_IntPow4.m3p",    0, 4.0,  16.0,    "Integer Power",    2, None),
    ("formula_0_IntPow5.m3p",    0, 5.0,  16.0,    "Integer Power",    2, None),
    ("formula_0_IntPow6.m3p",    0, 6.0,  16.0,    "Integer Power",    2, None),
    ("formula_0_IntPow7.m3p",    0, 7.0,  16.0,    "Integer Power",    2, None),
    ("formula_0_IntPow8.m3p",    0, 8.0,  16.0,    "Integer Power",    2, None),
    ("formula_1_RealPower.m3p",  1, 3.5,  16.0,    "Real Power",       2, None),
    ("formula_2_Quaternion.m3p", 2, 2.0,  16.0,    "Quaternion",       2, None),
    ("formula_3_Tricorn.m3p",    3, 8.0,  16.0,    "Tricorn",          2, MANDELBROT_CAM),
    ("formula_4_AmazingBox.m3p", 4, 2.0, 1024.0,   "AmazingBox",       2, AMAZINGBOX_CAM),
    ("formula_5_Bulbox.m3p",     5, 2.0, 1024.0,   "Bulbox",           2, BULBOX_CAM),
    ("formula_6_FoldIntPow.m3p", 6, 8.0,  16.0,    "FoldingIntPow",    2, None),
    ("formula_9_AexionC.m3p",    9, 3.5,  16.0,    "AexionC",          2, None),
]


def patch_m3p(template_data, ifnr, opt_val0, rstop, name, opt_count, camera=None):
    """Create a patched .m3p binary from the template."""
    data = bytearray(template_data)

    # Patch RStop (Double at offset 92)
    struct.pack_into('<d', data, OFF_RSTOP, rstop)

    # Apply camera overrides for formulas that need different view parameters
    if camera:
        data[OFF_BISJULIA] = camera.get("isJulia", 1)
        if "zoom" in camera:
            struct.pack_into('<d', data, OFF_ZOOM, camera["zoom"])
        if "center" in camera:
            cx, cy, cz = camera["center"]
            struct.pack_into('<d', data, OFF_XMID, cx)
            struct.pack_into('<d', data, OFF_YMID, cy)
            struct.pack_into('<d', data, OFF_ZMID, cz)
        if "iters" in camera:
            struct.pack_into('<i', data, OFF_ITERATIONS, camera["iters"])
        if camera.get("isJulia", 1) == 0:
            # Clear Julia constants for Mandelbrot mode
            struct.pack_into('<d', data, OFF_JX, 0.0)
            struct.pack_into('<d', data, OFF_JY, 0.0)
            struct.pack_into('<d', data, OFF_JZ, 0.0)
            struct.pack_into('<d', data, OFF_JW, 0.0)

    # Patch formula slot 0
    struct.pack_into('<i', data, OFF_F0_ITCOUNT, 1)       # 1 iteration per cycle
    struct.pack_into('<i', data, OFF_F0_IFNR, ifnr)       # formula ID
    struct.pack_into('<i', data, OFF_F0_OPTCOUNT, opt_count)
    # Write formula name (null-padded to 32 bytes)
    name_bytes = name.encode('ascii')[:31] + b'\x00' * 32
    data[OFF_F0_NAME:OFF_F0_NAME + 32] = name_bytes[:32]
    # Patch first option value (power / scale)
    struct.pack_into('<d', data, OFF_F0_OPTVAL0, opt_val0)

    # Ensure formula slot 1 is disabled (iItCount=0)
    struct.pack_into('<i', data, OFF_F1_ITCOUNT, 0)

    return bytes(data)


def main():
    if not os.path.exists(TEMPLATE):
        print(f"ERROR: Template not found: {TEMPLATE}", file=sys.stderr)
        sys.exit(1)

    with open(TEMPLATE, 'rb') as f:
        template_data = f.read()

    if len(template_data) != 1976:
        print(f"WARNING: Template size is {len(template_data)}, expected 1976", file=sys.stderr)

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    print(f"Template: {TEMPLATE} ({len(template_data)} bytes)")
    print(f"Output:   {OUTPUT_DIR}")
    print()

    for filename, ifnr, opt_val0, rstop, name, opt_count, camera in FORMULAS:
        outpath = os.path.join(OUTPUT_DIR, filename)
        patched = patch_m3p(template_data, ifnr, opt_val0, rstop, name, opt_count, camera)
        with open(outpath, 'wb') as f:
            f.write(patched)
        print(f"  {filename:35s} iFnr={ifnr} opt0={opt_val0:5.1f} RStop={rstop:7.1f}")

    print(f"\nGenerated {len(FORMULAS)} test files.")


if __name__ == '__main__':
    main()
