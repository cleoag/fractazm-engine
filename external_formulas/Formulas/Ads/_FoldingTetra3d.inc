// _FoldingTetra3d.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _FoldingTetra3d
// Hand-ported from ASM: 194 bytes, DEoption=-1

constexpr unsigned int FORMULA__FOLDINGTETRA3D = 7096;

// ASM trace:
//   Phase 1 (Mode1 != 0): fold across tetrahedral symmetry planes
//     if (x+y < 0): x=-x, y=-y  (fold across x+y=0 plane)
//     if (x+z < 0): x=-x, z=-z  (fold across x+z=0 plane)
//     if (y+z < 0): y=-y, z=-z  (fold across y+z=0 plane)
//   Phase 2 (Mode2 != 0, or always if Mode1 == 0): sort descending
//     if (x < y): swap(x, y)
//     if (x < z): swap(x, z)
//     if (y < z): swap(y, z)
//
// Two integer mode params control which phases are active.
// Note: when Mode1==0, Phase 2 always runs regardless of Mode2.
//
// C handling: Transform (DEoption=-1), no C involvement.

template<>
cpu_double3 execute_formula<FORMULA__FOLDINGTETRA3D>(cpu_double3 p, constant double* params, double &de) {
    const int Mode1 = int(params[0]);  // 1st half fold (default: 1)
    const int Mode2 = int(params[1]);  // 2nd half sort (default: 1)

    double x = p.x, y = p.y, z = p.z;

    if (Mode1 != 0) {
        // Phase 1: Tetrahedral fold — reflect across sum-planes
        if (x + y <= 0.0) { double t = x; x = -y; y = -t; }
        if (x + z <= 0.0) { double t = x; x = -z; z = -t; }
        if (y + z <= 0.0) { double t = y; y = -z; z = -t; }

        // Phase 2 only runs if Mode2 != 0
        if (Mode2 != 0) {
            if (x < y) { double t = x; x = y; y = t; }
            if (x < z) { double t = x; x = z; z = t; }
            if (y < z) { double t = y; y = z; z = t; }
        }
    } else {
        // When Mode1 == 0: always do sort regardless of Mode2
        if (x < y) { double t = x; x = y; y = t; }
        if (x < z) { double t = x; x = z; z = t; }
        if (y < z) { double t = y; y = z; z = t; }
    }

    return cpu_double3(x, y, z);
}

// Formula metadata
constant FormulaInfo formula_info__FoldingTetra3d = {
    .id = FORMULA__FOLDINGTETRA3D,
    .name = "_FoldingTetra3d",
    .param_count = 2,
    .complexity = 194,
    .uses_transcendental = false
};

/*
Original M3 Description:
Tetrahedral symmetry folding transform by Luca GN 2011.
Code taken from the forums, KIFS original thread.

1st half: fold across tetrahedral symmetry planes
  if(x+y<0){x1=-y;y=-x;x=x1;}
  if(x+z<0){x1=-z;z=-x;x=x1;}
  if(y+z<0){y1=-z;z=-y;y=y1;}

2nd half: sort coordinates descending (x >= y >= z)
  if(x-y<0){x1=y;y=x;x=x1;}
  if(x-z<0){x1=z;z=x;x=x1;}
  if(y-z<0){y1=z;z=y;y=y1;}

Side note: if you disable the 1st half, 2nd half will be
done even if you disable it (to avoid a NOP transform).

Parameters:
  [0] Fold_1st_half: int (default: 1)
  [1] Fold_2nd_half: int (default: 1)
*/
