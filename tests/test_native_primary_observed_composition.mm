#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "native_primary_trace_abi.h"
#include <algorithm>
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
using namespace p2_primary;
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(int argc,char** argv){@autoreleasepool{try{
 require(argc==5,"usage: probe library output-directory reference-library repeat-index");const unsigned selected_repeat=unsigned(std::stoul(argv[4]));require(selected_repeat<3,"repeat index");alarm(120);auto device=MTLCreateSystemDefaultDevice();require(device!=nil,"device");NSError* error=nil;
 auto library=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[1])] error:&error];require(library!=nil,"library");auto reference=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[3])] error:&error];require(reference!=nil,"reference");auto queue=[device newCommandQueue];
 auto pipeline=[&](id<MTLLibrary> source,NSString* name){NSError* e=nil;auto values=[MTLFunctionConstantValues new];uint32_t id=0x80000002u,zero=0,one=1;for(unsigned k=10;k<16;++k)[values setConstantValue:&id type:MTLDataTypeUInt atIndex:k];for(unsigned k=16;k<23;++k)[values setConstantValue:k==17?&one:&zero type:MTLDataTypeUInt atIndex:k];auto fn=[source newFunctionWithName:name constantValues:values error:&e];require(fn!=nil,"function");auto p=[device newComputePipelineStateWithFunction:fn error:&e];if(!p)throw std::runtime_error(std::string(source==reference?"reference:":"candidate:")+name.UTF8String+": "+e.localizedDescription.UTF8String);return p;};
 auto abi=pipeline(library,@"abi"),reference_abi=pipeline(reference,@"abi");
 auto alloc=[&](size_t n){auto b=[device newBufferWithLength:n options:MTLResourceStorageModeShared];require(b!=nil,"allocation");memset(b.contents,0,n);return b;};
 unsigned commands=0;double max_gpu_seconds=0,max_command_seconds=0;
 auto run=[&](id<MTLComputePipelineState> p,unsigned n,auto bind){const auto started=std::chrono::steady_clock::now();auto c=[queue commandBuffer];auto e=[c computeCommandEncoder];[e setComputePipelineState:p];bind(e);[e dispatchThreads:MTLSizeMake(n,1,1) threadsPerThreadgroup:MTLSizeMake(std::min(n,32u),1,1)];[e endEncoding];[c commit];[c waitUntilCompleted];require(c.status==MTLCommandBufferStatusCompleted,"command failed");++commands;max_gpu_seconds=std::max(max_gpu_seconds,c.GPUEndTime-c.GPUStartTime);max_command_seconds=std::max(max_command_seconds,std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count());};
 auto sizes=alloc(16);run(abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});auto a=(uint32_t*)sizes.contents;require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[2]==sizeof(RealState)&&a[3]==RealProbeAbi,"ABI");
 memset(sizes.contents,0,sizes.length);run(reference_abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[3]==RealProbeAbi,"frozen control ABI");
 auto trace_abi=pipeline(library,@"trace_abi");memset(sizes.contents,0,sizes.length);run(trace_abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});require(a[0]==p2_primary_trace::Version&&a[1]==sizeof(p2_primary_trace::Snapshot)&&a[2]==sizeof(p2_primary_trace::Trace)&&a[3]==p2_primary_trace::Capacity,"trace ABI");
 auto original=pipeline(reference,@"original"),new_original=pipeline(library,@"original"),advance=pipeline(library,@"advance"),observe=pipeline(library,@"observe");
 const unsigned count=256;auto inputs=alloc(count*sizeof(Input)),baseline=alloc(count*sizeof(Output)),candidate=alloc(count*sizeof(Output)),states=alloc(count*sizeof(RealState));auto in=(Input*)inputs.contents;auto reference_trace=alloc(count*sizeof(p2_primary_trace::Trace)),candidate_trace=alloc(count*sizeof(p2_primary_trace::Trace));
 for(unsigned i=0;i<count;++i){auto& v=in[i];auto& p=v.program;auto& scene=v.scene;
 p.version=kFormulaAbiVersion;p.driver=0;p.slot_count=6;p.max_iterations=4;p.work_limit=64;p.local_work_limit=16;p.radius_stop=16;p.gradient_radius_stop=16;p.log_radius_stop=1.4f;p.de_scale=1;p.de_offset=.001f;p.de_offset_floor=.0001f;p.stop_scale=1;p.inside=i%2?0xffffffffu:0;for(auto& n:p.hybrid_counts)n=1;
 for(auto& slot:p.slots){slot.version=kFormulaAbiVersion;slot.namespace_id=InternalFormula;slot.id=2;slot.enabled=1;slot.negative_params[1]=1;slot.negative_params[2]=.25f;}
 scene.version=4;scene.formula_kind=kGeneralFormulaKind;scene.capability_id=kGeneralCapability;scene.width=scene.height=1;scene.max_steps=8;scene.max_iterations=4;scene.min_iterations=1;scene.slot_count=6;scene.refinement_steps=(i/8)%5;
 scene.max_ray_length=std::bit_cast<uint64_t>(i%8==0?.6:10.);scene.de_stop=std::bit_cast<uint32_t>(.3f);scene.de_stop_factor=std::bit_cast<uint32_t>(.02f);scene.z_step_div=scene.step_limiter=scene.de_scale=scene.step_width_103=std::bit_cast<uint32_t>(1.f);scene.fog_iteration=(i/8)%3;
 v.normal.version=1;v.normal.normals_on_de=i%2;
 v.initial_refine=i%5;v.initial_de=i%9;v.initial_work=i%17;v.initial_ray=i%3;
 v.camera.version=mb3d_camera::Version;v.camera.direction[0]=.125f;v.camera.direction[2]=-1.f;v.camera.cut_depth=float(i%3)/10.f;v.camera.cut_axis=i%4;
 for(unsigned k=0;k<3;++k)v.camera.origin[k]=float(int((i*17+k*13)%199)-99)/23.f;
 for(unsigned j=0;j<Capacity;++j){v.distances[j]=1.f;v.iterations[j]=2;v.smooth[j]=2.f+float(j%3)*.25f;
 if(i%8==1&&j>0)v.distances[j]=j%2?.1f:.8f;
 if(i%8==2&&j==1)v.iterations[j]=4;if(i%8==2&&j>=2)v.distances[j]=.1f;
 if(i%8==3&&j>0){v.iterations[j]=4;v.distances[j]=.8f;}
 if(i%8==4)v.distances[j]=.1f;if(i%8==5)v.iterations[j]=4;
 if(i%8==7&&j>=2)v.distances[j]=.1f;}
 v.camera.direction[0]=0.f;v.camera.direction[2]=-1.f;v.camera.cut_depth=0.f;v.camera.cut_axis=0;
 v.camera.origin[0]=float(int(i%16)-8)*.16f;v.camera.origin[1]=float(int(i/16)-8)*.16f;v.camera.origin[2]=i%8==4?.1f:2.5f+float(i%3)*.2f;
 if(i%8==4){v.camera.origin[0]=.1f;v.camera.origin[1]=.1f;}
 p.max_iterations=8;scene.max_iterations=8;
 scene.max_steps=32;scene.max_ray_length=std::bit_cast<uint64_t>(6.);scene.fog_iteration=0;p.inside=0;
 if(i%31==0)p.work_limit=1;if(i%37==0)v.initial_fault=16;
 if(i%41==0)v.camera.cut_miss=1;if(i%43==0)v.camera.version=0;if(i%47==0)v.x=1;
 if(i%53==0)v.camera.origin[1]=std::bit_cast<float>(0x7fc00000u);
 }
 run(original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:baseline offset:0 atIndex:1];});
 auto save=[&](const char* name,id<MTLBuffer> b){std::ofstream f(std::string(argv[2])+"/"+name+".bin",std::ios::binary);f.write((const char*)b.contents,b.length);require(bool(f),"evidence write");};save("input",inputs);save("control",baseline);
 unsigned queries=0,no_fault=0,inside=0,hits=0,misses=0,refined=0,normal_calls=0,steps=0,work=0;auto expected=(Output*)baseline.contents;
 for(unsigned i=0;i<count;++i){const auto& v=in[i];const auto& out=expected[i];require(!out.trace.count&&!out.trace.overflow,"unobserved real control");queries+=out.geometry.de_calls-v.initial_de;no_fault+=out.geometry.fault==0;inside+=out.pixel.start_inside!=0;hits+=!out.pixel.start_inside&&out.pixel.rm_result!=0;misses+=!out.pixel.rm_result&&!out.pixel.fault;refined+=out.geometry.refine_calls>v.initial_refine;normal_calls+=out.normal_record.completed_evaluations;steps+=out.geometry.ray_steps-v.initial_ray;work+=out.geometry.formula_work-v.initial_work;
 require(out.pixel.fault==out.geometry.fault,"pixel/geometry fault");
 if(!v.camera.version||v.x>=v.scene.width)require(out.geometry.de_calls==v.initial_de&&out.geometry.fault==1,"admission before numerical children");
 }
 std::cerr<<"real fixture: queries="<<queries<<" no_fault="<<no_fault<<" inside="<<inside<<" hits="<<hits<<" misses="<<misses<<" refined="<<refined<<" normals="<<normal_calls<<" steps="<<steps<<" work="<<work<<"\n";
 require(queries>100&&no_fault>100&&inside>10&&hits>10&&misses>10&&refined>10&&normal_calls>10&&steps>100&&work>1000,"vacuous real primary fixture");
 run(new_original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];});require(!memcmp(baseline.contents,candidate.contents,baseline.length),"new original differs from frozen control");
 run(observe,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:reference_trace offset:0 atIndex:2];});save("observed",candidate);save("observed-trace",reference_trace);
 require(!memcmp(baseline.contents,candidate.contents,baseline.length),"observed primary differs from frozen original");
 unsigned boundaries=0;unsigned sites[7]={};auto traces=(p2_primary_trace::Trace*)reference_trace.contents;
 for(unsigned i=0;i<count;++i){const auto& t=traces[i];require(!t.overflow&&!t.reserved[0]&&!t.reserved[1]&&t.count<=p2_primary_trace::Capacity&&!(t.count%2),"trace capacity/pair count");boundaries+=t.count;
 for(unsigned j=0;j<t.count;j+=2){const auto& before=t.snapshots[j];const auto& after=t.snapshots[j+1];require(before.version==1&&after.version==1&&before.site>=1&&before.site<=6&&before.site==after.site&&before.phase==1&&after.phase==2&&before.mask==after.mask,"canonical boundary identity");++sites[before.site];
 const unsigned site=before.site;const uint32_t mask=1u|(site!=1?2u:0u)|(site>=2&&site<=5?4u:0u)|(site==2?8u:0u)|(site==4||site==5?16u:0u);require(before.mask==mask,"scope mask");
 for(const auto* q:{&before,&after}){require(q->scalars[15]==0.f&&!q->integers[6]&&!q->integers[7]&&!q->reserved[0]&&!q->reserved[1],"reserved snapshot fields");
 if(!(mask&2))require(q->scalars[11]==0.f&&!q->integers[0]&&!q->integers[1]&&!q->integers[2],"initial-query inactive fields");
 if(!(mask&4))require(q->scalars[12]==0.f&&q->scalars[13]==0.f&&!q->integers[3]&&!q->integers[4],"inactive march scope");
 if(!(mask&8))require(q->scalars[14]==0.f,"inactive backup step");if(!(mask&16))require(!q->integers[5],"inactive contact flag");}
 require(after.geometry.formula_work>=before.geometry.formula_work&&after.geometry.de_calls>=before.geometry.de_calls,"boundary work accounting");
 if(site<=3){require(!memcmp(before.vectors,after.vectors,sizeof(before.vectors))&&!memcmp(before.scalars,after.scalars,sizeof(before.scalars))&&!memcmp(before.integers,after.integers,sizeof(before.integers)),"DE cannot mutate caller locals");}
 }}
 require(boundaries>100&&sites[1]>100&&sites[3]>100&&sites[2]>10&&sites[4]>10&&sites[5]>10&&sites[6]>10,"vacuous boundary fixture");
 const std::vector<unsigned char> pending_poison(sizeof(Output),0xa5);
 unsigned comparisons=0;const unsigned quanta[]={1,2,3,7,31,1000,0};for(unsigned repeat=selected_repeat;repeat<selected_repeat+1;++repeat)for(auto quantum:quanta){const auto schedule_started=std::chrono::steady_clock::now();const unsigned before_commands=commands;memset(states.contents,0,states.length);memset(candidate.contents,0xa5,candidate.length);memset(candidate_trace.contents,0,candidate_trace.length);
 for(unsigned step=0;step<32768;++step){unsigned q=quantum?quantum:(step%4==0?7:step%4==1?1:step%4==2?3:2);
 run(advance,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:states offset:0 atIndex:2];[e setBytes:&q length:4 atIndex:3];[e setBuffer:candidate_trace offset:0 atIndex:4];});
 bool all=true;auto st=(RealState*)states.contents;for(unsigned i=0;i<count;++i){all&=st[i].done!=0;if(!st[i].done){auto bytes=(unsigned char*)candidate.contents+i*sizeof(Output);require(!memcmp(bytes,pending_poison.data(),pending_poison.size()),"pending publication");}}
 if(all)break;require(step!=32767,"unfinished");}
 if(quantum==1){save("candidate-first",candidate);save("candidate-trace-first",candidate_trace);}
 require(!memcmp(baseline.contents,candidate.contents,baseline.length),"primary state or boundary mismatch");require(!memcmp(reference_trace.contents,candidate_trace.contents,reference_trace.length),"canonical primary boundary mismatch");comparisons+=count;std::cerr<<"schedule="<<quantum<<" repeat="<<repeat<<" commands="<<commands-before_commands<<" seconds="<<std::chrono::duration<double>(std::chrono::steady_clock::now()-schedule_started).count()<<" max_gpu_seconds="<<max_gpu_seconds<<" max_command_seconds="<<max_command_seconds<<"\n";
 }
 save("candidate",candidate);save("candidate-trace",candidate_trace);
 std::cout<<"{\"scope\":\"real observed primary composition\",\"cases\":"<<count<<",\"comparisons\":"<<comparisons<<",\"queries_one_schedule\":"<<queries<<",\"without_fault\":"<<no_fault<<",\"boundaries_one_schedule\":"<<boundaries<<",\"backup_calls\":"<<sites[2]<<",\"iteration_refinements\":"<<sites[5]<<",\"inside\":"<<inside<<",\"later_hits\":"<<hits<<",\"misses\":"<<misses<<",\"refined_cases\":"<<refined<<",\"normal_calls\":"<<normal_calls<<",\"formula_work\":"<<work<<",\"ray_steps\":"<<steps<<",\"repeat\":"<<selected_repeat<<",\"commands\":"<<commands<<",\"max_gpu_seconds\":"<<max_gpu_seconds<<",\"max_command_seconds\":"<<max_command_seconds<<",\"differences\":0}\n";
 return 0;}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 2;}}}
