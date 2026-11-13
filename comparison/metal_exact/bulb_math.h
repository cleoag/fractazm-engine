#pragma once
#include "scene_types.h"
#include "soft_log.h"
#include "soft_extended_log.h"

namespace mb3d_metal_exact {

// The internal Integer Power formulas use the native Apple ARM64 CPU profile:
// long double is binary64 here. Keep each store and expression grouping from
// core/formulas.cpp; this is not the Windows x87 evaluation profile.
inline Vec bulb_step(Vec p, Vec julia, ExactScene scene) {
    const F64 z_multiplier = D(scene.params[0]);
    const F64 sx = p.x*p.x, sy = p.y*p.y, sz = p.z*p.z;
    const F64 r = sx+sy;
    if (selected_bulb_power(scene) == 2) {
        const F64 a = (r-sz)/r;
        F64 z = sqrt(r);
        z = z*z_multiplier;
        z = z*p.z;
        z = z+z;
        z = z+julia.z;
        F64 x = sx-sy;
        x = x*a;
        x = x+julia.x;
        F64 y = p.x*p.y;
        y = y*a;
        y = y+y;
        y = y+julia.y;
        return {x,y,z};
    }
    if (selected_bulb_power(scene) == 3) {
        const F64 a = F64(1)-F64(3)*sz/(r+D(0x2b2bff2ee48e0530ULL));
        const F64 x = a*(sx-F64(3)*sy)*p.x+julia.x;
        const F64 y = a*(F64(3)*sx-sy)*p.y+julia.y;
        const F64 z = julia.z-z_multiplier*p.z*(sz-F64(3)*r);
        return {x,y,z};
    }
    if (selected_bulb_power(scene) == 4) {
        const F64 a = F64(1)+(sz*(sz-F64(6)*r))/
            (r*r+D(0x37a16c262777579cULL));
        const F64 y = F64(4)*p.x*p.y*a*(sx-sy)+julia.y;
        const F64 z = z_multiplier*F64(4)*sqrt(r)*p.z*(r-sz)+julia.z;
        const F64 x = a*(sx*(sx-F64(6)*sy)+sy*sy)+julia.x;
        return {x,y,z};
    }
    if (selected_bulb_power(scene) == 5) {
        const F64 a = F64(1)+F64(5)*(sz*sz-F64(2)*r*sz)/
            (r*r+D(0x37a16c262777579cULL)); // native Double literal 1e-40
        const F64 y = a*p.y*(F64(5)*sx*sx-sy*(F64(10)*sx-sy))+julia.y;
        const F64 z = z_multiplier*p.z*(sz*(sz-F64(10)*r)+F64(5)*r*r)+julia.z;
        const F64 x = a*p.x*(sx*(sx-F64(10)*sy)+F64(5)*sy*sy)+julia.x;
        return {x,y,z};
    }
    if (selected_bulb_power(scene) == 6) {
        const F64 a = F64(1)-(sz*(sz*(sz-F64(15)*r)+F64(15)*r*r))/
            (r*r*r+D(0x37a16c262777579cULL));
        const F64 y = F64(2)*a*p.x*p.y*(sx*(F64(3)*sx-F64(10)*sy)+F64(3)*sy*sy)+julia.y;
        const F64 z = z_multiplier*F64(2)*p.z*sqrt(r)*
            (sz*(F64(3)*sz-F64(10)*r)+F64(3)*r*r)+julia.z;
        const F64 x = a*(sx*sx*(sx-F64(15)*sy)+sy*sy*(F64(15)*sx-sy))+julia.x;
        return {x,y,z};
    }
    if (selected_bulb_power(scene) == 7) {
        const F64 a = F64(1)-F64(7)*(sz*(sz*(sz-F64(5)*r)+F64(3)*r*r))/
            (r*r*r+D(0x37a16c262777579cULL));
        const F64 y = a*p.y*(sx*(sx*(F64(7)*sx-F64(35)*sy)+F64(21)*sy*sy)-sy*sy*sy)+julia.y;
        const F64 z_part = p.z*(sz*sz*sz-F64(7)*r*(sz*(F64(3)*sz-F64(5)*r)+r*r));
        const F64 z = julia.z-z_multiplier*z_part;
        const F64 x = a*p.x*(sx*(sx*(sx-F64(21)*sy)+F64(35)*sy*sy)-F64(7)*sy*sy*sy)+julia.x;
        return {x,y,z};
    }
    if (selected_bulb_power(scene) == 8) {
        const F64 tiny = D(scene.params[1]), one = D(scene.params[2]);
        const F64 c6 = D(scene.params[3]), c7 = D(scene.params[4]);
        const F64 c8 = D(scene.params[5]), c28 = D(scene.params[6]), c70 = D(scene.params[7]);
        const F64 rr = r*r, zzzz = sz*sz;
        F64 z_term = r*sz;
        z_term = z_term*c6;
        z_term = zzzz-z_term;
        z_term = z_term+rr;
        z_term = z_term*(sz-r);
        z_term = z_term*sqrt(r);
        z_term = z_term*p.z;
        z_term = z_term*c8;
        z_term = z_term*z_multiplier;
        z_term = -z_term;
        const F64 z = z_term+julia.z;

        F64 a_left = zzzz+rr;
        a_left = a_left*r;
        a_left = a_left*sz;
        a_left = a_left*c28;
        F64 a_right = rr*c70;
        a_right = a_right+zzzz;
        a_right = a_right*zzzz;
        F64 denominator = rr*rr;
        denominator = denominator+tiny;
        F64 a = a_right-a_left;
        a = a/denominator;
        a = a+one;

        const F64 xxxx = sx*sx, yyyy = sy*sy;
        const F64 y_poly1 = c7*sx-sy, y_poly2 = sx-c7*sy;
        F64 y_term = yyyy*y_poly1;
        y_term = y_term+xxxx*y_poly2;
        y_term = y_term*c8;
        y_term = y_term*p.x;
        y_term = y_term*p.y;
        y_term = y_term*a;
        const F64 y = y_term+julia.y;

        F64 x_term = c70*xxxx;
        x_term = x_term+yyyy;
        x_term = x_term*yyyy;
        F64 xy_term = yyyy+xxxx;
        xy_term = xy_term*sy;
        xy_term = xy_term*sx;
        xy_term = xy_term*c28;
        F64 x_core = xxxx*xxxx;
        x_core = x_core+x_term;
        x_core = x_core-xy_term;
        x_core = x_core*a;
        return {x_core+julia.x,y,z};
    }
    const F64 invalid = D(0x7ff8000000000000ULL);
    return {invalid,invalid,invalid};
}

struct BulbOrbit {
    Vec position;
    F64 radius, old_radius, orbit_trap;
    ExactU32 iteration;
    F64 smooth;
};

inline F64 bulb_smooth(BulbOrbit orbit, ExactScene scene) {
    if (orbit.radius <= F64(1)) return single(F64(orbit.iteration));
    const F64 d = native_log(D(0x3fe0000000000000ULL)*native_log(orbit.radius));
    if (orbit.old_radius < F64(1)) {
        // In the native C++ expression int + float occurs before the Double
        // product/subtraction. Preserve that intermediate Single addition.
        const F64 base = single(F64(orbit.iteration)+S(scene.bulb.smooth_lnrstop));
        return single(base-d*S(scene.bulb.smooth_hln));
    }
    const F64 old = native_log(D(0x3fe0000000000000ULL)*native_log(orbit.old_radius));
    const F64 denominator = (d-old)+D(0x2b2bff2ee48e0530ULL); // 1e-100
    return single(F64(orbit.iteration)+(S(scene.bulb.smooth_lnrstop)-d)/denominator);
}

inline BulbOrbit bulb_orbit(Vec origin, ExactScene scene, ExactU32 max_iterations,
                            F64 bailout, bool calculate_smooth = false) {
    // A single positive slot repeats the same internal formula; its count does
    // not change the mathematical orbit or positive iteration accounting.
    const F64 radius = dot(origin,origin);
    BulbOrbit result{origin,radius,F64(0),radius,0,F64(0)};
    do {
        result.old_radius = result.radius;
        result.position = bulb_step(result.position,origin,scene);
        ++result.iteration;
        result.radius = dot(result.position,result.position);
        if (result.radius < result.orbit_trap) result.orbit_trap = result.radius;
        // Pascal repeat-until semantics: NaN never satisfies Rout > RStop.
    } while (result.iteration < max_iterations && !(result.radius > bailout));
    if (calculate_smooth) result.smooth = bulb_smooth(result,scene);
    return result;
}

struct BulbDE {
    F64 distance;
    ExactU32 iteration;
    BulbOrbit state;
};

inline BulbDE bulb_distance(Vec origin, F64 stop, ExactScene scene,
                            bool calculate_smooth = false, ExactU32 max_override = 0) {
    const ExactU32 max_iterations = max_override ? max_override : scene.max_iterations;
    const BulbOrbit base = bulb_orbit(origin,scene,max_iterations,
                                      D(scene.bulb.bailout),calculate_smooth);
    BulbOrbit state = base;
    F64 result(0);
    if (!(base.radius < D(0x16687e92154ef7acULL))) { // 1e-200
        const F64 offset = D(scene.bulb.de_offset);
        const F64 gradient_bailout = D(scene.bulb.gradient_bailout);
        const auto x = bulb_orbit({origin.x+offset,origin.y,origin.z},
                                  scene,base.iteration,gradient_bailout);
        const F64 dx = base.radius-x.radius;
        const F64 dt = dx*dx;
        const auto y = bulb_orbit({origin.x,origin.y+offset,origin.z},
                                  scene,base.iteration,gradient_bailout);
        const F64 dy = base.radius-y.radius;
        const F64 wt = dy*dy;
        state = bulb_orbit({origin.x,origin.y,origin.z+offset},
                           scene,base.iteration,gradient_bailout);
        const F64 dz = base.radius-state.radius;
        const F64 rst = dz*dz;
        result = extended_numerical_de(base.radius,dt,wt,rst,S(scene.de_scale),
                                       D(scene.bulb.de_offset006)).raw;
        // CalcDEnoADE restores Rout and ItResultI only. Rold, xyz and OTrap
        // remain from the final offset orbit; SmoothItD remains from the base.
        state.radius = base.radius;
        state.iteration = base.iteration;
        state.smooth = base.smooth;
    }
    const F64 minimum = stop*D(0x3fd0000000000000ULL);
    if (result < minimum) result = minimum;
    return {result,base.iteration,state};
}

} // namespace mb3d_metal_exact
