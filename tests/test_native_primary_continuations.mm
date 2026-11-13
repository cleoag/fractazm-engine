#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "native_primary_probe_abi.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <unistd.h>
using namespace p2_primary;
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(int argc,char** argv){@autoreleasepool{try{
 require(argc==4,"usage: probe library output-directory reference-library");alarm(120);auto device=MTLCreateSystemDefaultDevice();require(device!=nil,"device");NSError* error=nil;
 auto library=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[1])] error:&error];require(library!=nil,"library");auto reference=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[3])] error:&error];require(reference!=nil,"reference");auto queue=[device newCommandQueue];
 auto pipeline=[&](id<MTLLibrary> source,NSString* name){NSError* e=nil;auto values=[MTLFunctionConstantValues new];uint32_t id=0x80000002u,zero=0,one=1;for(unsigned k=10;k<16;++k)[values setConstantValue:&id type:MTLDataTypeUInt atIndex:k];for(unsigned k=16;k<23;++k)[values setConstantValue:k==17?&one:&zero type:MTLDataTypeUInt atIndex:k];auto fn=[source newFunctionWithName:name constantValues:values error:&e];require(fn!=nil,"function");auto p=[device newComputePipelineStateWithFunction:fn error:&e];if(!p)throw std::runtime_error(std::string(source==reference?"reference:":"candidate:")+name.UTF8String+": "+e.localizedDescription.UTF8String);return p;};
 auto abi=pipeline(library,@"abi"),reference_abi=pipeline(reference,@"abi");
 auto alloc=[&](size_t n){auto b=[device newBufferWithLength:n options:MTLResourceStorageModeShared];require(b!=nil,"allocation");memset(b.contents,0,n);return b;};
 auto run=[&](id<MTLComputePipelineState> p,unsigned n,auto bind){auto c=[queue commandBuffer];auto e=[c computeCommandEncoder];[e setComputePipelineState:p];bind(e);[e dispatchThreads:MTLSizeMake(n,1,1) threadsPerThreadgroup:MTLSizeMake(std::min(n,32u),1,1)];[e endEncoding];[c commit];[c waitUntilCompleted];require(c.status==MTLCommandBufferStatusCompleted,"command failed");};
 auto sizes=alloc(16);run(abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});auto a=(uint32_t*)sizes.contents;require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[2]==sizeof(State)&&a[3]==SyntheticProbeAbi,"ABI");
 memset(sizes.contents,0,sizes.length);run(reference_abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[3]==SyntheticProbeAbi,"frozen control ABI");
 auto original=pipeline(reference,@"original"),new_original=pipeline(library,@"original"),advance=pipeline(library,@"advance");
 const unsigned count=1024;auto inputs=alloc(count*sizeof(Input)),baseline=alloc(count*sizeof(Output)),candidate=alloc(count*sizeof(Output)),states=alloc(count*sizeof(State));auto in=(Input*)inputs.contents;
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
 if(i%31==0)v.fault_at=1+i%5;if(i%37==0)v.initial_fault=16;
 if(i%41==0)v.camera.cut_miss=1;if(i%43==0)v.camera.version=0;if(i%47==0)v.x=1;
 if(i%53==0)v.camera.origin[1]=std::bit_cast<float>(0x7fc00000u);
 if(i%59==0)v.distances[1]=std::numeric_limits<float>::infinity();if(i%61==0)v.distances[0]=std::bit_cast<float>(0x7fc00000u);
 }
 run(original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:baseline offset:0 atIndex:1];});
 unsigned queries=0,no_fault=0,inside=0,hits=0,misses=0,refined=0,normal_calls=0,iteration_queries=0,steps=0;auto expected=(Output*)baseline.contents;
 for(unsigned i=0;i<count;++i){const auto& v=in[i];const auto& out=expected[i];const auto& t=out.trace;require(!t.overflow&&t.count<=Capacity,"trace capacity");queries+=t.count;no_fault+=out.geometry.fault==0;inside+=out.pixel.start_inside!=0;hits+=!out.pixel.start_inside&&out.pixel.rm_result!=0;misses+=!out.pixel.rm_result&&!out.pixel.fault;refined+=out.geometry.refine_calls>v.initial_refine;steps+=out.geometry.ray_steps-v.initial_ray;
 require(out.pixel.fault==out.geometry.fault,"pixel/geometry fault");require(out.geometry.fault!=0x10000000u,"unaudited opaque call");
 if(!v.camera.version||v.x>=v.scene.width)require(!t.count&&out.geometry.fault==1,"admission before numerical children");
 NativeFormulaState prior{};prior.tmp=17;prior.xyz[3]=.375f;prior.inside=v.program.inside;
 uint32_t de=v.initial_de,work=v.initial_work;
 for(unsigned j=0;j<t.count;++j){const auto& q=t.queries[j];if(j)require(!memcmp(&q.before,&t.queries[j-1].after,sizeof(prior)),"carried lane");
 if(q.maximum==-7){++normal_calls;require(q.smooth==v.normal.normals_on_de&&q.temporary==uint32_t(v.program.inside!=0),"normal mode/orientation");}
 else{if(q.temporary){++iteration_queries;require(q.smooth==1&&q.maximum==v.program.max_iterations+1,"refinement temporary maximum");}else require(!q.smooth&&!q.maximum,"default DE contract");
 if(!q.before_record.fault){++de;work+=1+j%3;}}
 }
 if(t.count)require(!memcmp(&out.lane,&t.queries[t.count-1].after,sizeof(prior)),"terminal lane");require(out.geometry.de_calls==de&&out.geometry.formula_work==work,"work accounting");
 }
 require(queries>1000&&no_fault>300&&inside>100&&hits>100&&misses>50&&refined>100&&normal_calls>100&&iteration_queries>100&&steps>1000,"vacuous primary fixture");
 run(new_original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];});require(!memcmp(baseline.contents,candidate.contents,baseline.length),"new original differs from frozen control");
 auto save=[&](const char* name,id<MTLBuffer> b){std::ofstream f(std::string(argv[2])+"/"+name+".bin",std::ios::binary);f.write((const char*)b.contents,b.length);require(bool(f),"evidence write");};save("input",inputs);save("control",baseline);
 unsigned comparisons=0;const unsigned quanta[]={1,2,3,7,31,1000,0};for(unsigned repeat=0;repeat<3;++repeat)for(auto quantum:quanta){memset(states.contents,0,states.length);memset(candidate.contents,0xa5,candidate.length);
 for(unsigned step=0;step<1024;++step){unsigned q=quantum?quantum:(step%4==0?7:step%4==1?1:step%4==2?3:2);
 run(advance,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:states offset:0 atIndex:2];[e setBytes:&q length:4 atIndex:3];});
 bool all=true;auto st=(State*)states.contents;for(unsigned i=0;i<count;++i){all&=st[i].done!=0;if(!st[i].done){auto bytes=(unsigned char*)candidate.contents+i*sizeof(Output);for(unsigned k=0;k<sizeof(Output);++k)require(bytes[k]==0xa5,"pending publication");}}
 if(all)break;require(step!=1023,"unfinished");}
 if(!repeat&&quantum==1)save("candidate-first",candidate);
 require(!memcmp(baseline.contents,candidate.contents,baseline.length),"primary state or boundary mismatch");comparisons+=count;
 }
 save("candidate",candidate);
 std::cout<<"{\"scope\":\"synthetic primary children\",\"cases\":"<<count<<",\"comparisons\":"<<comparisons<<",\"queries_one_schedule\":"<<queries<<",\"without_fault\":"<<no_fault<<",\"inside\":"<<inside<<",\"later_hits\":"<<hits<<",\"misses\":"<<misses<<",\"refined_cases\":"<<refined<<",\"normal_calls\":"<<normal_calls<<",\"iteration_queries\":"<<iteration_queries<<",\"ray_steps\":"<<steps<<",\"differences\":0}\n";
 return 0;}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 2;}}}
