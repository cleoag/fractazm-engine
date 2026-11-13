// Barnsley2_4Dc.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Barnsley2_4Dc
// Hand-ported from ASM: 316 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output
// Commutative quaternion product version

constexpr unsigned int FORMULA_BARNSLEY2_4DC = 10327;

// ASM trace:
//   esi = context, edi = PVar, ebx = &x
//   eax/ebx = &x, edx = &y, ecx = &z, [ecx+8] = w
//   Parameters:
//     [edi-0x10] = dx (double, default: 1)
//     [edi-0x18] = dy (double, default: 0)
//     [edi-0x20] = dz (double, default: 0)
//     [edi-0x28] = dw (double, default: 0)
//     [edi-0x30] = w_add (double, default: 0)
//     [edi-0x38] = xCy_mul (double, default: 1) — [edi-0x38] = 6th param
//     [edi-0x40] = yCx_mul (double, default: 1) — [edi-0x40] = 7th param
//     [edi-0x48] = Threshold (double, default: 0)
//   C values: [esi+0x18]=Cx, [esi+0x20]=Cy, [esi+0x28]=Cz, [esi-0x38]=Cw
//
//   Condition check:
//     x*Cy*xCy_mul + y*Cx*yCx_mul > Threshold ?
//     ASM: x*Cy*[edi-0x38] + y*Cx*[edi-0x40] compared with [edi-0x48]
//     FCOMPP then ja → if Threshold > condition, jump to subtract branch
//     Wait: fcompp compares st0 vs st1. st0 = Threshold, st1 = condition
//     If Threshold <= condition (i.e. condition >= Threshold): ja NOT taken → add branch
//     If Threshold > condition: ja taken → subtract branch
//
//   if condition >= Threshold:
//     S = S + K  (x+=dx, y+=dy, z+=dz, w+=dw+w_add)
//   else:
//     S = S - K  (x-=dx, y-=dy, z-=dz, w-=dw but still w+=w_add)
//     Wait: ASM at 0x77-0x80: w = w - dw + w_add
//
//   Then commutative quaternion multiply S' = S q* C:
//     Using: A q* B = (AxBx-AyBy-AzBz+AwBw, AyBx+AxBy-AwBz-AzBw,
//                      AzBx-AwBy+AxBz-AyBw, AwBx+AzBy+AyBz+AxBw)
//
//   ASM builds 4 components on stack, stores in reverse: w,z,y,x

template<>
cpu_double3 execute_formula<FORMULA_BARNSLEY2_4DC>(cpu_double3 p, constant double* params, double &de) {
    // Parameters
    const double dx       = params[0];   // [PVar-0x10]
    const double dy       = params[1];   // [PVar-0x18]
    const double dz       = params[2];   // [PVar-0x20]
    const double dw       = params[3];   // [PVar-0x28]
    const double w_add    = params[4];   // [PVar-0x30]
    const double xCy_mul  = params[5];   // [PVar-0x38]
    const double yCx_mul  = params[6];   // [PVar-0x40]
    const double threshold= params[7];   // [PVar-0x48]

    // C values
    const double Cx = params[8];
    const double Cy = params[9];
    const double Cz = params[10];
    const double Cw = params[11];

    double x = p.x, y = p.y, z = p.z, w = de;

    // Condition: x*Cy*xCy_mul + y*Cx*yCx_mul vs Threshold
    double condition = x * Cy * xCy_mul + y * Cx * yCx_mul;

    if (condition >= threshold) {
        // S = S - K
        x -= dx;
        y -= dy;
        z -= dz;
        w = w - dw + w_add;
    } else {
        // S = S + K
        x += dx;
        y += dy;
        z += dz;
        w += dw + w_add;
    }

    // Commutative quaternion multiply: S' = S q* C
    // A q* B := (AxBx - AyBy - AzBz + AwBw,
    //            AyBx + AxBy - AwBz - AzBw,
    //            AzBx - AwBy + AxBz - AyBw,
    //            AwBx + AzBy + AyBz + AxBw)
    double nx = x*Cx - y*Cy - z*Cz + w*Cw;
    double ny = y*Cx + x*Cy - w*Cz - z*Cw;
    double nz = z*Cx - w*Cy + x*Cz - y*Cw;
    double nw = w*Cx + z*Cy + y*Cz + x*Cw;

    de = nw;
    return cpu_double3(nx, ny, nz);
}

// Formula metadata
constant FormulaInfo formula_info_Barnsley2_4Dc = {
    .id = FORMULA_BARNSLEY2_4DC,
    .name = "Barnsley2_4Dc",
    .param_count = 12,
    .complexity = 316,
    .uses_transcendental = false
};

/*
Original M3 Description:
This is a 4-D extension of the (generalized) Barnsley 2 conditional formula.
Uses a commutative quat product.

S is the quaternion (x,y,z,w+wadd).
K is the quaternion (dx,dy,dz,dw).
C is the quaternion (Cx,Cy,Cz,Cw).
Commutative quat multiplication:
A q* B := (AxBx-AyBy-AzBz+AwBw, AyBx+AxBy-AwBz-AzBw,
           AzBx-AwBy+AxBz-AyBw, AwBx+AzBy+AyBz+AxBw)

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
