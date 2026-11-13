// RoundedBox.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: RoundedBox
// Hand-ported from ASM: 170 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS transform + SSE2 rounded box DE)
// Rounded box SDF with OTrap source options — entirely SSE2, no x87 for shape

constexpr unsigned int FORMULA_ROUNDEDBOX = 9624;

template<>
cpu_double3 execute_formula<FORMULA_ROUNDEDBOX>(cpu_double3 p, constant double* params, double &de) {
    const double Z_halfwidth = params[0];
    const double Y_halfwidth = params[1];
    const double X_halfwidth = params[2];
    const double Scale = params[3];
    const double Z_add = params[4];
    const double Y_add = params[5];
    const double X_add = params[6];
    const double Border = params[7];
    const int Apply_scale_add = int(params[8]);
    // params[9] = OTrap_option, params[10] = OTrap_offset, params[11] = OTrap_scale (coloring)

    // Step 1: SSE2 IFS transform (non-standard offsets due to extra params before Scale)
    // ASM: mulpd [edi-0x30], addpd [edi-0x50], addsd [edi-0x40]
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;    // w *= Scale
    }

    // Step 2: Rounded box SDF
    // ASM: andpd (abs) → subpd halfwidths → addpd Border → maxpd 0 → square → sum → sqrt → sub Border
    // q = max(|p| - halfwidths + Border, 0)
    // shape_de = length(q) - Border
    double qx = max(abs(x) - X_halfwidth + Border, 0.0);
    double qy = max(abs(y) - Y_halfwidth + Border, 0.0);
    double qz = max(abs(z) - Z_halfwidth + Border, 0.0);

    double shape_de = sqrt(qx * qx + qy * qy + qz * qz) - Border;

    // Step 3: Scale correction
    // ASM: if Apply_scale_add == 0: divsd Scale
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    // Step 4: OTrap writeback
    // ASM: OTrap = selected_value[option & 3] * OTrap_scale + OTrap_offset → [esi+0x80]
    // option 0=DEstop, 1=x, 2=y, 3=z
    const int OTrap_option = int(params[9]);
    const double OTrap_offset = params[10];
    const double OTrap_scale = params[11];

    const cpu_double3 committed = Apply_scale_add ? cpu_double3(x, y, z) : p;
    double val;
    switch (OTrap_option & 3) {
        case 0:  val = params[MAX_FORMULA_PARAMS - 2]; break; // RStopD at ESI-0x80
        case 1:  val = committed.x; break;
        case 2:  val = committed.y; break;
        case 3:  val = committed.z; break;
        default: val = 0.0; break;
    }
    params[63] = val * OTrap_scale + OTrap_offset;

    return committed;
}

// Alternative direct function implementation
cpu_double3 RoundedBox_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_ROUNDEDBOX>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_RoundedBox = {
    .id = FORMULA_ROUNDEDBOX,
    .name = "RoundedBox",
    .param_count = 12,
    .complexity = 200,
    .uses_transcendental = false
};

/*
Original M3 Description:
Rounded box shape dIFS with OTrap coloring options:

0: Change with DEstop
1: Change with X
2: Change with Y
3: Change with Z

DEstop might vary on distance, choose scale = 0 to switch off.

Mathematical Formula:
  DEoption=20 (dIFS shape — SSE2 IFS + SSE2 rounded box DE):
    - Non-standard IFS preamble (Scale at [edi-0x30] due to 3 extra params)
    - Rounded box SDF: length(max(|p| - hw + border, 0)) - border
    - Entirely SSE2 computation (no x87 for shape, only for OTrap)
    - OTrap with selectable coloring source

  Step 1: IFS transform:
    x = p.x * Scale + X_add;  y = p.y * Scale + Y_add;  z = p.z * Scale + Z_add
    if Apply_scale_add: w *= Scale

  Step 2: Rounded box SDF:
    q = max(|p| - halfwidths + Border, 0)
    shape_de = length(q) - Border

  Step 3: OTrap coloring:
    value = {DEstop, x, y, z}[option & 3]
    OTrap = value * OTrap_scale + OTrap_offset

Parameters:
  [0] Z_halfwidth: double (default: 1) — Z half-extent
  [1] Y_halfwidth: double (default: 1) — Y half-extent
  [2] X_halfwidth: double (default: 1) — X half-extent
  [3] Scale: double (default: 2) — IFS scale factor (.2Doubles)
  [4] Z_add: double (default: 0) — Z offset (.2Doubles)
  [5] Y_add: double (default: 0)
  [6] X_add: double (default: 2)
  [7] Border: double (default: 0.1) — rounding radius (.2Doubles)
  [8] Apply_scale_add: int (default: 1)
  [9] OTrap_option: int (default: 0) — coloring source (0..3)
  [10] OTrap_offset: single (default: 1)
  [11] OTrap_scale: single (default: 1)

PVar layout (verified against SSE2 ASM offsets):
  Constants (positive offsets):
  [PVar + 0x00] = $7FFFFFFFFFFFFFFF (abs mask, 16 bytes)

  Options (negative offsets):
  [PVar - 0x10] = Z_halfwidth (double)
  [PVar - 0x18] = Y_halfwidth (double)
  [PVar - 0x20] = X_halfwidth (double)
  [PVar - 0x30] = Scale (.2Doubles)
  [PVar - 0x40] = Z_add (.2Doubles)
  [PVar - 0x48] = Y_add (double)
  [PVar - 0x50] = X_add (double)
  [PVar - 0x60] = Border (.2Doubles)
  [PVar - 0x64] = Apply_scale_add (integer)
  [PVar - 0x68] = OTrap_option (integer)
  [PVar - 0x6c] = OTrap_offset (single)
  [PVar - 0x70] = OTrap_scale (single)

Code structure:
  0x00-0x3C: SSE2 IFS preamble (non-standard offsets)
  0x41-0x45: andpd abs(x,y); andpd abs(z)
  0x49-0x58: subtract halfwidths, add Border
  0x5D-0x61: max with 0
  0x65-0x76: square and sum components
  0x7A: sqrtsd
  0x7E: subtract Border
  0x83-0x91: Scale correction + store DE
  0x96-0xA9: OTrap computation + return

Key notes:
  - Entirely SSE2 for shape computation (no x87 stack)
  - Equivalent to standard rounded box SDF: length(max(|p|-b, 0)) - r
  - halfwidths define OUTER extent; Border is both rounding radius and inset
  - No stack allocation needed (no add esp, -0x1c)
  - OTrap uses indexed load: [esi + eax*8 - 0x80] for {DEstop, x, y, z}
  - 170 bytes — compact SSE2-only formula
*/
