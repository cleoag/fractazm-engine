// MengerSmt.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MengerSmt
// Hand-ported from ASM: 646 bytes, DEoption=2
// Smooth Menger sponge with two smoothing modes.
// When LSmoothing > 0: uses cubic approximation (|x|^3 / (x^2 + s))
// When LSmoothing <= 0: uses sqrt approximation (sqrt(x^2 + s))
// Edge smoothing (ESmoothing) similarly has two modes.
// Features rotation matrix and standard DE scale tracking.

constexpr unsigned int FORMULA_MENGERSMT = 10051;

// ASM trace summary:
//   edi = PVar, esi = context pointer
//   Pre-computed: s_l = |LSmoothing| stored in [ebp-8], s_e = |ESmoothing| in [ebp-0x18]
//   Half constant = 0.5 from [CONSTANTS]
//
//   Step 1: Smooth absolute value on each axis
//     if LSmoothing > 0 (cubic mode):
//       v = |v|; t = v^2; v = (v * t) / (t + s_l)  -- smooth abs via cubic/quadratic ratio
//     else (sqrt mode):
//       v = sqrt(v^2 + s_l)
//
//   Step 2: Smooth sorting / edge folding (3 pairs: x-y, x-z, y-z)
//     if ESmoothing > 0 (cubic mode):
//       d = a - b; |d|; t = d^2; smt = (|d|*t)/(t+s_e); diff = (d - smt) * half
//       a = a - diff; b = b + diff  (smooth min/max)
//     else (sqrt mode):
//       d = a - b; smt = sqrt(d^2 + s_e); diff = (d - smt) * half (using [edi+0x00])
//       a = a - diff; b = b + diff
//
//   Step 3: Apply Scale and rotation matrix
//     z = z - CScaleZ * (Scale-1)/Scale  -- actually: z subtracted
//     Then smooth-abs z (same mode as step 1 but negated):
//       z = -(smooth_abs(z))
//     Then z += CScaleZ * (Scale-1)/Scale
//
//     x = Scale * x - CScaleX * (Scale-1)
//     y = Scale * y - CScaleY * (Scale-1)
//     z = Scale * z
//     de = Scale * de
//
//   Step 4: Apply rotation matrix
//   Step 5: Final smooth-abs on result (same mode check)

// Smooth abs helper: cubic mode (LSmoothing > 0) or sqrt mode (LSmoothing <= 0)
static inline double smooth_abs(double v, double s, bool cubic) {
    if (cubic) {
        double av = fabs(v);
        double t = av * av;
        return (av * t) / (t + s);
    } else {
        return sqrt(v * v + s);
    }
}

// Smooth sort step helper: cubic or sqrt mode
static inline void smooth_sort_step(double& a, double& b, double s, double half, bool cubic) {
    double d = a - b;
    double smt;
    if (cubic) {
        double ad = fabs(d);
        double t = ad * ad;
        smt = (ad * t) / (t + s);
    } else {
        smt = sqrt(d * d + s);
    }
    double diff = (d - smt) * half;
    a -= diff;
    b += diff;
}

template<>
cpu_double3 execute_formula<FORMULA_MENGERSMT>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];
    // params[4..12] = pre-computed rotation matrix (9 floats from .3SingleAngles)
    // params[13] = LSmoothing, params[14] = ESmoothing
    const double LSmoothing = params[13];
    const double ESmoothing = params[14];
    const double half_val = 0.5;
    const double s_l = fabs(LSmoothing);
    const double s_e = fabs(ESmoothing);
    // x87 compare polarity here is inverted relative to the original M3 comment:
    // positive smoothing values take the sqrt branch, non-positive take the cubic branch.
    const bool l_cubic = (LSmoothing <= 0.0);
    const bool e_cubic = (ESmoothing <= 0.0);

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // Step 1: Smooth absolute value on each axis
    // ASM: if LSmoothing > 0 → cubic: |v|^3/(|v|^2+s), else → sqrt: sqrt(v^2+s)
    x = smooth_abs(x, s_l, l_cubic);
    y = smooth_abs(y, s_l, l_cubic);
    z = smooth_abs(z, s_l, l_cubic);

    // Step 2: Smooth sort (3 pairs: x-y, x-z, y-z)
    // ASM: if ESmoothing > 0 → cubic smooth, else → sqrt smooth
    smooth_sort_step(x, y, s_e, half_val, e_cubic);
    smooth_sort_step(x, z, s_e, half_val, e_cubic);
    smooth_sort_step(y, z, s_e, half_val, e_cubic);

    // Step 3: Apply rotation matrix.
    // Final x87 block loads (z, x, y), then forms rows as:
    //   rowA = x*params[10] + y*params[11] + z*params[12]
    //   rowB = x*params[4]  + y*params[5]  + z*params[6]
    //   rowC = x*params[7]  + y*params[8]  + z*params[9]
    double r0 = x * params[10] + y * params[11] + z * params[12];
    double r1 = x * params[4]  + y * params[5]  + z * params[6];
    double r2 = x * params[7]  + y * params[8]  + z * params[9];

    // Step 4: Scale + DE tracking + Menger fold
    // ASM: DE *= Scale, r1 *= Scale, r2 *= Scale (r0 NOT scaled here)
    de *= Scale;
    double k = Scale - 1.0;
    double kz = (k / Scale) * CScaleZ;

    // x = r1*Scale - k*CScaleX
    // y = r2*Scale - k*CScaleY
    double x_out = r1 * Scale - k * CScaleX;
    double y_out = r2 * Scale - k * CScaleY;

    // z-fold: smooth abs of (r0 - kz), then z = (kz - smooth_abs(r0-kz)) * Scale
    double z_val = r0 - kz;
    double sabs_z = smooth_abs(z_val, s_l, l_cubic);
    double z_out = (kz - sabs_z) * Scale;

    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_MengerSmt = {
    .id = FORMULA_MENGERSMT,
    .name = "MengerSmt",
    .param_count = 9,
    .complexity = 646,
    .uses_transcendental = true  // sqrt in smooth mode
};

/*
Original M3 Description:
A 3d menger sponge with rounded edges and organic look.
Downsides; details get (very!) quickly lost.
edit 27 october; added a different smoothing type
(visible when smoothing is less than 0) and differentiated
edge smoothing from linear (lots of effects possible now!).
Luca GN 2015

procedure MengerIFSsmooth:
  sc := 3.0; sc1 := sc-1.0; sc2 := sc1/sc;
  Cx := 1.0; Cy := 1.0; Cz := 0.5;
  s := 0.005;

  x := sqrt(x*x+s); y := sqrt(y*y+s); z := sqrt(z*z+s);

  t:=x-y; t:= 0.5*(t-sqrt(t*t+s));
  x:=x-t; y:= y+t;

  t:=x-z; t:= 0.5*(t-sqrt(t*t+s));
  x:=x-t; z:= z+t;

  t:=y-z; t:= 0.5*(t-sqrt(t*t+s));
  y:=y-t; z:= z+t;

  z := z - Cz*sc2;
  z := -sqrt(z*z+s);
  z := z + Cz*sc2;

  x := sc*x-Cx*sc1;
  y := sc*y-Cy*sc1;
  z := sc*z;
  w := sc*w;

Parameters:
  [0] Scale: double (default: 3)
  [1] CScale X: double (default: 1)
  [2] CScale Y: double (default: 1)
  [3] CScale Z: double (default: 0.5)
  [4..12] Rotation matrix: 9 floats (from .3SingleAngles Rotation = 0)
  [13] LSmoothing: double (default: 0.005, positive=cubic, negative=sqrt)
  [14] ESmoothing: double (default: 0.005, positive=cubic, negative=sqrt)
  [15] half: 0.5 from [CONSTANTS]

PVar offset layout (edi = PVar):
  [edi-0x10] = Scale
  [edi-0x18] = CScale X
  [edi-0x20] = CScale Y
  [edi-0x28] = CScale Z
  [edi-0x2c..-0x4c] = rotation matrix (9 singles)
  [edi-0x54] = LSmoothing
  [edi-0x5c] = ESmoothing
  [edi+0x00] = half (0.5 from CONSTANTS)
*/
