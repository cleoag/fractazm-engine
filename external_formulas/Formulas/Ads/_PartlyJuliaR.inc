// Original _PartlyJuliaR.m3f x86 contract: modify Julia C and optionally
// recenter xyz. No implicit wrapper-side addition of C.
constexpr unsigned int FORMULA__PARTLYJULIAR = 10021;

template<>
struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA__PARTLYJULIAR> { static constexpr bool value = true; };

template<>
cpu_double3 execute_formula<FORMULA__PARTLYJULIAR>(cpu_double3 p, constant double* params, double &de) {
    const double dx = p.x-params[0], dy = p.y-params[1], dz = p.z-params[2];
    const double radius = params[3], speed = params[4], recenter = params[9];
    const int mode = int(params[8]), shape = int(params[10]);
    double r;
    // Negative-side shortcuts jump past division by radius in the original.
    // Equality (dz=0) and unordered values continue into the division.
    if ((shape == -2 || shape == -4) && dz < 0) r = 1;
    else {
        if (shape == 1) {
            const double ax = fabs(dx), ay = fabs(dy), az = fabs(dz);
            const double yz = az > ay ? az : ay;
            r = yz > ax ? yz : ax;
        } else if (shape == 2) r = sqrt((dx*dx + dy*dy)/(dz*dz));
        else if (shape == -2) r = sqrt(dx*dx + dy*dy)/dz;
        else if (shape == 3) r = sqrt(dx*dx + dy*dy);
        else if (shape == 4 || shape == -4) {
            const double ax = fabs(dx), ay = fabs(dy);
            r = (ay > ax ? ay : ax)/fabs(dz);
        } else r = sqrt(dx*dx + (dy*dy + dz*dz));
        r /= radius;
    }
    double cx = params[11], cy = params[12], cz = params[13];
    double offset = 0;
    bool move = false;
    if (mode == 1) {
        if (r < 1) {
            move = true;
            r -= speed;
            if (r < 0) {
                cx = params[5]; cy = params[6]; cz = params[7];
                offset = recenter;
            } else {
                r /= 1-speed;
                cx = r*cx + (1-r)*params[5];
                cy = r*cy + (1-r)*params[6];
                cz = r*cz + (1-r)*params[7];
                offset = (1-r)*recenter;
            }
        }
    } else if (!(r < 1)) {
        cx = params[5]; cy = params[6]; cz = params[7];
    } else {
        move = true;
        r -= speed;
        if (r < 0) {
            cx -= recenter*params[0];
            cy -= recenter*params[1];
            cz -= recenter*params[2];
            offset = recenter;
        } else {
            r /= 1-speed;
            cx = r*params[5] + (1-r)*(cx-recenter*params[0]);
            cy = r*params[6] + (1-r)*(cy-recenter*params[1]);
            cz = r*params[7] + (1-r)*(cz-recenter*params[2]);
            offset = (1-r)*recenter;
        }
    }
    params[WRITE_J1_SLOT] = cx;
    params[WRITE_J2_SLOT] = cy;
    params[WRITE_J3_SLOT] = cz;
    if (move) return cpu_double3(p.x-offset*params[0],p.y-offset*params[1],p.z-offset*params[2]);
    return p;
}

constant FormulaInfo formula_info__PartlyJuliaR = {
    .id = FORMULA__PARTLYJULIAR,
    .name = "_PartlyJuliaR",
    .param_count = 11,
    .complexity = 734,
    .uses_transcendental = true
};
