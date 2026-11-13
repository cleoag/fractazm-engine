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
#include <zlib.h>
using namespace mb3d_native;
struct Input {float point[3],params[64];uint32_t id,local_limit,limit,work;NativeFormulaState lane;uint32_t pass_limit,has_radius;float stop,radius_stop;uint32_t inside_mode,observe_mask;};
struct Output {NativeOrbitResult result;NativeFormulaState lane;NativeSmoothingCounters smoothing;uint32_t carried,range,clamps;};
static constexpr uint32_t ProbeMagic=0x50324001;
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
uint32_t seed=0x3509f09a;
float random_float(){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return float(seed&0xffffff)/float(0xffffff);}
void save(const std::string& path,id<MTLBuffer> buffer){
 uLongf size=compressBound(buffer.length);std::vector<Bytef> data(size);
 require(compress2(data.data(),&size,(const Bytef*)buffer.contents,buffer.length,Z_BEST_SPEED)==Z_OK,"compress evidence");
 std::ofstream out(path+".zlib",std::ios::binary);out.write((const char*)data.data(),size);require(bool(out),"write compressed buffer");
}
int main(int argc,char** argv){@autoreleasepool{try{
 require(argc>=3&&argc<=8,"usage: probe library output-directory [frozen-control-library] [case-count] [core|gnarly] [driver]");alarm(120);
 auto device=MTLCreateSystemDefaultDevice();require(device!=nil,"Metal unavailable");NSError* error=nil;
 auto library=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[1])] error:&error];
 if(!library){std::cerr<<error.localizedDescription.UTF8String<<"\n";return 2;}
 id<MTLLibrary> reference=library;
 if(argc>=4){reference=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[3])] error:&error];require(reference!=nil,"frozen control library unavailable");}
 auto queue=[device newCommandQueue];require(queue!=nil,"queue allocation");
 const uint32_t driver=argc>=7?unsigned(std::stoul(argv[6])):0;require(driver<10,"invalid driver");
 const bool gnarly=argc>=6&&std::string(argv[5])=="gnarly";
 const uint32_t smoothing_policy=argc>=8?unsigned(std::stoul(argv[7])):0;require(smoothing_policy<=1,"invalid smoothing policy");
 auto pipeline=[&](id<MTLLibrary> source,NSString* name){MTLFunctionConstantValues* values=[MTLFunctionConstantValues new];
 const uint32_t ids[]={gnarly?10089u:10274u,gnarly?10089u:10265u,gnarly?10089u:10220u,gnarly?10089u:10321u,10089u,0x80000002};
 for(unsigned i=0;i<6;++i)[values setConstantValue:&ids[i] type:MTLDataTypeUInt atIndex:10+i];
 uint32_t policy=0;[values setConstantValue:&driver type:MTLDataTypeUInt atIndex:16];[values setConstantValue:&smoothing_policy type:MTLDataTypeUInt atIndex:17];[values setConstantValue:&policy type:MTLDataTypeUInt atIndex:18];
 NSError* specializationError=nil;auto function=[source newFunctionWithName:name constantValues:values error:&specializationError];require(function!=nil,"missing function");NSError* e=nil;auto p=[device newComputePipelineStateWithFunction:function error:&e];if(!p)throw std::runtime_error(e.localizedDescription.UTF8String);return p;};
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
 require(words[0]==ProbeMagic&&words[1]==sizeof(Input)&&words[2]==sizeof(Output)&&words[3]>=16&&words[3]<4096,"ABI mismatch");
 const uint32_t checkpoint_bytes=words[3];
 auto reference_abi=pipeline(reference,@"p2_abi");
 dispatch(reference_abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:abi_buffer offset:0 atIndex:0];});
 require(words[0]==ProbeMagic&&words[1]==sizeof(Input)&&words[2]==sizeof(Output),"frozen control ABI mismatch");

 const unsigned count=argc>=5?unsigned(std::stoul(argv[4])):1024;require(count>=1024&&count<=8192&&count%4==0,"invalid case count");auto input=alloc(count*sizeof(Input)),slots=alloc(count*sizeof(NativeFormulaProgram)),baseline=alloc(count*sizeof(Output)),candidate=alloc(count*sizeof(Output)),state=alloc(count*checkpoint_bytes),status=alloc(count*4),trace=alloc(size_t(count)*128*128*4);
 auto* ins=(Input*)input.contents;auto* programs=(NativeFormulaProgram*)slots.contents;std::vector<NativeFormulaSlot> descriptors(count);auto* desc=descriptors.data();
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
  auto& d=desc[i];for(unsigned j=0;j<64;++j)d.params[j]=v.params[j];
  const int options[]={-1,2,4,5,6,11,20,21};d.de_option=options[k%8];d.difs_style=(k/8)%2;d.wrapper_flags=(k/16)%2;
  const unsigned traits[]={0,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16383};d.traits=traits[(k/32)%15];
  auto& lane=v.lane;for(unsigned j=0;j<3;++j)lane.xyz[j]=v.point[j];lane.xyz[3]=1.2f;
  for(unsigned j=0;j<4;++j){lane.c[j]=.1f*(j+1);lane.j[j]=-.2f*(j+1);lane.original_j[j]=.3f*(j+1);lane.derivative[j]=1.1f*(j+1);lane.sse[j]=.7f*(j+1);}
  lane.vary_scale=1.1f;lane.first=k%3==0?16777217:k%3==1?-1:2147483647;lane.iteration=i%3;lane.rout=1.3f;lane.rold=-.25f;lane.rstop_d=4.f;lane.dfree1=.2f;lane.dfree2=.3f;lane.julia=k%2;
  if(k%139==0)d.version=0;
  if(k%149==0)d.namespace_id=0;
  if(k%151==0)v.local_limit=(k%2)?0:(1u<<20)+1;
  if(k%127==0)d.param_count=59; // Original ABI rejection, no child entered.
  if(k%131==0){d.param_count=58;d.traits=4096;} // Append overflow.

 }
 for(unsigned i=0;i<count;++i){
  auto& v=ins[i];auto& p=programs[i];auto& lane=v.lane;
  p.version=kFormulaAbiVersion;p.driver=driver;p.slot_count=6;p.start_slot=i%6;p.repeat_slot=p.start_slot;p.end_slot=p.start_slot;p.local_work_limit=16;
  p.work_limit=i%31==0?1:64;p.max_iterations=3;p.radius_stop=16.f;p.gradient_radius_stop=16.f;p.log_radius_stop=1.4f;
  p.de_scale=.001f;p.de_offset=i%17==0?1e10f:.001f;p.de_offset_floor=i%13==0?-1e30f:.0001f;p.stop_scale=1.f;
  p.de_option=int((i%3)*16)|(i%8==0?4:0);
  for(unsigned j=0;j<6;++j){
   auto& d=p.slots[j];unsigned at=(i/4)*4+j%4;d=desc[at];
   d.id=gnarly?10089u:j==0?10274u:j==1?10265u:j==2?10220u:j==3?10321u:10089u;
   d.version=kFormulaAbiVersion;d.namespace_id=ExternalFormula;d.enabled=1;d.param_count=d.id==10274?13:d.id==10265?19:d.id==10220?14:d.id==10089?13:12;
   if(d.id==10089){for(float& a:d.params)a=0;d.params[0]=1.3f;d.params[1]=.7f;d.params[5]=.2f;d.params[6]=i%2;d.params[7]=i%9;d.params[9]=1.1f;d.params[10]=.05f;d.params[11]=.7f;d.params[60]=1.1f;}
   if(j==5){d.namespace_id=InternalFormula;d.id=2;d.negative_params[1]=1;d.negative_params[2]=.25f;}
   p.smoothing_power[j]=1.1f;lane.hybrid_counts[j]=1;
  }
  if(i%73==0)p.version=0;
  if(i%79==0)p.slots[0].param_count=59;
  if(i%83==0)p.slots[0].id=10265;
  if(i%89==0)p.de_offset=1e-40f;
  for(unsigned j=0;j<4;++j)for(unsigned k=0;k<4;++k)p.rotation[j][k]=j==k?1.f:.03f*float(j+k);
  if(driver>=4&&driver<=7){float a=.3f,b=.7f;memcpy(&lane.hybrid_counts[0],&a,4);memcpy(&lane.hybrid_counts[1],&b,4);}
  lane.max_iterations=1+i%3;lane.calc_smooth=i%2;lane.inside=i%5?0:0xffffffffu;lane.tmp=i%3;
  for(unsigned j=0;j<6;++j)p.hybrid_counts[j]=lane.hybrid_counts[j];
  v.stop=i%97==0?0.f:.01f;v.inside_mode=i%3;v.radius_stop=i%7==0?8.f:0.f;v.observe_mask=i%16;
  if(i%101==0)lane.calc_smooth=2;
  if(i%37==0)v.point[0]=1e10f;
 }
 const std::string dir=argv[2];save(dir+"/input.bin",input);save(dir+"/slots.bin",slots);
 auto bind_common=[&](id<MTLComputeCommandEncoder> e,id<MTLBuffer> out){[e setBuffer:input offset:0 atIndex:0];[e setBuffer:out offset:0 atIndex:1];[e setBuffer:slots offset:0 atIndex:2];};
 dispatch(original,count,[&](id<MTLComputeCommandEncoder> e){bind_common(e,baseline);});save(dir+"/original.bin",baseline);
 dispatch(new_original,count,[&](id<MTLComputeCommandEncoder> e){bind_common(e,candidate);});save(dir+"/new-original.bin",candidate);
 require(!memcmp(baseline.contents,candidate.contents,baseline.length),"new original differs from frozen original");
 dispatch(observer,count,[&](id<MTLComputeCommandEncoder> e){bind_common(e,candidate);[e setBuffer:trace offset:0 atIndex:6];});
 save(dir+"/observer.bin",candidate);save(dir+"/canonical.bin",trace);require(!memcmp(baseline.contents,candidate.contents,baseline.length),"old observer changed final output");
 std::vector<uint8_t> canonical((uint8_t*)trace.contents,(uint8_t*)trace.contents+trace.length);save(dir+"/canonical.bin",trace);
 uint64_t differences=0,pending_observations=0,without_yield_differences=0;std::vector<uint8_t> first_schedule;std::array<unsigned,7> budgets={1,2,3,7,31,1048576,0};
 for(unsigned repetition=0;repetition<3;++repetition)for(unsigned schedule=0;schedule<budgets.size();++schedule){
  memset(state.contents,0,state.length);memset(trace.contents,0,trace.length);memset(candidate.contents,0xa5,candidate.length);unsigned sequence=0;std::vector<unsigned> finished_at(count,0);
  for(;;){require(sequence<2048,"nonprogressing checkpoint");unsigned quantum=budgets[schedule];if(!quantum){const unsigned irregular[]={1,7,2,31,3};quantum=irregular[sequence%5];}
   const uint32_t control[]={ProbeMagic,42,quantum,sequence};
   dispatch(advance,count,[&](id<MTLComputeCommandEncoder> e){bind_common(e,candidate);[e setBuffer:state offset:0 atIndex:3];[e setBuffer:status offset:0 atIndex:4];[e setBuffer:trace offset:0 atIndex:6];[e setBytes:control length:16 atIndex:5];});
   bool done=true;auto* progress=(uint32_t*)status.contents;
   for(unsigned i=0;i<count;++i){require(progress[i]<0x80000000,"invalid device checkpoint");if(progress[i]==99&&!finished_at[i])finished_at[i]=sequence+1;if(progress[i]!=99){done=false;++pending_observations;const auto* data=(const uint8_t*)candidate.contents+i*sizeof(Output);for(unsigned j=0;j<sizeof(Output);++j)require(data[j]==0xa5,"pending wrote output");}}
   ++sequence;if(done)break;
   if(sequence==1){ // Malformed/stale resumes must not mutate any checkpoint.
    std::vector<uint8_t> before((uint8_t*)state.contents,(uint8_t*)state.contents+state.length);
    std::vector<uint8_t> before_output((uint8_t*)candidate.contents,(uint8_t*)candidate.contents+candidate.length);
    for(auto bad:std::array<std::array<uint32_t,4>,6>{{{0,42,1,sequence},{ProbeMagic,43,1,sequence},{ProbeMagic,42,0,sequence},{ProbeMagic,42,1,sequence+1},{ProbeMagic,42,(1u<<20)+1,sequence},{ProbeMagic,42,1,sequence-1}}}){
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
