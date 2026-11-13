// Single-slot Manowar doHybridPas caller profile. Rows:
// Cx Cy Cz Jx Jy Jz julia maxIt RStop. No caller registers are seeded by the test.
#include "core/external_formulas.h"
#include "core/formulas.h"
#include "core/header_trafos.h"
#include "io/m3p_parser.h"
#include <cstddef>
#include <cstdio>
#include <cstring>

static const mb3d::ExternalFormulaInfo* formula;
static double lanes[64][4];
static unsigned calls;

static void plain(double& x, double& y, double& z, double& w, void* state) {
    formula->wrapperFunc(x, y, z, w, static_cast<TPIteration3D>(state));
}

static void capture(double& x, double& y, double& z, double& w, void* state) {
    auto it = static_cast<TPIteration3D>(state);
    auto ext = reinterpret_cast<TIteration3Dext*>(
        reinterpret_cast<char*>(it) - offsetof(TIteration3Dext, C1));
    if (calls < 64) std::memcpy(lanes[calls], ext->FormulaSse01, sizeof(lanes[0]));
    ++calls;
    formula->wrapperFunc(x, y, z, w, it);
}

int main(int argc, char** argv) {
    if (argc != 4) return 2;
    const bool trace = std::strcmp(argv[3], "trace") == 0;
    if (!trace && std::strcmp(argv[3], "plain") != 0) return 2;
    mb3d::InitExternalFormulaRegistry();
    formula = mb3d::FindExternalFormula("ManowarBulb");
    auto loaded = mb3d::LoadM3PFile(argv[1]);
    if (!formula || !loaded.success) return 3;
    int slot = -1;
    for (int i=0; i<6; ++i)
        if (std::strncmp(reinterpret_cast<const char*>(loaded.addon.Formulas[i].CustomFname), "ManowarBulb", 32) == 0) slot=i;
    if (slot != 0 || (loaded.addon.bOptions1 & 3) != 0) return 4;
    // Qualify the real loaded dispatcher/parameters before applying this probe's
    // explicit XYZ, Julia, maxIt and RStop matrix. Never force an arbitrary scene
    // into a single-slot profile and then describe that as its production caller.
    const auto runtime = mb3d::GetMCTparasFromHeader(loaded.header, &loaded.addon, false);
    if (runtime.mMandFunction != mb3d::doHybridPas || runtime.DEoption != 0 ||
        runtime.StartFrom1 != 0 || runtime.RepeatFrom1 != 0 || runtime.wEndTo != 0 ||
        runtime.nHybrid[0] != 1 || runtime.fHPVar[0] == nullptr) return 7;
    for (int i=1; i<6; ++i) if (runtime.nHybrid[i] != 0) return 7;
    alignas(16) unsigned char params[1024];
    std::memcpy(params, static_cast<const unsigned char*>(runtime.fHPVar[slot])-512, sizeof(params));
    FILE* out = std::fopen(argv[2], "wb");
    if (!out) return 5;
    const auto written = std::fwrite(params, 1, sizeof(params), out);
    std::fclose(out);
    if (written != sizeof(params)) return 5;
    std::fprintf(stderr, "caller_profile=fpc-i386-sse2-single-slot-doHybridPas-v1 driver=doHybridPas slot=0 counts=1,0,0,0,0,0 start=0 repeat=0 end=0 DEoption=0\n");
    double cx,cy,cz,jx,jy,jz,stop;
    int julia,limit;
    while (std::scanf("%lf %lf %lf %lf %lf %lf %d %d %lf", &cx,&cy,&cz,&jx,&jy,&jz,&julia,&limit,&stop)==9) {
        if (limit<1 || limit>64) return 6;
        TIteration3Dext ext{};
        ext.C1=cx; ext.C2=cy; ext.C3=cz;
        ext.Ju1=jx; ext.Ju2=jy; ext.Ju3=jz;
        ext.DoJulia=julia!=0; ext.maxIt=limit; ext.RStop=static_cast<float>(stop);
        ext.VaryScale=1; ext.Deriv1=1;
        ext.nHybrid[0]=1; ext.fHPVar[0]=params+512;
        ext.fHybrid[0]=trace ? capture : plain;
        calls=0;
        mb3d::doHybridPas(reinterpret_cast<TPIteration3D>(&ext.C1));
        std::printf("%.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g %d %d %u ",
            ext.x,ext.y,ext.z,ext.w,ext.Rout,ext.Rold,ext.OTrap,
            ext.Dfree1,ext.Dfree2,ext.Deriv1,ext.Deriv2,ext.ItResultI,ext.bFirstIt,calls);
        for(unsigned i=0;i<calls && i<64;++i) for(double value:lanes[i]) std::printf("%.17g ",value);
        std::puts("");
    }
}
