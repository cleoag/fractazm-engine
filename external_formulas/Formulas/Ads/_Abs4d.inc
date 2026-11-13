// _Abs4d.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Abs4d
// Hand-ported from ASM: 238 bytes, DEoption=-1

constexpr unsigned int FORMULA__ABS4D = 5004;

template<>
struct ExternalFormulaWritesWState<FORMULA__ABS4D> {
    static constexpr bool value = true;
};

// ASM trace:
//   DD4108        fld [ecx+8]        → w (4th dimension)
//   DD01          fld [ecx]          → z
//   DD02          fld [edx]          → y
//   DD00          fld [eax]          → x
//   D9E1          fabs               → |x|
//   DD18          fstp [eax]         → x' = |x|
//   D9E1          fabs               → |y| (now st0)
//   DD1A          fstp [edx]         → y' = |y|
//   D9E1          fabs               → |z|
//   DD19          fstp [ecx]         → z' = |z|
//   D9E1          fabs               → |w|
//   DD5908        fstp [ecx+8]       → w' = |w|
//
// Takes absolute value of all 4 components.
// In 3D mode, w is at [ecx+8].

template<>
cpu_double3 execute_formula<FORMULA__ABS4D>(cpu_double3 p, constant double* params, double &de) {
    params[FORMULA_STATE_W_SLOT] = abs(params[FORMULA_STATE_W_SLOT]);
    return cpu_double3(abs(p.x), abs(p.y), abs(p.z));
}

// Alternative direct function implementation
cpu_double3 _Abs4d_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(abs(p.x), abs(p.y), abs(p.z));
}

// Formula metadata
constant FormulaInfo formula_info__Abs4d = {
    .id = FORMULA__ABS4D,
    .name = "_Abs4d",
    .param_count = 0,
    .complexity = 120,
    .uses_transcendental = false
};

/*
Original M3 Description:
Takes the absolute values of all 4 vector components:
x' = abs(x), y' = abs(y), z' = abs(z), w' = abs(w)

Note: w (4th dimension) is stored at [ecx+8] in the ASM.
The Metal version handles the 3D components; w is managed
by the iteration engine for 4D formulas.

Parameters: none
*/
