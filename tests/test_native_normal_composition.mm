#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "src/render/native_metal/continuation_abi.h"
#include <bit>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <unistd.h>
using namespace mb3d_native;using namespace mb3d_native::continuation;
struct Input {ExactScene scene;NativeFormulaProgram program;NativeNormalParams params;float center[3],depth,stop;};
struct SampleWitness {uint32_t phase,direct,smooth;float point[3],stop;NativeFormulaState lane;NativeGeometryRecord geometry;NativeNormalRecord normal;float value,result_smooth;};
struct Witness {uint32_t count,overflow;SampleWitness samples[14];};
struct Output {NativeNormalResult result;NativeFormulaState lane;NativeGeometryRecord geometry;NativeNormalRecord record;};
struct State {NormalFrame normal;NormalSampleFrame sample;DistanceEntryFrame entry;CombinedFrame combined;DistanceFrame distance;OrbitReplayFrame replay;OrbitPassFrame orbit;FormulaStepFrame step;WrapperFrame wrapper;BodyFrame body;Output output;uint32_t initialized,done;};
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(int argc,char** argv){@autoreleasepool{try{
 require(argc>=4&&argc<=7,"usage: probe library output-directory reference-library [smoothing-policy] [driver] [nonzero-gradient]");alarm(120);auto device=MTLCreateSystemDefaultDevice();require(device!=nil,"device");NSError* error=nil;
 auto library=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[1])] error:&error];require(library!=nil,"library");auto queue=[device newCommandQueue];
 auto reference=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[3])] error:&error];require(reference!=nil,"reference");
 const uint32_t smoothing_policy=argc>=5?unsigned(std::stoul(argv[4])):0;require(smoothing_policy<=1,"invalid smoothing policy");
 const uint32_t driver=argc>=6?unsigned(std::stoul(argv[5])):0;require(driver<8,"DIFS full-normal fixture is not implemented");
 const bool nonzero_gradient=argc>=7?std::stoul(argv[6])!=0:true;
 auto pipeline=[&](id<MTLLibrary> source,NSString* name){auto values=[MTLFunctionConstantValues new];uint32_t id=0x80000002u,zero=0;
 for(unsigned k=10;k<16;++k)[values setConstantValue:&id type:MTLDataTypeUInt atIndex:k];for(unsigned k=16;k<21;++k){const uint32_t v=k==17?smoothing_policy:k==16?driver:zero;[values setConstantValue:&v type:MTLDataTypeUInt atIndex:k];}
 NSError* e=nil;auto fn=[source newFunctionWithName:name constantValues:values error:&e];require(fn!=nil,"function");auto p=[device newComputePipelineStateWithFunction:fn error:&e];if(!p)throw std::runtime_error(std::string(source==reference?"reference:":"candidate:")+name.UTF8String+": "+e.localizedDescription.UTF8String);return p;};
 auto abi=pipeline(library,@"abi"),original=pipeline(reference,@"original"),new_original=pipeline(library,@"original"),advance=pipeline(library,@"advance"),observe=pipeline(library,@"observe");
 auto alloc=[&](size_t n){auto b=[device newBufferWithLength:n options:MTLResourceStorageModeShared];require(b!=nil,"allocation");memset(b.contents,0,n);return b;};
 auto run=[&](id<MTLComputePipelineState> p,unsigned n,auto bind){auto c=[queue commandBuffer];auto e=[c computeCommandEncoder];[e setComputePipelineState:p];bind(e);[e dispatchThreads:MTLSizeMake(n,1,1) threadsPerThreadgroup:MTLSizeMake(std::min(n,32u),1,1)];[e endEncoding];[c commit];[c waitUntilCompleted];require(c.status==MTLCommandBufferStatusCompleted,"command failed");};
 auto sizes=alloc(16);run(abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});auto a=(uint32_t*)sizes.contents;require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[2]==sizeof(State)&&a[3]==sizeof(Witness),"ABI");
 auto reference_abi=pipeline(reference,@"abi");run(reference_abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});require(a[0]==sizeof(Input)&&a[1]==sizeof(Output),"frozen control ABI mismatch");
 const unsigned count=1024;auto inputs=alloc(count*sizeof(Input)),baseline=alloc(count*sizeof(Output)),candidate=alloc(count*sizeof(Output)),states=alloc(count*sizeof(State));auto witnesses=alloc(count*sizeof(Witness)),observed=alloc(count*sizeof(Witness));auto in=(Input*)inputs.contents;
 for(unsigned i=0;i<count;++i){auto& v=in[i];auto& s=v.scene;auto& p=v.program;
 p.version=kFormulaAbiVersion;p.driver=driver;p.slot_count=6;p.max_iterations=4;p.work_limit=64;p.local_work_limit=16;p.radius_stop=16;p.gradient_radius_stop=16;p.stop_scale=1;p.de_option=int(i%10);
 p.de_scale=1.f;if(nonzero_gradient){p.de_offset=.001f;p.de_offset_floor=.0001f;p.log_radius_stop=1.4f;}for(auto& value:p.hybrid_counts)value=1;
 if(driver==2||driver==3||driver==6||driver==7)for(unsigned j=0;j<4;++j)for(unsigned k=0;k<4;++k)p.rotation[j][k]=j==k?1.f:.03f*float(j+k);
 if(driver>=4&&driver<=7){p.hybrid_counts[0]=std::bit_cast<int32_t>(.3f);p.hybrid_counts[1]=std::bit_cast<int32_t>(.7f);}
 for(auto& slot:p.slots){slot.version=kFormulaAbiVersion;slot.namespace_id=InternalFormula;slot.id=2;slot.enabled=1;slot.negative_params[1]=1.f;slot.negative_params[2]=.25f;}
 s.version=4;s.formula_kind=kGeneralFormulaKind;s.capability_id=kGeneralCapability;s.width=s.height=1;s.max_steps=8;s.max_iterations=4;s.min_iterations=1;s.slot_count=6;s.refinement_steps=2;
 s.max_ray_length=std::bit_cast<uint64_t>(10.);s.de_stop=std::bit_cast<uint32_t>(.3f);s.de_stop_factor=std::bit_cast<uint32_t>(.02f);s.z_step_div=s.step_limiter=s.de_scale=s.step_width_103=std::bit_cast<uint32_t>(1.f);
 for(unsigned k=0;k<9;++k)s.vgrads[k]=std::bit_cast<uint64_t>(double(int((i*19+k*7)%81)-40)/17.);
 v.params.version=kGeneralNormalVersion;v.params.normals_on_de=i%2;v.params.inside=(i/2)%2;
 for(unsigned k=0;k<3;++k)v.center[k]=float(int((i*17+k*13)%199)-99)/23.f;v.depth=float(i%29)/7.f;v.stop=.01f;
 if(driver==1&&i%97==0){p.de_option=16;v.center[0]=1e10f;v.center[1]=-2e10f;v.center[2]=3e10f;}
 if(i%31==0)v.center[0]=std::bit_cast<float>(0x7fc00000u);if(i%37==0)v.params.reserved[0]=1;if(i%41==0)s.version=0;if(i%43==0)v.params.version=0;
 }
 run(original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:baseline offset:0 atIndex:1];});
 unsigned active=0,completed=0,ranges=0,direct_completed=0,de_completed=0;auto expected=(Output*)baseline.contents;for(unsigned i=0;i<count;++i){active+=expected[i].record.sample_calls;completed+=expected[i].result.completed;ranges+=expected[i].geometry.fp32_range_exits;if(in[i].params.normals_on_de)de_completed+=expected[i].result.completed;else direct_completed+=expected[i].result.completed;}require(active>1000&&completed>100,"vacuous fixture");if(driver==1)require(ranges>0,"missing range-exit coverage");
 if(nonzero_gradient){require(de_completed>100,"missing successful DE normals");if(driver==0||driver==2||driver==4||driver==6)require(direct_completed>100,"missing successful direct normals");}
 run(new_original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];});require(memcmp(baseline.contents,candidate.contents,baseline.length)==0,"new original differs from frozen control");
 run(observe,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:observed offset:0 atIndex:4];});
 require(memcmp(baseline.contents,candidate.contents,baseline.length)==0,"observer differs from frozen control");
 unsigned comparisons=0;const unsigned quanta[]={1,2,3,7,31,1000,0};for(unsigned repeat=0;repeat<3;++repeat)for(auto quantum:quanta){
 memset(states.contents,0,states.length);memset(candidate.contents,0xa5,candidate.length);
 for(unsigned step=0;step<8192;++step){unsigned q=quantum?quantum:(step%4==0?7:step%4==1?1:step%4==2?3:2);
 run(advance,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:states offset:0 atIndex:2];[e setBytes:&q length:4 atIndex:3];[e setBuffer:witnesses offset:0 atIndex:4];});
 bool all=true;auto st=(State*)states.contents;for(unsigned i=0;i<count;++i){all&=st[i].done!=0;if(!st[i].done){auto bytes=(unsigned char*)candidate.contents+i*sizeof(Output);for(unsigned k=0;k<sizeof(Output);++k)require(bytes[k]==0xa5,"pending publication");}}
 if(all)break;require(step!=8191,"unfinished");}
 if(repeat==0&&quantum==1){for(auto item:{std::pair<const char*,id<MTLBuffer>>{"input-first",inputs},{"control-first",baseline},{"candidate-first",candidate},{"observed-first",observed},{"witness-first",witnesses}}){std::ofstream f(std::string(argv[2])+"/"+item.first+".bin",std::ios::binary);f.write((const char*)item.second.contents,item.second.length);require(bool(f),"failure evidence");}}
 require(memcmp(observed.contents,witnesses.contents,observed.length)==0,"canonical sample boundaries differ");
 auto actual=(Output*)candidate.contents;for(unsigned i=0;i<count;++i){if(memcmp(expected+i,actual+i,sizeof(Output))){auto old=(uint32_t*)(expected+i),now=(uint32_t*)(actual+i);for(unsigned k=0;k<sizeof(Output)/4;++k)if(old[k]!=now[k])std::cerr<<"case "<<i<<" word "<<k<<" old "<<std::hex<<old[k]<<" new "<<now[k]<<std::dec<<"\n";throw std::runtime_error("outer mismatch");}++comparisons;}
 }
 for(auto item:{std::pair<const char*,id<MTLBuffer>>{"input",inputs},{"control",baseline},{"candidate",candidate}}){std::ofstream f(std::string(argv[2])+"/"+item.first+".bin",std::ios::binary);f.write((const char*)item.second.contents,item.second.length);require(bool(f),"write");}
 // Unsupported smoothing is refused before a scripted child or publication.
 for(unsigned i=0;i<count;++i)in[i].params.smoothing=8;
 memset(states.contents,0,states.length);memset(candidate.contents,0xa5,candidate.length);unsigned q=1;
 run(advance,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:states offset:0 atIndex:2];[e setBytes:&q length:4 atIndex:3];[e setBuffer:witnesses offset:0 atIndex:4];});
 for(unsigned i=0;i<count;++i){const auto& out=((Output*)candidate.contents)[i];require(out.record.sample_calls==0&&!out.result.completed&&out.result.fault==1,"smoothing admission");}
 std::cout<<"{\"scope\":\"real internal-formula normal composition\",\"cases\":"<<count<<",\"comparisons\":"<<comparisons<<",\"child_calls_one_schedule\":"<<active<<",\"completed_normals_one_schedule\":"<<completed<<",\"direct_completed_one_schedule\":"<<direct_completed<<",\"de_completed_one_schedule\":"<<de_completed<<",\"nonzero_gradient\":"<<unsigned(nonzero_gradient)<<",\"differences\":0}\n";
 return 0;}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 2;}}}
