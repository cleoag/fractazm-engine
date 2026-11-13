#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "native_refinement_probe_abi.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <unistd.h>
using namespace p2_refinement;
static void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
int main(int argc,char** argv){@autoreleasepool{try{
 require(argc==4,"usage: probe library output-directory reference-library");alarm(120);auto device=MTLCreateSystemDefaultDevice();require(device!=nil,"device");NSError* error=nil;
 auto library=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[1])] error:&error];require(library!=nil,"library");auto reference=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[3])] error:&error];require(reference!=nil,"reference");auto queue=[device newCommandQueue];
 auto pipeline=[&](id<MTLLibrary> source,NSString* name){NSError* e=nil;auto fn=[source newFunctionWithName:name];require(fn!=nil,"function");auto p=[device newComputePipelineStateWithFunction:fn error:&e];if(!p)throw std::runtime_error(std::string(source==reference?"reference:":"candidate:")+name.UTF8String+": "+e.localizedDescription.UTF8String);return p;};
 auto abi=pipeline(library,@"abi"),reference_abi=pipeline(reference,@"abi"),original=pipeline(reference,@"original"),new_original=pipeline(library,@"original"),advance=pipeline(library,@"advance");
 auto alloc=[&](size_t n){auto b=[device newBufferWithLength:n options:MTLResourceStorageModeShared];require(b!=nil,"allocation");memset(b.contents,0,n);return b;};
 auto run=[&](id<MTLComputePipelineState> p,unsigned n,auto bind){auto c=[queue commandBuffer];auto e=[c computeCommandEncoder];[e setComputePipelineState:p];bind(e);[e dispatchThreads:MTLSizeMake(n,1,1) threadsPerThreadgroup:MTLSizeMake(std::min(n,32u),1,1)];[e endEncoding];[c commit];[c waitUntilCompleted];require(c.status==MTLCommandBufferStatusCompleted,"command failed");};
 auto sizes=alloc(12);run(abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});auto a=(uint32_t*)sizes.contents;require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[2]==sizeof(State),"ABI");
 run(reference_abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});require(a[0]==sizeof(Input)&&a[1]==sizeof(Output),"frozen control ABI");
 const unsigned count=1024;auto inputs=alloc(count*sizeof(Input)),baseline=alloc(count*sizeof(Output)),candidate=alloc(count*sizeof(Output)),states=alloc(count*sizeof(State));auto in=(Input*)inputs.contents;
 for(unsigned i=0;i<count;++i){auto& v=in[i];v.program.max_iterations=4;v.program.de_option=i%2?20:0;v.steps=i%7;v.iteration=77+i%13;v.initial_refine=i%5;v.initial_de=i%9;v.initial_work=i%17;v.depth=float(i%31)/7.f;v.stop=.01f;
 for(unsigned k=0;k<3;++k){v.point[k]=float(int((i*17+k*13)%199)-99)/23.f;v.ray[k]=float(int((i*7+k*5)%19)-9)/11.f;}
 for(unsigned j=0;j<Capacity;++j){float value=0;
 switch(i%8){case 0:value=100.f;break;case 1:value=2.f;break;case 2:value=j%2?std::nextafter(2.f,3.f):2.f;break;case 3:value=j%2?std::nextafter(3.5f,4.f):3.5f;break;case 4:{const float t[]={3.f,2.f,4.f,3.05f};value=t[j%4];break;}case 5:{const float t[]={5.5f,2.5f,6.f,1.25f};value=t[j%4];break;}case 6:{const float t[]={-3.f,6.f,2.75f,3.125f};value=t[j%4];break;}default:value=float(int((i+j*7)%23)-6)/4.f;}
 v.smooth[j]=value;v.iterations[j]=(i+j)%7;}
 if(i%31==0)v.fault_at=1+i%12;if(i%37==0)v.initial_fault=16;if(i%41==0)v.steps=256;if(i%43==0)v.steps=0xffffffffu;if(i%47==0){v.steps=255;v.fault_at=1;v.initial_fault=0;}
 if(i%53==0)v.ray[1]=std::bit_cast<float>(0x7fc00000u);if(i%59==0)v.depth=std::numeric_limits<float>::infinity();if(i%61==0)v.smooth[1]=std::bit_cast<float>(0x7fc00000u);
 }
 run(original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:baseline offset:0 atIndex:1];});
 unsigned queries=0,completed=0,guards=0,child_faults=0,finite_faults=0,max_steps=0;auto expected=(Output*)baseline.contents;
 for(unsigned i=0;i<count;++i){const auto& v=in[i];const auto& out=expected[i];const auto& t=out.trace;require(!t.overflow&&t.count<=Capacity,"tape capacity");queries+=t.count;completed+=out.success;child_faults+=out.geometry.fault==32;finite_faults+=out.geometry.fault==4;
 require(out.success==uint32_t(!out.geometry.fault),"terminal fault");
 if(v.steps>255){++guards;require(t.count==0&&out.geometry.fault==1&&out.iteration==v.iteration&&!memcmp(out.point,v.point,sizeof(v.point))&&!memcmp(&out.depth,&v.depth,sizeof(float)),"refinement limit guard");}
 if(v.steps==255){++max_steps;require(t.count==1&&out.geometry.fault==32,"maximum refinement admission");}
 require(out.geometry.de_calls==v.initial_de+t.count,"DE count");require(out.geometry.refine_calls==v.initial_refine+(v.steps>255?0:v.initial_fault?1:t.count),"refine count");
 uint32_t work=v.initial_work,iteration=v.iteration;NativeFormulaState prior{};prior.tmp=17;prior.xyz[3]=.375f;
 for(unsigned j=0;j<t.count;++j){const auto& q=t.queries[j];require(q.maximum==v.program.max_iterations+(v.program.de_option!=20?1:0)&&q.smooth==1&&q.temporary==1,"temporary maximum contract");require(!memcmp(&q.before,&prior,sizeof(prior)),"carried lane");prior=q.after;work+=1+j%3;if(!q.after_record.fault)iteration=uint32_t(q.result.state.iteration);}
 require(!memcmp(&out.lane,&prior,sizeof(prior))&&out.geometry.formula_work==work&&out.iteration==iteration,"final state/work/iteration");
 }
 require(queries>1000&&completed>300&&guards>10&&child_faults>10&&finite_faults>10&&max_steps>10,"vacuous fixture");
 run(new_original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];});require(!memcmp(baseline.contents,candidate.contents,baseline.length),"new original differs from frozen control");
 auto save=[&](const char* name,id<MTLBuffer> b){std::ofstream f(std::string(argv[2])+"/"+name+".bin",std::ios::binary);f.write((const char*)b.contents,b.length);require(bool(f),"evidence write");};save("input",inputs);save("control",baseline);
 unsigned comparisons=0;const unsigned quanta[]={1,2,3,7,31,1000,0};for(unsigned repeat=0;repeat<3;++repeat)for(auto quantum:quanta){memset(states.contents,0,states.length);memset(candidate.contents,0xa5,candidate.length);
 for(unsigned step=0;step<256;++step){unsigned q=quantum?quantum:(step%4==0?7:step%4==1?1:step%4==2?3:2);
 run(advance,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:states offset:0 atIndex:2];[e setBytes:&q length:4 atIndex:3];});
 bool all=true;auto st=(State*)states.contents;for(unsigned i=0;i<count;++i){all&=st[i].done!=0;if(!st[i].done){auto bytes=(unsigned char*)candidate.contents+i*sizeof(Output);for(unsigned k=0;k<sizeof(Output);++k)require(bytes[k]==0xa5,"pending publication");}}
 if(all)break;require(step!=255,"unfinished");}
 if(!repeat&&quantum==1)save("candidate-first",candidate);
 require(!memcmp(baseline.contents,candidate.contents,baseline.length),"refinement state or boundary mismatch");comparisons+=count;
 }
 save("candidate",candidate);
 std::cout<<"{\"scope\":\"synthetic iteration-refinement children\",\"cases\":"<<count<<",\"comparisons\":"<<comparisons<<",\"queries_one_schedule\":"<<queries<<",\"completed_one_schedule\":"<<completed<<",\"limit_guards\":"<<guards<<",\"maximum_steps_admitted\":"<<max_steps<<",\"child_faults\":"<<child_faults<<",\"finite_faults\":"<<finite_faults<<",\"differences\":0}\n";
 return 0;}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 2;}}}
