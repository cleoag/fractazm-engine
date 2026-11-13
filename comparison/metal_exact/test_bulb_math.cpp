#include "scene_math.h"
#include "core/formula_registry.h"
#include "core/formulas.h"
#include "core/header_trafos.h"
#include "core/ray_marcher.h"
#include "core/render_context.h"
#include "io/m3p_parser.h"
#include <array>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <limits>
#include <stdexcept>
#include <vector>

using namespace mb3d_metal_exact;
namespace {
#include "tests/fixtures/radius_de_reference.h"
std::size_t checks=0, failures=0, step_records=0, orbit_records=0, de_records=0, refinement_records=0;
ExactU64 random_state=0xb8f108c49a5e1273ULL;
ExactU64 next_random() {
    random_state ^= random_state<<13;
    random_state ^= random_state>>7;
    random_state ^= random_state<<17;
    return random_state;
}
F64 exact(double value) { return D(std::bit_cast<ExactU64>(value)); }
Vec exact(std::array<double,3> p) { return {exact(p[0]),exact(p[1]),exact(p[2])}; }
void same(F64 actual,double expected,const char* field,unsigned power) {
    ++checks;
    // Arithmetic's established contract checks canonical NaN class, not CPU
    // payload/sign; every finite value, infinity and signed zero is exact.
    const bool success=std::isnan(expected)?isnan(actual):
        actual.bits==std::bit_cast<ExactU64>(expected);
    if(!success&&failures++<30)
        std::fprintf(stderr,"power%u %s got=%016llx expected=%016llx\n",power,field,
                     static_cast<unsigned long long>(actual.bits),
                     static_cast<unsigned long long>(std::bit_cast<ExactU64>(expected)));
}
void same_integer(unsigned actual,unsigned expected,const char* field,unsigned power) {
    ++checks;
    if(actual!=expected&&failures++<30)
        std::fprintf(stderr,"power%u %s got=%u expected=%u\n",power,field,actual,expected);
}
void test_numerical_de_fixtures() {
    unsigned finite_positive_rows = 0;
    for (const auto& row : radiusDEReferences) {
        const auto actual = extended_numerical_de(D(row.input[0]),D(row.input[1]),
            D(row.input[2]),D(row.input[3]),D(row.input[4]),D(row.input[5]));
        // Keep the Metal arithmetic contract explicit: all non-NaN bits are
        // exact; NaN class is checked because E80 deliberately canonicalizes it.
        same(actual.raw,std::bit_cast<double>(row.expected[1]),"frozen.raw_de",0);
        same(actual.denominator,std::bit_cast<double>(row.expected[2]),"frozen.denominator",0);
        const auto radius = D(row.input[0]);
        if (isfinite(radius) && radius > F64(0) && isfinite(actual.denominator) &&
            actual.denominator > F64(0) && isfinite(D(row.expected[1])))
            ++finite_positive_rows;
    }
    same_integer(finite_positive_rows,617,"frozen finite positive rows",0);
    const ExactU64 inputs[][8] = {
        {0x40F399CF13949B90ULL,0x3FE868151D9DBA71ULL,0x403F1D819BD2E4FFULL,
         0x405435C55B0AC97AULL,0x3F70624DE0000000ULL,0x3F2F751060000000ULL,
         0x4075597F4CFD8B6EULL,0x40253BF71D986422ULL},
        {0x40804822751856FBULL,0x3FCED566132F1AA3ULL,0x3FC3EE74CFFC130CULL,
         0x3FFCB7268AACEE4BULL,0x3F70624DE0000000ULL,0x3F2F751060000000ULL,
         0x40219C9619109BD8ULL,0x3FF7B054C6FFE053ULL}
    };
    for (const auto& row : inputs) {
        const auto actual=extended_numerical_de(D(row[0]),D(row[1]),D(row[2]),D(row[3]),D(row[4]),D(row[5]));
        same(actual.raw,std::bit_cast<double>(row[6]),"reflection.raw_de",8);
        same(actual.denominator,std::bit_cast<double>(row[7]),"reflection.denominator",8);
    }
}
void compare_orbit(const BulbOrbit& actual,const TIteration3Dext& expected,
                   unsigned power,bool smooth) {
    same(actual.position.x,expected.x,"orbit.x",power);
    same(actual.position.y,expected.y,"orbit.y",power);
    same(actual.position.z,expected.z,"orbit.z",power);
    same(actual.radius,expected.Rout,"orbit.Rout",power);
    same(actual.old_radius,expected.Rold,"orbit.Rold",power);
    same(actual.orbit_trap,expected.OTrap,"orbit.OTrap",power);
    same_integer(actual.iteration,expected.ItResultI,"orbit.iteration",power);
    if(smooth) same(actual.smooth,expected.SmoothItD,"orbit.smooth",power);
}
ExactScene device_scene(const TMCTparameter& m,unsigned power) {
    ExactScene out{};
    out.formula_kind=3;
    out.max_iterations=m.iMaxIt;
    out.slot_count=m.nHybrid[0];
    out.refinement_steps=m.iDEAddSteps;
    out.de_scale=std::bit_cast<ExactU32>(m.dDEscale);
    out.bulb.bailout=std::bit_cast<ExactU64>(double(m.dRStop));
    out.bulb.gradient_bailout=std::bit_cast<ExactU64>(double(float(m.Rstop3D)));
    out.bulb.de_offset=std::bit_cast<ExactU64>(double(m.mctDEoffset));
    out.bulb.de_offset006=std::bit_cast<ExactU64>(double(m.mctDEoffset006));
    out.bulb.smooth_hln=std::bit_cast<ExactU32>(m.fHln[0]);
    out.bulb.smooth_lnrstop=std::bit_cast<ExactU32>(m.dLNRStop);
    out.bulb.power=power;
    const auto* params=static_cast<const unsigned char*>(m.fHPVar[0]);
    const int offsets[]={-16,24,32,144,152,160,192,208};
    for(unsigned i=0;i<8;++i) std::memcpy(&out.params[i],params+offsets[i],8);
    return out;
}
void test_scene(const std::filesystem::path& path,unsigned power,bool override_power=false) {
    auto context=std::make_shared<mb3d::RenderContext>();
    mb3d::ScopedRenderContext binding(context);
    auto file=mb3d::LoadM3PFile(path.string());
    if(!file.success) throw std::runtime_error(file.error);
    // Reuse the tracked power-8 scene for other integer exponents, preparing
    // the complete CPU caller again so Hln, function pointers and DE inputs
    // remain independent of the device implementation.
    if(override_power) file.addon.Formulas[0].dOptionValue[0]=double(power);
    file.header.Width=64;file.header.Height=48;
    auto m=mb3d::GetMCTparasFromHeader(file.header,&file.addon,0);
    if(!m.bMCTisValid||m.DEoption!=0||m.bDoJulia||m.FormulaType!=0||
       m.CalcDE!=mb3d::CalcDEnoADE||m.mMandFunction!=mb3d::doHybridPas||
       m.bInsideRendering||m.bCalcInside)
        throw std::runtime_error("Builtin fixture differs from numerical-DE outside-only contract");
    if(m.fHybrid[0]!=reinterpret_cast<ThybridIteration2>(mb3d::GetInternalFormula(0,power,false)))
        throw std::runtime_error("Prepared CPU formula differs from requested integer power");
    const auto scene=device_scene(m,power);
    TIteration3Dext initial{};
    mb3d::IniIt3D(&m,&initial);
    const auto cpu_step=reinterpret_cast<ThybridIteration>(m.fHybrid[0]);
    initial.PVar=m.fHPVar[0];

    // Independent production single-step checks exercise axes, signed zero,
    // cancellation, subnormals, overflow and a bounded random mantissa corpus.
    std::vector<std::array<double,3>> points;
    const double grid[]={-2,-1,-0.25,-0.0,0.0,0.25,0.75,1,2};
    for(double x:grid) for(double y:grid) for(double z:grid) points.push_back({x,y,z});
    for(unsigned i=0;i<512;++i) {
        std::array<double,3> p;
        for(auto& value:p) value=(double(next_random()&0xffffff)/double(0x1000000)-.5)*4;
        points.push_back(p);
    }
    const std::array<double,3> julia{.125,-.25,.5};
    for(auto point:points) {
        auto it=initial;
        it.J1=julia[0];it.J2=julia[1];it.J3=julia[2];
        double x=point[0],y=point[1],z=point[2],w=.375;
        cpu_step(x,y,z,w,reinterpret_cast<TPIteration3D>(&it.C1));
        const auto actual=bulb_step(exact(point),exact(julia),scene);
        same(actual.x,x,"step.x",power);same(actual.y,y,"step.y",power);same(actual.z,z,"step.z",power);
        ++step_records;
    }
    for(ExactU64 bits:{ExactU64(1),ExactU64(0x0010000000000000ULL),
                      ExactU64(0x3fefffffffffffffULL),ExactU64(0x3ff0000000000001ULL),
                      ExactU64(0x5fe0000000000000ULL),ExactU64(0x7fefffffffffffffULL)})
        for(unsigned axis=0;axis<3;++axis) for(unsigned sign=0;sign<2;++sign) {
            auto point=julia;
            point[axis]=std::bit_cast<double>(bits|(ExactU64(sign)<<63));
            auto it=initial;
            it.J1=julia[0];it.J2=julia[1];it.J3=julia[2];
            double x=point[0],y=point[1],z=point[2],w=0;
            cpu_step(x,y,z,w,reinterpret_cast<TPIteration3D>(&it.C1));
            const auto actual=bulb_step(exact(point),exact(julia),scene);
            same(actual.x,x,"step.edge.x",power);same(actual.y,y,"step.edge.y",power);same(actual.z,z,"step.edge.z",power);
            ++step_records;
        }

    // Complete orbit and numerical DE are compared with the real CPU driver,
    // including state left by its final gradient sample and maxIt override.
    for(unsigned index=0;index<points.size();index+=5) {
        const auto point=points[index];
        for(unsigned limit:{1u,2u,5u,16u,32u}) {
            auto it=initial;
            it.C1=point[0];it.C2=point[1];it.C3=point[2];it.CalcSIT=1;it.maxIt=limit;
            m.mMandFunction(reinterpret_cast<TPIteration3D>(&it.C1));
            const auto actual=bulb_orbit(exact(point),scene,limit,D(scene.bulb.bailout),true);
            compare_orbit(actual,it,power,true);
            ++orbit_records;
        }
        for(float stop:{.0009765625f,.25f,1.f}) {
            for(bool smooth:{false,true}) {
                for(unsigned override_limit:{0u,2u,17u}) {
                    auto local=m;
                    auto it=initial;
                    local.pIt3Dext=&it;local.msDEstop=stop;
                    it.C1=point[0];it.C2=point[1];it.C3=point[2];it.CalcSIT=smooth;
                    if(override_limit) {it.maxIt=override_limit;local.iMaxIt=override_limit;}
                    const double expected=local.CalcDE(&it,&local);
                    const auto actual=bulb_distance(exact(point),exact(double(stop)),scene,smooth,override_limit);
                    same(actual.distance,expected,"DE",power);
                    same_integer(actual.iteration,it.ItResultI,"DE.iteration",power);
                    compare_orbit(actual.state,it,power,smooth);
                    ++de_records;
                }
            }
        }
    }
    // Exercise the separate maximum-iteration boundary refiner against the
    // CPU function, including its temporary maxIt+1 and rollback state.
    const std::array<double,3> rays[]={{0,0,1},{.125,-.25,.75},{-.5,.25,-.125}};
    for(unsigned index=0;index<points.size();index+=73) {
        for(const auto& ray:rays) {
            auto local=m;
            auto it=initial;
            local.pIt3Dext=&it;local.msDEstop=.25f;local.mZZ=3.5;
            std::memcpy(local.mVgradsFOV,ray.data(),sizeof(double)*3);
            it.C1=points[index][0];it.C2=points[index][1];it.C3=points[index][2];
            local.CalcDE(&it,&local);
            double depth=3.5;
            mb3d::RMdoBinSearchIt(&local,&depth);
            const auto actual=refine_iteration_boundary(exact(points[index]),exact(ray),exact(3.5),exact(.25),scene);
            same(actual.position.x,it.C1,"refine.x",power);
            same(actual.position.y,it.C2,"refine.y",power);
            same(actual.position.z,it.C3,"refine.z",power);
            same(actual.depth,depth,"refine.depth",power);
            same(actual.smooth,it.SmoothItD,"refine.smooth",power);
            same_integer(actual.iteration,it.ItResultI,"refine.iteration",power);
            same_integer(local.iMaxIt,m.iMaxIt,"refine.restoredMax",power);
            ++refinement_records;
        }
    }
}
}
int main() {
#if !defined(__APPLE__) || !defined(__aarch64__)
    std::fprintf(stderr,"Internal exact Metal formula contract requires native Apple ARM64.\n");
    return 2;
#else
    static_assert(sizeof(long double)==sizeof(double));
    try {
        test_numerical_de_fixtures();
        const auto root=std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
        test_scene(root/"comparison/scenes/test_01_intpow2.m3p",2);
        test_scene(root/"comparison/scenes/test_02_intpow5.m3p",5);
        test_scene(root/"comparison/scenes/test_03_intpow8.m3p",8);
        for(unsigned power:{3u,4u,6u,7u})
            test_scene(root/"comparison/scenes/test_03_intpow8.m3p",power,true);
        std::printf("Internal Mandelbulb exact: %zu checks, %zu step records, %zu orbit records, "
                    "%zu numerical-DE records, %zu refinement records, %zu failures.\n",
                    checks,step_records,orbit_records,de_records,refinement_records,failures);
        return failures?1:0;
    } catch(const std::exception& error) {
        std::fprintf(stderr,"Internal Mandelbulb exact FAILED: %s\n",error.what());
        return 1;
    }
#endif
}
