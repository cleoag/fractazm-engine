// Barnsley2_4Dnc.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Barnsley2_4Dnc
// Hand-ported from ASM: 316 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output
// Non-commutative (Hamilton) quaternion product version

constexpr unsigned int FORMULA_BARNSLEY2_4DNC = 10328;

// ASM trace:
//   Identical structure to Barnsley2_4Dc except the quaternion multiplication
//   uses Hamilton (non-commutative) product instead of commutative.
//
//   The condition check and S +/- K is the same.
//   The quaternion product is:
//     A q* B := (AxBx - AyBy - AzBz - AwBw,
//                AxBy + AyBx + AzBw - AwBz,
//                AxBz - AyBw + AzBx + AwBy,
//                AxBw + AyBz - AzBy + AwBx)
//
//   Verification from ASM quaternion multiply section:
//     x' component (0x87-0xAD):
//       x*Cx + y*Cy (fchs→-y*Cy, but then faddp, so x*Cx - y*Cy)
//       - z*Cz (fchs; faddp)
//       - w*Cw (fchs; faddp)  ← different from commutative (+w*Cw)
//       = x*Cx - y*Cy - z*Cz - w*Cw
//
//     y' component (0xAF-0xD5):
//       y*Cx + x*Cy + z*Cw (no fchs) - w*Cz (fchs)
//       Wait let me check ASM: at 0xAF: fld [ebx]; fmul [esi+0x20] = x*Cy
//       0xB7: fld [edx]; fmul [esi+0x18] = y*Cx; faddp → x*Cy + y*Cx
//       0xC1: fld [ecx]; fmul [esi-0x38] = z*Cw; faddp → + z*Cw
//       Wait: [esi-0x38] = Cw. So z*Cw.
//       0xCB: fld [ecx+8]; fmul [esi+0x28] = w*Cz; fchs; faddp → - w*Cz
//       y' = x*Cy + y*Cx + z*Cw - w*Cz
//
//     z' component (0xD7-0xFD):
//       x*Cz + y*Cw(fchs→-y*Cw) + z*Cx + w*Cy
//       Wait: 0xD7: fld [ebx]; fmul [esi+0x28] = x*Cz
//       0xDF: fld [edx]; fmul [esi-0x38] = y*Cw; fchs → -y*Cw; faddp
//       0xE9: fld [ecx]; fmul [esi+0x18] = z*Cx; faddp
//       0xF3: fld [ecx+8]; fmul [esi+0x20] = w*Cy; faddp
//       z' = x*Cz - y*Cw + z*Cx + w*Cy
//
//     w' component (0xFF-0x125):
//       x*Cw + y*Cz + z*Cy(fchs→-z*Cy) + w*Cx
//       Wait: 0xFF: fld [ebx]; fmul [esi-0x38] = x*Cw
//       0x107: fld [edx]; fmul [esi+0x28] = y*Cz; faddp
//       0x111: fld [ecx]; fmul [esi+0x20] = z*Cy; fchs → -z*Cy; faddp
//       0x11B: fld [ecx+8]; fmul [esi+0x18] = w*Cx; faddp
//       w' = x*Cw + y*Cz - z*Cy + w*Cx
//
//   This is Hamilton quaternion product (xyzw) q* C:
//     x' = x*Cx - y*Cy - z*Cz - w*Cw
//     y' = x*Cy + y*Cx + z*Cw - w*Cz
//     z' = x*Cz - y*Cw + z*Cx + w*Cy
//     w' = x*Cw + y*Cz - z*Cy + w*Cx

template<>
cpu_double3 execute_formula<FORMULA_BARNSLEY2_4DNC>(cpu_double3 p, constant double* params, double &de) {
    // Parameters (identical layout to 4Dc)
    const double dx       = params[0];
    const double dy       = params[1];
    const double dz       = params[2];
    const double dw       = params[3];
    const double w_add    = params[4];
    const double xCy_mul  = params[5];
    const double yCx_mul  = params[6];
    const double threshold= params[7];

    // C values
    const double Cx = params[8];
    const double Cy = params[9];
    const double Cz = params[10];
    const double Cw = params[11];

    double x = p.x, y = p.y, z = p.z, w = de;

    // Condition check (same as commutative version)
    double condition = x * Cy * xCy_mul + y * Cx * yCx_mul;

    if (condition >= threshold) {
        x -= dx;
        y -= dy;
        z -= dz;
        w = w - dw + w_add;
    } else {
        x += dx;
        y += dy;
        z += dz;
        w += dw + w_add;
    }

    // Hamilton (non-commutative) quaternion multiply: S' = S q* C
    // A q* B := (AxBx - AyBy - AzBz - AwBw,
    //            AxBy + AyBx + AzBw - AwBz,
    //            AxBz - AyBw + AzBx + AwBy,
    //            AxBw + AyBz - AzBy + AwBx)
    double nx = x*Cx - y*Cy - z*Cz - w*Cw;
    double ny = x*Cy + y*Cx + z*Cw - w*Cz;
    double nz = x*Cz - y*Cw + z*Cx + w*Cy;
    double nw = x*Cw + y*Cz - z*Cy + w*Cx;

    de = nw;
    return cpu_double3(nx, ny, nz);
}

// Formula metadata
constant FormulaInfo formula_info_Barnsley2_4Dnc = {
    .id = FORMULA_BARNSLEY2_4DNC,
    .name = "Barnsley2_4Dnc",
    .param_count = 12,
    .complexity = 316,
    .uses_transcendental = false
};

/*
Original M3 Description:
This is a 4-D extension of the (generalized) Barnsley conditional formula.
Uses the Hamilton non-commutative quat product (S*C, not C*S).

Barnsley 2 non-commutative 4D formula (xyzw q* C version)
S is the quaternion (x,y,z,w+wadd).
K is the quaternion (dx,dy,dz,dw).
C is the quaternion (Cx,Cy,Cz,Cw).

Hamilton quat multiplication:
A q* B := (AxBx-AyBy-AzBz-AwBw, AxBy+AyBx+AzBw-AwBz,
           AxBz-AyBw+AzBx+AwBy, AxBw+AyBz-AzBy+AwBx)

if (mul * Sx * Cy + mul * Sy * Cx > Threshold)
  S' = (S + K) q* C
else
  S' = (S - K) q* C

Parameters:
  [0] dx: double (default: 1)
  [1] dy: double (default: 0)
  [2] dz: double (default: 0)
  [3] dw: double (default: 0)
  [4] w add: double (default: 0)
  [5] xCy mul: double (default: 1)
  [6] yCx mul: double (default: 1)
  [7] Threshold: double (default: 0)
  [8..11] Cx, Cy, Cz, Cw (engine-provided)
*/
