// wavespIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: wavespIFS
// Hand-ported from ASM: 162 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS transform + x87 spiral wave DE)
// Generalized spiral heightfield with power/logarithmic spiral modes

constexpr unsigned int FORMULA_WAVESPIFS = 70840;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void WavespIFSDistance_x87(double*, const double*, int, int);
#endif

template<>
cpu_double3 execute_formula<FORMULA_WAVESPIFS>(cpu_double3 p, constant double* params, double &de) {
    const double N_of_spirals = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Amplitude = params[5];
    const int Apply_scale_add = int(params[6]);
    const double R_exponent = params[7];
    const int Pow_R_or_Log_R = int(params[8]);

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;    // w *= Scale
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
    }

#if defined(_MSC_VER) && defined(_M_X64)
    // Keep the entire logarithm/power/angle/sine expression on the x87 stack.
    double state[] = {x,y,z,0.};
    WavespIFSDistance_x87(state,params,Apply_scale_add,Pow_R_or_Log_R);
    de=state[3];
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    const E square=abs(E(x)*x+E(y)*y),infinity=std::numeric_limits<double>::infinity();
    const E logarithm=square==E(0) ? -infinity : square==infinity ? infinity : extendedLog2(square);
    E radial=E(R_exponent)*logarithm;
    if(Pow_R_or_Log_R==0) radial=extendedExp2(radial,false);
    const E angle=radial+extendedAtan2(E(y),E(x))*N_of_spirals;
    // Scalar FSIN keeps a finite out-of-range operand instead of pushing.
    const auto exponent=angle.signExponent()&0x7fff;
    const E wave=exponent>=16383+63 && exponent<0x7fff ? angle : extendedSinCos(angle).sin;
    E distance=-(abs(wave)*Amplitude+E(z));
    if(!Apply_scale_add) distance/=Scale;
    de=double(distance);
#else
    // Step 2: Cylindrical radius squared (no sqrt for speed)
    // ASM: x² + y² → fyl2x with R_exponent
    double rho_sq = x * x + y * y;

    // Step 3: Radial function (power or logarithmic)
    // ASM: R_exponent * log2(|rho_sq|) → then 2^result for pow mode
    double rho_result;
    if (Pow_R_or_Log_R == 0) {
        // Power mode: rho_sq^R_exponent via 2^(R_exp * log2(rho_sq))
        rho_result = x87_log_power(abs(rho_sq), R_exponent);
    } else {
        // Log mode: R_exponent * log2(rho_sq) — logarithmic spiral
        rho_result = R_exponent * log2(abs(rho_sq));
    }

    // Step 4: Spiral angle
    // ASM: fpatan(y, x) → atan2(y, x) = theta
    double theta = atan2(y, x);

    // Step 5: Spiral wave
    // ASM: sin(rho_result + N*theta) → fabs → * Amplitude → + z → fchs
    double wave = sin(rho_result + N_of_spirals * theta);
    double shape_de = -(abs(wave) * Amplitude + z);

    // Step 6: Scale correction
    // ASM: if Apply_scale_add == 0: fdiv Scale
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
#endif

    return Apply_scale_add ? cpu_double3(x, y, z) : p;
}

// Alternative direct function implementation
cpu_double3 wavespIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_WAVESPIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_wavespIFS = {
    .id = FORMULA_WAVESPIFS,
    .name = "wavespIFS",
    .param_count = 9,
    .complexity = 350,
    .uses_transcendental = true
};

/*
Original M3 Description:
Generalized "spiral heightfield".
You can choose any R exponent to get different spirals;

Exponent < 0 -> Whorls (spirals infinitely tight in the center)
0 ... 0.5 -> spirals that fade out in the distance
0.5 -> a "normal" spiral - every spire is at a fixed distance
0.5 + -> Spirals that get tighter when distance grows

Enable the last switch to see a logarithmic spiral (visually very appealing)

Equation:
  (rho,theta) = tocylindrical(x,y)
  DE = -(|sin(pow(rho_sq, exponent) + N * theta)| * Amplitude + z)

Luca GN 2012

Mathematical Formula:
  DEoption=20 (dIFS shape — SSE2 IFS + x87 spiral wave DE):
    - Standard SSE2 IFS preamble
    - Cylindrical coords: rho_sq = x² + y², theta = atan2(y,x)
    - Power mode: rho_sq^R_exponent (via fyl2x + f2xm1 + fscale)
    - Log mode: R_exponent * log2(rho_sq)
    - Spiral wave: |sin(rho_result + N*theta)| * Amplitude

  Step 1: IFS transform:
    x = p.x * Scale + X_add;  y = p.y * Scale + Y_add;  z = p.z * Scale + Z_add
    if Apply_scale_add: w *= Scale

  Step 2: Radial function:
    rho_sq = x² + y²
    if Pow mode: rho_result = rho_sq ^ R_exponent
    if Log mode: rho_result = R_exponent * log2(rho_sq)

  Step 3: Spiral wave:
    theta = atan2(y, x)
    shape_de = -(|sin(rho_result + N*theta)| * Amplitude + z)

Parameters:
  [0] N_of_spirals: double (default: 1) — number of spiral arms
  [1] Scale: double (default: 2) — IFS scale factor (.2Doubles)
  [2] Z_add: double (default: 0) — Z offset (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Amplitude: double (default: 0.1) — wave amplitude
  [6] Apply_scale_add: int (default: 1)
  [7] R_exponent: double (default: 0.8) — radial exponent
  [8] Pow_R_or_Log_R: int (default: 0) — 0=power, 1=logarithmic

PVar layout (verified against x87 ASM offsets):
  Constants (positive offsets):
  [PVar + 0x00] = 1.0 (unused in computation)

  Options (negative offsets):
  [PVar - 0x10] = N_of_spirals (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = Amplitude (double)
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x54] = R_exponent (double)
  [PVar - 0x58] = Pow_R_or_Log_R (integer)

Code structure:
  0x00-0x40: Standard SSE2 IFS preamble
  0x40-0x45: Store to stack for x87
  0x4B-0x56: rho_sq = x² + y²
  0x58-0x5: fyl2x: R_exponent * log2(rho_sq)
  0x61-0x65: Branch on Pow_R_or_Log_R
  0x67-0x75: Pow mode: 2^(result) via fprem + f2xm1 + fscale
  0x77-0x83: theta = atan2(y,x), * N_of_spirals, + rho_result
  0x85-0x90: sin → abs → * Amplitude → + z → negate
  0x92-0xA1: Scale correction + store + return

Key notes:
  - Uses rho_sq (NOT rho) directly for speed — omits sqrt
  - Power mode via standard x87 pow: 2^(y*log2(x))
  - Log mode just uses the fyl2x result directly (logarithmic spiral)
  - Negate at end: surface is "below" reference plane
  - Stack allocation 0x1c (28 bytes)
  - 162 bytes — compact spiral formula
*/
