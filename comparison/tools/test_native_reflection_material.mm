// Native package1 color stage test. Oracle calls production CPU pixel coloring,
// not GPU helper arithmetic. Fixed synthetic surfaces isolate lighting from DE.
#include "render/native_metal/general_scene_host.h"
#include "render/native_metal/generic_color_host.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include "comparison/metal_exact/metal_device.h"
#include "core/colorizer.h"
#include "core/header_trafos.h"
#include "core/math3d.h"
#include "io/m3p_parser.h"
#include <future>
#include <cstdio>
#include <bit>
using namespace mb3d;using namespace mb3d::native_metal;using namespace mb3d::metal_exact;
using namespace mb3d_native;using namespace mb3d_metal_exact;
namespace gpu=mb3d::metal_exact::gpu;
unsigned checks=0;void check(bool b,const char* why){++checks;if(!b)throw std::runtime_error(why);}
struct PaintSetup {
    TMandHeader10 header{};
    TLightVals lights{};
    TLValigned aligned{};
    TPaintParameter paint{};
    TPaintLightVals view{};
    TMatrix3 gradients{};
    TSVec start{}, y_add{}, x_add{};
    float aspect{};
    explicit PaintSetup(const PreparedExactScene& scene) {
        header=scene.host->file.header;
        lights.PLValigned=&aligned;
        MakeLightValsFromHeaderLight(&header,&lights,1.0,0);
        CompleteLightValsSetup(&header,&lights);
        // Production Render applies this state after completing the hard-shadow
        // pass, even when every light ray happened to remain unobstructed.
        const uint8_t completed=(header.bHScalculated&0xfd)|header.bCalculateHardShadow;
        if(header.bCalculateHardShadow&1) for(int x=0;x<6;++x) {
            const int y=lights.SortTab[x];
            lights.iHScalced[y]=(completed>>(x+2))&1;
            lights.iHSmask[y]=0x400<<x;
            if((completed&2)&&((header.bCalculateHardShadow>>(x+2))&1))
                lights.iLightFuncDiff[y]=0;
        }
        paint.ppWidth=header.Width;paint.ppHeight=header.Height;paint.ppYinc=1;
        paint.PLVals=&lights;
        CalcStepWidth(&header);paint.StepWidth=header.dStepWidth;
        TMatrix3 source{};std::memcpy(source,header.hVGrads,sizeof(source));
        NormaliseMatrixTo(gradients,header.dStepWidth,source);paint.pVgrads=&gradients;
        paint.sFOVy=float(header.dFOVy*Pid180);
        CalcPPZvals(header,paint.Zcorr,paint.ZcMul,paint.ZZstmitDif);
        const double fov=std::min(1.5,std::max(0.001,double(paint.sFOVy)*0.5));
        paint.ppPlOpticZ=float(std::cos(fov)*fov/std::sin(fov));
        paint.ppXOff=CalcXoff(&header);paint.ppPaintWidth=header.Width;paint.ppPaintHeight=header.Height;
        GetStartSPosAndAddVecs(view,paint,start,y_add,x_add);
        aspect=float(header.Width)/float(header.Height);
    }
};

#include <array>
using Result=std::array<float,12>;
Result execute(gpu::Metal& m,const PreparedGeneralColor& color,const ExactSurface& s,const float (&input)[8]) {
 auto a=m.buffer(sizeof(color.base.device),&color.base.device),b=m.buffer(sizeof(color.interior),&color.interior),
      c=m.buffer(sizeof(s),&s),d=m.buffer(sizeof(input),input),o=m.buffer(sizeof(Result));
 memset(o.contents,0xa5,o.length);
 gpu::dispatchStage(m,m.pipeline(@"native_inside_material_probe"),1,[&](id<MTLComputeCommandEncoder> e){
  [e setBuffer:a offset:0 atIndex:0];[e setBuffer:b offset:0 atIndex:1];[e setBuffer:c offset:0 atIndex:2];
  [e setBuffer:d offset:0 atIndex:3];[e setBuffer:o offset:0 atIndex:4];});
 Result r;memcpy(r.data(),o.contents,sizeof(r));return r;
}
// The camera-aware reflection material through its descriptor; inputs[2].xyz
// is the absolute hit position, inputs[3].xyz SR's CamPos and inputs[4].xyz
// its AbsViewVec.
Result executeDescriptor(gpu::Metal& m,const PreparedGeneralColor& color,const ExactSurface& s,const float (&input)[20]) {
 const float none=0;
 auto a=m.buffer(sizeof(color.base.device),&color.base.device),b=m.buffer(sizeof(color.interior),&color.interior),
      c=m.buffer(sizeof(s),&s),d=m.buffer(sizeof(input),input),o=m.buffer(sizeof(Result)),
      r=m.buffer(sizeof(color.reflection),&color.reflection),
      x=color.map_samples.empty()?m.buffer(sizeof(none),&none):m.buffer(color.map_samples.size()*sizeof(float),color.map_samples.data());
 memset(o.contents,0xa5,o.length);
 gpu::dispatchStage(m,m.pipeline(@"native_inside_material_descriptor_probe"),1,[&](id<MTLComputeCommandEncoder> e){
  [e setBuffer:a offset:0 atIndex:0];[e setBuffer:b offset:0 atIndex:1];[e setBuffer:c offset:0 atIndex:2];
  [e setBuffer:d offset:0 atIndex:3];[e setBuffer:o offset:0 atIndex:4];[e setBuffer:r offset:0 atIndex:5];
  [e setBuffer:x offset:0 atIndex:6];});
 Result out;memcpy(out.data(),o.contents,sizeof(out));return out;
}
// SR_CalcLightZPos for the ray (CamPos, AbsViewVec): each position light's
// depth along this ray, in the reflection pass's operation order.
void calcLightZPos(TLightVals& l,const TPaintLightVals& v,const TMCTparameter& mct) {
 for(int i=0;i<6;++i){
  if((l.iLightPos[i]&1)==0)continue;
  const auto& ln=l.PLValigned->LN[i];double t;
  if(std::fabs(v.AbsViewVec[0])>.5f)t=(ln[0]-v.CamPos[0])/v.AbsViewVec[0];
  else if(std::fabs(v.AbsViewVec[1])>.5f)t=(ln[1]-v.CamPos[1])/v.AbsViewVec[1];
  else t=(ln[2]-v.CamPos[2])/v.AbsViewVec[2];
  l.sPosLightZpos[i]=std::max(0.f,float(t));
  t=8388352.0-mct.ZcMul*(std::sqrt(l.sPosLightZpos[i]/l.sStepWidth*mct.Zcorr+1.0)-1.0);
  l.sPosLP[i]=std::min(std::max(float(t/256.0),0.f),32767.f);
 }
}
// Reflection material v2: three lit lights with distinct hard-shadow bits,
// squared color, CalcTotalLight2, one positional light and far fog, each
// prepared from the header exactly as production prepares it. Shapes 1..4
// make the positional light visible (iLightPos 3, 5, 7, 9); each case's ray
// passes its sphere at a fraction of the radius, with the hit point beyond
// the light or, every fourth case, before it (hidden).
unsigned descriptorCases(gpu::Metal& m,const TMandHeader10& base,const THeaderCustomAddon& addon,const char* root,float& maxError,
                         unsigned (&visibleCases)[5],unsigned& depthCases) {
 unsigned cases=0;
 for(unsigned variant=0;variant<80;++variant){
  const unsigned shape=variant>>4;if(shape&&!(variant&4))continue;
  auto h=base;h.bCalcSRautomatic|=1;h.bCalculateHardShadow=uint8_t(1|(7u<<2));h.bHScalculated=0;
  for(unsigned i=0;i<6;++i){auto& l=h.Light.Lights[i];l.Loption=uint8_t(i<3?0:1);l.LFunction&=uint8_t(~128u);}
  if(variant&1)h.Light.AdditionalOptions|=1;else h.Light.AdditionalOptions&=uint8_t(~1u);
  h.Light.Lights[2].FreeByte=(variant&2)?1:0;
  if(variant&8)h.Light.TBoptions|=0x40000;else h.Light.TBoptions&=~0x40000;
  // SR interpolates the depth colors along iDfunc's row (TBoptions bits 30-31).
  const unsigned depthFunction=(variant+(variant>>3))&3;
  h.Light.TBoptions=(h.Light.TBoptions&0x3fffffffu)|(depthFunction<<30);
  const double reach=h.dStepWidth>0?h.dStepWidth*h.Width*.25:1.0;
  if(variant&4){auto& l=h.Light.Lights[1];l.Loption|=4;
   DoubleToD7B(h.dXmid+reach*.5,l.LXpos);DoubleToD7B(h.dYmid-reach*.3,l.LYpos);DoubleToD7B(h.dZmid+reach*.4,l.LZpos);
   if(shape){const unsigned pos=1u|(2u*shape);l.Loption=uint8_t((l.Loption&~0x1cu)|((pos&7u)<<2));if(pos&8u)l.LFunction|=128u;}}
  auto scene=PrepareGeneralScene(h,addon,8,8,root);auto color=PrepareGeneralColor(scene.base,true);
  ScopedRenderContext binding(scene.base.host->context);PaintSetup cpu(scene.base);
  check(color.reflection.version==kNativeReflectionMaterialVersion,"Reflection material descriptor missing");
  check(bool(color.interior.squared)==bool(variant&1)&&bool(color.interior.flags&kNativeTotalLight2Flag)==bool(variant&2)&&
   bool(color.reflection.geometry.lights[cpu.lights.SortTab[1]].options&1u)==bool(variant&4),"Variant not prepared");
  check(!cpu.lights.bScaleAmbDiffDown,"Unsupported CPU ambient scaling in witness");
  check(cpu.lights.iDfunc==int(depthFunction)&&color.interior.depth_function==depthFunction,"Depth function not prepared");
  unsigned lit=0;
  for(const auto& l:color.interior.lights)lit+=l.enabled&&(l.color[0]|l.color[1]|l.color[2])&&std::popcount(l.hs_mask)==1;
  check(lit==3,"Descriptor witness lights not active");
  const auto& mct=scene.base.host->mct;const float mid[3]={float(mct.Xmit),float(mct.Ymit),float(mct.Zmit)};
  int visible=-1;
  for(int k=0;k<6;++k)if(cpu.lights.iLightPos[k]&14){check(visible<0,"Several visible witness lights");visible=k;}
  check((visible>=0)==(shape!=0)&&(!shape||cpu.lights.iLightPos[visible]==int(1u|(2u*shape))),"Visible light variant not prepared");
  for(unsigned i=0;i<12;++i){
   ExactSurface s{};s.normal[0]=int(i%4)*6000-9000;s.normal[1]=int(i/4)*7000-7000;
   s.normal[2]=-int(sqrt(32767.*32767-s.normal[0]*s.normal[0]-s.normal[1]*s.normal[1]));
   s.depth_packed=(uint32_t(18000+i*500)<<16)|(i*15);s.si_gradient=(i*2881)&32767;s.o_trap=i*503;
   s.amb_shadow=i*1100;s.shadow=(((i%4)*5120)&0xfc00)|(i*7);
   if(i==10)s.si_gradient|=32768;if(i==11){s.depth_packed=0x80000000;s.si_gradient=0;}
   float position[3]={mid[0]+float(reach*(int(i%3)-1)*.2),mid[1]+float(reach*(int(i%5)-2)*.1),mid[2]-float(reach*.05*i)};
   float cam[3]={0,0,0},dir[3]={0,0,0};
   if(shape){
    const auto& ln=cpu.lights.PLValigned->LN[visible];
    const float radius=std::sqrt(cpu.lights.sLmaxL[visible]*1e-8f),offset=radius*float(i%6)/6.f;
    const float length=std::sqrt(.3f*.3f+.2f*.2f+.93f*.93f),distance=float(reach*.5);
    dir[0]=.3f/length;dir[1]=-.2f/length;dir[2]=.93f/length;
    const float side=std::sqrt(dir[0]*dir[0]+dir[2]*dir[2]),perp[3]={-dir[2]/side,0,dir[0]/side};
    const float beyond=(i%4==3?-.25f:.25f)*distance;
    for(unsigned c=0;c<3;++c){cam[c]=ln[c]-dir[c]*distance+perp[c]*offset;position[c]=mid[c]+ln[c]+dir[c]*beyond+perp[c]*offset;}
   }
   float in[20]={0,0,1,0,float(i)/12.f,.3f,.3f,0,position[0],position[1],position[2],0,cam[0],cam[1],cam[2],0,dir[0],dir[1],dir[2],0};
   auto actual=executeDescriptor(m,color,s,in);
   if(std::bit_cast<uint32_t>(actual[11]))fprintf(stderr,"descriptor variant=%u i=%u fault=%u\n",variant,i,std::bit_cast<uint32_t>(actual[11]));
   check(std::bit_cast<uint32_t>(actual[11])==0,"Descriptor material GPU fault");
   for(unsigned repeat=0;repeat<2;++repeat){auto again=executeDescriptor(m,color,s,in);check(!memcmp(actual.data(),again.data(),sizeof(actual)),"Descriptor material repeat differs");}
   TsiLight5 psi{};psi.NormalX=s.normal[0];psi.NormalY=s.normal[1];psi.NormalZ=s.normal[2];
   psi.RoughZposFine=s.depth_packed&65535;psi.Zpos=s.depth_packed>>16;psi.Shadow=s.shadow;
   psi.AmbShadow=s.amb_shadow;psi.SIgradient=s.si_gradient;psi.OTrap=s.o_trap;
   cpu.view.yPos=in[4];cpu.view.zPos=in[5];cpu.lights.ZposDynFog=in[6];
   cpu.view.ViewVec[0]=0;cpu.view.ViewVec[1]=0;cpu.view.ViewVec[2]=1;
   for(unsigned c=0;c<3;++c){cpu.view.ObjPos[c]=position[c]-mid[c];cpu.view.CamPos[c]=cam[c];cpu.view.AbsViewVec[c]=dir[c];}
   cpu.view.ObjPos[3]=cpu.view.CamPos[3]=cpu.view.AbsViewVec[3]=0;
   if(shape)calcLightZPos(cpu.lights,cpu.view,mct);
   PreCalcDepthCol(cpu.lights.iDfunc,&cpu.view,cpu.lights.PLValigned);
   TSVec radiance{},transparency{};TLightSD sd{};
   CalcPixelColorSvec(&radiance,sd,&psi,&cpu.lights,&cpu.view,transparency);
   if(shape){
    // The witness counts only when the shape changes the CPU radiance.
    TSVec plain{},plainTransparency{};TLightSD plainSd{};const int pos=cpu.lights.iLightPos[visible];
    cpu.lights.iLightPos[visible]=1;CalcPixelColorSvec(&plain,plainSd,&psi,&cpu.lights,&cpu.view,plainTransparency);
    cpu.lights.iLightPos[visible]=pos;
    bool changed=false;for(unsigned c=0;c<3;++c)changed|=std::fabs(plain[c]-radiance[c])>1e-4f;
    visibleCases[shape]+=changed;
   }
   if(depthFunction&&!(color.interior.flags&64u)){
    // The witness counts only when the row function changes the CPU radiance.
    TSVec linear{},linearTransparency{};TLightSD linearSd{};TPaintLightVals view=cpu.view;
    PreCalcDepthCol(0,&view,cpu.lights.PLValigned);CalcPixelColorSvec(&linear,linearSd,&psi,&cpu.lights,&view,linearTransparency);
    bool changed=false;for(unsigned c=0;c<3;++c)changed|=std::fabs(linear[c]-radiance[c])>1e-4f;
    depthCases+=changed;
   }
   for(unsigned c=0;c<3;++c){
    for(auto pair:{std::pair{actual[c],radiance[c]},std::pair{actual[c+4],sd[0][c]},std::pair{actual[c+8],sd[1][c]}}){
     const float error=fabs(pair.first-pair.second)/std::max(1.f,fabs(pair.second));maxError=std::max(maxError,error);
     if(!(error<=.0002f))fprintf(stderr,"descriptor variant=%u i=%u c=%u gpu=%.9g cpu=%.9g\n",variant,i,c,pair.first,pair.second);
     check(std::isfinite(pair.first)&&error<=.0002f,"CPU/GPU descriptor material discrepancy > 0.0002");
    }
    check(fabs(actual[3]-transparency[c])<=.0002f,"Descriptor transparency disagreement");
   }
   ++cases;
  }
 }
 for(unsigned shape=1;shape<5;++shape)check(visibleCases[shape]>0,"Visible light shape witness inactive");
 check(depthCases>0,"Depth function witness inactive");
 return cases;
}
int main(int argc,char**argv){@autoreleasepool{try{
 if(argc!=4)throw std::runtime_error("usage: test LIBRARY SCENE RESOURCE_ROOT");
 const ScopedNumericPrecision numeric(PrecisionMode::Native);
 gpu::Metal m(argv[1],4,8,0x4e415431u,false,true);
 auto loaded=LoadM3PFile(argv[2]);check(loaded.success,"Load failed");
 auto h=loaded.header;h.bCalc3D=1;h.iOptions&=~1;h.bCalcAmbShadowAutomatic&=uint8_t(~128u);
 auto scene=PrepareGeneralScene(h,loaded.addon,8,8,argv[3]);auto original=PrepareGeneralColor(scene.base,true);
 ScopedRenderContext binding(scene.base.host->context);PaintSetup cpu(scene.base);
 check(!cpu.lights.bScaleAmbDiffDown,"Unsupported CPU ambient scaling in witness");
 float maxError=0;unsigned cases=0,active=0;
 for(unsigned func=0;func<4;++func)for(unsigned power:{1u,8u,64u,1024u})for(unsigned shadow=0;shadow<4;++shadow){
  auto color=original;auto& d=color.base.device;auto& l=color.interior.lights[0];
  d.light_function=l.function=func;d.light_pow=l.power=power;d.hs_calculated=l.hs_calculated=shadow&1;
  d.hs_enabled=l.hs_enabled=shadow>>1;cpu.lights.iLightFuncDiff[0]=func;cpu.lights.iLightPowFunc[0]=power;
  cpu.lights.iHScalced[0]=shadow&1;cpu.lights.iHSenabled[0]=shadow>>1;
  for(unsigned i=0;i<12;++i){
   ExactSurface s{};s.normal[0]=int(i%4)*6000-9000;s.normal[1]=int(i/4)*7000-7000;
   s.normal[2]=-int(sqrt(32767.*32767-s.normal[0]*s.normal[0]-s.normal[1]*s.normal[1]));
   s.depth_packed=(uint32_t(18000+i*500)<<16)|(i*15);s.si_gradient=(i*2881)&32767;s.o_trap=i*503;
   s.amb_shadow=i*1100;s.shadow=(i%2?1024:0)|(i*7);
   if(i==10)s.si_gradient|=32768;if(i==11){s.depth_packed=0x80000000;s.si_gradient=0;}
   float in[8]={0,0,1,0,float(i)/12.f,.3f,.3f,0};
   auto actual=execute(m,color,s,in);if(std::bit_cast<uint32_t>(actual[11]))fprintf(stderr,"case f=%u pow=%u shadow=%u i=%u fault=%u\n",func,power,shadow,i,std::bit_cast<uint32_t>(actual[11]));check(std::bit_cast<uint32_t>(actual[11])==0,"Material GPU fault");
   for(unsigned repeat=0;repeat<2;++repeat){auto again=execute(m,color,s,in);check(!memcmp(actual.data(),again.data(),sizeof(actual)),"Material repeat differs");}
   TsiLight5 psi{};psi.NormalX=s.normal[0];psi.NormalY=s.normal[1];psi.NormalZ=s.normal[2];
   psi.RoughZposFine=s.depth_packed&65535;psi.Zpos=s.depth_packed>>16;psi.Shadow=s.shadow;
   psi.AmbShadow=s.amb_shadow;psi.SIgradient=s.si_gradient;psi.OTrap=s.o_trap;
   cpu.view.yPos=in[4];cpu.view.zPos=in[5];cpu.lights.ZposDynFog=in[6];
   cpu.view.ViewVec[0]=0;cpu.view.ViewVec[1]=0;cpu.view.ViewVec[2]=1;
   PreCalcDepthCol(cpu.lights.iDfunc,&cpu.view,cpu.lights.PLValigned);
   TSVec radiance{},transparency{};TLightSD sd{};
   CalcPixelColorSvec(&radiance,sd,&psi,&cpu.lights,&cpu.view,transparency);
   for(unsigned c=0;c<3;++c){
    for(auto pair:{std::pair{actual[c],radiance[c]},std::pair{actual[c+4],sd[0][c]},std::pair{actual[c+8],sd[1][c]}}){
     float error=fabs(pair.first-pair.second);maxError=std::max(maxError,error);
     check(std::isfinite(pair.first)&&error<=.0002f,"CPU/GPU material discrepancy > 0.0002");
    }
    check(fabs(actual[3]-transparency[c])<=.0002f,"Transparency disagreement");
   }
   if(i<10&&actual[0]+actual[1]+actual[2]>0)++active;++cases;
  }
 }
 for(unsigned bad=0;bad<12;++bad){auto c=original;ExactSurface s{};s.depth_packed=18000u<<16;s.normal[2]=-32767;
  float in[8]={0,0,1,0,.2f,.3f,.3f,0};
  if(bad==0)c.base.device.light_function=4;if(bad==1)c.base.device.light_pow=3;
  if(bad==2)c.base.device.light_pow=0;if(bad==3)c.base.device.light_pow=2048;
  if(bad==4)c.interior.lights[0].function^=1;if(bad==5)c.interior.lights[1]=c.interior.lights[0];
  if(bad==6)c.interior.squared=1;if(bad==7)c.interior.gamma_mode=1;if(bad==8)c.interior.version=3;
  if(bad==9)c.base.device.light_color[0]=0x7fc00000;if(bad==10)c.base.device.hs_mask=2048;
  if(bad==11)c.interior.depth_function=4;
  auto v=execute(m,c,s,in);check(std::bit_cast<uint32_t>(v[11])!=0,"Invalid material admitted");
 }
 check(active>0,"No active material witnesses");
 float descriptorError=0;unsigned visible[5]={},depth=0;
 const unsigned descriptor=descriptorCases(m,h,loaded.addon,argv[3],descriptorError,visible,depth);
 printf("PASS %u checks; %u CPU/GPU material cases x3 exact repeats, 12 invalid descriptors; max error %.9g; "
        "%u reflection-material descriptor cases x3, max relative error %.9g; visible shapes 2/4/6/8 active in %u/%u/%u/%u cases; "
        "depth function active in %u cases\n",
        checks,cases,maxError,descriptor,descriptorError,visible[1],visible[2],visible[3],visible[4],depth);
 return 0;
}catch(const std::exception&e){fprintf(stderr,"FAIL after %u checks: %s\n",checks,e.what());return 1;}}}
