// _setw.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _setw
// Hand-ported from ASM: 217 bytes, DEoption=4
// 4D formula: uses de parameter as w coordinate input/output

constexpr unsigned int FORMULA_SETW = 10322;
template<> struct ExternalFormulaAlwaysWritesJuliaW<FORMULA_SETW> { static constexpr bool value = true; };

// ASM trace:
//   esi = context, edi = PVar
//   eax/ebx = &x, edx = &y, ecx = &z, [ecx+8] = w
//   Parameters:
//     [edi-0x0C] = Power mode (integer): 0 = linear, else = power mode
//     [edi-0x10] = Power (double)
//     [edi-0x1C] = X mul (double)
//     [edi-0x24] = Y mul (double)
//     [edi-0x2C] = Z mul (double)
//
//   Mode 0 (linear):
//     w = (xm*x + ym*y + zm*z) / (xm + ym + zm)
//
//   Mode != 0 (power):
//     w = ((xm*|x|^p + ym*|y|^p + zm*|z|^p) / (xm+ym+zm)) ^ p
//     (the shipped executable differs from the description below)
//
//   Result stored to [ecx+8] (w) and [esi-0x38] (Cw)
//   x, y, z remain unchanged (transform-like but DEoption=4)

template<>
cpu_double3 execute_formula<FORMULA_SETW>(cpu_double3 p, constant double* params, double &de) {
    // Integer parameter
    const int powerMode = int(params[0]);   // [PVar-0x0C]
    // Double parameters
    const double power = params[1];          // [PVar-0x10]
    const double xMul  = params[2];          // [PVar-0x1C]
    const double yMul  = params[3];          // [PVar-0x24]
    const double zMul  = params[4];          // [PVar-0x2C]

    double x = p.x, y = p.y, z = p.z;

    double w_val;
#if defined(_MSC_VER) && defined(_M_X64)
    // Preserve the two explicit Double stores and all intervening x87 values.
    // Signed weights can cancel: rounding every power to Double changes even
    // the result class (finite versus NaN) before the final exponentiation.
    double state[] = {x, y, z, power, xMul, yMul, zMul};
    SetWTransform_x87(state, powerMode);
    w_val = state[0];
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    const E weights=(E(xMul)+yMul)+zMul;
    auto pow = [&](E value) {
        return mb3d::numeric::extendedExp2(E(power)*mb3d::numeric::extendedLog2(value),false);
    };
    if (powerMode == 0) {
        w_val=double((E(x)*xMul+(E(y)*yMul+E(z)*zMul))/weights);
    } else {
        const double first=double(pow(abs(E(x)))*xMul);
        const double second=double(pow(abs(E(y)))*yMul+first);
        w_val=double(pow((pow(abs(E(z)))*zMul+second)/weights));
    }
#else
    double sumMul = xMul + yMul + zMul;

    if (powerMode == 0) {
        // Linear mode: w = (xm*x + ym*y + zm*z) / (xm + ym + zm)
        w_val = (xMul * x + yMul * y + zMul * z) / sumMul;
    } else {
        // Power mode follows the executable exponent, p.
        // ASM uses fyl2x + f2xm1 + fscale pattern = pow(|v|, p)
        double ax = x87_log_power(fabs(x), power);
        double ay = x87_log_power(fabs(y), power);
        double az = x87_log_power(fabs(z), power);
        double avg = (xMul * ax + yMul * ay + zMul * az) / sumMul;
        // The executable divides power by 1 (FDIVP), then raises avg to power.
        // Its description's reciprocal exponent does not match the shipped code.
        w_val = x87_log_power(avg, power);
    }

#endif
    // Store to w and Cw
    de = w_val;
    params[WRITE_J4_SLOT] = w_val;

    // x, y, z unchanged
    return cpu_double3(x, y, z);
}

// Formula metadata
constant FormulaInfo formula_info_setw = {
    .id = FORMULA_SETW,
    .name = "_setw",
    .param_count = 5,
    .complexity = 217,
    .uses_transcendental = true
};

/*
Original M3 Description:
Normally, w and cw are zero, so you see 4D effect indirectly.
This option sets those vars as a linear (or non linear) combination of x,y,z!
It's better use this together with Mandel bicomplex.
Will effectively work with any 4D formula, showing many new shapes/effects!

Shows similar effects as Fractint-Ultrafractal quats but in real 3D. :)
Sets w and Cw variables as follows:

in mode 0: (std average)
w := (xm*x+ym*y+zm*z)/(xm+ym+zm)
Cw := w

else (arbitrary pow average)
using absolute values of x,y,z (avoids troubles with powers):
w := ((xm*(x^p)+ym*(y^p)+zm*(z^p))/(xm+ym+zm))^(1/p)
Cw := w

Parameters:
  [0] Power mode: integer (default: 0)
  [1] Power: double (default: 2)
  [2] X mul: double (default: 1)
  [3] Y mul: double (default: 0)
  [4] Z mul: double (default: 0)

Note: The ASM also stores the result to [esi-0x38] (Cw).
In the engine, this should be handled by the formula dispatch layer
for DEoption=4 formulas.
*/
