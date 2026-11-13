// _FoldingInfy.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _FoldingInfy
// Hand-ported from ASM: 296 bytes, DEoption=-1

constexpr unsigned int FORMULA__FOLDINGINFY = 10015;

// ASM trace:
//   For each axis i (w, z, y, x) with fold size n = param[i]:
//     if (n != 0):
//       m = n * 4                       (fadd st0,st0 twice = *4)
//       t = xi - round(xi / m) * m      (fld xi; fld st0; fdiv m; frndint; fmul m; fsubp)
//       xi = |t + n| - |t - n| - t      (fchs; fld st0; fchs; fadd n; fabs; fld st1; fchs; fsub n; fabs; fchs; faddp; faddp)
//
//   Parameter layout (esi = PVar after mov esi,[esi+0x30]):
//     [esi-0x10] = x Fold (1st param, double)
//     [esi-0x18] = y Fold (2nd param, double)
//     [esi-0x20] = z Fold (3rd param, double)
//     [esi-0x28] = w Fold (4th param, double)
//
//   Coordinate writes:
//     [ecx+8] = w (DE slot, skipped for 3D)
//     [ecx]   = z
//     [edx]   = y
//     [ebx]   = x

template<>
cpu_double3 execute_formula<FORMULA__FOLDINGINFY>(cpu_double3 p, constant double* params, double &de) {
    // Parameters: params[0] = x Fold, params[1] = y Fold,
    //             params[2] = z Fold, params[3] = w Fold
    double xFold = params[0];  // [PVar-0x10] = x Fold, default 5.0
    double yFold = params[1];  // [PVar-0x18] = y Fold, default 5.0
    double zFold = params[2];  // [PVar-0x20] = z Fold, default 5.0
    // params[3] = w Fold (4th param, operates on [ecx+8] = w/DE slot, skipped in 3D)

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // Fold z axis (3rd param = zFold)
    if (zFold != 0.0) {
        double m = zFold * 4.0;
        double n = zFold;
        double t = z - round(z / m) * m;
        z = fabs(t + n) - fabs(t - n) - t;
    }

    // Fold y axis (2nd param = yFold)
    if (yFold != 0.0) {
        double m = yFold * 4.0;
        double n = yFold;
        double t = y - round(y / m) * m;
        y = fabs(t + n) - fabs(t - n) - t;
    }

    // Fold x axis (1st param = xFold)
    if (xFold != 0.0) {
        double m = xFold * 4.0;
        double n = xFold;
        double t = x - round(x / m) * m;
        x = fabs(t + n) - fabs(t - n) - t;
    }

    return cpu_double3(x, y, z);
}

// Formula metadata
constant FormulaInfo formula_info__FoldingInfy = {
    .id = FORMULA__FOLDINGINFY,
    .name = "_FoldingInfy",
    .param_count = 4,
    .complexity = 296,
    .uses_transcendental = false
};

/*
Original M3 Description:

Continuous, seamless folding of 4D space. Boxtiling "enhanced" version.

xi = every dimension (x,y,z,w)
i = 1 to 4
 m = sizei*4; n = sizei; t = 0
 if sizei <> 0
  t = xi - frndint(xi/m)*m // boxtiling (where xi is x,y,... etc. )
  xi = abs(t+n) - abs(t-n) - t // folding
 endif
next i

frndint -> rounds a double to the nearest integer

Parameters:
  params[0] = x Fold (double, default 5.0)
  params[1] = y Fold (double, default 5.0)
  params[2] = z Fold (double, default 5.0)
  params[3] = w Fold (double, default 5.0) - operates on w/DE slot, skipped in 3D

Note: The w fold operates on [ecx+8] which is the DE/w slot. In 3D mode
this is omitted. The formula processes axes in order: w, z, y, x in the ASM.

ASM verification: xi = |t+n| - |t-n| - t, matching the description exactly.
In Metal: fabs(t + n) - fabs(t - n) - t
*/
