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
std::vector<ExactColorPixel> ReferenceNativeColor(const PreparedExactScene& scene,
                                          const std::vector<ExactSurface>& surface,unsigned threads=1) {
    if(!scene.host||surface.size()!=size_t(scene.device.width)*scene.device.height)
        throw std::invalid_argument("Invalid exact color reference inputs");
    threads=std::clamp(threads,1u,scene.device.height);
    std::vector<ExactColorPixel> output(surface.size());
    std::vector<std::future<void>> tasks;
    for(unsigned lane=0;lane<threads;++lane) tasks.push_back(std::async(std::launch::async,[&,lane] {
        ScopedRenderContext binding(scene.host->context);PaintSetup p(scene);
        for(unsigned y=lane;y<scene.device.height;y+=threads) {
            TSVec row{};for(int c=0;c<3;++c)row[c]=p.start[c]+p.y_add[c]*float(y);
            p.view.yPos=float(y)/p.header.Height;PreCalcDepthCol(p.lights.iDfunc,&p.view,p.lights.PLValigned);
            for(unsigned x=0;x<scene.device.width;++x) {
                size_t i=size_t(y)*scene.device.width+x;const auto& s=surface[i];
                TsiLight5 psi{};
                psi.NormalX=s.normal[0];psi.NormalY=s.normal[1];psi.NormalZ=s.normal[2];
                psi.RoughZposFine=s.depth_packed&65535;psi.Zpos=s.depth_packed>>16;psi.Shadow=s.shadow;
                psi.AmbShadow=s.amb_shadow;psi.SIgradient=s.si_gradient;psi.OTrap=s.o_trap;
                TSVec pos{};for(int c=0;c<3;++c)pos[c]=row[c]+p.x_add[c]*float(x);
                p.view.xPos=float(x+1)/p.header.Width;CalcViewVec(&p.view,p.aspect);
                CalcObjPos(p.view,p.paint,&psi,pos);
                uint32_t dib=0;
                if(p.lights.bCalcPixColSqr)CalcPixelColorSqr(&dib,&psi,&p.lights,&p.view);else CalcPixelColor2(&dib,&psi,&p.lights,&p.view);
                output[i].rgb=((dib>>16)&255)|(dib&0xff00)|((dib&255)<<16);
            }
        }
    }));
    for(auto& t:tasks)t.get();return output;
}
std::vector<ExactColorPixel> execute(gpu::Metal& m,const PreparedGeneralColor& color,const std::vector<ExactSurface>& surfaces) {
    auto a=m.buffer(sizeof(color.base.device)),b=m.buffer(color.base.columns.size()*sizeof(ExactTrig)),c=m.buffer(color.base.rows.size()*sizeof(ExactTrig)),
        d=m.buffer(surfaces.size()*sizeof(ExactSurface)),e=m.buffer(surfaces.size()*sizeof(ExactColorPixel)),f=m.buffer(sizeof(color.interior));
    memcpy(a.contents,&color.base.device,a.length);memcpy(b.contents,color.base.columns.data(),b.length);memcpy(c.contents,color.base.rows.data(),c.length);
    memcpy(d.contents,surfaces.data(),d.length);memcpy(f.contents,&color.interior,f.length);memset(e.contents,0,e.length);
    uint32_t launch[]={0,color.base.device.width,color.base.device.height};
    auto pipeline=m.pipeline(@"native_general_color");auto command=[m.queue commandBuffer];auto encoder=[command computeCommandEncoder];
    [encoder setComputePipelineState:pipeline];[encoder setBuffer:a offset:0 atIndex:0];[encoder setBuffer:b offset:0 atIndex:1];
    [encoder setBuffer:c offset:0 atIndex:2];[encoder setBuffer:d offset:0 atIndex:3];[encoder setBuffer:e offset:0 atIndex:4];
    [encoder setBytes:launch length:sizeof(launch) atIndex:5];[encoder setBuffer:f offset:0 atIndex:6];
    [encoder dispatchThreads:MTLSizeMake(launch[1],launch[2],1) threadsPerThreadgroup:MTLSizeMake(8,1,1)];
    [encoder endEncoding];[command commit];[command waitUntilCompleted];check(command.status==MTLCommandBufferStatusCompleted,"Color command failed");
    std::vector<ExactColorPixel> output(surfaces.size());memcpy(output.data(),e.contents,e.length);return output;
}
int main(int argc,char** argv){@autoreleasepool{try{
    if(argc!=4)throw std::runtime_error("usage: test LIBRARY SCENE RESOURCE_ROOT");
    const ScopedNumericPrecision numeric(PrecisionMode::Native);
    gpu::Metal m(argv[1],4,8,0x4e415431u,false,true);
    auto marker=m.buffer(5*sizeof(uint32_t));memset(marker.contents,0,marker.length);
    gpu::dispatchStage(m,m.pipeline(@"native_general_color_abi"),1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:marker offset:0 atIndex:0];});
    const uint32_t expected[]={6,sizeof(NativeInteriorColorParams),sizeof(ExactColorScene),sizeof(ExactSurface),sizeof(ExactColorPixel)};
    check(!memcmp(marker.contents,expected,sizeof(expected)),"Color ABI mismatch");
    auto loaded=LoadM3PFile(argv[2]);check(loaded.success,"Fixture load failed");
    std::vector<ExactSurface> pixels(64);
    for(unsigned i=0;i<64;++i){auto& s=pixels[i];s.normal[0]=int(i%8)*4000-14000;s.normal[1]=int(i/8)*4000-14000;
        s.normal[2]=-int(std::sqrt(32767.0*32767-s.normal[0]*s.normal[0]-s.normal[1]*s.normal[1]));
        s.depth_packed=(uint32_t(18000+i*64)<<16)|(i*3);s.si_gradient=(i*997)&32767;s.o_trap=i*503;
        s.amb_shadow=(i*151)&16383;s.shadow=i%2?1024:0;
        if(i%13==0)s.si_gradient|=32768;
        if(i%17==0){s.depth_packed=0x80000000;s.si_gradient&=32767;}
    }
    std::vector<ExactColorPixel> baseline,object_baseline;
    for(unsigned variant=0;variant<15;++variant){
        auto h=loaded.header;h.bCalc3D=1;h.iOptions&=~1;h.bCalcAmbShadowAutomatic&=uint8_t(~128u);h.bCalcSRautomatic=0;h.bCalculateHardShadow=0;h.bHScalculated=0;h.bCalc1HSsoft=0;
        h.Light.TBoptions=(h.Light.TBoptions&~(uint32_t(63)<<23))|(uint32_t(32)<<23);
        h.Light.AdditionalOptions&=~1;for(unsigned i=1;i<6;++i)h.Light.Lights[i].Loption=1;
        h.Light.Lights[0].Loption=0;h.Light.Lights[0].LFunction=0x35;
        if(variant==1){h.Light.Lights[1]=h.Light.Lights[0];h.Light.Lights[1].Lcolor[0]=255;h.Light.Lights[1].Lcolor[1]=70;h.Light.Lights[1].Lcolor[2]=20;}
        // This byte also stores the high diffuse-map ID. Copying a light must
        // not introduce an unrelated material resource into the witness.
        h.Light.Lights[1].AdditionalByteEx=loaded.header.Light.Lights[1].AdditionalByteEx;
        if(variant==2)h.Light.Lights[0].LFunction=0x13;
        if(variant==3||variant==6)h.Light.TBoptions=(h.Light.TBoptions&~(uint32_t(63)<<23))|(uint32_t(52)<<23);
        if(variant==4)h.Light.TBoptions=(h.Light.TBoptions&~(uint32_t(63)<<23))|(uint32_t(12)<<23);
        if(variant==5||variant==6)h.Light.AdditionalOptions|=1;
        if(variant==7)h.bCalculateHardShadow=7;
        if(variant>=8&&variant<11)h.Light.TBoptions=(h.Light.TBoptions&0x3fffffffu)|((variant-7u)<<30);
        if(variant>=11) {
            h.Light.TBoptions=(h.Light.TBoptions&0x3fffffffu)|0x20000000u|((variant-11u)<<30);
            const double angle=.7,co=std::cos(angle),si=std::sin(angle);
            TMatrix3 rotation={{co,si,0},{-si,co,0},{0,0,1}};
            std::memcpy(h.hVGrads,rotation,sizeof(rotation));
        }
        auto scene=PrepareGeneralScene(h,loaded.addon,8,8,argv[3]);auto color=PrepareGeneralColor(scene.base);
        auto actual=execute(m,color,pixels);auto reference=ReferenceNativeColor(scene.base,pixels);
        for(int repeat=0;repeat<2;++repeat){auto again=execute(m,color,pixels);check(!memcmp(actual.data(),again.data(),actual.size()*sizeof(ExactColorPixel)),"Color repeat differs");}
        unsigned maximum=0,changed=0;
        for(unsigned i=0;i<actual.size();++i){check(!actual[i].fault,"Color GPU fault");if(!baseline.empty()&&actual[i].rgb!=baseline[i].rgb)++changed;
            for(unsigned channel=0;channel<3;++channel){int a=(actual[i].rgb>>(8*channel))&255,b=(reference[i].rgb>>(8*channel))&255;maximum=std::max(maximum,unsigned(std::abs(a-b)));}}
        printf("variant %u: max CPU/GPU channel difference=%u active pixels=%u exact repeats=3\n",variant,maximum,changed);
        check(maximum<=2,"Production CPU color disagreement exceeds stage tolerance");
        if(variant)check(changed>0,"Color feature has no effect");else baseline=actual;
        if(variant==11)object_baseline=actual;
        if(variant>=12)check(!memcmp(actual.data(),object_baseline.data(),actual.size()*sizeof(ExactColorPixel)),
            "Object-relative depth must bypass row gradient function");
        if(variant==0)for(unsigned bad=0;bad<16;++bad){auto invalid=color;
            if(bad==0)invalid.interior.version=3;if(bad==1)invalid.interior.lights[0].function=4;
            if(bad==2)invalid.interior.lights[0].power=3;if(bad==3)invalid.interior.gamma=0x7fc00000;
            if(bad==4)invalid.interior.lights[0].enabled=2;if(bad==5)invalid.interior.depth_function=4;
            if(bad==6)invalid.base.device.z_mul=0;
            if(bad==7)invalid.interior.version=4;
            if(bad==8)invalid.interior.version=5;
            if(bad==9)invalid.interior.object_y[3]=1;
            if(bad==10){invalid.interior.flags|=64;invalid.interior.object_y[0]=0x7fc00000;}
            if(bad==11)invalid.interior.object_y[1]=0x3f800000;
            if(bad>=12){invalid.interior.flags|=64;
                const uint32_t badBasis[]={0x3f800001u,0xbf800001u,0x7f7fffffu,0xff7fffffu};
                invalid.interior.object_y[0]=badBasis[bad-12];}
            auto rejected=execute(m,invalid,pixels);for(const auto& x:rejected)check(x.fault!=0,"Invalid descriptor accepted");}
    }
    printf("PASS %u checks; 15 lighting/color variants, 16 rejection controls, exact 3 repeats\n",checks);return 0;
}catch(const std::exception& e){fprintf(stderr,"FAIL after %u checks: %s\n",checks,e.what());return 1;}}}
