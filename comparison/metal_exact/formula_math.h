#pragma once
#include "scene_types.h"
#include "bulb_math.h"

namespace mb3d_metal_exact {

// The prepared formula is uniform across a dispatch. Each policy is only the
// source-ordered formula step; the dIFS minimum/stop/iteration driver below is
// shared by primary marching, normal probes and shadow rays.
struct DIFSFormulaStep {
    Vec position, trap_position;
    F64 varying, shape;
    bool finite;
};

inline F64 sphere_radius(F64 x,F64 y,F64 z) {
#ifdef MB3D_METAL_NATIVE_FLOAT
    return native_length3(x,y,z);
#else
    return sqrt((x*x+y*y)+z*z);
#endif
}

struct SphereIFSLiteralFormula {
    enum { constant_trap = true };
    static inline DIFSFormulaStep step(Vec p, F64, ExactU32 iteration, ExactScene) {
        // Keep the previously qualified SphereIFS specialization: its admitted
        // scale is two and its accumulator is exactly 2^iteration. Both +0
        // operations remain because they affect signed-zero behaviour.
        const F64 scale(2);
        p = {p.x*scale+F64(3), p.y*scale+F64(0), p.z*scale+F64(0)};
        const F64 varying = D((ExactU64(iteration)+1023ULL)<<52);
        const F64 radius = sphere_radius(p.x,p.y,p.z);
        const F64 shape = max(radius-F64(1), F64(-1)-radius);
        return {p, p, varying, shape,
                finite_vec(p) && isfinite(varying) && isfinite(radius)};
    }
};

struct SphereIFSFormula {
    enum { constant_trap = false };
    static inline DIFSFormulaStep step(Vec p, F64 varying, ExactU32, ExactScene s) {
        // SphereIFS.m3f keeps the affine transform and axis scaling as separate
        // SSE2 stores. A noncommitting shape evaluates transformed coordinates,
        // but preserves the next iteration's incoming position and scale.
        const F64 scale = D(s.params[1]);
        const Vec transformed = {p.x*scale+D(s.params[4]),
                                 p.y*scale+D(s.params[3]),
                                 p.z*scale+D(s.params[2])};
        const bool apply = D(s.params[8]) != F64(0);
        if (apply) varying = varying*scale;
        const F64 x = transformed.x*D(s.params[7]);
        const F64 y = transformed.y*D(s.params[6]);
        const F64 z = transformed.z*D(s.params[5]);
        const F64 radius = sphere_radius(x,y,z);
        const F64 shape = max(radius-D(s.params[0]), D(s.params[12])-radius);
        const F64 size = max(abs(D(s.params[7])),
                             max(abs(D(s.params[6])), abs(D(s.params[5]))));
        F64 de = shape/size;
        if (!apply) de = de/scale;
        // The current native SphereIFS body selects transformed scratch XYZ
        // for color even when Apply=0. Preserve that CPU contract explicitly.
        return {apply ? transformed : p, transformed, varying, de,
                finite_vec(p) && finite_vec(transformed) && isfinite(varying) &&
                isfinite(radius) && isfinite(de)};
    }
};

struct BoxIFSFormula {
    enum { constant_trap = false };
    static inline DIFSFormulaStep step(Vec p, F64 varying, ExactU32, ExactScene s) {
        // Immutable boxIFS.m3f and the production translated formula use
        // separate SSE2 stores. Trapez multiplies signed z; MAXSD selects its
        // second operand on ties/unordered comparisons. Twist remains +0 only.
        const F64 scale = D(s.params[3]);
        const Vec transformed = {p.x*scale+D(s.params[6]),
                                 p.y*scale+D(s.params[5]),
                                 p.z*scale+D(s.params[4])};
        const bool apply = D(s.params[9]) != F64(0);
        if (apply) varying = varying*scale;
        const F64 dx = (abs(transformed.x)-D(s.params[2]))-transformed.z*D(s.params[7]);
        const F64 dy = abs(transformed.y)-D(s.params[1]);
        const F64 dz = abs(transformed.z)-D(s.params[0]);
        F64 shape = dz>dx ? dz : dx;
        shape = shape>dy ? shape : dy;
        if (!apply) shape = shape/scale;
        const Vec committed = apply ? transformed : p;
        return {committed, committed, varying, shape,
                finite_vec(p) && finite_vec(transformed) && isfinite(varying) && isfinite(shape)};
    }
};

// Native CPU formulas have distinct explicit OTrap rounding contracts. Values
// 10/11 (Sphere) and 11/12 (box) were materialized as Single by the loader.
inline F64 difs_orbit_trap(Vec p, F64 rstop, ExactScene s) {
    const unsigned optionIndex = selected_formula(s) == 1 ? 9 : 10;
    const F64 option = D(s.params[optionIndex]);
    const F64 source = option == F64(1) ? p.x :
                       option == F64(2) ? p.y :
                       option == F64(3) ? p.z : rstop;
    const F64 offset = D(s.params[optionIndex+1]);
    const F64 scale = D(s.params[optionIndex+2]);
    if (selected_formula(s) == 1) return source*scale+offset;
    return (E80(source)*E80(scale)+E80(offset)).to_f64();
}
struct DETrap { F64 distance; ExactU32 iteration; F64 trap; };

template<class Formula, bool TrackTrap = false>
inline DETrap difs_distance(Vec p, F64 stop, ExactScene s) {
    F64 varying(1), best(65535), trap(0);
    ExactU32 bestIt = 0;
    // CalcDEanalytic computes this product as Single before widening it.
    const F64 rstop = single(stop*S(s.step_width_103));
    for (ExactU32 i = 1; i <= s.max_iterations; ++i) {
        const auto step = Formula::step(p, varying, i, s);
        p = step.position;
        varying = step.varying;
        const F64 de = step.shape/varying;
        // Preserve the previous terminal fault: an invalid/overflowing orbit
        // cannot leave the initial finite minimum and masquerade as empty sky.
        if (!step.finite || !isfinite(de))
            return {D(0x7ff8000000000000ULL), i, D(0x7ff8000000000000ULL)};
        // UCOMISD/JNC skips ordered >= only. Keep the minimum and early-stop
        // comparisons in this order, with the selected iteration after the call.
        if (!(de>=best)) {
            best = de;
            bestIt = i;
            if (TrackTrap && !Formula::constant_trap) {
                trap = difs_orbit_trap(step.trap_position, rstop, s);
                if (!isfinite(trap))
                    return {D(0x7ff8000000000000ULL), i, D(0x7ff8000000000000ULL)};
            }
            if (!(de>=rstop)) break;
        }
    }
    // doHybridIFS3D retains its caller's bTmp and Dfree2 when no DE is below
    // 65535. Independent rays do not carry that preceding CPU call state.
    // Reject this case instead of inventing iteration zero and orbit color.
    if (bestIt == 0)
        return {D(0x7ff8000000000000ULL), 0, D(0x7ff8000000000000ULL)};
    if (TrackTrap && Formula::constant_trap) {
        // The pinned Sphere policy has OTrap=0, scale=1, offset=1. Its trap
        // does not depend on the selected orbit; evaluate once after selection.
        trap=rstop+F64(1);
        if(!isfinite(trap))
            return {D(0x7ff8000000000000ULL),bestIt,D(0x7ff8000000000000ULL)};
    }
    return {best*S(s.de_scale), bestIt, trap};
}

inline DE sphere_distance(Vec p, F64 stop, ExactScene s) {
    const auto result = selected_capability(s) == 1 ?
        difs_distance<SphereIFSLiteralFormula>(p, stop, s) :
        difs_distance<SphereIFSFormula>(p, stop, s);
    return {result.distance, result.iteration};
}

inline DE box_distance(Vec p, F64 stop, ExactScene s) {
    const auto result = difs_distance<BoxIFSFormula>(p, stop, s);
    return {result.distance, result.iteration};
}

inline DETrap distance_with_trap(Vec p, F64 stop, ExactScene s) {
    if (selected_formula(s) == 1) {
        if (selected_capability(s) == 1)
            return difs_distance<SphereIFSLiteralFormula, true>(p, stop, s);
        return difs_distance<SphereIFSFormula, true>(p, stop, s);
    }
    if (selected_formula(s) == 2) return difs_distance<BoxIFSFormula, true>(p, stop, s);
    if (selected_formula(s) == 3) { const auto d=bulb_distance(p,stop,s); return {d.distance,d.iteration,d.state.orbit_trap}; }
    return {D(0x7ff8000000000000ULL), 0, D(0x7ff8000000000000ULL)};
}
inline F64 formula_orbit_trap(Vec p, F64 stop, ExactScene s) {
    return distance_with_trap(p, stop, s).trap;
}

inline DE distance(Vec p, F64 stop, ExactScene s) {
    // Every public render entry validates the complete scene descriptor before
    // calling this hot path. Keep only the uniform formula selector here.
    if (selected_formula(s) == 1) return sphere_distance(p, stop, s);
    if (selected_formula(s) == 2) return box_distance(p, stop, s);
    if (selected_formula(s) == 3) { const auto d=bulb_distance(p,stop,s); return {d.distance,d.iteration}; }
    return {D(0x7ff8000000000000ULL), 0};
}

} // namespace mb3d_metal_exact
