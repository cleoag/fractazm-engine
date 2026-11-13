#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "render/native_metal/formula_abi.h"
#include <vector>
#include <array>
#include <cstring>
#include <iostream>
#include <fstream>
#include <chrono>
#include <stdexcept>
#include <unistd.h>
using namespace mb3d_native;
struct Input {float point[3],params[64];uint32_t id,local_limit,limit,work;};
struct Output {float value[3],de,params[64];int32_t counter;uint32_t written;NativeFormulaFault fault;};
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
uint32_t seed=0x3509f09a;
float random_float(){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return float(seed&0xffffff)/float(0xffffff);}
void save(const std::string& path,id<MTLBuffer> buffer){std::ofstream out(path,std::ios::binary);out.write((const char*)buffer.contents,buffer.length);require(bool(out),"write buffer");}
int main(int argc,char** argv){@autoreleasepool{try{
 require(argc>=3&&argc<=6,"usage: probe library output-directory [frozen-control-library] [case-count] [core|gnarly]");alarm(120);
 auto device=MTLCreateSystemDefaultDevice();require(device!=nil,"Metal unavailable");NSError* error=nil;
 auto library=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[1])] error:&error];
 if(!library){std::cerr<<error.localizedDescription.UTF8String<<"\n";return 2;}
 id<MTLLibrary> reference=library;
 if(argc>=4){reference=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[3])] error:&error];require(reference!=nil,"frozen control library unavailable");}
 auto queue=[device newCommandQueue];require(queue!=nil,"queue allocation");
 auto pipeline=[&](id<MTLLibrary> source,NSString* name){auto function=[source newFunctionWithName:name];require(function!=nil,"missing function");NSError* e=nil;auto p=[device newComputePipelineStateWithFunction:function error:&e];if(!p)throw std::runtime_error(e.localizedDescription.UTF8String);return p;};
 auto setup=std::chrono::steady_clock::now();auto abi=pipeline(library,@"p2_abi"),original=pipeline(reference,@"p2_original"),new_original=pipeline(library,@"p2_original"),advance=pipeline(library,@"p2_advance"),observer=pipeline(library,@"p2_observer");
 double setup_seconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-setup).count();
 auto alloc=[&](size_t size){auto b=[device newBufferWithLength:size options:MTLResourceStorageModeShared];require(b!=nil,"buffer allocation");memset(b.contents,0,size);return b;};
 double total_gpu=0,max_gpu=0;uint64_t dispatches=0;
 auto dispatch=[&](id<MTLComputePipelineState> pipe,unsigned count,auto bind){
  auto command=[queue commandBuffer];require(command!=nil,"command allocation");auto enc=[command computeCommandEncoder];require(enc!=nil,"encoder allocation");[enc setComputePipelineState:pipe];bind(enc);
  [enc dispatchThreads:MTLSizeMake(count,1,1) threadsPerThreadgroup:MTLSizeMake(std::min(count,32u),1,1)];[enc endEncoding];[command commit];[command waitUntilCompleted];
  if(command.status!=MTLCommandBufferStatusCompleted)throw std::runtime_error(command.error.localizedDescription.UTF8String);
  auto duration=command.GPUEndTime-command.GPUStartTime;total_gpu+=duration;max_gpu=std::max(max_gpu,duration);++dispatches;
 };
 auto abi_buffer=alloc(16);dispatch(abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:abi_buffer offset:0 atIndex:0];});
 auto words=(uint32_t*)abi_buffer.contents;
 require(words[0]==0x50320004&&words[1]==sizeof(Input)&&words[2]==sizeof(Output)&&words[3]>=16&&words[3]<4096,"ABI mismatch");
 const uint32_t checkpoint_bytes=words[3];
 auto reference_abi=pipeline(reference,@"p2_abi");
 dispatch(reference_abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:abi_buffer offset:0 atIndex:0];});
 require((words[0]>=0x50320001&&words[0]<=0x50320004)&&words[1]==sizeof(Input)&&words[2]==sizeof(Output),"frozen control ABI mismatch");
 const bool gnarly=argc==6&&std::string(argv[5])=="gnarly";
 const unsigned count=argc>=5?unsigned(std::stoul(argv[4])):1024;require(count>=1024&&count<=8192&&count%4==0,"invalid case count");auto input=alloc(count*sizeof(Input)),slots=alloc(count*sizeof(NativeFormulaSlot)),baseline=alloc(count*sizeof(Output)),candidate=alloc(count*sizeof(Output)),state=alloc(count*checkpoint_bytes),status=alloc(count*4),trace=alloc(size_t(count)*128*128*4);
 auto* ins=(Input*)input.contents;auto* desc=(NativeFormulaSlot*)slots.contents;
 for(unsigned i=0;i<count;++i){const unsigned k=i/4;auto& v=ins[i];v.id=i%4==0?10274:i%4==1?10265:i%4==2?10220:10321;v.local_limit=(i%7==0)?1:64;v.limit=(i%11==0)?1:128;v.work=i%3;
  for(float& x:v.point)x=(random_float()-.5f)*(v.id==10274?300.f:2.f);
  if(v.id==10274){for(unsigned j=0;j<3;++j)v.params[j]=random_float()-.5f;v.params[3]=.8f;v.params[4]=1.1f;v.params[5]=1.3f;v.params[6]=float(k%4);v.params[7]=32;v.params[8]=1.3f;v.params[9]=1.1f;v.params[10]=2;v.params[11]=3;v.params[12]=.001f;}
  else if(v.id==10265){v.params[0]=.7f;v.params[1]=.1f;v.params[2]=.2f;v.params[3]=.3f;v.params[4]=.4f;v.params[5]=.312f;v.params[6]=0;v.params[7]=int(i%3)-1;v.params[8]=float(k%17?1+k%8:0);v.params[9]=0;v.params[10]=int(i%5)-2;v.params[11]=3+i%9;v.params[12]=.81649658f;v.params[13]=.577350269f;v.params[14]=.70710678f;v.params[15]=3.14159265f;v.params[16]=.2f;v.params[17]=-.3f;v.params[18]=.4f;v.params[59]=float(i%3);}
  else if(v.id==10220){v.params[0]=i%2?.3f:-.3f;v.params[1]=.7f;v.params[2]=.9f;v.params[3]=1.3f;v.params[4]=.11f;v.params[5]=-.21f;v.params[6]=.31f;v.params[7]=i%5?-.3f:.3f;v.params[8]=2+i%9;v.params[9]=.27f;v.params[10]=3.f;v.params[11]=.18f;v.params[12]=k%2;v.params[13]=.6f;v.params[60]=1.1f;}
  else {for(unsigned j=0;j<8;++j)v.params[j]=float(int(random_float()*9)-4);for(unsigned j=8;j<12;++j)v.params[j]=random_float()-.5f;}
  if(i>=1024){
   // Extra held-out values, preserving finite, terminating original controls.
   if(v.id==10265){v.params[0]=.1f+random_float()*.8f;v.params[1]=(random_float()-.5f)*.4f;
    v.params[5]=(random_float()-.5f)*6.28f;v.params[11]=1.f+float(k%127);
    for(unsigned j=2;j<=4;++j)v.params[j]=(random_float()-.5f)*2.f;
    for(unsigned j=16;j<=18;++j)v.params[j]=(random_float()-.5f)*2.f;
   }
   if(k%31==0){v.point[0]=-0.f;v.point[1]=0.f;v.point[2]=1.f;}
   if(v.id==10274){v.local_limit=k%4?1u+k%32:0;v.limit=k%13?128:0;}
  }
  if(gnarly){v.id=10089;for(float& a:v.params)a=0;v.params[0]=.1f+random_float()*2;
   v.params[1]=.2f+random_float()*.8f;for(unsigned j=2;j<=4;++j)v.params[j]=random_float()-.5f;
   v.params[5]=random_float()*2.f-.5f;v.params[6]=k%2;v.params[7]=int(i%97)-2;
   v.params[8]=i%2;v.params[9]=random_float()*2;v.params[10]=random_float()*.3f;v.params[11]=random_float()*2;
   v.params[12]=random_float()*2-1;v.params[58]=i%3?-.25f:1.f;v.params[60]=1.1f;
  }
  desc[i].id=v.id;desc[i].version=kFormulaAbiVersion;desc[i].namespace_id=ExternalFormula;desc[i].param_count=v.id==10274?13:v.id==10265?19:v.id==10220?14:v.id==10089?13:12;
 }
 const std::string dir=argv[2];save(dir+"/input.bin",input);save(dir+"/slots.bin",slots);
 auto bind_common=[&](id<MTLComputeCommandEncoder> e,id<MTLBuffer> out){[e setBuffer:input offset:0 atIndex:0];[e setBuffer:out offset:0 atIndex:1];[e setBuffer:slots offset:0 atIndex:2];};
 dispatch(original,count,[&](id<MTLComputeCommandEncoder> e){bind_common(e,baseline);});save(dir+"/original.bin",baseline);
 dispatch(new_original,count,[&](id<MTLComputeCommandEncoder> e){bind_common(e,candidate);});save(dir+"/new-original.bin",candidate);
 require(!memcmp(baseline.contents,candidate.contents,baseline.length),"new original differs from frozen original");
 dispatch(observer,count,[&](id<MTLComputeCommandEncoder> e){bind_common(e,candidate);[e setBuffer:trace offset:0 atIndex:6];});
 save(dir+"/observer.bin",candidate);require(!memcmp(baseline.contents,candidate.contents,baseline.length),"old observer changed final output");
 std::vector<uint8_t> canonical((uint8_t*)trace.contents,(uint8_t*)trace.contents+trace.length);save(dir+"/canonical.bin",trace);
 uint64_t differences=0,pending_observations=0,without_yield_differences=0;std::vector<uint8_t> first_schedule;std::array<unsigned,7> budgets={1,2,3,7,31,1048576,0};
 for(unsigned repetition=0;repetition<3;++repetition)for(unsigned schedule=0;schedule<budgets.size();++schedule){
  memset(state.contents,0,state.length);memset(trace.contents,0,trace.length);memset(candidate.contents,0xa5,candidate.length);unsigned sequence=0;std::vector<unsigned> finished_at(count,0);
  for(;;){require(sequence<2048,"nonprogressing checkpoint");unsigned quantum=budgets[schedule];if(!quantum){const unsigned irregular[]={1,7,2,31,3};quantum=irregular[sequence%5];}
   const uint32_t control[]={0x50320004,42,quantum,sequence};
   dispatch(advance,count,[&](id<MTLComputeCommandEncoder> e){bind_common(e,candidate);[e setBuffer:state offset:0 atIndex:3];[e setBuffer:status offset:0 atIndex:4];[e setBuffer:trace offset:0 atIndex:6];[e setBytes:control length:16 atIndex:5];});
   bool done=true;auto* progress=(uint32_t*)status.contents;
   for(unsigned i=0;i<count;++i){require(progress[i]<0x80000000,"invalid device checkpoint");if(progress[i]==99&&!finished_at[i])finished_at[i]=sequence+1;if(progress[i]!=99){done=false;++pending_observations;const auto* data=(const uint8_t*)candidate.contents+i*sizeof(Output);for(unsigned j=0;j<sizeof(Output);++j)require(data[j]==0xa5,"pending wrote output");}}
   ++sequence;if(done)break;
   if(sequence==1){ // Malformed/stale resumes must not mutate any checkpoint.
    std::vector<uint8_t> before((uint8_t*)state.contents,(uint8_t*)state.contents+state.length);
    std::vector<uint8_t> before_output((uint8_t*)candidate.contents,(uint8_t*)candidate.contents+candidate.length);
    for(auto bad:std::array<std::array<uint32_t,4>,6>{{{0,42,1,sequence},{0x50320004,43,1,sequence},{0x50320004,42,0,sequence},{0x50320004,42,1,sequence+1},{0x50320004,42,(1u<<20)+1,sequence},{0x50320004,42,1,sequence-1}}}){
     dispatch(advance,count,[&](id<MTLComputeCommandEncoder> e){bind_common(e,candidate);[e setBuffer:state offset:0 atIndex:3];[e setBuffer:status offset:0 atIndex:4];[e setBuffer:trace offset:0 atIndex:6];[e setBytes:bad.data() length:16 atIndex:5];});
     require(!memcmp(before.data(),state.contents,state.length),"invalid resume mutated state");
     require(!memcmp(before_output.data(),candidate.contents,candidate.length),"invalid resume published output");
     for(unsigned i=0;i<count;++i)require(((uint32_t*)status.contents)[i]>=0x80000000,"invalid resume was accepted");
    }
   }
  }
  std::string tag="r"+std::to_string(repetition)+"-q"+std::to_string(budgets[schedule]);save(dir+"/"+tag+".bin",candidate);
  for(unsigned i=0;i<count;++i){if(memcmp((Output*)baseline.contents+i,(Output*)candidate.contents+i,sizeof(Output))){++differences;if(finished_at[i]==1)++without_yield_differences;if(differences<=8){auto* a=(uint32_t*)((Output*)baseline.contents+i);auto* b=(uint32_t*)((Output*)candidate.contents+i);for(unsigned w=0;w<sizeof(Output)/4;++w)if(a[w]!=b[w]){std::cerr<<"difference "<<tag<<" case="<<i<<" formula="<<ins[i].id<<" finished_at="<<finished_at[i]<<" word="<<w<<" old="<<std::hex<<a[w]<<" new="<<b[w]<<std::dec<<"\n";break;}}}}
  if(repetition==0&&schedule==0){
   first_schedule.assign((uint8_t*)candidate.contents,(uint8_t*)candidate.contents+candidate.length);

  }
  if(memcmp(canonical.data(),trace.contents,trace.length)){save(dir+"/candidate-canonical.bin",trace);throw std::runtime_error("canonical live state differs from original observer");}
  require(!memcmp(first_schedule.data(),candidate.contents,candidate.length),"schedule/repeat mismatch");
 }
 std::cout<<"{\"cases\":"<<count<<",\"schedules\":7,\"repeats\":3,\"comparisons\":"<<count*21<<",\"differences\":"<<differences<<",\"without_yield_differences\":"<<without_yield_differences<<",\"pending_observations\":"<<pending_observations<<",\"dispatches\":"<<dispatches<<",\"gpu_seconds\":"<<total_gpu<<",\"max_command_seconds\":"<<max_gpu<<",\"pipeline_setup_seconds\":"<<setup_seconds<<",\"checkpoint_bytes\":"<<checkpoint_bytes<<"}\n";
 return differences?1:0;
 }catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 2;}}}
