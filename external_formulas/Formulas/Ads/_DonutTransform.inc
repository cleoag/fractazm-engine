// _DonutTransform.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _DonutTransform
// Hand-ported from ASM: 211 bytes, DEoption=-1

constexpr unsigned int FORMULA__DONUTTRANSFORM = 10008;
template<> struct ExternalFormulaWritesWState<FORMULA__DONUTTRANSFORM> { static constexpr bool value = true; };
template<> struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA__DONUTTRANSFORM> { static constexpr bool value = true; };
template<> struct ExternalFormulaAlwaysWritesJuliaW<FORMULA__DONUTTRANSFORM> { static constexpr bool value = true; };


// ASM trace:
//   esi = context, edi = PVar, ebx = &x
//   Params: [edi-0x10]=Offset, [edi-0x18]=ROffset (delta), [edi-0x20]=Order
//   Local vars: [ebp-0x10]=angle(f32), [ebp-0x18]=radius(f32), [ebp-0x20]=t_radius(f32)
//
//   --- Transform coordinates ---
//   fld [ebx]; fld [edx]; fpatan     ; angle = atan2(x, y) * Order
//   fmul [edi-0x20]; fstp [ebp-0x10]
//
//   fld [ebx]; fmul st0; fld [edx]; fmul st0; faddp; fsqrt
//   fst [ebp-0x18]                   ; radius = sqrt(x²+y²)
//   fsub [edi-0x10]; fstp [ebp-0x20] ; t_radius = radius - Offset
//
//   fld angle; fsincos               ; cos, sin
//   fld radius; fmul st(2), st(0)    ; sin *= radius
//   fmulp st(1)                      ; cos *= radius
//   fld ROffset; fsub st(2), st(0)   ; sin*r -= ROffset
//   fsubp st(1)                      ; cos*r -= ROffset
//   fstp [edx]                       ; y' = cos*radius - ROffset
//   fstp [ebx]                       ; x' = sin*radius - ROffset
//
//   w' = t_radius² + z² - ROffset     (stored at [ecx+8])
//   z' = z - ROffset
//
//   --- Same for C values ---
//
// Toroidal transform (3D→4D) by Aexion.

template<>
cpu_double3 execute_formula<FORMULA__DONUTTRANSFORM>(cpu_double3 p, constant double* params, double &de) {
    const double Offset  = params[0];  // default: 4
    const double ROffset = params[1];  // default: 0.5 (delta)
    const double Order   = params[2];  // default: 5

    // Toroidal transform of x/y plane
    float angle = float(atan2(p.x, p.y) * Order);
    double full_radius = sqrt(p.x * p.x + p.y * p.y);
    float radius = float(full_radius);
    float t_radius = float(full_radius - Offset);

    double s = sin(double(angle));
    double c = cos(double(angle));

    double new_x = s * radius - ROffset;
    double new_y = c * radius - ROffset;
    double new_z = p.z - ROffset;
    params[FORMULA_STATE_W_SLOT] = (p.z * p.z - ROffset) + double(t_radius) * double(t_radius);

    const double cx = params[3], cy = params[4], cz = params[5];
    const float cangle = float(atan2(cx, cy) * Order);
    const double cfull_radius = sqrt(cx * cx + cy * cy);
    const float cradius = float(cfull_radius);
    const float ctradius = float(cfull_radius - Offset);
    params[WRITE_J1_SLOT] = sin(double(cangle)) * cradius - ROffset;
    params[WRITE_J2_SLOT] = cos(double(cangle)) * cradius - ROffset;
    params[WRITE_J3_SLOT] = cz - ROffset;
    params[WRITE_J4_SLOT] = (cz * cz - ROffset) + double(ctradius) * double(ctradius);

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__donuttransform = {
    .id = FORMULA__DONUTTRANSFORM,
    .name = "_DonutTransform",
    .param_count = 3,
    .complexity = 70,
    .uses_transcendental = true
};

/*
Original M3 Description:
Based on Aexions ideas, this performs a toroidal transform to be used
as first formula as pre-transform, and choosing 'repeat from here'
on the next formula(s).

It is a 3d transform, but the result is 4d, so a combination with
4d formulas would make a better sense (like the 4D ABox).

angle = atan2(x, y) * Order
radius = sqrt(x² + y²)
t_radius = radius - Offset
x' = sin(angle) * radius - ROffset
y' = cos(angle) * radius - ROffset
w' = t_radius² + z² - ROffset
z' = z - ROffset

Same pattern applied to C values.

Note: ASM computes atan2(x,y) not atan2(y,x). The fpatan instruction
with st1=x, st0=y gives atan2(x,y). This matches the code.

Parameters:
  [0] Offset: double (default: 4)
  [1] ROffset: double (default: 0.5)
  [2] Order: double (default: 5)
*/
