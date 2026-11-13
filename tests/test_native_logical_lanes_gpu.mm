// Real-device logical-task ordering/bounds controls. The oracle below follows
// closed-form conditional-count and plane-dIFS transitions, not shader helpers.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "render/native_metal/formula_abi.h"
#include "render/native_metal/formula_selector.h"
#include "render/native_metal/general_geometry_abi.h"
#include "render/native_metal/generic_color_abi.h"
#include "render/native_metal/logical_lane_abi.h"
#include "render/native_metal/native_camera_abi.h"
#include "render/native_metal/native_inside_abi.h"
#include "render/native_metal/multi_shadow_readback.h"
#include "comparison/metal_exact/scene_abi.h"
#include "comparison/metal_exact/color_abi.h"
#include "comparison/metal_exact/shadow_abi.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <sys/sysctl.h>

namespace {
using namespace mb3d_native;
constexpr unsigned width=3,height=5,workers=2,pixel_count=width*height;
unsigned checks=0,failures=0;
void check(bool ok,const char* what){++checks;if(!ok&&++failures<=30)std::fprintf(stderr,"FAIL: %s\n",what);}
void require(bool ok,const std::string& what){if(!ok)throw std::runtime_error(what);}
std::string errorText(NSError* e){return e?e.localizedDescription.UTF8String:"Metal failure";}
U32 f32(float value){return std::bit_cast<U32>(value);}
std::uint64_t f64(double value){return std::bit_cast<std::uint64_t>(value);}
NativeFormulaProgram program(){
    NativeFormulaProgram p{};p.version=1;p.driver=HybridDifs;p.slot_count=6;
    p.end_slot=1;p.de_option=20;p.max_iterations=1;p.work_limit=4096;p.local_work_limit=4096;
    p.radius_stop=100;p.log_radius_stop=1;p.de_scale=1;p.de_offset=.125f;
    p.gradient_radius_stop=100;p.stop_scale=1;
    p.julia_value[0]=.125f;p.julia_value[1]=.25f;p.julia_value[2]=.5f;p.julia_value[3]=.75f;
    for(unsigned i=0;i<4;++i)p.rotation[i][i]=1;
    for(auto& s:p.slots)s.version=1;
    // Negative count means the conditional does not contribute an iteration
    // or a dIFS minimum. Its first execution changes its own next-orbit count.
    p.hybrid_counts[0]=std::bit_cast<I32>(0x80000001u);p.hybrid_counts[1]=1;
    auto& c=p.slots[0];c.namespace_id=ExternalFormula;c.id=10354;c.enabled=1;c.de_option=-1;
    auto raw=[&](int offset,int value){c.raw_words[(256+offset)/4]=std::bit_cast<U32>(I32(value));};
    raw(-12,1);raw(-16,1);raw(-20,2);raw(-24,2);raw(-28,2);
    auto& plane=p.slots[1];plane.enabled=1;plane.id=8;plane.de_option=20;
    plane.negative_params[3]=1;plane.raw_words[(256-68)/4]=1; // signed X plane
    return p;
}
ExactScene scene(){
    ExactScene s{};s.version=4;s.width=width;s.height=height;s.max_steps=2;
    s.max_iterations=1;s.slot_count=6;s.min_iterations=1;s.refinement_steps=8;
    s.formula_kind=kGeneralFormulaKind;s.capability_id=kGeneralCapability;
    s.vgrads[0]=s.vgrads[4]=s.vgrads[8]=f64(1);s.y_start[0]=f64(4);
    s.max_ray_length=f64(.25);s.de_stop=f32(.125f);s.z_step_div=f32(1);
    s.step_limiter=f32(128);s.de_scale=f32(1);s.step_width_103=f32(1);
    return s;
}
NativeLogicalLane descriptor(unsigned lane,unsigned cursor,unsigned count){
    return {lane,workers,1+(height-1-lane)/workers,cursor,count,pixel_count,{0,0}};
}
// Each DE executes conditional count1 (first task pixel), then count2 (later
// pixels), followed by the plane. Only the plane contributes iteration1.
NativeFormulaState expectedState(const NativeFormulaProgram& p,unsigned x,unsigned y,bool first=false){
    NativeFormulaState s{};s.xyz[0]=s.c[0]=s.j[0]=float(4+x);
    s.xyz[1]=s.c[1]=s.j[1]=float(y);s.j[3]=p.julia_value[3];
    for(unsigned i=0;i<4;++i)s.original_j[i]=p.julia_value[i];
    s.derivative[0]=1;s.rout=float(4+x);s.rold=s.rstop_d=.125f;s.rstop=100;
    s.vary_scale=1;s.smooth=1;s.iteration=1;s.first=first?1:2;s.tmp=1;s.max_iterations=1;
    s.hybrid_counts[0]=std::bit_cast<I32>(0x80000002u);s.hybrid_counts[1]=1;s.current_slot=1;
    return s;
}
struct Snapshot {
    std::array<ExactPixel,pixel_count+2> pixels;
    std::array<NativeGeometryRecord,pixel_count+2> records;
    std::array<NativeFormulaState,workers+2> states;
};
}

int main(int argc,char** argv){@autoreleasepool {try{
    require(argc==3,"usage: test_native_logical_lanes_gpu native.metallib normal-case.json");
    int translated=0;size_t translated_size=sizeof(translated);
    (void)sysctlbyname("sysctl.proc_translated",&translated,&translated_size,nullptr,0);
    require(!translated,"Native logical-lane tests require native Apple execution");
    id<MTLDevice> device=MTLCreateSystemDefaultDevice();require(device!=nil,"Metal unavailable");
    NSError* error=nil;id<MTLLibrary> library=[device newLibraryWithURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:argv[1]]] error:&error];
    require(library!=nil,errorText(error));id<MTLCommandQueue> queue=[device newCommandQueue];require(queue!=nil,"No command queue");
    auto pipeline=[&](const char* name,bool specialize){
        id<MTLFunction> fn=nil;NSError* e=nil;
        if(specialize){MTLFunctionConstantValues* values=[[MTLFunctionConstantValues alloc] init];
            const U32 selectors[]{4,0,kGeneralCapability};for(unsigned i=0;i<3;++i)[values setConstantValue:&selectors[i] type:MTLDataTypeUInt atIndex:i];
            for(unsigned i=0;i<6;++i){const U32 id=i==0?10354:i==1?FormulaPipelineSelector(InternalFormula,8,1):kDisabledFormulaSelector;
                [values setConstantValue:&id type:MTLDataTypeUInt atIndex:10+i];}
            const U32 policy=1;[values setConstantValue:&policy type:MTLDataTypeUInt atIndex:17];
            const U32 driver=HybridDifs;[values setConstantValue:&driver type:MTLDataTypeUInt atIndex:16];
            fn=[library newFunctionWithName:[NSString stringWithUTF8String:name] constantValues:values error:&e];
        }else fn=[library newFunctionWithName:[NSString stringWithUTF8String:name]];
        require(fn!=nil,std::string("Missing kernel ")+name+": "+errorText(e));
        id<MTLComputePipelineState> result=[device newComputePipelineStateWithFunction:fn error:&e];require(result!=nil,errorText(e));return result;
    };
    auto buffer=[&](size_t size){auto b=[device newBufferWithLength:size options:MTLResourceStorageModeShared];require(b!=nil,"GPU allocation failed");return b;};
    unsigned dispatchGroupWidth=1;
    auto dispatch=[&](id<MTLComputePipelineState> kernel,unsigned count,auto bind){
        auto command=[queue commandBuffer];auto encoder=[command computeCommandEncoder];[encoder setComputePipelineState:kernel];bind(encoder);
        [encoder dispatchThreads:MTLSizeMake(count,1,1) threadsPerThreadgroup:MTLSizeMake(dispatchGroupWidth,1,1)];[encoder endEncoding];[command commit];[command waitUntilCompleted];
        require(command.status==MTLCommandBufferStatusCompleted,errorText(command.error));
    };
    auto marker=[&](const char* name,const std::vector<U32>& expected){
        auto b=buffer((expected.size()+1)*4);std::memset(b.contents,0xa5,b.length);
        dispatch(pipeline(name,false),1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:b offset:0 atIndex:0];});
        check(std::memcmp(b.contents,expected.data(),expected.size()*4)==0,name);
        check(static_cast<const U32*>(b.contents)[expected.size()]==0xa5a5a5a5u,"ABI trailing canary");
    };
    marker("exact_arithmetic_profile",{2,4,32});marker("native_general_lane_abi",{1,32,188,16,32});
    marker("native_formula_abi",{1,8192,188,236,32});
    marker("native_general_abi",{6,sizeof(ExactScene),8192,sizeof(ExactPixel),sizeof(ExactSurface),112,32});
    {
        // Independent binary64 inequalities, plus invalid-domain controls for
        // the FP32 certificate. No shader implementation runs as CPU oracle.
        using Inputs=std::array<float,6>;
        const Inputs base{-3.4013226e38f,1.2202671e35f,1.0015283f,4.e34f,.00024f,2.5757656f};
        std::vector<Inputs> input{base};std::vector<U32> expected{1};
        const float inf=std::numeric_limits<float>::infinity(),nan=std::numeric_limits<float>::quiet_NaN();
        auto change=[&](unsigned field,float value,U32 wanted){auto x=base;x[field]=value;input.push_back(x);expected.push_back(wanted);};
        change(0,inf,0);change(0,nan,0);change(0,0x1p123f,0);
        change(1,0.f,0);change(1,-1.f,0);change(1,0x1p123f,0);change(1,std::bit_cast<float>(1u),0);
        change(2,0.f,0);change(2,-1.f,0);change(2,2.01f,0);change(2,std::bit_cast<float>(1u),0);
        change(3,inf,0);change(3,nan,0);change(3,std::bit_cast<float>(1u),0);change(3,3.e38f,0);
        change(3,0.f,1);change(3,-1.f,1);
        change(4,-.01f,0);change(4,nan,0);change(4,-0.f,1);
        change(5,0.f,0);change(5,-1.f,0);change(5,std::bit_cast<float>(1u),0);
        for(unsigned i=0;i<input.size();++i)if(expected[i]){
            const auto& x=input[i];
            const double lower=std::abs(double(x[0]))*double(x[2]);
            check(lower>0&&double(x[3])/(lower+double(x[4]))<double(x[5])*.25,
                  "independent quotient bound proves existing minimum");
        }
        const U32 count=U32(input.size());auto ib=buffer(input.size()*sizeof(Inputs)),ob=buffer((count+2)*8);
        std::memcpy(ib.contents,input.data(),input.size()*sizeof(Inputs));
        auto probe=pipeline("native_gradient_difference_certificate",false);std::vector<U32> first;
        for(unsigned repeat=0;repeat<3;++repeat){
            std::memset(ob.contents,0xa5,ob.length);
            dispatch(probe,count+2,[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:ib offset:0 atIndex:0];[e setBuffer:ob offset:0 atIndex:1];[e setBytes:&count length:4 atIndex:2];});
            const auto* result=static_cast<const U32*>(ob.contents);
            for(U32 i=0;i<count;++i)check(result[i*2]==expected[i],"difference certificate domain control");
            check(result[1]==0xff800000u,"observed finite difference normalization really overflows FP32");
            check(std::all_of(result+count*2,result+(count+2)*2,[](U32 v){return v==0xa5a5a5a5u;}),"difference probe overdispatch guards");
            const std::vector<U32> now(result,result+(count+2)*2);
            if(repeat)check(now==first,"difference certificate exact repeats");else first=now;
        }
        check(std::memcmp(ib.contents,input.data(),input.size()*sizeof(Inputs))==0,"difference probe immutable inputs");
    }
    {
        NSData* data=[NSData dataWithContentsOfFile:[NSString stringWithUTF8String:argv[2]]];
        require(data!=nil,"Missing captured normal regression fixture");
        NSDictionary* fixture=[NSJSONSerialization JSONObjectWithData:data options:0 error:&error];
        require([fixture isKindOfClass:[NSDictionary class]],"Invalid normal regression fixture");
        auto words=[&](NSString* key,void* destination,size_t size){
            NSArray* values=fixture[key];require([values isKindOfClass:[NSArray class]]&&values.count*4==size,"Fixture ABI size");
            auto* out=static_cast<U32*>(destination);for(NSUInteger i=0;i<values.count;++i)out[i]=[values[i] unsignedIntValue];
        };
        std::array<NativeFormulaProgram,2> programs{};words(@"program_words",&programs[0],sizeof(programs[0]));
        programs[0].formula_type=CombineMaximum;programs[0].combination_smooth=1.f;programs[1]=programs[0];
        std::array<NativeFormulaState,2> inputs{};words(@"state_words",&inputs[0],sizeof(inputs[0]));inputs[1]=inputs[0];
        for(unsigned axis=0;axis<3;++axis)inputs[1].xyz[axis]=inputs[1].c[axis]=inputs[1].j[axis]=.05f*float(axis+1);
        auto values=[[MTLFunctionConstantValues alloc] init];
        for(unsigned i=0;i<6;++i){const auto& slot=programs[0].slots[i];
            const U32 selector=slot.enabled?FormulaPipelineSelector(slot.namespace_id,slot.id,1):kDisabledFormulaSelector;
            [values setConstantValue:&selector type:MTLDataTypeUInt atIndex:10+i];}
        const U32 driver=Hybrid3D,type=CombineMaximum;
        [values setConstantValue:&driver type:MTLDataTypeUInt atIndex:16];
        [values setConstantValue:&driver type:MTLDataTypeUInt atIndex:20];
        [values setConstantValue:&type type:MTLDataTypeUInt atIndex:19];
        auto fn=[library newFunctionWithName:@"native_general_gradient_count_probe" constantValues:values error:&error];require(fn!=nil,errorText(error));
        auto probe=[device newComputePipelineStateWithFunction:fn error:&error];require(probe!=nil,errorText(error));
        auto pb=buffer(sizeof(programs)),ib=buffer(sizeof(inputs)),rb=buffer(4*sizeof(NativeGeometryRecord)),ob=buffer(4*16);
        std::memcpy(pb.contents,programs.data(),sizeof(programs));std::memcpy(ib.contents,inputs.data(),sizeof(inputs));
        std::vector<unsigned char> first;const U32 count=2;
        for(unsigned repeat=0;repeat<3;++repeat){
            std::memset(rb.contents,0xa5,rb.length);std::memset(ob.contents,0xa5,ob.length);
            dispatch(probe,count+2,[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:pb offset:0 atIndex:0];[e setBuffer:ib offset:0 atIndex:1];[e setBuffer:rb offset:0 atIndex:2];
                [e setBuffer:ob offset:0 atIndex:3];[e setBytes:&count length:4 atIndex:4];});
            const auto* records=static_cast<const NativeGeometryRecord*>(rb.contents);const auto* output=static_cast<const U32*>(ob.contents);
            for(U32 i=0;i<count;++i){
                check(!records[i].fault&&!records[i].formula_fault.code&&!output[i*4+3],"combined gradient probe succeeds");
                check(records[i].de_calls==1,"one combined distance query");
                check(output[i*4]==(i==0?2u:0u),"two certified parts, or zero in bounded control");
                check(records[i].gradient_range_clamps==(i==0?1u:0u),"count certified queries, not parts");
                check(output[i*4+1]==output[i*4+2],"wrapper preserves distance bits");
            }
            const auto* bytes=static_cast<const unsigned char*>(rb.contents);
            check(std::all_of(bytes+count*sizeof(NativeGeometryRecord),bytes+rb.length,[](auto b){return b==0xa5;}),"gradient record overdispatch guards");
            check(std::all_of(output+count*4,output+16,[](U32 v){return v==0xa5a5a5a5u;}),"gradient result overdispatch guards");
            std::vector<unsigned char> now(bytes,bytes+rb.length);
            const auto* more=static_cast<const unsigned char*>(ob.contents);now.insert(now.end(),more,more+ob.length);
            if(repeat)check(now==first,"combined gradient exact repeats");else first=now;
        }
        check(std::memcmp(pb.contents,programs.data(),sizeof(programs))==0&&std::memcmp(ib.contents,inputs.data(),sizeof(inputs))==0,"combined gradient immutable inputs");
    }
    {
        struct Case {float rout,rold,multiplier;U32 expected;};
        const Case cases[]={{1,0,512,0},{2,1,512,0},{4,1,512,355},{16,1,512,1065},
            {1,1,512,0},{1,1,-512,355},{0,0,512,0},{0,0,-512,32767},
            {-1,0,512,32767},{1,-1,512,32767},{0,-1,512,32767},
            {0,0,0,32767},{4,1,0,0},{4,1,1e8f,32767}};
        constexpr U32 count=sizeof(cases)/sizeof(cases[0]);
        std::array<NativeHitColorPayload,count> input{};std::array<float,count> mul{};
        for(U32 i=0;i<count;++i){input[i]={1,cases[i].rout,cases[i].rold,0};mul[i]=cases[i].multiplier;}
        auto ib=buffer(sizeof(input)),mb=buffer(sizeof(mul)),ob=buffer((count+2)*8);
        auto probe=pipeline("native_general_orbit_color_probe",false);
        auto runProbe=[&](){
            std::memcpy(ib.contents,input.data(),sizeof(input));std::memcpy(mb.contents,mul.data(),sizeof(mul));
            std::memset(ob.contents,0xa5,ob.length);
            dispatch(probe,count+2,[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:ib offset:0 atIndex:0];[e setBuffer:mb offset:0 atIndex:1];
                [e setBuffer:ob offset:0 atIndex:2];[e setBytes:&count length:4 atIndex:3];
            });
            std::array<U32,count*2> result{};std::memcpy(result.data(),ob.contents,sizeof(result));
            const auto* tail=static_cast<const U32*>(ob.contents)+count*2;
            for(U32 i=0;i<4;++i)check(tail[i]==0xa5a5a5a5u,"orbit color canary");
            check(!std::memcmp(ib.contents,input.data(),sizeof(input))&&!std::memcmp(mb.contents,mul.data(),sizeof(mul)),"orbit inputs immutable");
            return result;
        };
        auto first=runProbe();for(U32 i=0;i<count;++i)check(first[2*i]==cases[i].expected&&!first[2*i+1],"independent orbit color scalar/exception oracle");
        for(U32 repeat=0;repeat<2;++repeat)check(runProbe()==first,"orbit color exact raw repeats");
        for(U32 bad=0;bad<6;++bad){auto saved=input[0];float oldmul=mul[0];
            if(bad==0)input[0].version=0;if(bad==1)input[0].reserved=1;
            if(bad==2)input[0].rout=std::bit_cast<float>(0x7fc00000u);
            if(bad==3)input[0].rold=std::bit_cast<float>(0x7f800000u);
            if(bad==4)mul[0]=std::bit_cast<float>(0xff800000u);
            if(bad==5)input[0].rold=std::bit_cast<float>(0x7fc00000u);
            auto result=runProbe();check(result[1]==(bad<2?1u:4u)&&!result[0],"invalid orbit payload fails closed");
            input[0]=saved;mul[0]=oldmul;
        }
    }
    auto kernel=pipeline("native_general_primary_lanes",true);
    auto p=program();auto s=scene();
    auto pb=buffer(sizeof(p));auto sb=buffer(sizeof(s));
    std::array<ExactTrig,width> columns{};std::array<ExactTrig,height> rows{};
    for(auto& t:columns)t.cos_value=f64(1);for(auto& t:rows)t.cos_value=f64(1);
    auto cb=buffer(sizeof(columns)),rb=buffer(sizeof(rows));std::memcpy(cb.contents,columns.data(),cb.length);std::memcpy(rb.contents,rows.data(),rb.length);
    auto pixels=buffer((pixel_count+2)*sizeof(ExactPixel)),records=buffer((pixel_count+2)*sizeof(NativeGeometryRecord));
    auto states=buffer((workers+2)*sizeof(NativeFormulaState)),descriptors=buffer((workers+2)*sizeof(NativeLogicalLane));
    auto statuses=buffer((workers+2)*sizeof(NativeLogicalLaneRecord));auto maps=buffer(sizeof(NativeMapDescriptor)),data=buffer(sizeof(float));
    auto reset=[&](){for(auto b:{pixels,records,states,descriptors,statuses})std::memset(b.contents,0xa5,b.length);};
    auto run=[&](const std::array<NativeLogicalLane,workers>& lanes,const std::array<U32,4>& launch){
        std::memcpy(pb.contents,&p,sizeof(p));std::memcpy(sb.contents,&s,sizeof(s));
        std::memcpy(descriptors.contents,lanes.data(),sizeof(lanes));std::memset(statuses.contents,0xa5,statuses.length);
        const U32 resources[]{0,0};
        dispatch(kernel,workers+2,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:pixels offset:0 atIndex:0];[e setBuffer:records offset:0 atIndex:1];[e setBuffer:sb offset:0 atIndex:2];[e setBuffer:pb offset:0 atIndex:3];
            [e setBuffer:cb offset:0 atIndex:4];[e setBuffer:rb offset:0 atIndex:5];[e setBuffer:maps offset:0 atIndex:6];[e setBuffer:data offset:0 atIndex:7];
            [e setBytes:launch.data() length:sizeof(launch) atIndex:8];[e setBytes:resources length:sizeof(resources) atIndex:9];
            [e setBuffer:descriptors offset:0 atIndex:10];[e setBuffer:states offset:0 atIndex:11];[e setBuffer:statuses offset:0 atIndex:12];
        });
        check(std::memcmp(pb.contents,&p,sizeof(p))==0&&std::memcmp(sb.contents,&s,sizeof(s))==0,"program/scene immutable");
        check(std::memcmp(descriptors.contents,lanes.data(),sizeof(lanes))==0,"lane descriptors immutable");
        for(auto pair:{std::pair<id<MTLBuffer>,size_t>{pixels,pixel_count*sizeof(ExactPixel)},
                      {records,pixel_count*sizeof(NativeGeometryRecord)},{states,workers*sizeof(NativeFormulaState)},
                      {statuses,workers*sizeof(NativeLogicalLaneRecord)}}){
            const auto* bytes=static_cast<const unsigned char*>(pair.first.contents);
            check(std::all_of(bytes+pair.second,bytes+pair.first.length,[](auto v){return v==0xa5;}),"independent allocation/overdispatch canary");
        }
    };
    auto render=[&](unsigned batch){
        reset();std::array<NativeLogicalLane,workers> lanes{};for(unsigned i=0;i<workers;++i)lanes[i]=descriptor(i,0,0);
        for(;;){bool pending=false;for(auto& lane:lanes){lane.pixel_count=std::min(batch,lane.row_count*width-lane.cursor);pending|=lane.pixel_count!=0;}if(!pending)break;
            run(lanes,{0,workers+2,workers,pixel_count}); // Request deliberately exceeds independent lane allocation.
            const auto* status=static_cast<const NativeLogicalLaneRecord*>(statuses.contents);
            for(unsigned i=0;i<workers;++i){check(status[i].version==1&&!status[i].fault&&status[i].processed==lanes[i].pixel_count&&status[i].next_cursor==lanes[i].cursor+lanes[i].pixel_count,"processed count and next cursor");
                require(!status[i].fault,"valid lane fault");lanes[i].cursor+=lanes[i].pixel_count;}
        }
        Snapshot out;std::memcpy(out.pixels.data(),pixels.contents,pixels.length);std::memcpy(out.records.data(),records.contents,records.length);std::memcpy(out.states.data(),states.contents,states.length);
        for(unsigned i=0;i<workers;++i){const unsigned last_y=i+(lanes[i].row_count-1)*workers;auto expected=expectedState(p,width-1,last_y);
            check(std::memcmp(&out.states[i],&expected,sizeof(expected))==0,"all47 final state lanes match independent CPU task-order oracle");}
        for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x){const auto& r=out.records[y*width+x];const auto& v=out.pixels[y*width+x];
            check(r.version==6&&r.initial_rout==0.f&&!r.difs_carried_reads&&!r.fp32_range_exits&&r.gradient_range_clamps<=r.de_calls,"geometry v6 non-interior payload and reserved word");
            check(!r.fault&&r.de_calls==1&&r.ray_steps==1&&r.formula_work==((y<workers&&x==0)?2u:3u)&&!r.normal_calls&&!r.refine_calls&&!r.hit,"conditional persistent count and bounded per-ray work");
            check(!v.fault&&!v.rm_result&&v.depth==f64(4+x)&&v.axial_depth==f64(4+x)&&v.final_iterations==1,"independent plane distance/miss oracle");}
        return out;
    };
    const auto whole=render(32),single=render(1),split=render(3);
    dispatchGroupWidth=32;const auto grouped=render(32);dispatchGroupWidth=1;
    check(std::memcmp(&whole,&grouped,sizeof(whole))==0,
          "group1/group32 pixels, geometry and carried state exactly equal");
    check(std::memcmp(&whole,&single,sizeof(whole))==0,"one-pixel batches equal complete logical task");
    check(std::memcmp(&whole,&split,sizeof(whole))==0,"three-pixel batches equal complete logical task");
    // Resume a genuine dIFS lane whose last minimum/color did not occur in this
    // orbit: the source retains tmp, Dfree2, J4 and the unused derivative lane.
    reset();p.slots[1].negative_params[4]=-65536.f;
    auto retained=expectedState(p,0,0);retained.tmp=7;retained.dfree2=.75f;retained.j[3]=.375f;retained.derivative[0]=3;
    std::memcpy(states.contents,&retained,sizeof(retained));
    std::array<NativeLogicalLane,workers> resumed{descriptor(0,1,1),descriptor(1,6,0)};
    run(resumed,{0,workers+2,workers,pixel_count});
    const auto& retained_result=*static_cast<const NativeFormulaState*>(states.contents);
    check(retained_result.tmp==7&&retained_result.iteration==7&&retained_result.smooth==7&&retained_result.dfree2==.75f&&
          retained_result.otrap==.75f&&retained_result.j[3]==.375f&&retained_result.derivative[0]==3,"dIFS retained minimum/color/J4/derivative survive resume");
    check(static_cast<const NativeLogicalLaneRecord*>(statuses.contents)[0].fault==0,"retained dIFS control has zero fault");
    // Positive control of the conditional dIFS contract: every query of the
    // resumed pixel (lane 0 cursor 1 = x 1, y 0) returned carried state.
    const auto& carried=static_cast<const NativeGeometryRecord*>(records.contents)[1];
    check(carried.de_calls>0&&carried.difs_carried_reads==carried.de_calls,"dIFS carried reads counted per query");
    p=program();
    auto invalid=[&](auto mutate,const char* what){
        reset();std::array<NativeLogicalLane,workers> lanes{descriptor(0,0,1),descriptor(1,6,0)};mutate(lanes[0]);
        run(lanes,{0,1,workers,pixel_count});const auto& status=*static_cast<const NativeLogicalLaneRecord*>(statuses.contents);
        check(status.version==1&&status.fault==1&&!status.processed&&status.next_cursor==lanes[0].cursor,what);
        for(auto b:{pixels,records,states}){const auto* bytes=static_cast<const unsigned char*>(b.contents);
            check(std::all_of(bytes,bytes+b.length,[](auto v){return v==0xa5;}),"invalid lane has no pixel/geometry/state writes");}
    };
    invalid([](auto& d){d.row_stride=0;},"zero stride rejected");
    invalid([](auto& d){d.row_start=1;},"overlapping row ownership rejected");
    invalid([](auto& d){d.row_stride=1;},"wrong logical worker stride rejected");
    invalid([](auto& d){d.row_count=0xffffffffu;},"overflowing row span rejected");
    invalid([](auto& d){d.cursor=0xffffffffu;},"overflowing cursor rejected");
    invalid([](auto& d){d.cursor=8;d.pixel_count=2;},"cursor range crossing task boundary rejected");
    invalid([](auto& d){d.pixel_count=33;},"oversized batch rejected");
    invalid([](auto& d){--d.allocation_count;},"pixel allocation mismatch rejected");
    invalid([](auto& d){d.reserved[1]=1;},"reserved lane word rejected");

    // Literal no-cut initial-inside controls from renderer.cpp's separate
    // branch and RMcalcNanglesForCut, not the device packing implementation.
    // Rout/RStop = 0, 1/2, 1. The middle gradient exercises nearest-even 16383.5.
    auto surfaceKernel=pipeline("native_general_surface",true);
    auto surfaceBuffer=buffer((pixel_count+2)*sizeof(ExactSurface));
    std::array<ExactPixel,pixel_count> packingPixels{};
    std::array<NativeGeometryRecord,pixel_count> packingRecords{};
    for(auto& record:packingRecords)record.version=6;
    p=program();p.radius_stop=2;s=scene();
    for(unsigned i=0;i<3;++i){
        auto& pixel=packingPixels[i];pixel.rm_result=1;pixel.start_inside=1;
        pixel.orbit_trap=f64(.125*double(1u<<i));
        packingRecords[i].initial_rout=float(i);
    }
    // One exterior object plus the remaining background pixels distinguish
    // interior-only palette changes from accidental global color changes.
    packingPixels[3].rm_result=1;packingPixels[3].normal[2]=-32767;
    NativeSurfaceParams packing{};packing.version=kNativeSurfaceVersion;
    packing.z_corr=packing.z_c_mul=packing.step_width=f64(1);
    packing.color_multiplier=f64(1);
    auto pack=[&](){
        std::memcpy(pixels.contents,packingPixels.data(),sizeof(packingPixels));
        std::memcpy(records.contents,packingRecords.data(),sizeof(packingRecords));
        std::memcpy(pb.contents,&p,sizeof(p));std::memcpy(sb.contents,&s,sizeof(s));
        std::memset(surfaceBuffer.contents,0xa5,surfaceBuffer.length);
        const U32 launch[]{0,pixel_count+2,pixel_count,0};
        dispatch(surfaceKernel,pixel_count+2,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:pixels offset:0 atIndex:0];[e setBuffer:surfaceBuffer offset:0 atIndex:1];
            [e setBuffer:sb offset:0 atIndex:2];[e setBytes:&packing length:sizeof(packing) atIndex:3];
            [e setBuffer:pb offset:0 atIndex:4];[e setBytes:launch length:sizeof(launch) atIndex:5];
            [e setBuffer:records offset:0 atIndex:6];
        });
        check(std::memcmp(pixels.contents,packingPixels.data(),sizeof(packingPixels))==0&&
              std::memcmp(records.contents,packingRecords.data(),sizeof(packingRecords))==0,"inside pack input records immutable");
        const auto* tail=static_cast<const unsigned char*>(surfaceBuffer.contents)+pixel_count*sizeof(ExactSurface);
        check(std::all_of(tail,static_cast<const unsigned char*>(surfaceBuffer.contents)+surfaceBuffer.length,
            [](auto v){return v==0xa5;}),"inside surface overdispatch canaries");
        std::array<ExactSurface,pixel_count> result{};std::memcpy(result.data(),surfaceBuffer.contents,sizeof(result));return result;
    };
    const auto inside=pack();const U32 expectedGradient[]{32768,49152,65535};
    for(unsigned i=0;i<3;++i){ExactSurface expected{};expected.depth_packed=0x7fff0000u;
        expected.normal[2]=-32767;expected.amb_shadow=5000;expected.si_gradient=expectedGradient[i];expected.o_trap=512u<<i;
        check(std::memcmp(&inside[i],&expected,sizeof(expected))==0,"exact no-cut initial-inside packed surface");}
    check(!inside[3].fault&&inside[3].si_gradient<32768&&inside[4].depth_packed==0x80000000u,"exterior/background packing controls");
    for(unsigned bad=0;bad<9;++bad){auto saved=packingRecords[0];
        if(bad==0)packingRecords[0].version=1;
        if(bad==1)packingRecords[0].initial_rout=std::bit_cast<float>(0x7fc00000u);
        if(bad==2)packingRecords[0].difs_carried_reads=2*(packingRecords[0].de_calls+packingRecords[0].normal_calls)+1;
        if(bad==3)packingRecords[0].smoothing.calls=1; // no formula work
        if(bad==4){packingRecords[0].formula_work=1;packingRecords[0].smoothing={1,1,1,0};}
        if(bad==5){packingRecords[0].formula_work=1;packingRecords[0].smoothing={1,0,0,1};}
        if(bad==6){packingRecords[0].formula_work=~0u;packingRecords[0].smoothing={~0u,0,0,0};}
        if(bad==7)packingRecords[0].gradient_range_clamps=packingRecords[0].de_calls+1;
        if(bad==8)packingRecords[0].fp32_range_exits=kNativeRangeExitsPerDistance*packingRecords[0].de_calls+packingRecords[0].normal_calls+1;
        const auto rejected=pack();check(rejected[0].fault!=0,"invalid interior geometry payload rejected");packingRecords[0]=saved;
    }
    (void)pack();

    // With zero direct/specular light, unit ambient and no fog/bounce,
    // CalcTotalLight1 reduces exactly to the independently chosen RGB palette.
    // Interior interpolation below is dyadic; no CPU/GPU math helper is oracle.
    marker("native_general_color_abi",{6,sizeof(NativeInteriorColorParams),sizeof(ExactColorScene),sizeof(ExactSurface),sizeof(ExactColorPixel)});
    auto colorKernel=pipeline("native_general_color",false);
    ExactColorScene color{};color.version=1;color.width=width;color.height=height;
    color.z_corr=color.z_mul=color.step_width=f64(1);
    color.light_pow=64;color.light_function=3;color.hs_mask=1024;color.hs_enabled=1;
    for(unsigned c=0;c<3;++c){
        color.ambient_colors[0][c]=color.ambient_colors[1][c]=f32(1);
        for(unsigned i=0;i<10;++i)color.palette_diffuse[i][c]=f32(float(8+8*c));
    }
    NativeInteriorColorParams interior{};interior.version=6;interior.flags=5;interior.palette_start=f32(32768);
    interior.palette_mul=f32(1.f/65536.f);
    for(unsigned i=0;i<4;++i){interior.positions[i]=8192*i;interior.divisors[i]=f32(1.f/8192.f);
        for(unsigned c=0;c<3;++c)interior.colors[i][c]=f32(float(16+32*i+16*c));
        interior.colors[i][3]=f32(.25f);}
    auto colorScene=buffer(sizeof(color)),interiorBuffer=buffer(sizeof(interior));
    auto rgb=buffer((pixel_count+2)*sizeof(ExactColorPixel));
    auto paint=[&](){
        std::memcpy(colorScene.contents,&color,sizeof(color));std::memcpy(interiorBuffer.contents,&interior,sizeof(interior));
        std::memset(rgb.contents,0xa5,rgb.length);const U32 launch[]{0,width,height};
        auto command=[queue commandBuffer];auto encoder=[command computeCommandEncoder];
        [encoder setComputePipelineState:colorKernel];[encoder setBuffer:colorScene offset:0 atIndex:0];
        [encoder setBuffer:cb offset:0 atIndex:1];[encoder setBuffer:rb offset:0 atIndex:2];
        [encoder setBuffer:surfaceBuffer offset:0 atIndex:3];[encoder setBuffer:rgb offset:0 atIndex:4];
        [encoder setBytes:launch length:sizeof(launch) atIndex:5];[encoder setBuffer:interiorBuffer offset:0 atIndex:6];
        [encoder dispatchThreads:MTLSizeMake(width+1,height+1,1) threadsPerThreadgroup:MTLSizeMake(1,1,1)];
        [encoder endEncoding];[command commit];[command waitUntilCompleted];
        require(command.status==MTLCommandBufferStatusCompleted,errorText(command.error));
        check(std::memcmp(colorScene.contents,&color,sizeof(color))==0&&
              std::memcmp(interiorBuffer.contents,&interior,sizeof(interior))==0,"interior palette descriptors immutable");
        const auto* tail=static_cast<const unsigned char*>(rgb.contents)+pixel_count*sizeof(ExactColorPixel);
        check(std::all_of(tail,static_cast<const unsigned char*>(rgb.contents)+rgb.length,
            [](auto v){return v==0xa5;}),"interior color independent allocation canaries");
        std::array<ExactColorPixel,pixel_count> result{};std::memcpy(result.data(),rgb.contents,sizeof(result));return result;
    };
    const auto baseColor=paint();
    for(unsigned i=0;i<3;++i){const U32 red=16+16*i,expected=red|((red+16)<<8)|((red+32)<<16);
        check(!baseColor[i].fault&&baseColor[i].rgb==expected&&!baseColor[i].reserved0&&!baseColor[i].reserved1,"independent exact interior palette blend RGB");}
    check(!baseColor[3].fault&&baseColor[3].rgb==(8u|(16u<<8)|(24u<<16)),"independent exterior palette RGB");
    for(unsigned i=0;i<4;++i)interior.colors[i][0]=f32(float(24+32*i));
    const auto changedColor=paint();
    for(unsigned i=0;i<pixel_count;++i){
        check(!changedColor[i].fault,"valid changed interior palette has no fault");
        check(i<3?changedColor[i].rgb==baseColor[i].rgb+8:
              std::memcmp(&changedColor[i],&baseColor[i],sizeof(ExactColorPixel))==0,"interior palette influence leaves exterior/background unchanged");
    }
    interior.version=3;const auto staleColor=paint();
    check(std::all_of(staleColor.begin(),staleColor.end(),[](const auto& v){return v.fault==1;}),
          "stale interior color ABI rejected without publishing RGB");
    interior.version=6;interior.reserved[1]=1;const auto invalidColor=paint();
    check(invalidColor[0].fault!=0,"reserved interior descriptor rejected");
    interior.reserved[1]=0;interior.colors[1][2]=0x7fc00000u;
    const auto nonfiniteColor=paint();check(nonfiniteColor[0].fault!=0,"nonfinite interior palette rejected");
    interior.colors[1][2]=f32(80);--color.width;
    const auto wrongDimensions=paint();
    check(std::all_of(wrongDimensions.begin(),wrongDimensions.end(),[](const auto& v){return v.fault==1;}),
          "color dimensions rejected against independent allocation before reads");
#include "native_multi_shadow_cases.h"
}catch(const std::exception& e){check(false,e.what());}
std::printf("Native logical-lane GPU controls: %u checks, %u failures\n",checks,failures);return failures?1:0;}}
