// _FoldingCTetra3d.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _FoldingCTetra3d
// Hand-ported from ASM: 422 bytes, DEoption=-1

constexpr unsigned int FORMULA__FOLDINGCTETRA3D = 10018;
template<> struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA__FOLDINGCTETRA3D> { static constexpr bool value = true; };

// ASM trace:
//   Register setup:
//     esi = context (NOT overwritten to PVar)
//     edi = PVar = [esi+0x30]
//     ebx = eax = &x
//     [esi+0x18] = Cx, [esi+0x20] = Cy, [esi+0x28] = Cz
//
//   Parameters (edi = PVar, all integers):
//     [edi-0x0c] = C-Fold 1st half (integer, default 1)
//     [edi-0x10] = C-Fold 2nd half (integer, default 1)
//     [edi-0x14] = Fold xyz too (integer, default 0)
//
//   Algorithm:
//     Phase A+B on C vector:
//       if (1st half enabled):
//         tetra3d fold on C (swap-negate pairs for Cx+Cy, Cx+Cz, Cy+Cz)
//         if (2nd half enabled): sorting fold on C
//         else: skip sorting
//       else (1st half disabled):
//         sorting fold on C (unconditionally - "2nd half done even if disabled")
//
//     Phase C on xyz (if Fold xyz too enabled):
//       Same control flow: 1st half → tetra3d, 2nd half → sorting
//
//   Comparison pattern: fcompp compares st0=0 vs st1=val;
//     shr ah,1 puts C0 in CF; jb jumps if CF=1 (0 < val, i.e. val > 0).
//     So swap block executes when val <= 0.

// Helper: tetra3d 1st half fold (swap-negate when sum of pair <= 0)
static inline void tetra3d_fold_1st(double &a, double &b, double &c) {
    if (a + b <= 0.0) { double t = a; a = -b; b = -t; }
    if (a + c <= 0.0) { double t = a; a = -c; c = -t; }
    if (b + c <= 0.0) { double t = b; b = -c; c = -t; }
}

// Helper: tetra3d 2nd half fold (sort descending)
static inline void tetra3d_fold_2nd(double &a, double &b, double &c) {
    if (a - b <= 0.0) { double t = a; a = b; b = t; }
    if (a - c <= 0.0) { double t = a; a = c; c = t; }
    if (b - c <= 0.0) { double t = b; b = c; c = t; }
}

// Original code folds Julia C first, then optionally XYZ. Disabling the
// first half jumps directly into sorting, bypassing the second-half flag.
template<>
cpu_double3 execute_formula<FORMULA__FOLDINGCTETRA3D>(cpu_double3 p, constant double* params, double &de) {
    const bool first = params[0] != 0.0;
    const bool second = !first || params[1] != 0.0;
    double cx = params[3], cy = params[4], cz = params[5];
    if (first) tetra3d_fold_1st(cx, cy, cz);
    if (second) tetra3d_fold_2nd(cx, cy, cz);
    params[WRITE_J1_SLOT] = cx;
    params[WRITE_J2_SLOT] = cy;
    params[WRITE_J3_SLOT] = cz;
    if (params[2] != 0.0) {
        if (first) tetra3d_fold_1st(p.x, p.y, p.z);
        if (second) tetra3d_fold_2nd(p.x, p.y, p.z);
    }
    return p;
}

// Formula metadata
constant FormulaInfo formula_info__FoldingCTetra3d = {
    .id = FORMULA__FOLDINGCTETRA3D,
    .name = "_FoldingCTetra3d",
    .param_count = 3,
    .complexity = 422,
    .uses_transcendental = false
};

/*
Original M3 Description:

Referring to C vector components (and optionally to xyz too, after it);
1st half tetrahedral symmetry planes:
     if(x+y<0){x1=-y;y=-x;x=x1;}
      if(x+z<0){x1=-z;z=-x;x=x1;}
      if(y+z<0){y1=-z;z=-y;y=y1;}
2nd half tetrahedral symmetry planes:
     if(x-y<0){x1=y;y=x;x=x1;}
      if(x-z<0){x1=z;z=x;x=x1;}
      if(y-z<0){y1=z;z=y;y=y1;}

Code taken from the forums, KIFS original side note - if you disable the 1st half, 2nd half will be
done even if you disable it... (to avoid a NOP transform)
Coded by Luca GN 2011

Parameters:
  params[0] = C-Fold 1st half (integer, default 1) - enable tetrahedral fold on C
  params[1] = C-Fold 2nd half (integer, default 1) - enable sorting fold on C
  params[2] = Fold xyz too (integer, default 0) - apply same folds to xyz

Note: This formula modifies the C (Julia constant) vector. The engine must
provide access to the C vector for the C-folding phases to work. The xyz
folding uses the same 1st/2nd half flags to control which fold types apply.

ASM flow control detail:
  - If 1st half disabled: je at 0x12 jumps to 0x85, entering 2nd half (sorting)
    directly, bypassing both the tetra3d fold and the 2nd half enable check.
  - If 1st half enabled but 2nd half disabled: je at 0x7 jumps to 0xe8 (xyz check).
  - If both enabled: 1st half runs, falls through to 2nd half.
  - The description confirms: "if you disable the 1st half, 2nd half will be done
    even if you disable it" - this is modeled by do_c_sorting = true when 1st half off.

Note on the description's conditions: The description uses "< 0" but the ASM
uses "<= 0" (swap executes when FCOMPP of 0 vs (a+b) does NOT set CF, meaning
0 >= a+b, i.e., a+b <= 0). The Metal code follows the ASM behavior.
*/
