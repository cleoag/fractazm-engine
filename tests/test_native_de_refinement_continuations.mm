#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "native_de_refinement_probe_abi.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <unistd.h>
using namespace p2_de_refinement;
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(int argc,char** argv){@autoreleasepool{try{
 require(argc==4,"usage: probe library output-directory reference-library");alarm(120);auto device=MTLCreateSystemDefaultDevice();require(device!=nil,"device");NSError* error=nil;
 auto library=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[1])] error:&error];require(library!=nil,"library");auto reference=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[3])] error:&error];require(reference!=nil,"reference");auto queue=[device newCommandQueue];
 auto pipeline=[&](id<MTLLibrary> source,NSString* name){NSError* e=nil;auto fn=[source newFunctionWithName:name];require(fn!=nil,"function");auto p=[device newComputePipelineStateWithFunction:fn error:&e];if(!p)throw std::runtime_error(std::string(source==reference?"reference:":"candidate:")+name.UTF8String+": "+e.localizedDescription.UTF8String);return p;};
 auto abi=pipeline(library,@"abi"),reference_abi=pipeline(reference,@"abi");
 auto alloc=[&](size_t n){auto b=[device newBufferWithLength:n options:MTLResourceStorageModeShared];require(b!=nil,"allocation");memset(b.contents,0,n);return b;};
 auto run=[&](id<MTLComputePipelineState> p,unsigned n,auto bind){auto c=[queue commandBuffer];auto e=[c computeCommandEncoder];[e setComputePipelineState:p];bind(e);[e dispatchThreads:MTLSizeMake(n,1,1) threadsPerThreadgroup:MTLSizeMake(std::min(n,32u),1,1)];[e endEncoding];[c commit];[c waitUntilCompleted];require(c.status==MTLCommandBufferStatusCompleted,"command failed");};
 auto sizes=alloc(16);run(abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});auto a=(uint32_t*)sizes.contents;require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[2]==sizeof(State)&&a[3]==SyntheticProbeAbi,"ABI");
 memset(sizes.contents,0,sizes.length);run(reference_abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[3]==SyntheticProbeAbi,"frozen control ABI");
 auto original=pipeline(reference,@"original"),new_original=pipeline(library,@"original"),advance=pipeline(library,@"advance");
 const unsigned count=1024;auto inputs=alloc(count*sizeof(Input)),baseline=alloc(count*sizeof(Output)),candidate=alloc(count*sizeof(Output)),states=alloc(count*sizeof(State));auto in=(Input*)inputs.contents;
 for(unsigned i=0;i<count;++i){auto& v=in[i];auto& p=v.program;auto& scene=v.scene;
 p.version=kFormulaAbiVersion;p.driver=0;p.slot_count=6;p.max_iterations=4;p.work_limit=64;p.local_work_limit=16;p.radius_stop=16;p.gradient_radius_stop=16;p.log_radius_stop=1.4f;p.de_scale=1;p.de_offset=.001f;p.de_offset_floor=.0001f;p.stop_scale=1;for(auto& n:p.hybrid_counts)n=1;
 for(auto& slot:p.slots){slot.version=kFormulaAbiVersion;slot.namespace_id=InternalFormula;slot.id=2;slot.enabled=1;slot.negative_params[1]=1;slot.negative_params[2]=.25f;}
 scene.version=4;scene.formula_kind=kGeneralFormulaKind;scene.capability_id=kGeneralCapability;scene.width=scene.height=1;scene.max_steps=8;scene.max_iterations=4;scene.min_iterations=1;scene.slot_count=6;scene.refinement_steps=i%7;
 scene.max_ray_length=std::bit_cast<uint64_t>(10.);scene.de_stop=std::bit_cast<uint32_t>(.3f);scene.de_stop_factor=std::bit_cast<uint32_t>(.02f);scene.z_step_div=scene.step_limiter=scene.de_scale=scene.step_width_103=std::bit_cast<uint32_t>(1.f);
 v.iteration=77+i%13;v.initial_refine=i%5;v.initial_de=i%9;v.initial_work=i%17;v.depth=float(i%31)/7.f;v.base_stop=.3f;v.stop_factor=.02f;v.stop=.3f;v.de=v.stop+(i%4==0?.0005f:.5f);v.last_step=float(i%17)/9.f;
 for(unsigned k=0;k<3;++k){v.point[k]=float(int((i*17+k*13)%199)-99)/23.f;v.ray[k]=float(int((i*7+k*5)%19)-9)/11.f;}
 for(unsigned j=0;j<Capacity;++j){v.distances[j]=i%3==0?.1f:i%3==1?1.f:(j%2?.1f:1.f);v.iterations[j]=(i+j)%7;}
 if(i%31==0)v.fault_at=1+i%5;if(i%37==0)v.initial_fault=16;
 if(i%53==0)v.ray[1]=std::bit_cast<float>(0x7fc00000u);if(i%59==0)v.depth=std::numeric_limits<float>::infinity();if(i%61==0)v.distances[0]=std::bit_cast<float>(0x7fc00000u);
 if(i%47==0){scene.refinement_steps=255;v.fault_at=1;v.initial_fault=0;v.de=.8f;}
 }
 run(original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:baseline offset:0 atIndex:1];});
 unsigned queries=0,no_fault=0,no_moves=0,final_move=0,child_faults=0,max_steps=0;auto expected=(Output*)baseline.contents;
 for(unsigned i=0;i<count;++i){const auto& v=in[i];const auto& out=expected[i];const auto& t=out.trace;require(!t.overflow&&t.count<=Capacity,"tape capacity");queries+=t.count;no_fault+=out.geometry.fault==0;child_faults+=out.geometry.fault==32;
 const uint32_t moves=out.geometry.refine_calls-v.initial_refine;no_moves+=moves==0;final_move+=!out.geometry.fault&&moves==t.count+1;
 if(!v.scene.refinement_steps||!(std::abs(v.de-v.stop)>.001f))require(!moves&&!t.count&&out.iteration==v.iteration&&!memcmp(out.point,v.point,sizeof(v.point))&&!memcmp(&out.depth,&v.depth,sizeof(float)),"zero-work loop exit");
 if(v.scene.refinement_steps==1&&std::abs(v.de-v.stop)>.001f)require(moves==1&&!t.count&&out.iteration==v.iteration&&!memcmp(&out.de,&v.de,sizeof(float)),"last move has no DE");
 if(v.scene.refinement_steps==255){++max_steps;require(t.count==1&&out.geometry.fault==32,"maximum refinement first-child fault");}
 require(out.geometry.de_calls==v.initial_de+t.count&&moves>=t.count&&moves<=t.count+1,"query/move accounting");
 uint32_t work=v.initial_work,iteration=v.iteration;float de=v.de;NativeFormulaState prior{};prior.tmp=17;prior.xyz[3]=.375f;
 for(unsigned j=0;j<t.count;++j){const auto& q=t.queries[j];require(q.maximum==0&&q.smooth==0&&q.temporary==0,"default query contract");require(!memcmp(&q.before,&prior,sizeof(prior)),"carried lane");prior=q.after;work+=1+j%3;if(!q.after_record.fault){iteration=uint32_t(q.result.state.iteration);de=q.result.distance;}}
 require(!memcmp(&out.lane,&prior,sizeof(prior))&&out.geometry.formula_work==work&&out.iteration==iteration&&!memcmp(&out.de,&de,sizeof(float)),"final state/work/iteration/distance");
 }
 require(queries>1000&&no_fault>300&&no_moves>100&&final_move>100&&child_faults>10&&max_steps>10,"vacuous fixture");
 run(new_original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];});require(!memcmp(baseline.contents,candidate.contents,baseline.length),"new original differs from frozen control");
 auto save=[&](const char* name,id<MTLBuffer> b){std::ofstream f(std::string(argv[2])+"/"+name+".bin",std::ios::binary);f.write((const char*)b.contents,b.length);require(bool(f),"evidence write");};save("input",inputs);save("control",baseline);
 unsigned comparisons=0;const unsigned quanta[]={1,2,3,7,31,1000,0};for(unsigned repeat=0;repeat<3;++repeat)for(auto quantum:quanta){memset(states.contents,0,states.length);memset(candidate.contents,0xa5,candidate.length);
 for(unsigned step=0;step<256;++step){unsigned q=quantum?quantum:(step%4==0?7:step%4==1?1:step%4==2?3:2);
 run(advance,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:states offset:0 atIndex:2];[e setBytes:&q length:4 atIndex:3];});
 bool all=true;auto st=(State*)states.contents;for(unsigned i=0;i<count;++i){all&=st[i].done!=0;if(!st[i].done){auto bytes=(unsigned char*)candidate.contents+i*sizeof(Output);for(unsigned k=0;k<sizeof(Output);++k)require(bytes[k]==0xa5,"pending publication");}}
 if(all)break;require(step!=255,"unfinished");}
 if(!repeat&&quantum==1)save("candidate-first",candidate);
 require(!memcmp(baseline.contents,candidate.contents,baseline.length),"DE-refinement state or boundary mismatch");comparisons+=count;
 }
 save("candidate",candidate);
 std::cout<<"{\"scope\":\"synthetic primary DE-refinement children\",\"cases\":"<<count<<",\"comparisons\":"<<comparisons<<",\"queries_one_schedule\":"<<queries<<",\"without_child_fault\":"<<no_fault<<",\"zero_move_cases\":"<<no_moves<<",\"final_move_without_query\":"<<final_move<<",\"maximum_steps_faults\":"<<max_steps<<",\"child_faults\":"<<child_faults<<",\"differences\":0}\n";
 return 0;}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 2;}}}
