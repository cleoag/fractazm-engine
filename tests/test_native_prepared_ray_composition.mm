#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "native_inside_trace_abi.h"
#include <algorithm>
#include <filesystem>
#include "src/render/native_metal/formula_selector.h"
#include <bit>
#include <cmath>
#include <cstring>
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include <stdexcept>
#include <unistd.h>
using namespace p2_inside;using namespace mb3d_native;
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(int argc,char** argv){@autoreleasepool{try{
 require(argc==7||argc==8,"usage: probe library output-directory reference-library repeat-index prepared-directory camera-library [fixed|bounded]");const unsigned selected_repeat=unsigned(std::stoul(argv[4]));require(selected_repeat<3,"repeat index");const bool bounded=argc==8&&std::string(argv[7])=="bounded";require(argc==7||bounded||std::string(argv[7])=="fixed","schedule profile");alarm(120);auto device=MTLCreateSystemDefaultDevice();require(device!=nil,"device");NSError* error=nil;
 auto library=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[1])] error:&error];require(library!=nil,"library");auto reference=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[3])] error:&error];require(reference!=nil,"reference");auto queue=[device newCommandQueue];
 Input prepared{};auto read=[&](const char* name,void* dst,size_t n){const auto path=std::filesystem::path(argv[5])/name;require(std::filesystem::file_size(path)==n,"prepared input ABI");std::ifstream f(path,std::ios::binary);f.read((char*)dst,n);require(bool(f),"input read");};
 read("scene.bin",&prepared.base.scene,sizeof(prepared.base.scene));read("program.bin",&prepared.base.program,sizeof(prepared.base.program));read("normal.bin",&prepared.base.normal,sizeof(prepared.base.normal));read("inside.bin",&prepared.policy,sizeof(prepared.policy));require(!prepared.base.program.formula_type&&!prepared.base.normal.smoothing,"single unsmoothed program required");
 auto camera_library=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[6])] error:&error];require(camera_library!=nil,"camera library");

 auto pipeline=[&](id<MTLLibrary> source,NSString* name){const auto pipeline_started=std::chrono::steady_clock::now();NSError* e=nil;auto values=[MTLFunctionConstantValues new];for(unsigned k=0;k<6;++k){const auto& slot=prepared.base.program.slots[k];require(ValidFormulaSelectorIdentity(slot.namespace_id,slot.id,slot.enabled),"selector");uint32_t id=FormulaPipelineSelector(slot.namespace_id,slot.id,slot.enabled);[values setConstantValue:&id type:MTLDataTypeUInt atIndex:10+k];}for(unsigned k=16;k<23;++k){uint32_t value=k==16?prepared.base.program.driver:k==17?1:0;[values setConstantValue:&value type:MTLDataTypeUInt atIndex:k];}auto fn=[source newFunctionWithName:name constantValues:values error:&e];require(fn!=nil,"function");auto p=[device newComputePipelineStateWithFunction:fn error:&e];if(!p)throw std::runtime_error(std::string(source==reference?"reference:":"candidate:")+name.UTF8String+": "+e.localizedDescription.UTF8String);std::cerr<<"pipeline status=complete reference="<<(source==reference)<<" function="<<name.UTF8String<<" seconds="<<std::chrono::duration<double>(std::chrono::steady_clock::now()-pipeline_started).count()<<"\n";return p;};
 auto abi=pipeline(library,@"abi"),reference_abi=pipeline(reference,@"abi");
 auto alloc=[&](size_t n){auto b=[device newBufferWithLength:n options:MTLResourceStorageModeShared];require(b!=nil,"allocation");memset(b.contents,0,n);return b;};
 unsigned commands=0;double max_gpu_seconds=0,max_command_seconds=0;
 auto run=[&](id<MTLComputePipelineState> p,unsigned n,auto bind){const auto started=std::chrono::steady_clock::now();auto c=[queue commandBuffer];auto e=[c computeCommandEncoder];[e setComputePipelineState:p];bind(e);[e dispatchThreads:MTLSizeMake(n,1,1) threadsPerThreadgroup:MTLSizeMake(std::min(n,32u),1,1)];[e endEncoding];[c commit];[c waitUntilCompleted];require(c.status==MTLCommandBufferStatusCompleted,"command failed");++commands;max_gpu_seconds=std::max(max_gpu_seconds,c.GPUEndTime-c.GPUStartTime);max_command_seconds=std::max(max_command_seconds,std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count());};
 auto sizes=alloc(16);run(abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});auto a=(uint32_t*)sizes.contents;require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[2]==sizeof(RealState)&&a[3]==RealProbeAbi,"ABI");
 memset(sizes.contents,0,sizes.length);run(reference_abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[3]==RealProbeAbi,"frozen control ABI");
 auto trace_abi=pipeline(library,@"trace_abi");memset(sizes.contents,0,sizes.length);run(trace_abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});require(a[0]==p2_inside_trace::Version&&a[1]==sizeof(p2_inside_trace::Snapshot)&&a[2]==sizeof(p2_inside_trace::Trace)&&a[3]==p2_inside_trace::Capacity,"inside trace ABI");
 auto original=pipeline(reference,@"original"),new_original=pipeline(library,@"original"),advance=pipeline(library,@"advance"),observe=pipeline(library,@"observe");
 const unsigned count=16;auto inputs=alloc(count*sizeof(Input)),baseline=alloc(count*sizeof(Output)),candidate=alloc(count*sizeof(Output)),states=alloc(count*sizeof(RealState));auto in=(Input*)inputs.contents;auto reference_trace=alloc(count*sizeof(p2_inside_trace::Trace)),candidate_trace=alloc(count*sizeof(p2_inside_trace::Trace));
 auto load=[&](const char* name){const auto path=std::filesystem::path(argv[5])/name;const size_t n=std::filesystem::file_size(path);require(n>0,"empty camera buffer");auto buffer=alloc(n);read(name,buffer.contents,n);return buffer;};
 auto camera=load("camera.bin"),columns=load("camera-columns.bin"),rows=load("camera-rows.bin");
 require(camera.length==sizeof(mb3d_camera::Params)&&columns.length==prepared.base.scene.width*sizeof(mb3d_camera::Axis)&&rows.length==prepared.base.scene.height*sizeof(mb3d_camera::Axis),"camera input layout");
 auto prepared_buffer=alloc(sizeof(prepared));memcpy(prepared_buffer.contents,&prepared,sizeof(prepared));auto prepare=pipeline(camera_library,@"prepare_inputs");run(prepare,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:prepared_buffer offset:0 atIndex:0];[e setBuffer:camera offset:0 atIndex:1];[e setBuffer:columns offset:0 atIndex:2];[e setBuffer:rows offset:0 atIndex:3];[e setBuffer:inputs offset:0 atIndex:4];});
 run(original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:baseline offset:0 atIndex:1];});
 auto save=[&](const char* name,id<MTLBuffer> b){std::ofstream f(std::string(argv[2])+"/"+name+".bin",std::ios::binary);f.write((const char*)b.contents,b.length);require(bool(f),"evidence write");};save("input",inputs);save("control",baseline);
 unsigned queries=0,no_fault=0,retries=0,smoothed=0,guards=0,attempts=0,hits=0,normal_calls=0,steps=0,work=0,direct_normals=0,de_normals=0;auto expected=(Output*)baseline.contents;
 for(unsigned i=0;i<count;++i){const auto& v=in[i].base;const auto& policy=in[i].policy;const auto& out=expected[i].base;const auto& activity=expected[i].activity;require(!out.trace.count&&!out.trace.overflow,"unobserved real control");queries+=out.geometry.de_calls-v.initial_de;no_fault+=out.geometry.fault==0;retries+=activity.retries;smoothed+=activity.smooth_calls;attempts+=activity.attempts;hits+=out.pixel.rm_result!=0;normal_calls+=out.normal_record.completed_evaluations;steps+=out.geometry.ray_steps-v.initial_ray;work+=out.geometry.formula_work-v.initial_work;
 if(v.normal.normals_on_de)de_normals+=out.normal_record.completed_evaluations;else direct_normals+=out.normal_record.completed_evaluations;
 require(out.pixel.fault==activity.fault&&activity.fault==out.geometry.fault,"terminal fault ownership");require(activity.retries<=1&&activity.attempts<=2&&activity.smooth_calls<=1,"inside bounds");
 if(!policy.version||policy.initial_inside==2){++guards;require(!activity.attempts&&out.geometry.de_calls==v.initial_de&&activity.fault==1,"policy guard before attempt");continue;}
 require(activity.attempts==activity.retries+1,"attempt accounting");require(activity.final_inside==(activity.retries?(policy.initial_inside?0:-1):policy.initial_inside),"raw signed inside retry word");require(activity.start_inside==out.pixel.start_inside,"start-inside ownership");
 }
 std::cerr<<"real inside fixture: queries="<<queries<<" no_fault="<<no_fault<<" retries="<<retries<<" smooth="<<smoothed<<" hits="<<hits<<" direct_normals="<<direct_normals<<" de_normals="<<de_normals<<" steps="<<steps<<" work="<<work<<"\n";
 require(queries>0&&no_fault>0&&attempts>0&&direct_normals+de_normals>0&&work>0,"vacuous prepared-ray fixture");
 run(new_original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];});require(!memcmp(baseline.contents,candidate.contents,baseline.length),"new original differs from frozen control");
 run(observe,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:reference_trace offset:0 atIndex:2];});save("observed",candidate);save("observed-trace",reference_trace);
 require(!memcmp(baseline.contents,candidate.contents,baseline.length),"observed inside differs from frozen original");
 unsigned boundaries=0,attempt_boundaries=0,smooth_boundaries=0;auto traces=(p2_inside_trace::Trace*)reference_trace.contents;
 for(unsigned i=0;i<count;++i){const auto& t=traces[i];require(!t.overflow&&!t.reserved[0]&&!t.reserved[1]&&t.count<=p2_inside_trace::Capacity&&!(t.count%2),"trace capacity/pair count");boundaries+=t.count;
 for(unsigned j=0;j<t.count;j+=2){const auto& before=t.snapshots[j];const auto& after=t.snapshots[j+1];require(before.version==1&&after.version==1&&before.site>=1&&before.site<=2&&before.site==after.site&&before.phase==1&&after.phase==2&&before.mask==before.site&&after.mask==before.mask,"canonical owner boundary identity");
 require(!memcmp(&before.retry,&after.retry,sizeof(before.retry))&&before.attempt==after.attempt&&!memcmp(&before.policy,&after.policy,sizeof(before.policy))&&!memcmp(&before.camera,&after.camera,sizeof(before.camera))&&!memcmp(&before.params,&after.params,sizeof(before.params))&&before.x==after.x&&before.y==after.y&&!memcmp(&before.activity,&after.activity,sizeof(before.activity)),"child mutated inside owner controls");
 for(const auto* q:{&before,&after})require(!q->reserved0&&!q->reserved[0]&&!q->reserved[1]&&!q->retry.reserved,"reserved owner fields");
 require(after.geometry.formula_work>=before.geometry.formula_work&&after.geometry.de_calls>=before.geometry.de_calls,"owner boundary work");
 if(before.site==1){++attempt_boundaries;require(before.attempt<2&&before.activity.attempts==before.attempt+1,"owner attempt index");}
 else{++smooth_boundaries;require(!before.attempt&&!after.attempt&&!memcmp(&before.pixel,&after.pixel,sizeof(before.pixel)),"post-smooth preserves pixel until result assignment");
 require(!memcmp(&before.geometry.initial_rout,&after.geometry.initial_rout,sizeof(float))&&!memcmp(&before.geometry.hit_color,&after.geometry.hit_color,sizeof(before.geometry.hit_color)),"post-smooth preserves captured geometry");}
 }}
 require(attempt_boundaries>0,"vacuous owner boundary fixture");
 const std::vector<unsigned char> pending_poison(sizeof(Output),0xa5);
 unsigned comparisons=0;const unsigned BurstOne=0xffffffffu,BurstThree=0xfffffffeu;const std::vector<unsigned> quanta=bounded?std::vector<unsigned>{31,127,1000,0,BurstOne,BurstThree}:std::vector<unsigned>{1,2,3,7,31,1000,0};for(unsigned repeat=selected_repeat;repeat<selected_repeat+1;++repeat)for(auto quantum:quanta){const auto schedule_started=std::chrono::steady_clock::now();const unsigned before_commands=commands;memset(states.contents,0,states.length);memset(candidate.contents,0xa5,candidate.length);memset(candidate_trace.contents,0,candidate_trace.length);
 for(unsigned step=0;step<32768;++step){const unsigned changing[]={1,2,3,7,31,127,511,1000};unsigned q=quantum==BurstOne?(step<512?1:511):quantum==BurstThree?(step<512?3:127):quantum?quantum:bounded?changing[step%8]:(step%4==0?7:step%4==1?1:step%4==2?3:2);
 run(advance,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:states offset:0 atIndex:2];[e setBytes:&q length:4 atIndex:3];[e setBuffer:candidate_trace offset:0 atIndex:4];});
 bool all=true;auto st=(RealState*)states.contents;for(unsigned i=0;i<count;++i){all&=st[i].done!=0;if(!st[i].done){auto bytes=(unsigned char*)candidate.contents+i*sizeof(Output);require(!memcmp(bytes,pending_poison.data(),pending_poison.size()),"pending publication");}}
 if(all)break;require(step!=32767,"unfinished");}
 if(quantum==(bounded?31u:1u)){save("candidate-first",candidate);save("candidate-trace-first",candidate_trace);}
 require(!memcmp(baseline.contents,candidate.contents,baseline.length),"primary state or boundary mismatch");require(!memcmp(reference_trace.contents,candidate_trace.contents,reference_trace.length),"canonical inside owner boundary mismatch");comparisons+=count;std::cerr<<"schedule="<<quantum<<" repeat="<<repeat<<" commands="<<commands-before_commands<<" seconds="<<std::chrono::duration<double>(std::chrono::steady_clock::now()-schedule_started).count()<<" max_gpu_seconds="<<max_gpu_seconds<<" max_command_seconds="<<max_command_seconds<<"\n";
 }
 save("candidate",candidate);save("candidate-trace",candidate_trace);
 std::cout<<"{\"scope\":\"prepared real camera inside/primary composition\",\"cases\":"<<count<<",\"comparisons\":"<<comparisons<<",\"queries_one_schedule\":"<<queries<<",\"without_fault\":"<<no_fault<<",\"boundaries_one_schedule\":"<<boundaries<<",\"attempt_boundaries\":"<<attempt_boundaries<<",\"smooth_boundaries\":"<<smooth_boundaries<<",\"retries\":"<<retries<<",\"smoothed\":"<<smoothed<<",\"guards\":"<<guards<<",\"attempts\":"<<attempts<<",\"hits\":"<<hits<<",\"normal_calls\":"<<normal_calls<<",\"direct_normals\":"<<direct_normals<<",\"de_normals\":"<<de_normals<<",\"formula_work\":"<<work<<",\"ray_steps\":"<<steps<<",\"repeat\":"<<selected_repeat<<",\"commands\":"<<commands<<",\"max_gpu_seconds\":"<<max_gpu_seconds<<",\"max_command_seconds\":"<<max_command_seconds<<",\"schedule_profile\":\""<<(bounded?"bounded":"fixed")<<"\",\"schedules\":"<<quanta.size()<<",\"differences\":0}\n";
 return 0;}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 2;}}}
