// _PartlyJulia.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _PartlyJulia
// Hand-ported from ASM: 109 bytes, DEoption=-1

constexpr unsigned int FORMULA__PARTLYJULIA = 10004;

template<>
struct ExternalFormulaAppendsPersistentCLanes<FORMULA__PARTLYJULIA> { static constexpr bool value = true; };
template<>
struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA__PARTLYJULIA> { static constexpr bool value = true; };

// ASM trace:
//   esi = context (esi[0]=Px, esi[8]=Py, esi[0x10]=Pz), edi = PVar
//   Params: [edi-0x10]=X_above, [edi-0x18]=X_below, [edi-0x20]=X_add
//           [edi-0x28]=Y_above, [edi-0x30]=Y_below, [edi-0x38]=Y_add
//           [edi-0x40]=Z_above, [edi-0x48]=Z_below, [edi-0x50]=Z_add
//
//   For each axis (X, Y, Z):
//     fld Pn
//     fcom N_above     ; compare with upper threshold
//     and ah, 0x41
//     je modify        ; if Pn > N_above → modify
//     fcom N_below     ; compare with lower threshold
//     and ah, 0x41
//     je skip          ; if Pn > N_below → skip (in range)
//   modify:
//     fadd N_add
//     fst [esi+Cn]     ; Cn = Pn + N_add
//
//   Note: fcom sets C0=0,C3=0 when st0>operand → ah&0x41=0 → je taken
//   So modify when Pn > N_above OR Pn <= N_below
//   Position (x/y/z) is NOT modified. Only C values change.

template<>
cpu_double3 execute_formula<FORMULA__PARTLYJULIA>(cpu_double3 p, constant double* params, double &de) {
    const double X_above = params[0];  // default: 999
    const double X_below = params[1];  // default: -999
    const double X_add   = params[2];  // default: 0
    const double Y_above = params[3];  // default: 999
    const double Y_below = params[4];  // default: -999
    const double Y_add   = params[5];  // default: 0
    const double Z_above = params[6];  // default: 999
    const double Z_below = params[7];  // default: -999
    const double Z_add   = params[8];  // default: 0

    double Cx = params[9];
    double Cy = params[10];
    double Cz = params[11];

    // Nine options + four Julia lanes + two extended-state lanes precede P.
    const double px = params[15], py = params[16], pz = params[17];
    // FCOM/AND/JE modifies on unordered lower comparisons as well as equality.
    if (px > X_above || !(px > X_below)) Cx = px + X_add;
    if (py > Y_above || !(py > Y_below)) Cy = py + Y_add;
    if (pz > Z_above || !(pz > Z_below)) Cz = pz + Z_add;

    params[WRITE_J1_SLOT] = Cx;
    params[WRITE_J2_SLOT] = Cy;
    params[WRITE_J3_SLOT] = Cz;

    // This is a C-only transform. Position is unchanged.
    return p;
}

// Formula metadata
constant FormulaInfo formula_info__partlyjulia = {
    .id = FORMULA__PARTLYJULIA,
    .name = "_PartlyJulia",
    .param_count = 9,
    .complexity = 40,
    .uses_transcendental = false
};

/*
Original M3 Description:
A pretransform to disable partly the julia mode.

if (Px > X_above) or (Px < X_below) then Cx := Px + X_add
if (Py > Y_above) or (Py < Y_below) then Cy := Py + Y_add
if (Pz > Z_above) or (Pz < Z_below) then Cz := Pz + Z_add

P is the actual position and C the adding constant.

For example if you want to change the julia mode into the non-julia mode at x=5
towards lower values, set X_below to 5 and X_add to -5 if julia X is zero.
If X_add is the difference between the Julia value and X_below (or X_above),
then you get a smooth change towards the non-julia mode.

Note: ASM uses <= for the lower bound (fcom + je), description says <.
Equality is retained explicitly in the implementation and boundary checks.

Parameters:
  [0] X_above: double (default: 999)
  [1] X_below: double (default: -999)
  [2] X_add: double (default: 0)
  [3] Y_above: double (default: 999)
  [4] Y_below: double (default: -999)
  [5] Y_add: double (default: 0)
  [6] Z_above: double (default: 999)
  [7] Z_below: double (default: -999)
  [8] Z_add: double (default: 0)
*/
