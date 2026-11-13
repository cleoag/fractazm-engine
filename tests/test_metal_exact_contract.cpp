#include "comparison/metal_exact/scene_host.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include "comparison/metal_exact/scene_math.h"
#include "core/formulas.h"
#include "core/external_formulas.h"
#include "core/header_trafos.h"

#include <array>
#include <bit>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <limits>
#include <stdexcept>

namespace {
void check(bool pass, const char* message) {
    if (!pass) throw std::runtime_error(message);
}

// Frozen pre-specialization literal body. Keep its identity operations and
// repeated scale multiplication independent of the production distance helper.
mb3d_metal_exact::DE distance_literal_reference(mb3d_metal_exact::Vec p,
                                                mb3d_metal_exact::F64 stop,
                                                ExactScene s) {
    using namespace mb3d_metal_exact;
    F64 scale=F64(2), varying(1), best(65535);ExactU32 bestIt=0;
    F64 rstop=single(stop*S(s.step_width_103));
    for(ExactU32 i=1;i<=s.max_iterations;++i) {
        p={p.x*scale+F64(3),p.y*scale+F64(0),p.z*scale+F64(0)};
        varying=varying*scale;
        F64 x=p.x*F64(1),y=p.y*F64(1),z=p.z*F64(1);
        F64 radius=sqrt((x*x+y*y)+z*z);
        F64 shape=max(radius-F64(1),F64(-1)-radius);
        F64 sizes=max(abs(F64(1)),max(abs(F64(1)),abs(F64(1))));
        F64 de=(shape/sizes)/varying;
        if(!finite_vec(p)||!isfinite(varying)||!isfinite(radius)||!isfinite(de))
            return {F64::from_bits(0x7ff8000000000000ULL),i};
        if(!(de>=best)) { best=de;bestIt=i;if(!(de>=rstop))break; }
    }
    return {best*S(s.de_scale),bestIt};
}

struct DistanceCorpus {
    std::size_t finite_exact = 0, overflow_rejections = 0, stateful_rejections = 0;
};
DistanceCorpus check_distance_specialization(const ExactScene& original) {
    using namespace mb3d_metal_exact;
    std::size_t records = 0;
    DistanceCorpus counts{};
    auto compare = [&](Vec p, F64 stop, const ExactScene& scene) {
        const auto expected = distance_literal_reference(p, stop, scene);
        const auto actual = distance(p, stop, scene);
        const auto generic = difs_distance<SphereIFSFormula>(p, stop, scene);
        if (isfinite(expected.distance) && expected.iteration == 0) {
            // The frozen helper fabricated caller state when no minimum was
            // selected. Keep it unchanged as a witness, but do not count these
            // newly rejected outcomes as exact DE parity.
            check(actual.distance.bits == 0x7ff8000000000000ULL && actual.iteration == 0 &&
                  generic.distance.bits == 0x7ff8000000000000ULL && generic.iteration == 0,
                  "No-minimum dIFS orbit fabricated independent caller state");
            ++counts.stateful_rejections;
            ++records;
            return actual;
        }
        check(generic.distance.bits == expected.distance.bits &&
              generic.iteration == expected.iteration,
              "Generic SphereIFS diverged from its literal coefficient profile");
        if (actual.distance.bits != expected.distance.bits || actual.iteration != expected.iteration) {
            std::fprintf(stderr,
                "SphereIFS DE mismatch at record %zu: point=%016llx/%016llx/%016llx "
                "stop=%016llx width=%08x scale=%08x actual=%016llx/%u expected=%016llx/%u\n",
                records, static_cast<unsigned long long>(p.x.bits),
                static_cast<unsigned long long>(p.y.bits), static_cast<unsigned long long>(p.z.bits),
                static_cast<unsigned long long>(stop.bits), scene.step_width_103, scene.de_scale,
                static_cast<unsigned long long>(actual.distance.bits), actual.iteration,
                static_cast<unsigned long long>(expected.distance.bits), expected.iteration);
            throw std::runtime_error("SphereIFS identity specialization changed exact DE bits or iteration");
        }
        if (!finite_vec(p))
            check(actual.distance.bits == 0x7ff8000000000000ULL && actual.iteration == 1,
                  "Non-finite DE input lost its canonical first-iteration rejection");
        if (isfinite(expected.distance)) ++counts.finite_exact;
        else ++counts.overflow_rejections;
        ++records;
        return actual;
    };
    const F64 stop = S(original.de_stop);
    const ExactU64 finite_magnitudes[] = {
        0, 1, 2, 0x000ffffffffffffeULL, 0x000fffffffffffffULL,
        0x0010000000000000ULL, 0x0010000000000001ULL,
        0x3fdfffffffffffffULL, 0x3fe0000000000000ULL, 0x3fe0000000000001ULL,
        0x3fefffffffffffffULL, 0x3ff0000000000000ULL, 0x3ff0000000000001ULL,
        0x3ff7ffffffffffffULL, 0x3ff8000000000000ULL, 0x3ff8000000000001ULL,
        0x3fffffffffffffffULL, 0x4000000000000000ULL, 0x4000000000000001ULL,
        0x4007ffffffffffffULL, 0x4008000000000000ULL, 0x4008000000000001ULL,
        0x5fcfffffffffffffULL, 0x5fd0000000000000ULL, 0x5fd0000000000001ULL,
        0x5fdfffffffffffffULL, 0x5fe0000000000000ULL, 0x5fe0000000000001ULL,
        0x7fdffffffffffffeULL, 0x7fdfffffffffffffULL, 0x7fefffffffffffffULL
    };
    for (ExactU64 magnitude : finite_magnitudes)
        for (ExactU64 sign : {ExactU64(0), ExactU64(1) << 63}) {
            const F64 value = D(magnitude | sign);
            compare({value, F64(0), F64(0)}, stop, original);
            compare({F64(-3), value, F64(0)}, stop, original);
            compare({F64(-3), F64(0), value}, stop, original);
        }
    const ExactU64 nonfinite_magnitudes[] = {
        0x7ff0000000000000ULL, 0x7ff0000000000001ULL, 0x7ff8000000000000ULL,
        0x7ff8000000000123ULL, 0x7fffffffffffffffULL
    };
    for (ExactU64 magnitude : nonfinite_magnitudes)
        for (ExactU64 sign : {ExactU64(0), ExactU64(1) << 63}) {
            const F64 value = D(magnitude | sign);
            compare({value, F64(0), F64(0)}, stop, original);
            compare({F64(-3), value, F64(0)}, stop, original);
            compare({F64(-3), F64(0), value}, stop, original);
        }
    // Separate zero signs and multi-axis square/sum overflow boundaries.
    for (unsigned signs = 0; signs < 8; ++signs) {
        compare({D(ExactU64(signs & 1) << 63), D(ExactU64((signs >> 1) & 1) << 63),
                 D(ExactU64((signs >> 2) & 1) << 63)}, stop, original);
        for (ExactU64 magnitude : {0x5fcfffffffffffffULL, 0x5fd0000000000000ULL,
                                   0x5fd0000000000001ULL, 0x5fdfffffffffffffULL})
            compare({D(magnitude | (ExactU64(signs & 1) << 63)),
                     D(magnitude | (ExactU64((signs >> 1) & 1) << 63)),
                     D(magnitude | (ExactU64((signs >> 2) & 1) << 63))}, stop, original);
    }
    // Cover every finite exponent, retaining small coordinates on the y axis
    // instead of absorbing them into x's addition of three.
    const ExactU64 fractions[] = {0, 1, 0x0008000000000000ULL,
                                 0x000ffffffffffffeULL, 0x000fffffffffffffULL};
    for (ExactU64 exponent = 0; exponent < 2047; ++exponent)
        for (ExactU64 sign : {ExactU64(0), ExactU64(1) << 63})
            for (ExactU64 fraction : fractions)
                compare({F64(-3), D(sign | (exponent << 52) | fraction), F64(0)}, stop, original);

    ExactU64 random = 0x6a09e667f3bcc909ULL;
    auto next_finite = [&]() {
        random ^= random << 13;
        random ^= random >> 7;
        random ^= random << 17;
        ExactU64 bits = random;
        if ((bits & 0x7ff0000000000000ULL) == 0x7ff0000000000000ULL)
            bits ^= 0x0010000000000000ULL;
        return D(bits);
    };
    for (unsigned i = 0; i < 1024; ++i) {
        const Vec p = {next_finite(), next_finite(), next_finite()};
        compare(p, stop, original);
    }

    const Vec scalar_points[] = {
        {F64(0), F64(0), F64(0)}, {zero(true), zero(true), zero(true)},
        {D(0xbff8000000000000ULL), F64(0), F64(0)},
        {D(0xbff8000000000000ULL), D(1), D(0x8000000000000001ULL)},
        {F64(-1), F64(0), F64(0)}, {F64(-2), F64(0), F64(0)},
        {F64(-3), F64(0), F64(0)},
        {F64(-3), D(0x0010000000000000ULL), D(0x8010000000000000ULL)},
        {F64(1), F64(2), F64(3)},
        {F64(-3), D(0x3fe0000000000000ULL), D(0xbfe0000000000000ULL)},
        {F64(-3), D(0x5fd0000000000000ULL), D(0x5fd0000000000000ULL)},
        {F64(-3), D(0x5fe0000000000000ULL), F64(0)},
        {D(0x7fefffffffffffffULL), F64(0), F64(0)},
        {infinity(false), F64(0), F64(0)},
        {D(0x7ff8000000000123ULL), F64(0), F64(0)},
        {F64(-3), F64(0), D(0xfff0000000000001ULL)}
    };
    const ExactU32 scalars[] = {1, 0x3f800000U, 0x7f7fffffU};
    const ExactU32 stops[] = {1, original.de_stop, 0x3f800000U, 0x7f7fffffU};
    for (ExactU32 width : scalars)
        for (ExactU32 scale : scalars)
            for (ExactU32 stop_bits : stops) {
                auto scene = original;
                scene.step_width_103 = width;
                scene.de_scale = scale;
                scene.de_stop = stop_bits;
                check(valid_scene(scene), "DE scalar boundary scene violates the admitted device contract");
                for (Vec p : scalar_points) compare(p, S(stop_bits), scene);
            }

    // The fixed x=-3 orbit leaves the y square as the first failure. Its
    // exponent reaches 512 on precisely the selected iteration, never earlier.
    for (unsigned iteration = 1; iteration <= 60; ++iteration)
        for (ExactU64 sign : {ExactU64(0), ExactU64(1) << 63}) {
            const F64 y = D(sign | ((ExactU64(1023 + 512 - iteration)) << 52));
            const auto result = compare({F64(-3), y, F64(0)}, stop, original);
            check(result.distance.bits == 0x7ff8000000000000ULL && result.iteration == iteration,
                  "Finite DE overflow moved to a different iteration or lost its terminal NaN");
        }
    auto full = original;
    full.de_stop = full.step_width_103 = 1;
    full.de_scale = 0x3f800000U;
    check(valid_scene(full), "Full-60 DE witness violates the admitted device contract");
    const auto all_iterations = compare({F64(-3), F64(0), F64(0)}, S(full.de_stop), full);
    check(all_iterations.iteration == 60 &&
          all_iterations.distance.bits == (ExactU64(1023 - 59) << 52),
          "Finite full-60 DE witness changed its exact final result");
    const auto surface = compare({F64(-1), F64(0), F64(0)}, stop, original);
    check(surface.distance.bits == 0 && surface.iteration == 1,
          "Exact surface DE lost its positive zero or first minimum iteration");
    check(records <= 24000, "DE specialization corpus exceeded its bounded record count");
    check(counts.finite_exact && counts.overflow_rejections && counts.stateful_rejections,
          "SphereIFS corpus lost finite, overflow, or stateful no-minimum coverage");
    return counts;
}

std::size_t check_box_distance(const mb3d::metal_exact::PreparedExactScene& prepared) {
    using namespace mb3d_metal_exact;
    using namespace mb3d;
    check(prepared.device.formula_kind == 2 && valid_scene(prepared.device),
          "Prepared box scene does not satisfy the device formula contract");
    ScopedRenderContext binding(prepared.host->context);
    TMCTparameter base = prepared.host->mct;
    TIteration3Dext initial{};
    base.pIt3Dext = &initial;
    IniIt3D(&base, &initial);
    check(base.CalcDE == CalcDEanalytic && base.mMandFunctionDE == doHybridIFS3D,
          "Box DE oracle is not the independent production dIFS driver");
    const auto& scene = prepared.device;
    std::size_t records = 0, positive = 0, negative = 0, first = 0, later = 0;
    auto compare = [&](std::array<double, 3> point, float stop) {
        auto m = base;
        auto it = initial;
        m.pIt3Dext = &it;
        m.msDEstop = stop;
        it.C1 = point[0]; it.C2 = point[1]; it.C3 = point[2];
        it.CalcSIT = 1;
        // This oracle calls the production parameter loader, translated boxIFS,
        // doHybridIFS3D and CalcDEanalytic. It does not use a device policy.
        const double expected = m.CalcDE(&it, &m);
        const Vec p = {D(std::bit_cast<ExactU64>(point[0])),
                       D(std::bit_cast<ExactU64>(point[1])),
                       D(std::bit_cast<ExactU64>(point[2]))};
        const auto actual = distance_with_trap(p, S(std::bit_cast<ExactU32>(stop)), scene);
        if (actual.distance.bits != std::bit_cast<ExactU64>(expected) ||
            actual.iteration != static_cast<ExactU32>(it.ItResultI) ||
            actual.trap.bits != std::bit_cast<ExactU64>(it.OTrap)) {
            std::fprintf(stderr,
                "boxIFS DE mismatch at %s record %zu: point=%.17g/%.17g/%.17g "
                "stop=%08x actual=%016llx/%u expected=%016llx/%d\n",
                prepared.source_path.c_str(), records, point[0], point[1], point[2],
                std::bit_cast<ExactU32>(stop),
                static_cast<unsigned long long>(actual.distance.bits), actual.iteration,
                static_cast<unsigned long long>(std::bit_cast<ExactU64>(expected)), it.ItResultI);
            throw std::runtime_error("boxIFS changed production DE bits or selected minimum iteration");
        }
        check(std::isfinite(expected), "Finite bounded box corpus produced an invalid CPU DE");
        check(m.MaxItsResult == int(scene.max_iterations+1),
              "dIFS MaxItsResult lost its extra iteration");
        check(it.SmoothItD == static_cast<float>(it.ItResultI),
              "dIFS smooth iteration differs from selected minimum iteration");
        positive += expected > 0;
        negative += expected < 0;
        first += actual.iteration == 1;
        later += actual.iteration > 1;
        ++records;
    };
    const float stops[] = {std::bit_cast<float>(scene.de_stop), 0.0009765625f, 0.25f};
    const double grid[] = {-4.0, -2.0, -1.0, -0.0, 0.0, 0.5, 1.0, 2.0, 4.0};
    for (float stop : stops) {
        for (double x : grid)
            for (double y : grid)
                for (double z : grid) compare({x, y, z}, stop);

        // Invert the first affine step to approach each signed box face and
        // its MAXSD ties. For Trapez, signed transformed z changes the x face.
        auto param = [&](unsigned i) { return std::bit_cast<double>(scene.params[i]); };
        for (int xs : {-1, 0, 1})
            for (int ys : {-1, 0, 1})
                for (int zs : {-1, 0, 1}) {
                    const double tz = zs*param(0);
                    const std::array<double, 3> face = {
                        (xs*(param(2)+tz*param(7))-param(6))/param(3),
                        (ys*param(1)-param(5))/param(3),
                        (tz-param(4))/param(3)};
                    compare(face, stop);
                    for (unsigned axis = 0; axis < 3; ++axis)
                        for (double direction : {-std::numeric_limits<double>::infinity(),
                                                  std::numeric_limits<double>::infinity()}) {
                            auto nearby = face;
                            nearby[axis] = std::nextafter(nearby[axis], direction);
                            compare(nearby, stop);
                        }
                }
        // Dyadic random points exercise all octants and non-power-of-two
        // accumulator rounding in the asymmetric scene without host RNG drift.
        ExactU64 random = 0x243f6a8885a308d3ULL;
        auto coordinate = [&]() {
            random ^= random << 13;
            random ^= random >> 7;
            random ^= random << 17;
            return (double(random & 0xffffU)-32768.0)/4096.0;
        };
        for (unsigned i = 0; i < 256; ++i)
            compare({coordinate(), coordinate(), coordinate()}, stop);
    }
    check(records == 3522 && positive && negative && first && later,
          "Box DE corpus lost boundary coverage or a discriminating orbit class");
    // Non-finite/overflow input remains an explicit terminal device fault,
    // rather than accepting the production driver's stale finite minimum.
    for (ExactU64 bits : {0x7ff0000000000000ULL, 0xfff0000000000000ULL,
                          0x7ff8000000000123ULL, 0x7fefffffffffffffULL})
        for (unsigned axis = 0; axis < 3; ++axis) {
            Vec p{F64(0), F64(0), F64(0)};
            if (axis == 0) p.x = D(bits);
            if (axis == 1) p.y = D(bits);
            if (axis == 2) p.z = D(bits);
            const auto actual = distance(p, S(scene.de_stop), scene);
            check(actual.distance.bits == 0x7ff8000000000000ULL && actual.iteration == 1,
                  "Invalid box orbit did not stop on its first non-finite step");
            ++records;
        }
    return records;
}

// Build each oracle independently through the production option loader. These
// are capability tests over options, not acceptance of a new hash whitelist.
mb3d::metal_exact::PreparedExactScene prepare_variant(
        const mb3d::metal_exact::PreparedExactScene& original,
        const std::array<double, 13>& options, unsigned iterations, unsigned count) {
    using namespace mb3d;
    auto out = original;
    out.host = std::make_shared<metal_exact::SceneHostState>();
    out.host->context = std::make_shared<RenderContext>();
    out.host->file = original.host->file;
    auto& file = out.host->file;
    file.header.Iterations = static_cast<int32_t>(iterations);
    file.addon.Formulas[0].iItCount = static_cast<int32_t>(count);
    for (unsigned i = 0; i < 13; ++i) file.addon.Formulas[0].dOptionValue[i] = options[i];
    ScopedRenderContext binding(out.host->context);
    out.host->mct = GetMCTparasFromHeader(file.header, &file.addon, 0);
    const auto& m = out.host->mct;
    check(m.bMCTisValid && m.CalcDE == CalcDEanalytic &&
          m.mMandFunctionDE == doHybridIFS3D, "Generalized IFS oracle preparation failed");
    const char* name = original.device.formula_kind == 1 ? "SphereIFS" : "boxIFS";
    const auto* formula = FindExternalFormula(name);
    check(formula && formula->paramMap, "Generalized IFS formula metadata missing");
    double params[MAX_FORMULA_PARAMS]{};
    check(ReadFormulaParams(m.fHPVar[0], *formula->paramMap, params) == 13,
          "Generalized IFS parameter loader returned an unexpected count");
    auto& scene = out.device;
    scene.capability_id = original.device.formula_kind == 1 ? 101 : 102;
    scene.max_iterations = m.iMaxIt;
    scene.slot_count = m.nHybrid[0];
    scene.de_stop = std::bit_cast<ExactU32>(m.DEstop);
    scene.de_scale = std::bit_cast<ExactU32>(m.dDEscale);
    scene.step_width_103 = std::bit_cast<ExactU32>(m.sStepWm103);
    for (unsigned i = 0; i < 13; ++i) scene.params[i] = std::bit_cast<ExactU64>(params[i]);
    check(mb3d_metal_exact::valid_scene(scene), "Supported generalized IFS options rejected");
    return out;
}

std::size_t check_stateful_ifs_no_minimum(
        const mb3d::metal_exact::PreparedExactScene& sphere,
        const mb3d::metal_exact::PreparedExactScene& box) {
    using namespace mb3d;
    using namespace mb3d_metal_exact;
    std::size_t records = 0;
    for (unsigned kind : {1u, 2u})
        for (unsigned apply : {0u, 1u}) {
            const auto options = kind == 1 ?
                std::array<double,13>{1,2,0,0,3,1,1,1,double(apply),1,1,1,-1} :
                std::array<double,13>{1,1,1,2,0,0,3,0,0,double(apply),1,1,1};
            const auto prepared = prepare_variant(kind == 1 ? sphere : box, options, 7, 3);
            ScopedRenderContext binding(prepared.host->context);
            const float stop = 0.000001f;
            std::array<int,2> selected{};
            std::array<ExactU64,2> colors{};
            for (unsigned previous = 0; previous < 2; ++previous) {
                auto m = prepared.host->mct;
                TIteration3Dext it{};
                m.pIt3Dext = &it;
                IniIt3D(&m, &it);
                m.msDEstop = stop;
                it.C1 = previous ? -3 : -1;
                it.C2 = it.C3 = 0;
                it.CalcSIT = 1;
                check(std::isfinite(m.CalcDE(&it, &m)) && it.ItResultI > 0,
                      "Stateful dIFS witness could not seed a production minimum");
                selected[previous] = it.ItResultI;
                colors[previous] = std::bit_cast<ExactU64>(it.OTrap);
                // Reuse the actual production caller state left by that call.
                it.C1 = 100000000.0;
                it.C2 = it.C3 = 0;
                const double cpu = m.CalcDE(&it, &m);
                check(cpu == 65535.0 * double(m.dDEscale) &&
                      it.ItResultI == selected[previous] &&
                      std::bit_cast<ExactU64>(it.OTrap) == colors[previous],
                      "No-minimum production dIFS did not retain its preceding caller state");
            }
            check(selected[0] != selected[1] || colors[0] != colors[1],
                  "No-minimum witness failed to distinguish preceding production calls");
            const auto& scene = prepared.device;
            const Vec point{F64(100000000),F64(0),F64(0)};
            const F64 exact_stop = S(std::bit_cast<ExactU32>(stop));
            const auto plain = distance(point, exact_stop, scene);
            const auto colored = distance_with_trap(point, exact_stop, scene);
            check(plain.distance.bits == 0x7ff8000000000000ULL && plain.iteration == 0 &&
                  colored.distance.bits == 0x7ff8000000000000ULL && colored.iteration == 0 &&
                  colored.trap.bits == 0x7ff8000000000000ULL,
                  "Independent dIFS helper published fabricated no-minimum state");
            auto far_camera = scene;
            far_camera.y_start[0] = point.x.bits;
            far_camera.y_start[1] = far_camera.y_start[2] = 0;
            const auto rejected = render_primary(far_camera, prepared.columns[0], prepared.rows[0], 0, 0);
            check(rejected.fault == 4 && rejected.rm_result == 0,
                  "Stateful no-minimum dIFS orbit masqueraded as completed primary geometry");
            ++records;
        }
    return records;
}

std::size_t check_generalized_ifs(const mb3d::metal_exact::PreparedExactScene& sphere,
                                 const mb3d::metal_exact::PreparedExactScene& box) {
    using namespace mb3d_metal_exact;
    using namespace mb3d;
    std::size_t records = 0;
    for (unsigned kind : {1u, 2u})
        for (unsigned apply : {0u, 1u})
            for (unsigned trap : {0u, 1u, 2u, 3u})
                for (unsigned shape : {0u, 1u, 2u}) {
                    std::array<double, 13> options = kind == 1 ?
                        std::array<double,13>{1.25,1.5,-.25,.125,1.25,.75,1.5,1.25,
                                              double(apply),double(trap),-.375,.625,.25} :
                        std::array<double,13>{.75,1.25,.875,1.5,.125,-.25,1.75,.125,
                                              0,double(apply),double(trap),-.375,.625};
                    const unsigned scale_index = kind == 1 ? 1 : 3;
                    const unsigned offset_index = kind == 1 ? 10 : 11;
                    if (shape == 1) {
                        options[scale_index] = .75;
                        options[offset_index] = .1;
                        options[offset_index+1] = -.3;
                        if (kind == 1) options[12] = -1;
                    } else if (shape == 2) {
                        options[scale_index] = 1.1;
                        options[offset_index] = -0.0;
                        options[offset_index+1] = 0.0;
                        options[0] = .7;
                    }
                    const auto prepared = prepare_variant(kind == 1 ? sphere : box, options,
                                                           shape == 0 ? 60 : shape == 1 ? 7 : 31,
                                                           shape == 0 ? 15 : shape == 1 ? 1 : 23);
                    ScopedRenderContext binding(prepared.host->context);
                    TMCTparameter base = prepared.host->mct;
                    TIteration3Dext initial{};
                    base.pIt3Dext = &initial;
                    IniIt3D(&base, &initial);
                    const auto& scene = prepared.device;
                    auto compare = [&](std::array<double,3> point, float stop) {
                        auto m = base;
                        auto it = initial;
                        m.pIt3Dext = &it;
                        m.msDEstop = stop;
                        it.C1 = point[0]; it.C2 = point[1]; it.C3 = point[2]; it.CalcSIT = 1;
                        const double expected = m.CalcDE(&it, &m);
                        const Vec p{D(std::bit_cast<ExactU64>(point[0])),
                                    D(std::bit_cast<ExactU64>(point[1])),
                                    D(std::bit_cast<ExactU64>(point[2]))};
                        const F64 exact_stop = S(std::bit_cast<ExactU32>(stop));
                        const auto actual = distance_with_trap(p, exact_stop, scene);
                        const auto ordinary = distance(p, exact_stop, scene);
                        if (actual.distance.bits != std::bit_cast<ExactU64>(expected) ||
                            actual.iteration != static_cast<ExactU32>(it.ItResultI) ||
                            actual.trap.bits != std::bit_cast<ExactU64>(it.OTrap) ||
                            actual.distance.bits != ordinary.distance.bits ||
                            actual.iteration != ordinary.iteration) {
                            std::fprintf(stderr,
                                "Generalized IFS mismatch: kind=%u apply=%u trap=%u shape=%u "
                                "point=%.17g/%.17g/%.17g stop=%08x "
                                "DE=%016llx/%016llx iteration=%u/%d trap=%016llx/%016llx\n",
                                kind,apply,trap,shape,point[0],point[1],point[2],std::bit_cast<ExactU32>(stop),
                                (unsigned long long)actual.distance.bits,
                                (unsigned long long)std::bit_cast<ExactU64>(expected),actual.iteration,it.ItResultI,
                                (unsigned long long)actual.trap.bits,
                                (unsigned long long)std::bit_cast<ExactU64>(it.OTrap));
                            throw std::runtime_error("Generalized IFS changed exact production DE, iteration or orbit color");
                        }
                        check(std::isfinite(expected) && std::isfinite(it.OTrap),
                              "Finite generalized corpus unexpectedly produced nonfinite output");
                        ++records;
                    };
                    for (float stop : {std::bit_cast<float>(scene.de_stop), 0.0009765625f, .25f}) {
                        for (double x : {-4.0,-1.0,-0.0,0.0,.5,2.0})
                            for (double y : {-2.0,0.0,1.0})
                                for (double z : {-1.0,0.0,3.0}) compare({x,y,z},stop);
                        ExactU64 random = 0x243f6a8885a308d3ULL;
                        auto coordinate = [&]() {
                            random ^= random << 13; random ^= random >> 7; random ^= random << 17;
                            return (double(random&0xffffU)-32768.0)/4096.0;
                        };
                        for (unsigned i=0;i<32;++i) {
                            auto point = std::array<double,3>{coordinate(),coordinate(),coordinate()};
                            compare(point,stop);
                            point[i%3] = std::nextafter(point[i%3], std::numeric_limits<double>::infinity());
                            compare(point,stop);
                        }
                    }
                }
    check(records == 16992, "Generalized IFS option matrix lost coverage");
    return records;
}

std::size_t check_runtime_contract(const ExactScene& sphere, const ExactScene& box) {
    using namespace mb3d_metal_exact;
    std::size_t records = 0;
    for (auto scene : {sphere, box}) {
        scene.capability_id = scene.formula_kind == 1 ? 101 : 102;
        check(valid_scene(scene), "Generic capability rejected the existing coefficient profile");
        auto reject = [&](auto change) {
            auto invalid = scene;
            change(invalid);
            check(!valid_scene(invalid), "Invalid generalized IFS contract accepted");
            ++records;
        };
        // Primary-only malformed-input controls must not mistake a supported
        // nearby coefficient for corruption. Sphere Scale and box half-size
        // both admit this finite positive bit neighbour under runtime profiles.
        auto neighbor = scene;
        neighbor.params[1] ^= 1;
        check(valid_scene(neighbor),
              "Runtime IFS rejected a supported adjacent params[1] coefficient");
        ++records;
        reject([](auto& s) { s.params[1] = 0x7ff8000000000000ULL; });
        reject([](auto& s) { s.max_iterations = 0; });
        reject([](auto& s) { s.max_iterations = 257; });
        reject([](auto& s) { s.slot_count = 0; });
        reject([](auto& s) { s.slot_count = 65536; });
        for (unsigned i = 0; i < 13; ++i) {
            reject([i](auto& s) { s.params[i] = 0x7ff8000000000001ULL; });
            reject([i](auto& s) { s.params[i] = 0x7ff0000000000000ULL; });
        }
        const unsigned apply = scene.formula_kind == 1 ? 8 : 9;
        const unsigned trap = apply+1;
        reject([apply](auto& s) { s.params[apply] = 0x4000000000000000ULL; });
        reject([apply](auto& s) { s.params[apply] = 0x8000000000000000ULL; });
        reject([trap](auto& s) { s.params[trap] = 0x4010000000000000ULL; });
        reject([trap](auto& s) { s.params[trap] = 0x3fe0000000000000ULL; });
        for (unsigned index : {trap+1, trap+2})
            reject([index](auto& s) { s.params[index] = 0x3fb999999999999aULL; });
        if (scene.formula_kind == 1) {
            for (unsigned index : {0u,1u,5u,6u,7u}) {
                reject([index](auto& s) { s.params[index] = 0; });
                reject([index](auto& s) { s.params[index] = 0xbff0000000000000ULL; });
            }
        } else {
            for (unsigned index : {0u,1u,2u,3u}) {
                reject([index](auto& s) { s.params[index] = 0; });
                reject([index](auto& s) { s.params[index] = 0xbff0000000000000ULL; });
            }
            reject([](auto& s) { s.params[8] = 0x8000000000000000ULL; });
            reject([](auto& s) { s.params[8] = 0x3ff0000000000000ULL; });
        }
        // Valid values are admitted by their numerical branches, without a
        // matching manifest entry or dependence on the source file's hash.
        for (unsigned limit : {1u, 256u}) {
            auto changed = scene;
            changed.max_iterations = limit;
            changed.slot_count = limit == 1 ? 1 : 65535;
            changed.params[0] ^= 1;
            check(valid_scene(changed), "Supported runtime coefficients/limits require a pinned record");
            ++records;
        }
    }
    return records;
}

}

int main() {
    try {
        using namespace mb3d_metal_exact;
        const auto root = std::filesystem::path(__FILE__).parent_path().parent_path();
        const auto prepared = mb3d::metal_exact::PrepareExactScene(
            (root / "comparison/scenes/extall_dIFS_SphereIFS.m3p").string(), 16, 12);
        const auto& original = prepared.device;
        check(valid_scene(original), "Production-prepared pinned scene rejected by device contract");
        const auto distance_records = check_distance_specialization(original);
        std::size_t box_records = 0;
        mb3d_metal_exact::F64 original_box_zero;
        for (const char* path : {"comparison/scenes/extall_dIFS_boxIFS.m3p",
                                 "comparison/scenes/ext_16_boxifs.m3p",
                                 "comparison/metal_exact/fixtures/box_asymmetric.m3p"}) {
            const auto box = mb3d::metal_exact::PrepareExactScene((root/path).string(), 16, 12);
            box_records += check_box_distance(box);
            const auto zero_de = distance({F64(0), F64(0), F64(0)}, S(box.device.de_stop), box.device);
            if (box.device.capability_id == 2) original_box_zero = zero_de.distance;
            if (box.device.capability_id == 4)
                check(original_box_zero.bits != zero_de.distance.bits,
                      "Asymmetric fixture did not change box geometry at a discriminating point");
            for (unsigned i = 0; i < 13; ++i) {
                auto invalid = box.device;
                invalid.params[i] ^= 1;
                check(!valid_scene(invalid), "Unadmitted box parameter accepted");
            }
            for (unsigned index : {8u, 9u, 10u}) {
                auto invalid = box.device;
                invalid.params[index] = index == 8 ? 0x8000000000000000ULL :
                                        index == 9 ? 0 : 0x3ff0000000000000ULL;
                check(!valid_scene(invalid), "Unsupported box Twist/Apply/OTrap mode accepted");
            }
            auto wrong_formula = box.device;
            wrong_formula.formula_kind = 1;
            check(!valid_scene(wrong_formula), "Box parameters accepted as SphereIFS");
        }
        const auto general_box = mb3d::metal_exact::PrepareExactScene(
            (root / "comparison/scenes/extall_dIFS_boxIFS.m3p").string(), 16, 12);
        const std::size_t generalized_records = check_generalized_ifs(prepared, general_box);
        const std::size_t stateful_records = check_stateful_ifs_no_minimum(prepared, general_box);
        const std::size_t runtime_records = check_runtime_contract(original, general_box.device);
        std::size_t scenarios = 0;
        auto rejects = [&](const ExactScene& input) {
            check(!valid_scene(input), "Unsupported scene record accepted");
            const auto result = render_primary(input, prepared.columns[0], prepared.rows[0], 0, 0);
            check(result.fault == 1 && result.rm_result == 0,
                  "Unsupported scene record published geometry or wrong terminal fault");
            ++scenarios;
        };
        auto mutate = [&](auto mutation) { auto input = original; mutation(input); rejects(input); };
        mutate([](auto& s) { ++s.version; });
        mutate([](auto& s) { s.fog_iteration = 1; });
        mutate([](auto& s) { s.width = 0; });
        mutate([](auto& s) { s.height = 0; });
        mutate([](auto& s) { s.width = 4097; });
        mutate([](auto& s) { s.height = 4097; });
        mutate([](auto& s) { s.max_steps = 0; });
        mutate([](auto& s) { s.max_steps = 65537; });
        mutate([](auto& s) { s.max_iterations = 59; });
        mutate([](auto& s) { s.slot_count = 14; });
        mutate([](auto& s) { s.min_iterations = 0; });
        mutate([](auto& s) { s.refinement_steps = 7; });
        mutate([](auto& s) { s.capability_id = 0; });
        mutate([](auto& s) { s.capability_id = 0xffffffffU; });
        mutate([](auto& s) { s.formula_kind = 0; });
        mutate([](auto& s) { s.formula_kind = 2; });
        mutate([](auto& s) { s.formula_kind = 3; });
        mutate([](auto& s) { s.formula_kind = 0xffffffffU; });
        mutate([](auto& s) { s.color_on_it = 1; });
        mutate([](auto& s) { s.color_radius_stop = 1; });
        mutate([](auto& s) { s.bulb.power = 8; });
        for (ExactU32 selector : {0u, 4u, 0xffffffffU}) {
            auto unsupported = original;
            unsupported.formula_kind = selector;
            const auto de = distance({F64(0), F64(0), F64(0)}, S(original.de_stop), unsupported);
            check(de.distance.bits == 0x7ff8000000000000ULL && de.iteration == 0,
                  "Unsupported formula selector published a finite DE");
        }
        for (int i = 0; i < 13; ++i) mutate([i](auto& s) { s.params[i] ^= 1; });
        mutate([](auto& s) { s.vgrads[0] = 0x7ff8000000000000ULL; });
        mutate([](auto& s) { s.vgrads[8] = 0x7ff0000000000000ULL; });
        mutate([](auto& s) { s.y_start[2] = 0xfff0000000000000ULL; });
        mutate([](auto& s) { s.max_ray_length = 0; });
        mutate([](auto& s) { s.max_ray_length = 0x7ff0000000000000ULL; });
        mutate([](auto& s) { s.de_stop = 0; });
        mutate([](auto& s) { s.de_stop = 0x7fc00000; });
        mutate([](auto& s) { s.de_stop_factor = 0xbf800000; });
        mutate([](auto& s) { s.de_stop_factor = 0x7f800000; });
        mutate([](auto& s) { s.z_step_div = 0; });
        mutate([](auto& s) { s.step_limiter = 0x7f800000; });
        mutate([](auto& s) { s.de_scale = 0; });
        mutate([](auto& s) { s.step_width_103 = 0; });
        mutate([](auto& s) { s.de_sub = 0x7fc00000; });

        for (const auto& pixel : {render_primary(original, prepared.columns[0], prepared.rows[0], original.width, 0),
                                  render_primary(original, prepared.columns[0], prepared.rows[0], 0, original.height),
                                  render_primary(original, {0x7ff8000000000000ULL, 0}, prepared.rows[0], 0, 0),
                                  render_primary(original, prepared.columns[0], {0,0x7ff0000000000000ULL}, 0, 0)}) {
            check(pixel.fault == 1 && pixel.rm_result == 0, "Invalid coordinate/trig input published geometry");
            ++scenarios;
        }
        // Finite input storage can still overflow during camera arithmetic.
        // It must fail instead of accepting NaN comparisons as a surface hit.
        auto overflowing = original;
        overflowing.vgrads[0] = 0x7fefffffffffffffULL;
        overflowing.y_start[0] = 0x7fefffffffffffffULL;
        check(valid_scene(overflowing), "Finite overflow witness unexpectedly rejected before arithmetic");
        const auto fault = render_primary(overflowing, prepared.columns[2], prepared.rows[0], 2, 0);
        check(fault.fault == 4 && fault.rm_result == 0, "Camera overflow masqueraded as completed geometry");
        ++scenarios;
        std::printf("Metal exact device contract: %zu native rejection/finite-overflow scenarios passed; "
                    "SphereIFS corpus: %zu finite exact records, %zu overflow controls, "
                    "%zu stateful no-minimum rejections; %zu independent boxIFS DE records; "
                    "%zu generalized IFS DE/iteration/orbit-color records; %zu runtime capability checks; "
                    "%zu stateful production no-minimum witnesses; "
                    "four pinned M3P preparations accepted.\n",
                    scenarios, distance_records.finite_exact, distance_records.overflow_rejections,
                    distance_records.stateful_rejections, box_records, generalized_records,
                    runtime_records, stateful_records);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Metal exact device contract FAILED: %s\n", error.what());
        return 1;
    }
}
