#include "core/external_formulas.h"
#include "core/header_trafos.h"
#include "core/thread_pool.h"

#include <array>
#include <climits>
#include <cstdio>
#include <stdexcept>
#include <string>

#ifdef MB3D_SOFTWARE_EXT80
#include "core/extended_transcendentals.h"
namespace mb3d {
#include "../external_formulas/Formulas/Ads/extended_polygon_sector.h"
#include "../external_formulas/Formulas/dIFS/ifs_original_state.h"
}
#endif

// Compile the real portable bodies even on the native Win64 host. The local
// adapter changes only the test budget, so boundary tests need eight steps.
namespace portable_test {
// A local value type keeps ADL from importing the production execute_formula
// template into these independently instantiated portable specializations.
struct cpu_double3 {
    double x,y,z;
    cpu_double3(double x_,double y_,double z_) : x(x_),y(y_),z(z_) {}
};
using mb3d::DIFS_VARYSCALE_SLOT;
using mb3d::DIFS_COMMIT_XYZ_SLOT;
struct FormulaInfo {
    unsigned int id; const char* name; int param_count; int complexity;
    bool uses_transcendental;
};
class FormulaWorkBudget : public mb3d::FormulaWorkBudget {
public:
    explicit FormulaWorkBudget(const char* name) : mb3d::FormulaWorkBudget(name, 8) {}
};
template<unsigned int> cpu_double3 execute_formula(cpu_double3, double*, double&);
#define constant
#define MB3D_FORCE_PORTABLE_FORMULA_LOOPS
#include "../external_formulas/Formulas/3D/_e_3Dtess.inc"
#include "../external_formulas/Formulas/3D/_IFS_tess.inc"
#include "../external_formulas/Formulas/dIFS/Knots2IFS.inc"
#undef MB3D_FORCE_PORTABLE_FORMULA_LOOPS
#undef constant
}

#ifdef MB3D_SOFTWARE_EXT80
namespace arm64_test {
struct cpu_double3 {
    double x,y,z;
    cpu_double3(double x_,double y_,double z_) : x(x_),y(y_),z(z_) {}
};
using portable_test::FormulaInfo;
using mb3d::DIFS_VARYSCALE_SLOT;
using mb3d::DIFS_COMMIT_XYZ_SLOT;
using mb3d::DIFS_ROLD_SLOT;
using mb3d::MAX_FORMULA_PARAMS;
namespace numeric=mb3d::numeric;
class FormulaWorkBudget : public mb3d::FormulaWorkBudget {
public:
    inline static uint32_t limit=8, completed=0;
    explicit FormulaWorkBudget(const char* name) : mb3d::FormulaWorkBudget(name,limit) {
        completed=0;
    }
    ~FormulaWorkBudget() { completed=steps(); }
};
// Only adapt the local value type: execute the production IFS preamble and
// finish helper while keeping ADL separate from production formula dispatch.
struct Arm64ApplyIFSState : mb3d::Arm64ApplyIFSState {
    Arm64ApplyIFSState(cpu_double3 p,double* params)
        : mb3d::Arm64ApplyIFSState({p.x,p.y,p.z},params) {}
    cpu_double3 finish(E distance,double& de) const {
        const auto v=mb3d::Arm64ApplyIFSState::finish(distance,de);
        return {v.x,v.y,v.z};
    }
};
template<unsigned int> cpu_double3 execute_formula(cpu_double3, double*, double&);
#define constant
#include "../external_formulas/Formulas/3D/_IFS_tess.inc"
#include "../external_formulas/Formulas/dIFS/ducksIFS.inc"
#include "../external_formulas/Formulas/dIFS/gnarlyIFS.inc"
#undef constant
}
#endif

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" unsigned short GetX87ControlWord();
extern "C" void BeginFixtureX87(void*);
extern "C" void EndFixtureX87(void*);
extern "C" void SetFixtureX87ControlWord(unsigned short);
#endif

namespace {
using Params = std::array<double, mb3d::MAX_FORMULA_PARAMS>;
int checks = 0;
void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
template<class F> void exhausted(F&& action, const char* name, uint32_t limit) {
    try { action(); }
    catch (const mb3d::FormulaWorkError& error) {
        check(error.steps() == limit && error.limit() == limit,
              "work error does not report the exact completed step count");
        check(std::string(error.what()).find("Formula work budget exhausted (formula=" +
              std::string(name) + ",") == 0, "work error lacks formula identity");
        return;
    }
    throw std::runtime_error("expected typed work exhaustion, got success");
}

Params e3Params(int count) {
    Params p{};
    p[3] = p[4] = p[5] = 1; p[7] = count; p[9] = 2;
    // Synthetic constant-buffer witness: zero growth prevents the geometric
    // exit and isolates the original signed maxDepth branch (0189..019D).
    p[8] = 0; p[12] = -1;
    return p;
}
Params ifsParams(int count) {
    Params p{};
    p[3] = p[5] = p[10] = 1; p[7] = count; p[9] = .5;
    p[11] = M_PI / 180.; p[12] = 500;
    // saved=2,x=y=0, displacement=0 is stationary. The original unsigned
    // depth starts at zero and is compared AFTER the body (0291..02B1).
    return p;
}
Params knotsParams(int count) {
    Params p{};
    p[0] = p[1] = p[7] = p[8] = p[9] = 1; p[11] = count;
    p[13] = 2 * M_PI; p[14] = 1e20;
    p[mb3d::DIFS_VARYSCALE_SLOT] = 1;
    return p;
}

void portableChecks() {
    double de = 123;
    for (int count : {INT_MIN, -1, 0, 1, 8}) {
        auto e = e3Params(count);
        auto v = portable_test::execute_formula<portable_test::FORMULA_E3DTESS>({0,0,0}, e.data(), de);
        check(v.x == 0 && v.y == 0 && v.z == 0, "portable signed tess finite result");
        auto k = knotsParams(count);
        v = portable_test::execute_formula<portable_test::FORMULA_KNOTS2IFS>({1,0,0}, k.data(), de);
        check(v.x == 1 && v.y == 0 && v.z == 0 && de == 1, "portable DEC/JG finite result");
    }
    for (int count : {9, INT_MAX}) {
        auto e = e3Params(count);
        exhausted([&] { portable_test::execute_formula<portable_test::FORMULA_E3DTESS>({0,0,0}, e.data(), de); }, "_e_3Dtess", 8);
        auto k = knotsParams(count);
        exhausted([&] { portable_test::execute_formula<portable_test::FORMULA_KNOTS2IFS>({1,0,0}, k.data(), de); }, "Knots2IFS", 8);
    }
    for (int count : {0, 1, 7}) {
        auto p = ifsParams(count);
        const auto v = portable_test::execute_formula<portable_test::FORMULA_IFSTESS>({2,0,0}, p.data(), de);
        check(v.x == 1000 && v.y == 0 && v.z == 0, "portable unsigned tess finite result");
    }
    for (int count : {8, -1, INT_MIN, INT_MAX}) {
        auto p = ifsParams(count);
        exhausted([&] { portable_test::execute_formula<portable_test::FORMULA_IFSTESS>({2,0,0}, p.data(), de); }, "_IFS_tess", 8);
        const auto v = portable_test::execute_formula<portable_test::FORMULA_IFSTESS>({0,0,0}, p.data(), de);
        check(v.x == 0 && v.y == 0 && v.z == 0, "huge unsigned tess must retain geometric early return");
    }
    auto p = e3Params(INT_MAX); p[8] = .5;
    const auto v = portable_test::execute_formula<portable_test::FORMULA_E3DTESS>({1.5,0,0}, p.data(), de);
    check(v.x == 0 && v.y == 1 && v.z == 1, "huge signed tess must retain geometric early return");
}

TIteration3Dext invoke(const char* name, const double* options, int count, mb3d::cpu_double3 xyz) {
    const auto* info = mb3d::FindExternalFormula(name);
    check(info && info->wrapperFunc, "production formula missing");
    THeaderCustomAddon addon{};
    auto& f = addon.Formulas[0]; f.iFnr = 20; f.iOptionCount = count;
    std::memcpy(f.CustomFname, name, std::strlen(name));
    std::memcpy(f.dOptionValue, options, count * sizeof(double));
    mb3d::ApplyExternalFormulaOptionTypes(f);
    mb3d::BuildFormulaConstBuffer(0, &addon, 0);
    TIteration3Dext ext{};
    ext.x=xyz.x; ext.y=xyz.y; ext.z=xyz.z; ext.w=.375; ext.VaryScale=1;
    ext.PVar=mb3d::GetFormulaConstBuffers().pConstPointer[0]; ext.fHPVar[0]=ext.PVar;
    info->wrapperFunc(ext.x,ext.y,ext.z,ext.w,reinterpret_cast<TPIteration3D>(&ext.C1));
    return ext;
}

void productionChecks() {
    // Independent immutable x86 witnesses already captured in the named
    // fixture headers; these are literal rows, not candidate-derived values.
    const double e[] = {0,0,0,2,1,1,0,5};
    auto r=invoke("_e_3Dtess",e,8,{-2,-.125,.5}); // e3dtess_regressions.h row 0
    check(r.x==2 && r.y==1 && r.z==-2, "original tess fixture changed");
    const double i[] = {0,0,0,1,1.707,1,-45,10,1};
    r=invoke("_IFS_tess",i,9,{0,0,2}); // ifstess_regressions.h row 2
#if (defined(_MSC_VER) && defined(_M_X64)) || defined(MB3D_SOFTWARE_EXT80)
    check(r.x==0 && r.y==.79296870791205132 && r.z==-.79296874217234725,
          "original IFS tess fixture changed");
#else
    check(std::isfinite(r.x) && std::isfinite(r.y) && std::isfinite(r.z),
          "portable IFS tess unexpectedly failed finite input");
#endif
    double k[] = {2,-2,.125,-.25,.5,.25,0,-3,2,4,-1,double(INT_MIN)};
    r=invoke("Knots2IFS",k,12,{-2,-.125,.5}); // knots_weird_parameters_regressions.h row 0
#if defined(_MSC_VER) && defined(_M_X64)
    check(r.x==-2 && r.y==-.125 && r.z==.5 && r.Rout==-1.6802686038634622,
          "original knots INT_MIN fixture changed");
#else
    check(r.x==-2 && r.y==-.125 && r.z==.5 && std::isfinite(r.Rout),
          "portable knots INT_MIN did not finish");
#endif
    k[11]=INT_MAX;
    exhausted([&] { invoke("Knots2IFS",k,12,{-2,-.125,.5}); }, "Knots2IFS",
              mb3d::FormulaWorkPolicy::maxInnerStepsPerCall);
    k[11]=1;
    r=invoke("Knots2IFS",k,12,{-2,-.125,.5});
    check(std::isfinite(r.Rout), "finite call after exhausted native callback failed");
}

#if defined(_MSC_VER) && defined(_M_X64)
class X87Environment {
public:
    X87Environment() { BeginFixtureX87(state_.data()); }
    ~X87Environment() { EndFixtureX87(state_.data()); }
private:
    std::array<unsigned char,108> state_{};
};

uint32_t knotsNative(double* state, Params& p, mb3d::FormulaWorkCounter* counter) {
    alignas(16) unsigned char buffer[176]{};
    auto* var=buffer+128;
    auto real=[&](int offset,double value){std::memcpy(var+offset,&value,8);};
    auto integer=[&](int offset,double value){const int32_t v=int32_t(value);std::memcpy(var+offset,&v,4);};
    real(-16,p[0]); real(-32,p[1]); real(-24,p[1]); real(-48,p[2]); real(-40,p[2]);
    real(-56,p[3]); real(-64,p[4]); real(-72,p[5]); integer(-76,p[6]);
    integer(-80,p[7]); integer(-84,p[8]); integer(-88,p[9]); real(-96,p[10]); integer(-100,p[11]);
    for(int j=0;j<4;++j) real(8*j,p[12+j]);
    return mb3d::Knots2IFSOriginal_x87(state,var,counter);
}

void nativeChecks() {
    X87Environment environment;
    for (unsigned short control : {0x037fu,0x077fu,0x0b7fu,0x0f7fu}) {
        SetFixtureX87ControlWord(control);
        for (int count : {INT_MIN,-1,0,1,8,9,INT_MAX}) {
            auto e=e3Params(count); double state[9]{};
            mb3d::FormulaWorkBudget budget("_e_3Dtess",8);
            const uint32_t status=mb3d::E3DtessOriginal_x87(state,e.data(),0,count,budget.nativeCounter());
            check(GetX87ControlWord()==control,"e3tess x87 CW leaked");
            const unsigned expected=count>8?8:unsigned(std::max(count,1));
            check(budget.steps()==expected && status==unsigned(count>8),"native signed depth step boundary");
            if(status) exhausted([&]{budget.checkNativeStatus(status);},"_e_3Dtess",8);

            auto k=knotsParams(count); double ks[]={1,0,0,1,0};
            mb3d::FormulaWorkBudget kb("Knots2IFS",8);
            const auto kr=knotsNative(ks,k,kb.nativeCounter());
            check(GetX87ControlWord()==control,"knots x87 CW leaked");
            check(kb.steps()==expected && kr==unsigned(count>8),"native DEC/JG step boundary");
            if(kr) exhausted([&]{kb.checkNativeStatus(kr);},"Knots2IFS",8);
            else check(ks[4]==1,"native finite knots arithmetic changed");
        }
        for(int count : {INT_MIN,-1,0,1,7,8,INT_MAX}) {
            // Original flag branches keep a point outside the central square
            // rotating at norm=1. The uncertified portable geometry differs;
            // its own stationary witness above checks the same work policy.
            auto p=ifsParams(count); double state[]={0,2,0};
            mb3d::FormulaWorkBudget budget("_IFS_tess",8);
            const auto status=mb3d::IFSTessOriginal_x87(state,p.data(),budget.nativeCounter());
            const bool over=count<0 || count>=8;
            check(GetX87ControlWord()==control,"IFS tess x87 CW leaked");
            check(budget.steps()==(over?8:unsigned(count+1)) && status==unsigned(over),
                  "native unsigned depth step boundary");
            if(status) exhausted([&]{budget.checkNativeStatus(status);},"_IFS_tess",8);
        }
        auto e=e3Params(INT_MAX); e[8]=.5; double es[9]={1.5,0,0};
        mb3d::FormulaWorkCounter ec{0,1};
        check(mb3d::E3DtessOriginal_x87(es,e.data(),0,INT_MAX,&ec)==0 && ec.steps==1,
              "huge signed depth geometric return rejected");
        auto p=ifsParams(INT_MAX); double is[]={2,0,0};
        mb3d::FormulaWorkCounter ic{0,0};
        check(mb3d::IFSTessOriginal_x87(is,p.data(),&ic)==0 && ic.steps==0,
              "huge unsigned depth geometric return consumed work");
        check(GetX87ControlWord()==control,"geometric early exit x87 CW leaked");
    }
}
#endif

void workerChecks() {
    mb3d::ThreadPool pool(2);
    auto failure=pool.submit([]{
        auto p=knotsParams(INT_MAX); double de=0;
        portable_test::execute_formula<portable_test::FORMULA_KNOTS2IFS>({1,0,0},p.data(),de);
    });
    pool.wait_all();
    exhausted([&]{failure.get();},"Knots2IFS",8);
    bool recovered=false;
    auto success=pool.submit([&]{ recovered=true; });
    pool.wait_all(); success.get();
    check(recovered,"pool failed to recover from formula work exception");
}

#ifdef MB3D_SOFTWARE_EXT80
void arm64TessChecks() {
    double de=123;
    for(int count : {INT_MIN,-1,0,1,7,8,INT_MAX}) {
        auto p=ifsParams(count);
        const auto call=[&] {
            return arm64_test::execute_formula<arm64_test::FORMULA_IFSTESS>({0,2,0},p.data(),de);
        };
        if(count<0 || count>=8) exhausted(call,"_IFS_tess",8);
        else {
            const auto v=call();
            check(v.x==0 && v.y==1000 && v.z==0,"ARM64 unsigned tess depth boundary");
        }
        const auto early=arm64_test::execute_formula<arm64_test::FORMULA_IFSTESS>({2,0,0},p.data(),de);
        check(early.x==2 && early.y==0 && early.z==0,"ARM64 unsigned tess geometric early return");
    }
    // A constant-buffer witness isolates the original high-word read: 50 is
    // skipped by the loader, not a second iteration bound imposed by the port.
    auto p=ifsParams(INT_MAX);
    p[12]=std::bit_cast<double>(uint64_t(0));
    const auto clipped=arm64_test::execute_formula<arm64_test::FORMULA_IFSTESS>({0,2,0},p.data(),de);
    check(clipped.x==0 && clipped.y==0 && clipped.z==0,"ARM64 tess constant high-word bound ignored");
    check(de==123,"ARM64 tess must preserve caller DE");
}

void arm64HeightfieldChecks() {
    using Budget=arm64_test::FormulaWorkBudget;
    for (bool ducks : {false,true}) {
        const char* name=ducks ? "ducksIFS" : "gnarlyIFS";
        auto params=[&](int count) {
            Params p{};
            p[1]=1; p[5]=1; p[7]=count;
            p[mb3d::DIFS_VARYSCALE_SLOT]=1;
            if (ducks) {
                p[9]=1; p[11]=1; p[13]=1;
                p[14]=.5; p[15]=.03; p[16]=.1;
            } else {
                p[12]=1; p[13]=1.5; p[14]=.1;
            }
            return p;
        };
        auto invokeLocal=[&](Params& p,arm64_test::cpu_double3 xyz,double& de) {
            return ducks
                ? arm64_test::execute_formula<arm64_test::FORMULA_DUCKSIFS>(xyz,p.data(),de)
                : arm64_test::execute_formula<arm64_test::FORMULA_GNARLYIFS>(xyz,p.data(),de);
        };
        for (int count : {INT_MIN,-1,0,1,8,9,INT_MAX}) {
            auto p=params(count);
            double de=123;
            Budget::limit=8; Budget::completed=0;
            if (count>8) {
                exhausted([&]{invokeLocal(p,{1,1,0},de);},name,8);
                check(Budget::completed==8,"ARM64 heightfield failure must occur after eight bodies");
            } else {
                const auto v=invokeLocal(p,{1,1,0},de);
                check(Budget::completed==unsigned(std::max(count,1)),
                      "ARM64 DEC/JG body count changed, including INT_MIN overflow flags");
                check(v.x==1 && v.y==1 && v.z==0 && std::isfinite(p[mb3d::MAX_FORMULA_PARAMS-1]),
                      "ARM64 finite heightfield did not preserve orbit state");
                check(ducks && count==0 ? std::isinf(de) : std::isfinite(de),
                      "ARM64 heightfield zero-count divisor or finite result changed");
            }
            // Zero loop allowance proves the geometric exit precedes the
            // guard even for a positive count that would otherwise exhaust.
            p=params(count); Budget::limit=0;
            p[mb3d::MAX_FORMULA_PARAMS-1]=.625;
            const auto v=invokeLocal(p,{1,1,10},de);
            check(v.x==1 && v.y==1 && v.z==10 && de==-10
                  && p[mb3d::MAX_FORMULA_PARAMS-1]==.625,
                  "ARM64 heightfield early exit consumed work or overwrote the orbit trap");
        }
        Budget::limit=8;
        auto p=params(1); double de=123;
        invokeLocal(p,{1,1,0},de);
        check(std::isfinite(de) && Budget::completed==1,
              "ARM64 heightfield failed to recover after work rejection");
    }
}

void polygonChecks() {
    using E=mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    for (int steps : {0,1,7,8}) {
        mb3d::FormulaWorkBudget budget("polygon-test",8);
        const E result=mb3d::extendedPolygonSector(E(2*steps+1),1,2,budget);
        check(result==E(1) && budget.steps()==unsigned(steps),
              "polygon sector equality/last permitted step changed");
    }
    mb3d::FormulaWorkBudget excess("polygon-test",8);
    exhausted([&]{ mb3d::extendedPolygonSector(E(19),1,2,excess); },"polygon-test",8);
    mb3d::FormulaWorkBudget stalled("polygon-test",8);
    exhausted([&]{ mb3d::extendedPolygonSector(E(2),1,0,stalled); },"polygon-test",8);
    mb3d::FormulaWorkBudget unordered("polygon-test",0);
    const E nan=mb3d::numeric::extendedNaN();
    const E unorderedResult=mb3d::extendedPolygonSector(nan,1,2,unordered);
    check(unorderedResult!=unorderedResult
          && unordered.steps()==0,"unordered polygon must exit without work");
    // Production dispatch must reject stalled sectors, rather than returning
    // a truncated transform; the next valid call must still work.
    const double polygon[]={-4};
    const double polyhedron[]={4,-4};
    const double stella[]={-4,4,0,0};
    for (const char* name : {"_BPolygonFromCircle","_BPolygonToCircle"})
        exhausted([&]{ invoke(name,polygon,1,{1,1,1}); },name,
                  mb3d::FormulaWorkPolicy::maxInnerStepsPerCall);
    exhausted([&]{ invoke("_BPolyhedronToSphere",polyhedron,2,{1,1,1}); },
              "_BPolyhedronToSphere",mb3d::FormulaWorkPolicy::maxInnerStepsPerCall);
    exhausted([&]{ invoke("_BStellahedron_F_Sphere",stella,4,{1,1,1}); },
              "_BStellahedron_F_Sphere",mb3d::FormulaWorkPolicy::maxInnerStepsPerCall);
    const double valid[]={4};
    const auto result=invoke("_BPolygonFromCircle",valid,1,{1,1,0});
    check(result.x==1 && result.y==1 && result.z==0,
          "finite polygon dispatch after work rejection failed");
}
#endif
}

int main() {
    try {
        static_assert(mb3d::FormulaWorkPolicy::maxInnerStepsPerCall==(1u<<20));
        portableChecks();
#if defined(_MSC_VER) && defined(_M_X64)
        nativeChecks();
#endif
        productionChecks(); workerChecks();
#ifdef MB3D_SOFTWARE_EXT80
        arm64TessChecks();
        arm64HeightfieldChecks();
        polygonChecks();
#endif
        std::printf("Formula work guard: %d checks PASS (native and forced portable where available)\n",checks);
        return 0;
    } catch(const std::exception& error) {
        std::fprintf(stderr,"Formula work guard FAIL after %d checks: %s\n",checks,error.what());
        return 1;
    }
}
