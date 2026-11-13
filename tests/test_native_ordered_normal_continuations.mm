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
struct Trace {uint32_t count;float points[7][3];uint32_t direct[7],smooth[7];};
struct Input {ExactScene scene;NativeFormulaProgram program;NativeNormalParams params;float center[3],depth,stop;};
struct Output {NativeNormalResult result;NativeFormulaState lane;NativeGeometryRecord geometry;NativeNormalRecord record;Trace trace;};
struct State {NormalFrame normal;NormalSampleFrame sample;Output output;uint32_t initialized,done;};
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(int argc,char** argv){@autoreleasepool{try{
 require(argc==3,"usage: probe library output-directory");alarm(120);auto device=MTLCreateSystemDefaultDevice();require(device!=nil,"device");NSError* error=nil;
 auto library=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[1])] error:&error];require(library!=nil,"library");auto queue=[device newCommandQueue];
 auto pipeline=[&](NSString* name){auto values=[MTLFunctionConstantValues new];uint32_t id=0x80000002u,zero=0;
 for(unsigned k=10;k<16;++k)[values setConstantValue:&id type:MTLDataTypeUInt atIndex:k];for(unsigned k=16;k<21;++k)[values setConstantValue:&zero type:MTLDataTypeUInt atIndex:k];
 NSError* e=nil;auto fn=[library newFunctionWithName:name constantValues:values error:&e];require(fn!=nil,"function");auto p=[device newComputePipelineStateWithFunction:fn error:&e];if(!p)throw std::runtime_error(e.localizedDescription.UTF8String);return p;};
 auto abi=pipeline(@"abi"),original=pipeline(@"original"),advance=pipeline(@"advance");
 auto alloc=[&](size_t n){auto b=[device newBufferWithLength:n options:MTLResourceStorageModeShared];require(b!=nil,"allocation");memset(b.contents,0,n);return b;};
 auto run=[&](id<MTLComputePipelineState> p,unsigned n,auto bind){auto c=[queue commandBuffer];auto e=[c computeCommandEncoder];[e setComputePipelineState:p];bind(e);[e dispatchThreads:MTLSizeMake(n,1,1) threadsPerThreadgroup:MTLSizeMake(std::min(n,32u),1,1)];[e endEncoding];[c commit];[c waitUntilCompleted];require(c.status==MTLCommandBufferStatusCompleted,"command failed");};
 auto sizes=alloc(12);run(abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});auto a=(uint32_t*)sizes.contents;require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[2]==sizeof(State),"ABI");
 const unsigned count=1024;auto inputs=alloc(count*sizeof(Input)),baseline=alloc(count*sizeof(Output)),candidate=alloc(count*sizeof(Output)),states=alloc(count*sizeof(State));auto in=(Input*)inputs.contents;
 for(unsigned i=0;i<count;++i){auto& v=in[i];auto& s=v.scene;auto& p=v.program;
 p.version=kFormulaAbiVersion;p.driver=0;p.slot_count=6;p.max_iterations=4;p.work_limit=64;p.local_work_limit=16;p.radius_stop=16;p.gradient_radius_stop=16;p.stop_scale=1;p.de_option=int(i%10);
 for(auto& slot:p.slots){slot.version=kFormulaAbiVersion;slot.namespace_id=InternalFormula;slot.id=2;slot.enabled=1;}
 s.version=4;s.formula_kind=kGeneralFormulaKind;s.capability_id=kGeneralCapability;s.width=s.height=1;s.max_steps=8;s.max_iterations=4;s.min_iterations=1;s.slot_count=6;s.refinement_steps=2;
 s.max_ray_length=std::bit_cast<uint64_t>(10.);s.de_stop=std::bit_cast<uint32_t>(.3f);s.de_stop_factor=std::bit_cast<uint32_t>(.02f);s.z_step_div=s.step_limiter=s.de_scale=s.step_width_103=std::bit_cast<uint32_t>(1.f);
 for(unsigned k=0;k<9;++k)s.vgrads[k]=std::bit_cast<uint64_t>(double(int((i*19+k*7)%81)-40)/17.);
 v.params.version=kGeneralNormalVersion;v.params.normals_on_de=i%2;v.params.inside=(i/2)%2;
 for(unsigned k=0;k<3;++k)v.center[k]=float(int((i*17+k*13)%199)-99)/23.f;v.depth=float(i%29)/7.f;v.stop=.01f;
 if(i%31==0)v.center[0]=std::bit_cast<float>(0x7fc00000u);if(i%37==0)v.params.reserved[0]=1;if(i%41==0)s.version=0;if(i%43==0)v.params.version=0;
 }
 run(original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:baseline offset:0 atIndex:1];});
 unsigned active=0,completed=0;auto expected=(Output*)baseline.contents;for(unsigned i=0;i<count;++i){active+=expected[i].trace.count;completed+=expected[i].result.completed;}require(active>3000&&completed>100,"vacuous fixture");
 unsigned comparisons=0;const unsigned quanta[]={1,2,3,7,31,1000,0};for(unsigned repeat=0;repeat<3;++repeat)for(auto quantum:quanta){
 memset(states.contents,0,states.length);memset(candidate.contents,0xa5,candidate.length);
 for(unsigned step=0;step<100;++step){unsigned q=quantum?quantum:(step%4==0?7:step%4==1?1:step%4==2?3:2);
 run(advance,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:states offset:0 atIndex:2];[e setBytes:&q length:4 atIndex:3];});
 bool all=true;auto st=(State*)states.contents;for(unsigned i=0;i<count;++i){all&=st[i].done!=0;if(!st[i].done){auto bytes=(unsigned char*)candidate.contents+i*sizeof(Output);for(unsigned k=0;k<sizeof(Output);++k)require(bytes[k]==0xa5,"pending publication");}}
 if(all)break;require(step!=99,"unfinished");}
 auto actual=(Output*)candidate.contents;for(unsigned i=0;i<count;++i){if(memcmp(expected+i,actual+i,sizeof(Output))){auto old=(uint32_t*)(expected+i),now=(uint32_t*)(actual+i);for(unsigned k=0;k<sizeof(Output)/4;++k)if(old[k]!=now[k])std::cerr<<"case "<<i<<" word "<<k<<" old "<<std::hex<<old[k]<<" new "<<now[k]<<std::dec<<"\n";throw std::runtime_error("outer mismatch");}++comparisons;}
 }
 for(auto item:{std::pair<const char*,id<MTLBuffer>>{"input",inputs},{"control",baseline},{"candidate",candidate}}){std::ofstream f(std::string(argv[2])+"/"+item.first+".bin",std::ios::binary);f.write((const char*)item.second.contents,item.second.length);require(bool(f),"write");}
 // Unsupported smoothing is refused before a scripted child or publication.
 for(unsigned i=0;i<count;++i)in[i].params.smoothing=8;
 memset(states.contents,0,states.length);memset(candidate.contents,0xa5,candidate.length);unsigned q=1;
 run(advance,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:states offset:0 atIndex:2];[e setBytes:&q length:4 atIndex:3];});
 for(unsigned i=0;i<count;++i){const auto& out=((Output*)candidate.contents)[i];require(out.trace.count==0&&!out.result.completed&&out.result.fault==1,"smoothing admission");}
 std::cout<<"{\"scope\":\"synthetic child only\",\"cases\":"<<count<<",\"comparisons\":"<<comparisons<<",\"child_calls_one_schedule\":"<<active<<",\"completed_normals_one_schedule\":"<<completed<<",\"differences\":0}\n";
 return 0;}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 2;}}}
