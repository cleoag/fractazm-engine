// SphereIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: SphereIFS
// Hand-ported from ASM: 184 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS transform + SSE2 ellipsoid DE)
// Ellipsoid with axis scaling, inside radius (hollow), and OTrap writeback

constexpr unsigned int FORMULA_SPHEREIFS = 5622;

template<>
cpu_double3 execute_formula_dIFS<FORMULA_SPHEREIFS>(cpu_double3 p, double* params,
                                                      double &varyScale, double &rout) {
    const double Radius = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Z_size_recip = params[5];
    const double Y_size_recip = params[6];
    const double X_size_recip = params[7];
    const int Apply_scale_add = int(params[8]);
    const double Inside_radius = params[12];

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        varyScale *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
    } else {
        params[DIFS_COMMIT_XYZ_SLOT] = 0.0;
    }

    double sx = x * X_size_recip;
    double sy = y * Y_size_recip;
    double sz = z * Z_size_recip;

    double r_scaled = sqrt(sx * sx + sy * sy + sz * sz);

    double outer_dist = r_scaled - Radius;
    double inner_dist = Inside_radius - r_scaled;
    double sphere_dist = max(outer_dist, inner_dist);

    double max_size = max(abs(X_size_recip), max(abs(Y_size_recip), abs(Z_size_recip)));
    double shape_de = sphere_dist / max_size;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    rout = shape_de;

    // OTrap writeback: compute Dfree1 value for IFS color tracking
    // ASM: source_value * OTrap_scale + OTrap_offset → Dfree1
    // Source: 0=RStopD, 1=x(after transform), 2=y, 3=z
    // Store result in params[MAX_FORMULA_PARAMS-1] for wrapper to write to ext->Dfree1
    {
        int otrap_opt = int(params[9]) & 3;
        float otrap_scale = static_cast<float>(params[11]);
        float otrap_offset = static_cast<float>(params[10]);
        double source;
        switch (otrap_opt) {
            default:
            case 0: source = params[MAX_FORMULA_PARAMS - 2]; break;  // RStopD passed by wrapper
            case 1: source = x; break;
            case 2: source = y; break;
            case 3: source = z; break;
        }
        params[MAX_FORMULA_PARAMS - 1] = source * otrap_scale + otrap_offset;
    }

    return cpu_double3(x, y, z);
}

template<>
cpu_double3 execute_formula<FORMULA_SPHEREIFS>(cpu_double3 p, double* params, double &de) {
    double vs = 1.0, rout = 0.0;
    auto r = execute_formula_dIFS<FORMULA_SPHEREIFS>(p, params, vs, rout);
    de = rout;
    return r;
}

// Alternative direct function implementation
cpu_double3 SphereIFS_formula(cpu_double3 p, double* params, double &de) {
    return execute_formula<FORMULA_SPHEREIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_SphereIFS = {
    .id = FORMULA_SPHEREIFS,
    .name = "SphereIFS",
    .param_count = 13,
    .complexity = 220,
    .uses_transcendental = false
};

/*
Original M3 Description:
Sphere dIFS with OTrap coloring options:

0: Change with DEstop
1: Change with X
2: Change with Y
3: Change with Z

DEstop might vary on distance, choose OTrap_scale = 0 to switch off.

Choose Inside radius a little lower than Radius to make a hollow sphere
for transparent or inside rendering.

Mathematical Formula:
  DEoption=20 (dIFS shape — SSE2 IFS + SSE2 ellipsoid DE):
    - Standard SSE2 IFS preamble
    - Axis scaling via DRecipro (reciprocal) parameters
    - Scaled radius: r = sqrt((x*Xr)² + (y*Yr)² + (z*Zr)²)
    - Hollow sphere: max(r - Radius, Inside_radius - r)
    - Lipschitz correction: divide by max(|Xr|, |Yr|, |Zr|)
    - Entirely SSE2 computation (no x87 for shape, only for OTrap)

  Step 1: IFS transform:
    x = p.x * Scale + X_add;  y = p.y * Scale + Y_add;  z = p.z * Scale + Z_add
    if Apply_scale_add: w *= Scale

  Step 2: Axis scaling:
    sx = x * X_size_recip;  sy = y * Y_size_recip;  sz = z * Z_size_recip

  Step 3: Scaled radius:
    r_scaled = sqrt(sx² + sy² + sz²)

  Step 4: Sphere distance:
    sphere_dist = max(r_scaled - Radius, Inside_radius - r_scaled)

  Step 5: Lipschitz correction:
    shape_de = sphere_dist / max(|X_size_recip|, |Y_size_recip|, |Z_size_recip|)

Parameters:
  [0] Radius: double (default: 1) — outer sphere radius
  [1] Scale: double (default: 2) — IFS scale factor (.2Doubles)
  [2] Z_add: double (default: 0) — Z offset (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 3) — X offset (.2Doubles)
  [5] Z_size_recip: DRecipro (default: 1) — Z axis reciprocal scale
  [6] Y_size_recip: DRecipro (default: 1) — Y axis reciprocal scale
  [7] X_size_recip: DRecipro (default: 1) — X axis reciprocal scale
  [8] Apply_scale_add: int (default: 1)
  [9] OTrap_option: int (default: 0) — coloring source (0..3)
  [10] OTrap_offset: single (default: 1)
  [11] OTrap_scale: single (default: 1)
  [12] Inside_radius: double (default: -1) — inner radius (< 0 = solid)

PVar layout (verified against SSE2 ASM offsets):
  Constants (positive offsets):
  [PVar + 0x00] = $7FFFFFFFFFFFFFFF (abs mask, 16 bytes)

  Options (negative offsets):
  [PVar - 0x10] = Radius (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (.2Doubles)
  [PVar - 0x50] = Z_size_recip (DRecipro)
  [PVar - 0x58] = Y_size_recip (DRecipro)
  [PVar - 0x60] = X_size_recip (DRecipro, packed with Y for movapd)
  [PVar - 0x64] = Apply_scale_add (integer)
  [PVar - 0x68] = OTrap_option (integer)
  [PVar - 0x6c] = OTrap_offset (single)
  [PVar - 0x70] = OTrap_scale (single)
  [PVar - 0x78] = Inside_radius (double)

Code structure:
  0x00-0x3C: SSE2 IFS preamble
  0x3D-0x4B: Load axis reciprocals, scale coordinates
  0x4-0x63: Square, sum, sqrt → r_scaled
  0x67-0x75: Hollow sphere: max(r-R, IR-r)
  0x79-0x8D: Lipschitz: max(abs(recips)) → divide
  0x91-0x9: Scale correction + store DE
  0xA4-0xB7: OTrap computation + return

Key notes:
  - Entirely SSE2 for shape computation (no x87 stack)
  - DRecipro params: user enters size, stored as 1/size for multiplication
  - When Inside_radius < 0 (default): inner_dist always negative → solid sphere
  - Lipschitz correction essential for non-uniform scaling (ellipsoid)
  - max(abs(recips)) ensures conservative DE even with extreme axis ratios
  - No stack allocation needed (no add esp)
  - unpckhpd used to extract high double from packed pair
  - 184 bytes — compact SSE2-only ellipsoid formula
*/
