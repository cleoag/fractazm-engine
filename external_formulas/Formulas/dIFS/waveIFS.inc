// waveIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: waveIFS
// Hand-ported from ASM: 179 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS transform + x87 wave heightfield DE)
// Wave surface with circular/flat modes, XZ flip, and radial amplitude decay

constexpr unsigned int FORMULA_WAVEIFS = 5197;

template<>
cpu_double3 execute_formula<FORMULA_WAVEIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Frequency = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Amplitude = params[5];
    const int Apply_scale_add = int(params[6]);
    const int Circular = int(params[7]);
    const int Flip_XZ = int(params[8]);
    const int Circ_R_off = int(params[9]);

    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;    // w *= Scale
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    // Step 2: Select wave axis via Flip_XZ
    // ASM: edx = (Flip_XZ & 1) * 16 → selects x (edx=0) or z (edx=16) from stack
    double h_var = (Flip_XZ & 1) != 0 ? z : x;    // wave input variable
    double other_var = (Flip_XZ & 1) != 0 ? x : z; // DE reference axis

    // Step 3: Compute wave value
    double amp_eff = Amplitude;
    double wave;

    if (Circular != 0) {
        // Circular wave: cos(r * Frequency) where r = sqrt(h_var² + y²)
        // ASM: h_var² + y² → sqrt → * Frequency
        double r = sqrt(h_var * h_var + y * y);
        double angle = r * Frequency;

        // Optional radial amplitude decay
        // ASM: if Circ_R_off != 0: amp_eff = Amplitude / (1 + r*Frequency)
        if (Circ_R_off != 0) {
            amp_eff = Amplitude / (1.0 + r * Frequency);
        }

        wave = cos(angle);
    } else {
        // Flat wave: cos(h_var * Freq) + cos(y * Freq)
        // ASM: h_var * Freq → fcos; y * Freq → fcos; faddp
        wave = cos(h_var * Frequency) + cos(y * Frequency);
    }

    // Step 4: Shape DE = wave * amplitude - other_var
    // ASM: fmul [esp+0x18] → fsub [esp+edx+0x10]
    // neg edx trick: edx=0→[esp+0x10]=z; edx=-16→[esp+0]=x
    double shape_de = wave * amp_eff - other_var;

    // Step 5: Scale correction
    // ASM: if Apply_scale_add == 0: fdiv Scale
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    return cpu_double3(state_x, state_y, state_z);
}

// Alternative direct function implementation
cpu_double3 waveIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_WAVEIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_waveIFS = {
    .id = FORMULA_WAVEIFS,
    .name = "waveIFS",
    .param_count = 10,
    .complexity = 320,
    .uses_transcendental = true
};

/*
Original M3 Description:
Wave surface heightfield with circular and flat modes.

Mathematical Formula:
  DEoption=20 (dIFS shape — SSE2 IFS + x87 wave heightfield DE):
    - Standard SSE2 IFS preamble
    - Axis selection: Flip_XZ swaps wave input (x or z)
    - Circular mode: cos(r * Freq) where r = cylindrical radius
    - Flat mode: cos(h*Freq) + cos(y*Freq) — Cartesian sum
    - Optional radial amplitude decay: A / (1 + r*Freq)

  Step 1: IFS transform:
    x = p.x * Scale + X_add;  y = p.y * Scale + Y_add;  z = p.z * Scale + Z_add
    if Apply_scale_add: w *= Scale

  Step 2: Axis selection:
    if Flip_XZ=0: h_var = x, other = z (wave in XY, measured along Z)
    if Flip_XZ=1: h_var = z, other = x (wave in ZY, measured along X)

  Step 3: Wave computation:
    Circular: r = sqrt(h_var² + y²); wave = cos(r * Freq)
      if Circ_R_off: Amplitude /= (1 + r*Freq)
    Flat: wave = cos(h_var*Freq) + cos(y*Freq)

  Step 4: Shape DE:
    shape_de = wave * Amplitude - other_var

Parameters:
  [0] Frequency: double (default: 4) — wave frequency
  [1] Scale: double (default: 2) — IFS scale factor (.2Doubles)
  [2] Z_add: double (default: 0) — Z offset (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Amplitude: double (default: 0.1) — wave amplitude
  [6] Apply_scale_add: int (default: 0)
  [7] Circular: int (default: 1) — 0=flat, 1=circular wave
  [8] Flip_XZ: int (default: 0) — 0=wave in x, 1=wave in z
  [9] Circ_R_off: int (default: 0) — 0=constant amp, 1=decay with radius

PVar layout (verified against x87 ASM offsets):
  Options (negative offsets):
  [PVar - 0x10] = Frequency (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = Amplitude (double)
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x50] = Circular (integer)
  [PVar - 0x54] = Flip_XZ (integer)
  [PVar - 0x58] = Circ_R_off (integer)

Code structure:
  0x00-0x40: Standard SSE2 IFS preamble
  0x40-0x45: Store to stack for x87
  0x4B-0x57: Load Flip_XZ, compute edx offset, store Amplitude, load h_var
  0x5E-0x62: Branch on Circular
  0x64-0x87: Circular path: r=sqrt(h²+y²), r*Freq, optional amp decay, cos
  0x89-0x97: Flat path: cos(h*F) + cos(y*F)
  0x99-0x9: neg edx → wave * Amplitude → subtract other_var
  0xA3-0xB2: Scale correction + store + return

Key notes:
  - Flip_XZ uses edx register trick: shl 4 → selects stack offset 0 or 16
  - neg edx at 0x99 inverts offset: 0→0 or 16→-16, selecting other axis
  - For flat mode: 2D Cartesian wave sum (no radial decay)
  - Circ_R_off divides amplitude by (1 + r*Freq) for natural falloff
  - Stack allocation 0x20 (32 bytes) for coords + amplitude
  - 179 bytes — moderate complexity with 3 branch options
*/
