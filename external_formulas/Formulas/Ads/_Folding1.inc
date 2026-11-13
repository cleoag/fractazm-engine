// _Folding1.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Folding1
// Hand-ported from ASM: 284 bytes, DEoption=-1

constexpr unsigned int FORMULA__FOLDING1 = 6001;

// ASM trace:
//   8B7630          mov esi,[esi+0x30]  → esi = PVar
//   DD46F0          fld [esi-0x10]      → fold (R fold param)
//
//   For each axis (z, y, x):
//   fld [reg]            → coord
//   fld st0              → dup coord
//   fsub st0,st2         → coord - fold
//   fabs                 → |coord - fold|
//   fxch st1             → bring coord back
//   fadd st0,st2         → coord + fold
//   fabs                 → |coord + fold|
//   fsubrp st1,st0       → |coord+fold| - |coord-fold|  (DE E0+i = FSUBRP)
//   fsub [reg]           → subtract original coord
//   fstp [reg]           → store result
//
// Box fold (Mandelbox-style):
//   coord' = |coord + fold| - |coord - fold| - coord
// Equivalent to: if coord > fold: 2*fold - coord
//                if coord < -fold: -2*fold - coord
//                else: coord (unchanged)

template<>
cpu_double3 execute_formula<FORMULA__FOLDING1>(cpu_double3 p, double* params, double &de) {
    const double fold = params[0];

    double x = abs(p.x + fold) - abs(p.x - fold) - p.x;
    double y = abs(p.y + fold) - abs(p.y - fold) - p.y;
    double z = abs(p.z + fold) - abs(p.z - fold) - p.z;

    return cpu_double3(x, y, z);
}

// Alternative direct function implementation
cpu_double3 _Folding1_formula(cpu_double3 p, double* params, double &de) {
    double f = params[0];
    return cpu_double3(
        abs(p.x+f) - abs(p.x-f) - p.x,
        abs(p.y+f) - abs(p.y-f) - p.y,
        abs(p.z+f) - abs(p.z-f) - p.z
    );
}

// Formula metadata
constant FormulaInfo formula_info__Folding1 = {
    .id = FORMULA__FOLDING1,
    .name = "_Folding1",
    .param_count = 1,
    .complexity = 290,
    .uses_transcendental = false
};

/*
Original M3 Description:
Box folding like in the amazing box (Mandelbox).

coord' = |coord + fold| - |coord - fold| - coord

This clamps each coordinate to [-fold, fold] and reflects
the excess. Standard box fold used in Mandelbox/AmazingBox.

Parameters:
  [0] R fold: double (default: 2)
*/
