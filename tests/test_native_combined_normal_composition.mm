#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "src/render/native_metal/continuation_abi.h"
#include <bit>
#include <filesystem>
#include "src/render/native_metal/formula_selector.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <unistd.h>
using namespace mb3d_native;using namespace mb3d_native::continuation;
struct Input {ExactScene scene;NativeFormulaProgram programs[2];NativeNormalParams params;float center[3],depth,stop;};
struct LegacyInput {ExactScene scene;NativeFormulaProgram program;NativeNormalParams params;float center[3],depth,stop;};
struct SampleWitness {uint32_t phase,direct,smooth;float point[3],stop;NativeFormulaState lane;NativeGeometryRecord geometry;NativeNormalRecord normal;float value,result_smooth;};
struct Witness {uint32_t count,overflow;SampleWitness samples[14];};
struct Output {NativeNormalResult result;NativeFormulaState lane;NativeGeometryRecord geometry;NativeNormalRecord record;};
struct State {NormalFrame normal;NormalSampleFrame sample;DistanceEntryFrame entry;CombinedFrame combined;DistanceFrame distance;OrbitReplayFrame replay;OrbitPassFrame orbit;FormulaStepFrame step;WrapperFrame wrapper;BodyFrame body;Output output;uint32_t initialized,done;};
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(int argc,char** argv){@autoreleasepool{try{
 require(argc==7,"usage: probe library output reference prepared-directory repeat legacy-reference");alarm(120);
 const unsigned repeat_index=unsigned(std::stoul(argv[5]));require(repeat_index<3,"repeat bound");
 Input prepared{};auto read=[&](const char* name,void* dst,size_t n){const auto path=std::filesystem::path(argv[4])/name;require(std::filesystem::file_size(path)==n,"prepared ABI");std::ifstream f(path,std::ios::binary);f.read((char*)dst,n);require(bool(f),"prepared read");};
 read("scene.bin",&prepared.scene,sizeof(prepared.scene));read("program.bin",&prepared.programs[0],sizeof(prepared.programs[0]));read("normal.bin",&prepared.params,sizeof(prepared.params));
 require(!prepared.params.smoothing,"unsmoothed normal fixture required");
 if(prepared.programs[0].formula_type)read("second.bin",&prepared.programs[1],sizeof(prepared.programs[1]));

 auto device=MTLCreateSystemDefaultDevice();require(device!=nil,"device");NSError* error=nil;
 auto library=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[1])] error:&error];require(library!=nil,"library");auto queue=[device newCommandQueue];auto legacy=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[6])] error:&error];require(legacy!=nil,"legacy library");
 auto reference=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[3])] error:&error];require(reference!=nil,"reference");
 auto pipeline=[&](id<MTLLibrary> source,NSString* name){auto values=[MTLFunctionConstantValues new];
 for(unsigned k=0;k<6;++k){auto& slot=prepared.programs[0].slots[k];require(ValidFormulaSelectorIdentity(slot.namespace_id,slot.id,slot.enabled),"selector");uint32_t id=FormulaPipelineSelector(slot.namespace_id,slot.id,slot.enabled);[values setConstantValue:&id type:MTLDataTypeUInt atIndex:10+k];}
 for(unsigned k=16;k<21;++k){const uint32_t v=k==17?1:k==16?prepared.programs[0].driver:k==19?prepared.programs[0].formula_type:k==20?prepared.programs[1].driver:0;[values setConstantValue:&v type:MTLDataTypeUInt atIndex:k];}
 NSError* e=nil;auto fn=[source newFunctionWithName:name constantValues:values error:&e];require(fn!=nil,"function");auto p=[device newComputePipelineStateWithFunction:fn error:&e];if(!p)throw std::runtime_error(std::string(source==reference?"reference:":"candidate:")+name.UTF8String+": "+e.localizedDescription.UTF8String);return p;};
 auto abi=pipeline(library,@"abi"),original=pipeline(reference,@"original"),new_original=pipeline(library,@"original"),advance=pipeline(library,@"advance"),observe=pipeline(library,@"observe");
 auto alloc=[&](size_t n){auto b=[device newBufferWithLength:n options:MTLResourceStorageModeShared];require(b!=nil,"allocation");memset(b.contents,0,n);return b;};
 auto run=[&](id<MTLComputePipelineState> p,unsigned n,auto bind){auto c=[queue commandBuffer];auto e=[c computeCommandEncoder];[e setComputePipelineState:p];bind(e);[e dispatchThreads:MTLSizeMake(n,1,1) threadsPerThreadgroup:MTLSizeMake(std::min(n,32u),1,1)];[e endEncoding];[c commit];[c waitUntilCompleted];require(c.status==MTLCommandBufferStatusCompleted,"command failed");};
 auto sizes=alloc(16);run(abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});auto a=(uint32_t*)sizes.contents;require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[2]==sizeof(State)&&a[3]==sizeof(Witness),"ABI");
 static_assert(sizeof(Input)==sizeof(LegacyInput)+sizeof(NativeFormulaProgram));
 auto reference_abi=pipeline(reference,@"abi");run(reference_abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});require(a[0]==sizeof(Input)&&a[1]==sizeof(Output),"frozen control ABI mismatch");
 const unsigned count=16;auto inputs=alloc(count*sizeof(Input)),baseline=alloc(count*sizeof(Output)),candidate=alloc(count*sizeof(Output)),states=alloc(count*sizeof(State));auto witnesses=alloc(count*sizeof(Witness)),observed=alloc(count*sizeof(Witness));auto in=(Input*)inputs.contents;
 for(unsigned i=0;i<count;++i){auto& v=in[i];v=prepared;
 // Deliberately bounded arbitrary points, not scene hits or rendered pixels.
 for(unsigned k=0;k<3;++k)v.center[k]=float(int((i*17+k*13)%199)-99)/23.f;
 v.depth=float(i%29)/7.f;v.stop=.01f;
 }
 run(original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:baseline offset:0 atIndex:1];});
 // The frozen legacy ABI cannot hold two programs. For single programs it
 // independently checks that the larger input ABI did not change the original.
 if(!prepared.programs[0].formula_type){
  auto legacy_abi=pipeline(legacy,@"abi");run(legacy_abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});require(a[0]==sizeof(LegacyInput)&&a[1]==sizeof(Output),"legacy ABI");
  auto legacy_inputs=alloc(count*sizeof(LegacyInput));auto li=(LegacyInput*)legacy_inputs.contents;
  for(unsigned i=0;i<count;++i){li[i].scene=in[i].scene;li[i].program=in[i].programs[0];li[i].params=in[i].params;memcpy(li[i].center,in[i].center,sizeof(li[i].center));li[i].depth=in[i].depth;li[i].stop=in[i].stop;}
  auto legacy_original=pipeline(legacy,@"original");run(legacy_original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:legacy_inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];});require(!memcmp(baseline.contents,candidate.contents,baseline.length),"larger input original differs from frozen legacy");
 }
 unsigned active=0,completed=0,ranges=0,direct_completed=0,de_completed=0;auto expected=(Output*)baseline.contents;for(unsigned i=0;i<count;++i){active+=expected[i].record.sample_calls;completed+=expected[i].result.completed;ranges+=expected[i].geometry.fp32_range_exits;if(in[i].params.normals_on_de)de_completed+=expected[i].result.completed;else direct_completed+=expected[i].result.completed;}require(active>=7&&completed>0,"vacuous prepared-program fixture");
 run(new_original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];});require(memcmp(baseline.contents,candidate.contents,baseline.length)==0,"new original differs from frozen control");
 run(observe,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:observed offset:0 atIndex:4];});
 for(auto item:{std::pair<const char*,id<MTLBuffer>>{"input-observer",inputs},{"control-observer",baseline},{"candidate-observer",candidate},{"observed-observer",observed}}){std::ofstream f(std::string(argv[2])+"/"+item.first+".bin",std::ios::binary);f.write((const char*)item.second.contents,item.second.length);require(bool(f),"observer evidence");}
 require(memcmp(baseline.contents,candidate.contents,baseline.length)==0,"observer differs from frozen control");
 unsigned comparisons=0;const unsigned quanta[]={1,2,3,7,31,1000,0};for(unsigned repeat=repeat_index;repeat==repeat_index;++repeat)for(auto quantum:quanta){
 memset(states.contents,0,states.length);memset(candidate.contents,0xa5,candidate.length);
 for(unsigned step=0;step<8192;++step){unsigned q=quantum?quantum:(step%4==0?7:step%4==1?1:step%4==2?3:2);
 run(advance,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:states offset:0 atIndex:2];[e setBytes:&q length:4 atIndex:3];[e setBuffer:witnesses offset:0 atIndex:4];});
 bool all=true;auto st=(State*)states.contents;for(unsigned i=0;i<count;++i){all&=st[i].done!=0;if(!st[i].done){auto bytes=(unsigned char*)candidate.contents+i*sizeof(Output);for(unsigned k=0;k<sizeof(Output);++k)require(bytes[k]==0xa5,"pending publication");}}
 if(all)break;require(step!=8191,"unfinished");}
 if(quantum==1){for(auto item:{std::pair<const char*,id<MTLBuffer>>{"input-first",inputs},{"control-first",baseline},{"candidate-first",candidate},{"observed-first",observed},{"witness-first",witnesses}}){std::ofstream f(std::string(argv[2])+"/"+item.first+".bin",std::ios::binary);f.write((const char*)item.second.contents,item.second.length);require(bool(f),"failure evidence");}}
 require(memcmp(observed.contents,witnesses.contents,observed.length)==0,"canonical sample boundaries differ");
 auto actual=(Output*)candidate.contents;for(unsigned i=0;i<count;++i){if(memcmp(expected+i,actual+i,sizeof(Output))){auto old=(uint32_t*)(expected+i),now=(uint32_t*)(actual+i);for(unsigned k=0;k<sizeof(Output)/4;++k)if(old[k]!=now[k])std::cerr<<"case "<<i<<" word "<<k<<" old "<<std::hex<<old[k]<<" new "<<now[k]<<std::dec<<"\n";throw std::runtime_error("outer mismatch");}++comparisons;}
 }
 for(auto item:{std::pair<const char*,id<MTLBuffer>>{"input",inputs},{"control",baseline},{"candidate",candidate}}){std::ofstream f(std::string(argv[2])+"/"+item.first+".bin",std::ios::binary);f.write((const char*)item.second.contents,item.second.length);require(bool(f),"write");}
 // Unsupported smoothing is refused before a scripted child or publication.
 for(unsigned i=0;i<count;++i)in[i].params.smoothing=8;
 memset(states.contents,0,states.length);memset(candidate.contents,0xa5,candidate.length);unsigned q=1;
 run(advance,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:states offset:0 atIndex:2];[e setBytes:&q length:4 atIndex:3];[e setBuffer:witnesses offset:0 atIndex:4];});
 for(unsigned i=0;i<count;++i){const auto& out=((Output*)candidate.contents)[i];require(out.record.sample_calls==0&&!out.result.completed&&out.result.fault==1,"smoothing admission");}
 std::cout<<"{\"scope\":\"two-program ABI normal composition\",\"cases\":"<<count<<",\"comparisons\":"<<comparisons<<",\"child_calls_one_schedule\":"<<active<<",\"completed_normals_one_schedule\":"<<completed<<",\"direct_completed_one_schedule\":"<<direct_completed<<",\"de_completed_one_schedule\":"<<de_completed<<",\"nonzero_gradient\":"<<1u<<",\"range_exits\":"<<ranges<<",\"repeat\":"<<repeat_index<<",\"differences\":0}\n";
 return 0;}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 2;}}}
