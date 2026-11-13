// _PartlyJuliaRoff.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _PartlyJuliaRoff
// Hand-ported from ASM: 133 bytes, DEoption=-1

constexpr unsigned int FORMULA__PARTLYJULIAROFF = 10005;
template<> struct ExternalFormulaAppendsPersistentCLanes<FORMULA__PARTLYJULIAROFF> { static constexpr bool value = true; };
template<> struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA__PARTLYJULIAROFF> { static constexpr bool value = true; };

// ASM trace:
//   esi = context (esi[0]=Px), edi = PVar
//   Params: [edi-0x10]=X_sphere, [edi-0x18]=Y_sphere, [edi-0x20]=Z_sphere,
//           [edi-0x28]=Radius_sphere
//   Constants: [edi+0x00]=1e-10 (epsilon)
//
//   xt = Px - X_sphere
//   yt = Py - Y_sphere
//   zt = Pz - Z_sphere
//   r = sqrt(xt² + yt² + zt²)
//
//   fcom [edi-0x28]    ; compare r with Radius
//   jne cleanup        ; if r <= Radius → skip
//
//   rt = r - Radius
//   if |r| < 1e-10:
//     r_safe = sign(r) * 1e-10
//   else:
//     r_safe = r
//   m = rt / r_safe     ; (DE F1 = fdivrp: st(1)=st(0)/st(1)=rt/r)
//   Cx += xt * m
//   Cy += yt * m
//   Cz += zt * m
//
// Disables Julia mode outside a sphere radius.
// Position is NOT modified. Only C values change.

template<>
cpu_double3 execute_formula<FORMULA__PARTLYJULIAROFF>(cpu_double3 p, constant double* params, double &de) {
    const double X_sphere      = params[0];  // default: 0
    const double Y_sphere      = params[1];  // default: 0
    const double Z_sphere      = params[2];  // default: 0
    const double Radius_sphere = params[3];  // default: 0

    // C values from engine (input/output)
    // params[4] is the embedded epsilon constant from +0x00 in the formula buffer.
    // Appended Julia lanes start after the mapped params; saved P follows extended state.
    double Cx = params[5];
    double Cy = params[6];
    double Cz = params[7];

    double xt = params[11] - X_sphere;
    double yt = params[12] - Y_sphere;
    double zt = params[13] - Z_sphere;
    double r = sqrt(xt * xt + yt * yt + zt * zt);

    if (r > Radius_sphere) {
        double rt = r - Radius_sphere;
        double r_safe = (fabs(r) < 1e-10) ? copysign(1e-10, r) : r;
        double m = rt / r_safe;
        Cx += xt * m;
        Cy += yt * m;
        Cz += zt * m;
    }
    params[WRITE_J1_SLOT] = Cx;
    params[WRITE_J2_SLOT] = Cy;
    params[WRITE_J3_SLOT] = Cz;

    // Position unchanged
    return p;
}

// Formula metadata
constant FormulaInfo formula_info__partlyjuliaroff = {
    .id = FORMULA__PARTLYJULIAROFF,
    .name = "_PartlyJuliaRoff",
    .param_count = 4,
    .complexity = 60,
    .uses_transcendental = false
};

/*
Original M3 Description:
A pretransform to disable the julia mode outside a spheres radius.
Julia offsets are kept, so a real non-julia mode will usually not appear.

xt = Px - X_sphere
yt = Py - Y_sphere
zt = Pz - Z_sphere
r = Sqrt(Sqr(xt) + Sqr(yt) + Sqr(zt))
if r > Radius_sphere {
  rt = r - Radius_sphere;
  if Abs(r) < 1e-10: r = sign(r) * 1e-10
  m = rt / r
  Cx = Cx + xt * m
  Cy = Cy + yt * m
  Cz = Cz + zt * m
}

There will also be some strange boxes if used in non-julia mode.
Thanks to Luca for the idea of using a radius.

Parameters:
  [0] X_sphere: double (default: 0)
  [1] Y_sphere: double (default: 0)
  [2] Z_sphere: double (default: 0)
  [3] Radius_sphere: double (default: 0)
*/
