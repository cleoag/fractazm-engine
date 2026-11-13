// Actual loaded ordinary two-slot drivers; register state is observed, not seeded.
#include "core/external_formulas.h"
#include "core/formulas.h"
#include "core/header_trafos.h"
#include "io/m3p_parser.h"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
static ThybridIteration2 callbacks[2];
static double lanes[128][4];
static unsigned slots[128], calls;
template<unsigned S> void capture(double& x,double& y,double& z,double& w,void* p) {
    auto it=static_cast<TPIteration3D>(p);
    auto ext=reinterpret_cast<TIteration3Dext*>(reinterpret_cast<char*>(it)-offsetof(TIteration3Dext,C1));
    if(calls<128) {slots[calls]=S;std::memcpy(lanes[calls],ext->FormulaSse01,sizeof(lanes[0]));}
    ++calls;callbacks[S](x,y,z,w,p);
}
int main(int argc,char**argv) {
    if(argc!=4) return 2;
    bool trace=std::strcmp(argv[3],"trace")==0;
    if(!trace&&std::strcmp(argv[3],"plain"))return 2;
    mb3d::InitExternalFormulaRegistry();auto loaded=mb3d::LoadM3PFile(argv[1]);
    if(!loaded.success)return 3;
    auto rt=mb3d::GetMCTparasFromHeader(loaded.header,&loaded.addon,false);
    int driver=-1; const char* name="";
    if(rt.IsCustomDE) {
        if(rt.mMandFunctionDE==mb3d::doInterpolHybridPasDE){driver=2;name="doInterpolHybridPasDE";}
        if(rt.mMandFunctionDE==mb3d::doInterpolHybridPas4DDE){driver=4;name="doInterpolHybridPas4DDE";}
    }else{
        if(rt.mMandFunction==mb3d::doHybridPas){driver=0;name="doHybridPas";}
        if(rt.mMandFunction==mb3d::doInterpolHybridPas){driver=1;name="doInterpolHybridPas";}
        if(rt.mMandFunction==mb3d::doInterpolHybridPas4D){driver=3;name="doInterpolHybridPas4D";}
    }
    if(driver<0)return 4;
    for(int i=2;i<6;++i)if(rt.nHybrid[i])return 5;
    if(!rt.fHPVar[0]||!rt.fHPVar[1])return 6;
    std::filesystem::path out(argv[2]);std::filesystem::create_directories(out);
    alignas(16) unsigned char params[2][1024];
    for(int i=0;i<2;++i){
        std::memcpy(params[i],static_cast<const unsigned char*>(rt.fHPVar[i])-512,1024);
        FILE*f=std::fopen((out/("params"+std::to_string(i)+".bin")).string().c_str(),"wb");
        if(!f)return 7;auto n=std::fwrite(params[i],1,1024,f);std::fclose(f);if(n!=1024)return 7;
        callbacks[i]=rt.fHybrid[i];
    }
    FILE*f=std::fopen((out/"runtime.json").string().c_str(),"w");if(!f)return 7;
    std::fprintf(f,"{\"driver\":\"%s\",\"driver_id\":%d,\"DEoption\":%d,\"is_custom_de\":%s,\"start\":%d,\"repeat\":%d,\"end\":%d,\"counts\":[%d,%d]}\n",
        name,driver,rt.DEoption,rt.IsCustomDE?"true":"false",rt.StartFrom1,rt.RepeatFrom1,rt.wEndTo,rt.nHybrid[0],rt.nHybrid[1]);std::fclose(f);
    double cx,cy,cz,jx,jy,jz,stop;int julia,limit;
    while(std::scanf("%lf %lf %lf %lf %lf %lf %d %d %lf",&cx,&cy,&cz,&jx,&jy,&jz,&julia,&limit,&stop)==9){
        if(limit<1||limit>32)return 8;
        TIteration3Dext e{};e.C1=cx;e.C2=cy;e.C3=cz;e.Ju1=jx;e.Ju2=jy;e.Ju3=jz;
        e.DoJulia=julia!=0;e.maxIt=limit;e.RStop=static_cast<float>(stop);e.VaryScale=1;e.Deriv1=1;e.DEoption=rt.DEoption;
        for(int i=0;i<4;++i)e.SMatrix4[i][i]=1;
        e.iStartFrom=rt.StartFrom1;e.iRepeatFrom=rt.RepeatFrom1;e.EndTo=rt.wEndTo;
        for(int i=0;i<2;++i){e.nHybrid[i]=rt.nHybrid[i];e.fHPVar[i]=params[i]+512;e.fHybrid[i]=callbacks[i];}
        if(trace){e.fHybrid[0]=capture<0>;e.fHybrid[1]=capture<1>;} calls=0;
        double de=0; if(rt.IsCustomDE)de=rt.mMandFunctionDE(reinterpret_cast<TPIteration3D>(&e.C1));else rt.mMandFunction(reinterpret_cast<TPIteration3D>(&e.C1));
        std::printf("%.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g %d %d %u ",
            e.x,e.y,e.z,e.w,e.Rout,e.Rold,e.OTrap,e.Dfree1,e.Dfree2,e.Deriv1,e.Deriv2,de,e.ItResultI,e.bFirstIt,calls);
        for(unsigned i=0;i<calls&&i<128;++i){std::printf("%u ",slots[i]);for(double v:lanes[i])std::printf("%.17g ",v);}std::puts("");
    }
}
