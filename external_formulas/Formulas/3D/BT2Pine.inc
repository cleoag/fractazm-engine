// BT2Pine.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: BT2Pine
// Hand-ported from ASM: 313 bytes, DEoption=0
// Benesi Transform 2 + Pine Tree Mandelbulb
// STEP 1: "Benesi fold 2" rotation → squaring → |sqrt()-offset|*scale folding → inverse rotation
// STEP 2: "Benesi pinetree" power-2 iteration with C handling via pixel_x/y/z

constexpr unsigned int FORMULA_BT2PINE = 129;

// ASM trace (313 bytes, branchless):
//
// STEP 1: Benesi fold 2
//   Let c1 = sqrt(2/3), s1 = sqrt(1/3), h = sqrt(1/2)
//
//   Forward rotation:
//     tx = (x*c1 - z*s1) * h
//     y  = y * h
//     z_sq = (x*s1 + z*c1)^2
//
//   Squaring:
//     ty_sq = (tx + y)^2
//     tx_sq = (tx - y)^2
//
//   Fold (|sqrt() - offset| * scale):
//     x = |sqrt(ty_sq + z_sq) - xOff| * Scale * h
//     y = |sqrt(tx_sq + z_sq) - xOff| * Scale * h
//     z = |sqrt(tx_sq + ty_sq) - xOff| * Scale
//
//   Inverse rotation:
//     tmp = y + x;  y = y - x
//     x = tmp*c1 + z*s1
//     z = z*c1 - tmp*s1
//
// STEP 2: Benesi pinetree (power-2)
//     xt = x^2, yt = y^2, zt = z^2
//     t = 2*x / sqrt(yt + zt)
//     x = xt - yt - zt + Cx * pixel_x
//     y = 2*t*z*y + Cy * pixel_z
//     z = t*(yt - zt) + Cz * pixel_y
//
// C handling: Formula handles C internally (DEoption=0).
//   pixel_x/y/z select which C components contribute.
//   Default (1,0,0): only Cx added to x output.

template<>
cpu_double3 execute_formula<FORMULA_BT2PINE>(cpu_double3 p, constant double* params, double &de) {
    const double Scale   = params[0];   // [PVar-0x10] (default: 2.0)
    const double xOff    = params[1];   // [PVar-0x18] (default: 2.0)
    const double pixel_x = params[2];   // [PVar-0x20] (default: 1.0)
    const double pixel_y = params[3];   // [PVar-0x28] (default: 0.0)
    const double pixel_z = params[4];   // [PVar-0x30] (default: 0.0)
    // [CONSTANTS] embedded as compile-time values (positive PVar offsets):
    const double c1      = 0.816496580927726;   // [PVar+0x00] sqrt(2/3)
    const double s1      = 0.577350269189626;   // [PVar+0x08] sqrt(1/3)
    const double h       = 0.707106781186548;   // [PVar+0x10] sqrt(1/2)
    // C values passed by engine from context
    const double Cx      = params[8];   // from engine context [esi+0x18]
    const double Cy      = params[9];   // from engine context [esi+0x20]
    const double Cz      = params[10];  // from engine context [esi+0x28]

    double x = p.x, y = p.y, z = p.z;

    // ===== STEP 1: Benesi fold 2 =====

    // Forward rotation
    double tx = (x * c1 - z * s1) * h;
    y = y * h;
    double z_sq = (x * s1 + z * c1);
    z_sq = z_sq * z_sq;  // squared

    // Squaring of (tx ± y)
    double ty_sq = (tx + y) * (tx + y);
    double tx_sq = (tx - y) * (tx - y);

    // Fold: |sqrt(sum) - offset| * scale
    x = fabs(sqrt(ty_sq + z_sq) - xOff) * Scale * h;
    y = fabs(sqrt(tx_sq + z_sq) - xOff) * Scale * h;
    z = fabs(sqrt(tx_sq + ty_sq) - xOff) * Scale;

    // Inverse rotation
    double tmp = y + x;
    y = y - x;
    x = tmp * c1 + z * s1;
    z = z * c1 - tmp * s1;

    // ===== STEP 2: Benesi pinetree (power-2) =====

    double xt = x * x;
    double yt = y * y;
    double zt = z * z;

    double t = 2.0 * x / sqrt(yt + zt);

    double new_x = xt - yt - zt + Cx * pixel_x;
    double new_y = 2.0 * t * z * y + Cy * pixel_z;
    double new_z = t * (yt - zt) + Cz * pixel_y;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_BT2Pine = {
    .id = FORMULA_BT2PINE,
    .name = "BT2Pine",
    .param_count = 8,
    .complexity = 313,
    .uses_transcendental = true
};

/*
Original M3 Description:
Transform 2 then Pine Tree Mandelbulb.
Formula translated to Mandelbulb 3D by Luca.
  M Benesi

// STEP1: "Benesi fold 2"
tx=(x*sqrt(2/3)-z*sqrt(1/3))*sqrt(1/2);
y=y*sqrt(1/2);
z=(x*sqrt(1/3) + z*sqrt(2/3))^2;
tx_sq=(tx-y)^2;
ty_sq=(tx+y)^2;

x=abs(sqrt(ty_sq+z)-offset)*scale*sqrt(1/2) ;
y=abs(sqrt(tx_sq+z)-offset)*scale*sqrt(1/2) ;
z=abs(sqrt(tx_sq+ty_sq)-offset)*scale ;

tmp=y+x; y=y-x;
x=tmp*sqrt(2/3)+z*sqrt(1/3);
z=z*sqrt(2/3)-tmp*sqrt(1/3);

// STEP2: "Benesi pinetree"
xt = x*x; yt = y*y; zt = z*z;
t = 2*x/sqrt(yt+zt);
x = xt-yt-zt + Cx*pixel_x;
y = 2*t*y*z  + Cy*pixel_z;
z = t*(yt-zt) + Cz*pixel_y;

C handling: NON-STANDARD — pixel_x/y/z select C contributions.
Engine should NOT add C; template handles it.

Parameters:
  [0] Scale: double (default: 2.0) - fold scale factor
  [1] xOff: double (default: 2.0) - fold offset
  [2] pixel_x: double (default: 1.0) - Cx contribution to x
  [3] pixel_y: double (default: 0.0) - Cz contribution to z
  [4] pixel_z: double (default: 0.0) - Cy contribution to y
  [5] sqrt(2/3): double (default: 0.816496580927726) - rotation constant
  [6] sqrt(1/3): double (default: 0.5773502691896258) - rotation constant
  [7] sqrt(1/2): double (default: 0.7071067811865475) - rotation constant
  [8..10] Cx, Cy, Cz: passed by engine from context
*/
