// A real-device point/orbit harness. Emits raw FP32/state words for independent
// CPU-corpus comparison; successful dispatch alone is never formula coverage.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "formula_host.h"
#include "io/m3p_parser.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace mb3d_native;
namespace fs=std::filesystem;
std::string quoted(const std::string& value) {
    std::ostringstream s;s<<'"';
    for(unsigned char c:value) {
        if(c=='"'||c=='\\')s<<'\\'<<c;
        else if(c<32)s<<"\\u"<<std::hex<<std::setw(4)<<std::setfill('0')<<unsigned(c)<<std::dec;
        else s<<c;
    }
    return s.str()+'"';
}
std::string message(NSError* e) {return e?std::string(e.localizedDescription.UTF8String):"unknown Metal failure";}
void require(bool ok,const std::string& why) {if(!ok)throw std::runtime_error(why);}
struct Options {std::string scene,program,library,output,states,root;std::vector<int> map_ids;
    unsigned mode=2,slot=0,repeats=3,wrapper_policy=1;bool map_controls=false,range_controls=false;};
unsigned number(const std::string& s) {
    require(!s.empty()&&std::all_of(s.begin(),s.end(),[](unsigned char c){return c>='0'&&c<='9';}),"invalid unsigned integer");
    std::size_t n=0;const auto v=std::stoul(s,&n);require(n==s.size()&&v<=0xffffffffu,"invalid integer");return unsigned(v);
}
Options parse(int argc,char** argv) {
    Options o;
    for(int i=1;i<argc;++i) {
        std::string key=argv[i];require(i+1<argc,"missing value: "+key);std::string value=argv[++i];
        if(key=="--scene")o.scene=value;else if(key=="--program")o.program=value;else if(key=="--library")o.library=value;
        else if(key=="--output")o.output=value;else if(key=="--states")o.states=value;
        else if(key=="--resource-root")o.root=value;else if(key=="--slot")o.slot=number(value);
        else if(key=="--map-id") {const unsigned id=number(value);require(id<=unsigned(std::numeric_limits<int>::max()),"map ID out of range");o.map_ids.push_back(int(id));}
        else if(key=="--map-controls") {require(value=="active"||value=="none","invalid map controls mode");o.map_controls=value=="active";}
        else if(key=="--range-controls") {require(value=="active"||value=="none","invalid range controls mode");o.range_controls=value=="active";}
        else if(key=="--repeats")o.repeats=number(value);
        else if(key=="--wrapper-policy")o.wrapper_policy=number(value);
        else if(key=="--mode") {require(value=="step"||value=="orbit"||value=="distance","invalid mode");o.mode=value=="step"?0:value=="orbit"?1:2;}
        else throw std::runtime_error("unknown option: "+key);
    }
    require(o.scene.empty()!=o.program.empty(),"provide exactly one of --scene FILE or --program RAW8192");
    require(!o.library.empty()&&!o.output.empty(),"required: --library FILE --output NEW_JSON");
    require(o.slot<6&&o.repeats>=2&&o.repeats<=16,"slot/repeat bounds");
    require(!o.range_controls||o.repeats>=3,"range controls require at least three repeats");
    require(o.wrapper_policy<=1,"wrapper policy bounds");
    require(o.map_ids.empty()||o.scene.empty(),"--map-id is only valid with --program");
    require(!o.map_controls||(!o.map_ids.empty()&&!o.program.empty()),"active map controls require raw program and map IDs");
    require(o.program.empty()||o.root.empty()||!o.map_ids.empty(),"raw resource root requires explicit map IDs");
    if(!o.map_ids.empty()) {
        require(o.map_ids.size()<=4,"at most three nonzero map IDs plus zero");
        auto ids=o.map_ids;std::sort(ids.begin(),ids.end());
        require(std::adjacent_find(ids.begin(),ids.end())==ids.end(),"duplicate map ID");
        require(std::count_if(ids.begin(),ids.end(),[](int id){return id!=0;})<=3,"at most three nonzero map IDs");
        require(!o.root.empty()&&fs::is_directory(o.root),"map IDs require an existing explicit resource root");
        o.root=fs::canonical(o.root).string();
    }
    require(!fs::exists(o.output),"output already exists");
    require(fs::exists(fs::path(o.output).parent_path().empty()?fs::path("."):fs::path(o.output).parent_path()),"output parent missing");
    return o;
}
std::vector<NativeFormulaState> states(const Options& o,const NativeFormulaProgram& program) {
    std::vector<NativeFormulaState> result;
    if(!o.states.empty()) {
        const auto size=fs::file_size(o.states);
        require(size&&size%sizeof(NativeFormulaState)==0&&size/sizeof(NativeFormulaState)<=4096,"invalid raw state corpus size");
        result.resize(size/sizeof(NativeFormulaState));std::ifstream in(o.states,std::ios::binary);
        in.read(reinterpret_cast<char*>(result.data()),size);require(bool(in),"read state corpus failed");
    } else {
        constexpr float points[][3]={{.1f,.2f,.3f},{.25f,-.5f,.75f},{-.7f,.3f,.2f},
            {1.1f,.01f,-.4f},{-.2f,-.3f,1.2f},{2.f,.5f,-.25f},{.03125f,.0625f,.125f}};
        for(const auto& p:points) {
            auto s=mb3d::native_metal::MakeFormulaPointState(program,p[0],p[1],p[2]);
            for(unsigned k=0;k<3;++k) {s.xyz[k]=p[k];s.j[k]=program.julia?program.julia_value[k]:p[k];}
            s.j[3]=program.julia?program.julia_value[3]:0.f;
            s.rout=(p[0]*p[0]+p[1]*p[1])+p[2]*p[2];s.otrap=s.rout;s.vary_scale=1.f;
            s.derivative[0]=1.f;s.xyz[3]=1.f;result.push_back(s);
        }
    }
    return result;
}
void words(std::ostream& out,const void* data,std::size_t bytes) {
    const auto* p=static_cast<const unsigned char*>(data);out<<'[';
    for(std::size_t i=0;i<bytes/4;++i) {U32 v;std::memcpy(&v,p+i*4,4);if(i)out<<',';out<<v;}
    out<<']';
}
} // namespace

int main(int argc,char** argv) {@autoreleasepool {try {
    const auto options=parse(argc,argv);
    mb3d::native_metal::PreparedFormulaProgram prepared;
    if(!options.scene.empty()) {
        const auto loaded=mb3d::LoadM3PFile(options.scene);require(loaded.success,loaded.error);
        prepared=mb3d::native_metal::PrepareLoadedFormulaProgram(loaded.header,loaded.addon,options.root);
    } else {
        require(fs::file_size(options.program)==sizeof(prepared.device),"invalid raw program size");
        std::ifstream stream(options.program,std::ios::binary);stream.read(reinterpret_cast<char*>(&prepared.device),sizeof(prepared.device));
        require(bool(stream),"read program failed");
        if(!options.map_ids.empty())mb3d::native_metal::LoadRawFormulaMapResources(prepared,options.map_ids,options.root);
    }
    const auto inputs=states(options,prepared.device);
    id<MTLDevice> device=MTLCreateSystemDefaultDevice();require(device!=nil,"Metal device unavailable");
    NSError* error=nil;
    id<MTLLibrary> library=[device newLibraryWithURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:options.library.c_str()]] error:&error];
    require(library!=nil,message(error));
    auto pipeline=[&](const char* name,bool specialize) {
        MTLFunctionConstantValues* values=[[MTLFunctionConstantValues alloc] init];
        if(specialize)for(unsigned slot=0;slot<6;++slot) {
            const auto& s=prepared.device.slots[slot];U32 id=FormulaPipelineSelector(s.namespace_id,s.id,s.enabled);
            [values setConstantValue:&id type:MTLDataTypeUInt atIndex:10+slot];
        }
        if(specialize)[values setConstantValue:&prepared.device.driver type:MTLDataTypeUInt atIndex:16];
        if(specialize)[values setConstantValue:&options.wrapper_policy type:MTLDataTypeUInt atIndex:18];
        NSError* functionError=nil;
        id<MTLFunction> function=specialize?[library newFunctionWithName:[NSString stringWithUTF8String:name] constantValues:values error:&functionError]:
            [library newFunctionWithName:[NSString stringWithUTF8String:name]];
        require(function!=nil,std::string("missing kernel: ")+name);
        NSError* e=nil;id<MTLComputePipelineState> p=[device newComputePipelineStateWithFunction:function error:&e];
        require(p!=nil,message(e));return p;
    };
    auto abiPipeline=pipeline("native_formula_abi",false);
    auto profilePipeline=pipeline("exact_arithmetic_profile",false);
    id<MTLCommandQueue> queue=[device newCommandQueue];require(queue!=nil,"command queue unavailable");
    auto buffer=[&](std::size_t bytes) {id<MTLBuffer> b=[device newBufferWithLength:bytes options:MTLResourceStorageModeShared];require(b!=nil,"GPU allocation failed");return b;};
    auto execute=[&](id<MTLComputeCommandEncoder> encoder,id<MTLCommandBuffer> command,unsigned count) {
        [encoder dispatchThreads:MTLSizeMake(count,1,1) threadsPerThreadgroup:MTLSizeMake(1,1,1)];
        [encoder endEncoding];[command commit];[command waitUntilCompleted];
        require(command.status==MTLCommandBufferStatusCompleted,message(command.error));
    };
    id<MTLBuffer> marker=buffer(5*sizeof(U32));std::memset(marker.contents,0,marker.length);
    {auto command=[queue commandBuffer];auto encoder=[command computeCommandEncoder];
        [encoder setComputePipelineState:abiPipeline];[encoder setBuffer:marker offset:0 atIndex:0];execute(encoder,command,1);}
    const U32 expected[]={kFormulaAbiVersion,sizeof(NativeFormulaProgram),sizeof(NativeFormulaState),sizeof(NativeOrbitResult),32};
    require(std::memcmp(marker.contents,expected,sizeof(expected))==0,"formula library ABI mismatch");
    id<MTLBuffer> profile=buffer(3*sizeof(U32));std::memset(profile.contents,0,profile.length);
    {auto command=[queue commandBuffer];auto encoder=[command computeCommandEncoder];
        [encoder setComputePipelineState:profilePipeline];[encoder setBuffer:profile offset:0 atIndex:0];execute(encoder,command,1);}
    const U32 expectedProfile[]={2,4,32};
    require(std::memcmp(profile.contents,expectedProfile,sizeof(expectedProfile))==0,"formula library arithmetic profile mismatch");
    id<MTLBuffer> mapMarker=nil;
    if(!prepared.maps.empty()) {
        auto mapAbiPipeline=pipeline("native_formula_map_abi",false);
        mapMarker=buffer(4*sizeof(U32));std::memset(mapMarker.contents,0,mapMarker.length);
        auto command=[queue commandBuffer];auto encoder=[command computeCommandEncoder];
        [encoder setComputePipelineState:mapAbiPipeline];[encoder setBuffer:mapMarker offset:0 atIndex:0];execute(encoder,command,1);
        const U32 expectedMaps[]={1,sizeof(NativeMapDescriptor),3,1};
        require(std::memcmp(mapMarker.contents,expectedMaps,sizeof(expectedMaps))==0,"formula map library contract mismatch");
    }
    auto pointPipeline=pipeline("native_formula_probe",true);
    id<MTLComputePipelineState> rangePipeline=options.range_controls?pipeline("native_formula_range_probe",true):nil;
    id<MTLBuffer> program=buffer(sizeof(NativeFormulaProgram)),input=buffer(inputs.size()*sizeof(NativeFormulaState));
    id<MTLBuffer> output=buffer((inputs.size()+2)*sizeof(NativeOrbitResult));
    constexpr std::size_t rangeGuard=16*sizeof(U32);
    const std::size_t rangePayload=inputs.size()*sizeof(U32);
    id<MTLBuffer> rangeOutput=options.range_controls?buffer(rangePayload+2*rangeGuard):nil;
    constexpr std::size_t mapGuard=sizeof(NativeMapDescriptor),pixelGuard=16*sizeof(float);
    const std::size_t mapPayload=std::max(std::size_t(1),prepared.maps.size())*sizeof(NativeMapDescriptor);
    const std::size_t pixelPayload=std::max(std::size_t(1),prepared.map_pixels.size())*sizeof(float);
    id<MTLBuffer> maps=buffer(mapPayload+2*mapGuard),pixels=buffer(pixelPayload+2*pixelGuard);
    std::memcpy(input.contents,inputs.data(),input.length);
    auto runResources=[&](const NativeFormulaProgram& p,unsigned mode,unsigned slot,
        const std::vector<NativeMapDescriptor>& descriptors,const std::vector<float>& samples,unsigned visibleMaps,
        std::vector<U32>* rangeCounts=nullptr) {
        require(descriptors.size()==prepared.maps.size()&&samples.size()==prepared.map_pixels.size()&&visibleMaps<=descriptors.size(),"resource control allocation mismatch");
        auto upload=[&](id<MTLBuffer> b,std::size_t guard,std::size_t payload,const void* data,std::size_t bytes) {
            std::memset(b.contents,0xa5,b.length);auto* target=static_cast<unsigned char*>(b.contents)+guard;
            std::memset(target,0,payload);if(bytes)std::memcpy(target,data,bytes);
        };
        upload(maps,mapGuard,mapPayload,descriptors.data(),descriptors.size()*sizeof(NativeMapDescriptor));
        upload(pixels,pixelGuard,pixelPayload,samples.data(),samples.size()*sizeof(float));
        std::memcpy(program.contents,&p,sizeof(p));std::memset(output.contents,0xa5,output.length);
        if(rangeCounts)std::memset(rangeOutput.contents,0xa5,rangeOutput.length);
        // Requested work deliberately exceeds the independent allocation limit.
        const U32 launch[]={U32(inputs.size()+1),mode,slot,U32(inputs.size())};
        const U32 counts[]={visibleMaps,U32(samples.size())};
        auto command=[queue commandBuffer];auto encoder=[command computeCommandEncoder];
        [encoder setComputePipelineState:rangeCounts?rangePipeline:pointPipeline];
        [encoder setBuffer:program offset:0 atIndex:0];[encoder setBuffer:input offset:0 atIndex:1];
        [encoder setBuffer:output offset:0 atIndex:2];[encoder setBuffer:maps offset:mapGuard atIndex:3];[encoder setBuffer:pixels offset:pixelGuard atIndex:4];
        [encoder setBytes:launch length:sizeof(launch) atIndex:5];[encoder setBytes:counts length:sizeof(counts) atIndex:6];
        if(rangeCounts)[encoder setBuffer:rangeOutput offset:rangeGuard atIndex:7];
        execute(encoder,command,U32(inputs.size()+2));
        const auto* bytes=static_cast<const unsigned char*>(output.contents);
        for(std::size_t i=inputs.size()*sizeof(NativeOrbitResult);i<output.length;++i)require(bytes[i]==0xa5,"probe allocation canary modified");
        require(std::memcmp(input.contents,inputs.data(),input.length)==0,"probe mutated input states");
        require(std::memcmp(program.contents,&p,sizeof(p))==0,"probe mutated input program");
        if(rangeCounts) {
            const auto* bytes=static_cast<const unsigned char*>(rangeOutput.contents);
            for(std::size_t i=0;i<rangeGuard;++i)
                require(bytes[i]==0xa5&&bytes[rangeGuard+rangePayload+i]==0xa5,"range probe allocation canary modified");
            const auto* counts=reinterpret_cast<const U32*>(bytes+rangeGuard);
            rangeCounts->assign(counts,counts+inputs.size());
        }
        auto unchanged=[&](id<MTLBuffer> b,std::size_t guard,std::size_t payload,const void* data,std::size_t size) {
            const auto* bytes=static_cast<const unsigned char*>(b.contents);
            for(std::size_t i=0;i<guard;++i)require(bytes[i]==0xa5&&bytes[guard+payload+i]==0xa5,"resource allocation canary modified");
            if(size)require(std::memcmp(bytes+guard,data,size)==0,"probe mutated resource data");
            for(std::size_t i=size;i<payload;++i)require(bytes[guard+i]==0,"probe mutated dummy resource payload");
        };
        unchanged(maps,mapGuard,mapPayload,descriptors.data(),descriptors.size()*sizeof(NativeMapDescriptor));
        unchanged(pixels,pixelGuard,pixelPayload,samples.data(),samples.size()*sizeof(float));
        return std::vector<NativeOrbitResult>(static_cast<const NativeOrbitResult*>(output.contents),static_cast<const NativeOrbitResult*>(output.contents)+inputs.size());
    };
    auto run=[&](const NativeFormulaProgram& p,unsigned mode,unsigned slot) {
        return runResources(p,mode,slot,prepared.maps,prepared.map_pixels,U32(prepared.maps.size()));
    };
    const auto baseline=run(prepared.device,options.mode,options.slot);
    auto wrapperAudit=buffer(18*sizeof(U32));std::memset(wrapperAudit.contents,0xa5,wrapperAudit.length);
    auto wrapperAuditPipeline=pipeline("native_formula_wrapper_audit",true);
    {auto command=[queue commandBuffer];auto encoder=[command computeCommandEncoder];
        [encoder setComputePipelineState:wrapperAuditPipeline];
        [encoder setBuffer:program offset:0 atIndex:0];[encoder setBuffer:wrapperAudit offset:0 atIndex:1];
        execute(encoder,command,6);}
    const auto* wrapperWords=static_cast<const U32*>(wrapperAudit.contents);
    for(unsigned i=0;i<6;++i)require(wrapperWords[i*3]==1&&wrapperWords[i*3+1]==options.wrapper_policy,"wrapper policy not observed on GPU");
    for(unsigned i=1;i<options.repeats;++i) {
        const auto repeat=run(prepared.device,options.mode,options.slot);
        require(std::memcmp(baseline.data(),repeat.data(),baseline.size()*sizeof(NativeOrbitResult))==0,"point state repeat mismatch");
    }
    struct RangeControl {std::vector<NativeOrbitResult> rows;std::vector<U32> counts;};
    std::vector<RangeControl> rangeControls;
    bool rangeMatches=true,rangeRepeats=true;
    if(options.range_controls)for(unsigned repeat=0;repeat<options.repeats;++repeat) {
        RangeControl captured;
        captured.rows=runResources(prepared.device,options.mode,options.slot,prepared.maps,prepared.map_pixels,
                                  U32(prepared.maps.size()),&captured.counts);
        rangeMatches=rangeMatches&&std::memcmp(baseline.data(),captured.rows.data(),baseline.size()*sizeof(NativeOrbitResult))==0;
        if(repeat)rangeRepeats=rangeRepeats&&captured.counts==rangeControls[0].counts&&
            std::memcmp(captured.rows.data(),rangeControls[0].rows.data(),captured.rows.size()*sizeof(NativeOrbitResult))==0;
        rangeControls.push_back(std::move(captured));
    }
    for(unsigned control=0;control<7;++control) {
        auto invalid=prepared.device;if(control==0)++invalid.version;if(control==1)invalid.work_limit=0;
        if(control==4){invalid.slots[0].enabled=1;invalid.slots[0].namespace_id=ExternalFormula;
            invalid.slots[0].id=prepared.device.slots[0].id==0xfffffffeu?0xfffffffdu:0xfffffffeu;}
        if(control==5)invalid.driver=(prepared.device.driver+1)%10;
        if(control==6){invalid.slots[0].enabled=1;invalid.slots[0].namespace_id=1u-prepared.device.slots[0].namespace_id;}
        const auto records=run(invalid,control==2?3:options.mode,control==3?6:options.slot);
        for(const auto& r:records)require(r.fault.code==InvalidFormulaAbi&&!r.completed&&!r.fault.work,"invalid descriptor was not rejected before formula work");
    }
    struct ResourceControl {std::string name;unsigned expected_fault;std::vector<NativeOrbitResult> rows;};
    std::vector<ResourceControl> resourceControls;
    if(options.map_controls) {
        for(const auto& r:baseline)require(!r.fault.code&&r.completed,"active resource controls require successful baseline cases");
        const char* names[]={"reserved","channels","offset","flags","extent","element_count","missing",
            "empty_reserved","empty_channels","empty_offset","nonfinite_pixel"};
        const unsigned count=prepared.map_pixels.empty()?10:11;
        for(unsigned control=0;control<count;++control) {
            auto descriptors=prepared.maps;auto samples=prepared.map_pixels;
            const unsigned expectedFault=control==6?MissingFormulaResource:control==10?NonFiniteFormula:FormulaResourceBounds;
            for(auto& m:descriptors) {
                if(control>=7&&control<=9) {const int id=m.map_id;m={};m.map_id=id;m.channels=3;m.flags=FormulaMapEmpty;}
                if(control==0||control==7)m.reserved=1;
                if(control==1||control==8)m.channels=4;
                if(control==2||control==9)m.offset=0xffffffffu;
                if(control==3)m.flags=3;
                if(control==4)m.width=m.width?0:1;
                if(control==5)++m.element_count;
            }
            if(control==10) {const U32 nan=0x7fc00000u;for(auto& sample:samples)std::memcpy(&sample,&nan,sizeof(sample));}
            auto records=runResources(prepared.device,options.mode,options.slot,descriptors,samples,control==6?0:U32(descriptors.size()));
            for(const auto& r:records)require(r.fault.code==expectedFault&&!r.completed&&r.fault.work>0,
                std::string("active resource control failed: ")+names[control]);
            for(unsigned repeat=1;repeat<options.repeats;++repeat) {
                const auto repeated=runResources(prepared.device,options.mode,options.slot,descriptors,samples,control==6?0:U32(descriptors.size()));
                require(std::memcmp(records.data(),repeated.data(),records.size()*sizeof(NativeOrbitResult))==0,"resource control state repeat mismatch");
            }
            resourceControls.push_back({names[control],expectedFault,std::move(records)});
        }
    }
    unsigned faults=0;for(const auto& row:baseline)faults+=row.fault.code!=0||!row.completed;
    std::ofstream report(options.output);require(bool(report),"cannot create report");
    report<<"{\"schema\":\"mb3d.native-formula-probe.v1\",\"status\":"<<quoted(!rangeMatches||!rangeRepeats?"RANGE_CONTROL_MISMATCH":faults?"POINT_FAULTS":"COMPLETE")
        <<",\"qualification\":\"device execution, bounds and repeatability only; arithmetic corpus comparison required\",\"profile\":\"native_gpu_fp32\",\"library_profile\":";
    words(report,profile.contents,profile.length);report<<",\"formula_abi\":";
    words(report,marker.contents,marker.length);report<<",\"map_abi\":";
    if(mapMarker)words(report,mapMarker.contents,mapMarker.length);else report<<"null";
    report<<",\"wrapper_audit_words\":";words(report,wrapperAudit.contents,wrapperAudit.length);
    report<<",\"device\":"<<quoted(device.name.UTF8String)
        <<",\"scene\":"<<(options.scene.empty()?"null":quoted(fs::absolute(options.scene).string()))
        <<",\"program\":"<<(options.program.empty()?"null":quoted(fs::absolute(options.program).string()))<<",\"library\":"<<quoted(fs::absolute(options.library).string())
        <<",\"mode\":"<<options.mode<<",\"slot\":"<<options.slot<<",\"point_count\":"<<inputs.size()<<",\"repeats\":"<<options.repeats<<",\"wrapper_policy\":"<<options.wrapper_policy
        <<",\"exact_state_repeats\":true,\"input_immutable\":true,\"allocation_canaries\":true,\"invalid_descriptor_controls\":7,\"fault_count\":"<<faults<<",\"program_words\":";
    words(report,&prepared.device,sizeof(prepared.device));report<<",\"rows\":[";
    for(std::size_t i=0;i<inputs.size();++i) {if(i)report<<',';report<<"{\"index\":"<<i<<",\"input_words\":";words(report,&inputs[i],sizeof(inputs[i]));
        report<<",\"result_words\":";words(report,&baseline[i],sizeof(baseline[i]));report<<",\"fault\":"<<baseline[i].fault.code<<",\"work\":"<<baseline[i].fault.work<<'}';}
    report<<"],\"resources\":{\"schema\":\"mb3d.native-formula-resources.v1\",\"resource_root\":"<<quoted(options.root)
        <<",\"loader\":\"LoadLightMapNr via fresh Native RenderContext\",\"requested_map_ids\":[";
    for(std::size_t i=0;i<options.map_ids.size();++i){if(i)report<<',';report<<options.map_ids[i];}
    report<<"],\"descriptor_words\":[";
    for(std::size_t i=0;i<prepared.maps.size();++i){if(i)report<<',';words(report,&prepared.maps[i],sizeof(prepared.maps[i]));}
    report<<"],\"pixel_words\":";words(report,prepared.map_pixels.data(),prepared.map_pixels.size()*sizeof(float));
    report<<",\"map_count\":"<<prepared.maps.size()<<",\"pixel_count\":"<<prepared.map_pixels.size()
        <<",\"input_immutable\":true,\"allocation_canaries\":true,\"program_immutable\":true,\"control_mode\":"
        <<quoted(options.map_controls?"active":"none")<<",\"exact_control_repeats\":true,\"controls\":[";
    for(std::size_t i=0;i<resourceControls.size();++i){if(i)report<<',';const auto& control=resourceControls[i];
        report<<"{\"name\":"<<quoted(control.name)<<",\"expected_fault\":"<<control.expected_fault<<",\"repeats\":"<<options.repeats<<",\"rows\":[";
        for(std::size_t j=0;j<control.rows.size();++j){if(j)report<<',';report<<"{\"index\":"<<j<<",\"result_words\":";
            words(report,&control.rows[j],sizeof(control.rows[j]));report<<",\"fault\":"<<control.rows[j].fault.code
                <<",\"work\":"<<control.rows[j].fault.work<<",\"completed\":"<<control.rows[j].completed<<'}';}
        report<<"]}";
    }
    report<<"]}";
    if(options.range_controls) {
        report<<",\"range_controls\":{\"schema\":\"mb3d.native-formula-range-controls.v1\",\"qualification\":false,\"status\":"
            <<quoted(rangeMatches&&rangeRepeats?"EXACT_NONINTERFERENCE":"MISMATCH")
            <<",\"kernel\":\"native_formula_range_probe\",\"repeats\":"<<options.repeats
            <<",\"exact_result_matches_baseline\":"<<(rangeMatches?"true":"false")
            <<",\"exact_control_repeats\":"<<(rangeRepeats?"true":"false")
            <<",\"allocation_canaries\":true,\"input_immutable\":true,\"program_immutable\":true,\"resources_immutable\":true,\"runs\":[";
        for(std::size_t i=0;i<rangeControls.size();++i) {
            if(i)report<<',';report<<"{\"repeat\":"<<i+1<<",\"rows\":[";
            const auto& captured=rangeControls[i];
            for(std::size_t j=0;j<captured.rows.size();++j) {
                if(j)report<<',';report<<"{\"index\":"<<j<<",\"range_exits\":"<<captured.counts[j]<<",\"result_words\":";
                words(report,&captured.rows[j],sizeof(captured.rows[j]));
                report<<",\"fault\":"<<captured.rows[j].fault.code<<",\"work\":"<<captured.rows[j].fault.work
                    <<",\"completed\":"<<captured.rows[j].completed<<'}';
            }
            report<<"]}";
        }
        report<<"]}";
    }
    report<<"}\n";report.close();require(bool(report),"report write failed");
    std::cout<<"points="<<inputs.size()<<" repeats="<<options.repeats<<" faults="<<faults<<" controls=7 canaries=PASS\n";
    return !rangeMatches||!rangeRepeats?3:faults?2:0;
} catch(const std::exception& e) {std::cerr<<"native formula probe: "<<e.what()<<'\n';return 1;}}}
