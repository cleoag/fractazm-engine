// Golden inputs/outputs come from original i386 Pascal drivers and CODE.
#include "core/external_formulas.h"
#include "core/formulas.h"
#include "core/header_trafos.h"
#include "io/m3p_parser.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#ifndef MB3D_MANOWAR_FIXTURE_DIR
#define MB3D_MANOWAR_FIXTURE_DIR "tests/fixtures/manowar_hybrids"
#define MB3D_MANOWAR_FIXTURE_NAMES "alt_mm","alt_ms","alt_sm","alt_tm","alt_mt","alt_rm","interp_mm","interp_ms","interp_sm","interp_zero","interp_de_zero","interp_4d_zero","interp_4dde_zero"
#define MB3D_MANOWAR_FIXTURE_CHECKS 41693
#endif
static ThybridIteration2 callbacks[2];
static double lanes[128][4];static int slots[128],calls;
template<int S>void capture(double&x,double&y,double&z,double&w,void*p){
 auto e=reinterpret_cast<TIteration3Dext*>(static_cast<char*>(p)-offsetof(TIteration3Dext,C1));
 if(calls>=128)throw std::runtime_error("capture overflow");slots[calls]=S;std::memcpy(lanes[calls],e->FormulaSse01,sizeof(lanes[0]));++calls;callbacks[S](x,y,z,w,p);
}
template<class T>void read(std::ifstream&f,T&v){f.read(reinterpret_cast<char*>(&v),sizeof(v));if(!f)throw std::runtime_error("truncated independent fixture");}
bool same(double x,double y){return x==y||(std::isnan(x)&&std::isnan(y))||(std::isfinite(x)&&std::isfinite(y)&&std::abs(x-y)<=2e-12*std::max(1.,std::abs(y)));}
int main(int argc,char**argv){try{
 mb3d::InitExternalFormulaRegistry();std::filesystem::path dir=std::filesystem::path(MB3D_TEST_SOURCE_DIR)/MB3D_MANOWAR_FIXTURE_DIR;
 const char* names[]={MB3D_MANOWAR_FIXTURE_NAMES};
 constexpr uint32_t expectedCases=sizeof(names)/sizeof(names[0]);
 if(argc>2)throw std::runtime_error("expected at most one fixture override");
 std::ifstream f(argc==2?std::filesystem::path(argv[1]):dir/"original.bin",std::ios::binary);char magic[8];read(f,magic);if(std::memcmp(magic,"MANOHYB1",8))throw std::runtime_error("fixture format");uint32_t nc;read(f,nc);if(nc!=expectedCases)throw std::runtime_error("wrong exact caller profile count");int failures=0,checks=0,orbits=0;
 for(uint32_t ci=0;ci<nc;++ci){char name[64];int32_t cfg[8];read(f,name);read(f,cfg);if(!std::memchr(name,0,64)||std::strcmp(name,names[ci]))throw std::runtime_error("unexpected caller profile name/order");if(cfg[7]!=96)throw std::runtime_error("expected exactly 96 orbits per profile");auto loaded=mb3d::LoadM3PFile((dir/(std::string(name)+".m3p")).string());if(!loaded.success)throw std::runtime_error("fixture load");auto rt=mb3d::GetMCTparasFromHeader(loaded.header,&loaded.addon,false);
  int driver=rt.IsCustomDE?(rt.mMandFunctionDE==mb3d::doInterpolHybridPasDE?2:rt.mMandFunctionDE==mb3d::doInterpolHybridPas4DDE?4:-1):rt.mMandFunction==mb3d::doHybridPas?0:rt.mMandFunction==mb3d::doInterpolHybridPas?1:rt.mMandFunction==mb3d::doInterpolHybridPas4D?3:-1;
  if(driver!=cfg[0]||rt.DEoption!=cfg[1]||rt.StartFrom1!=cfg[2]||rt.RepeatFrom1!=cfg[3]||rt.wEndTo!=cfg[4]||rt.nHybrid[0]!=cfg[5]||rt.nHybrid[1]!=cfg[6])throw std::runtime_error("loaded caller prerequisites changed");
  const int caseChecks=checks, caseFailures=failures;
  bool manowar[2];for(int i=0;i<2;++i){callbacks[i]=rt.fHybrid[i];manowar[i]=std::strncmp(reinterpret_cast<const char*>(loaded.addon.Formulas[i].CustomFname),"ManowarBulb",32)==0;}
  for(int row=0;row<cfg[7];++row){double in[9],expected[12];int32_t counters[3];read(f,in);read(f,expected);read(f,counters);if(counters[0]<1||counters[0]>32||counters[1]<0||counters[1]>1||counters[2]<1||counters[2]>128)throw std::runtime_error("invalid fixture counters");for(double v:in)if(!std::isfinite(v))throw std::runtime_error("nonfinite fixture input");if(in[7]<1||in[7]>32||std::floor(in[7])!=in[7])throw std::runtime_error("invalid iteration limit");TIteration3Dext e{};
   e.C1=in[0];e.C2=in[1];e.C3=in[2];e.Ju1=in[3];e.Ju2=in[4];e.Ju3=in[5];e.DoJulia=in[6]!=0;e.maxIt=static_cast<int>(in[7]);e.RStop=static_cast<float>(in[8]);e.VaryScale=1;e.Deriv1=1;e.DEoption=rt.DEoption;for(int i=0;i<4;++i)e.SMatrix4[i][i]=1;
   e.iStartFrom=rt.StartFrom1;e.iRepeatFrom=rt.RepeatFrom1;e.EndTo=rt.wEndTo;for(int i=0;i<2;++i){e.nHybrid[i]=rt.nHybrid[i];e.fHPVar[i]=rt.fHPVar[i];}e.fHybrid[0]=capture<0>;e.fHybrid[1]=capture<1>;calls=0;
   double de=0;if(rt.IsCustomDE)de=rt.mMandFunctionDE(reinterpret_cast<TPIteration3D>(&e.C1));else rt.mMandFunction(reinterpret_cast<TPIteration3D>(&e.C1));
   double actual[]={e.x,e.y,e.z,e.w,e.Rout,e.Rold,e.OTrap,e.Dfree1,e.Dfree2,e.Deriv1,e.Deriv2,de};
   auto check=[&](bool ok,const char*kind,int lane){++checks;if(!ok){++failures;if(failures-caseFailures<8)std::printf("FAIL %s row=%d %s lane=%d\n",name,row,kind,lane);}};
   for(int j=0;j<12;++j){const bool ok=same(actual[j],expected[j]); check(ok,"orbit",j); if(!ok && failures-caseFailures<8)std::printf("  actual=%.17g expected=%.17g normalized_error=%.17g\n",actual[j],expected[j],std::abs(actual[j]-expected[j])/std::max(1.,std::abs(expected[j])));}
   check(e.ItResultI==counters[0],"iteration",0);check(e.bFirstIt==counters[1],"first",0);check(calls==counters[2],"calls",0);
   for(int j=0;j<counters[2];++j){int32_t slot;double saved[4];read(f,slot);read(f,saved);if(slot<0||slot>1)throw std::runtime_error("invalid fixture slot");check(j<calls&&slots[j]==slot,"order",j);if(manowar[slot])for(int k=0;k<4;++k)check(j<calls&&same(lanes[j][k],saved[k]),"Manowar incoming SSE",k);}
   ++orbits;
  }
  std::printf("MANOWAR_RESULT\t%s\t%d\t%d\t%d\n",name,cfg[7],checks-caseChecks,failures-caseFailures);
 }
 char extra;if(f.read(&extra,1))throw std::runtime_error("fixture trailing bytes");if(orbits!=96*expectedCases||checks!=MB3D_MANOWAR_FIXTURE_CHECKS)throw std::runtime_error("fixture coverage count changed");std::printf("%d orbits, %d checks, %d failures\n",orbits,checks,failures);return failures?1:0;
}catch(const std::exception&e){std::fprintf(stderr,"%s\n",e.what());return 2;}}
