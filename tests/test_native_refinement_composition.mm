#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "native_refinement_composition_abi.h"
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
 require(argc>=4&&argc<=5,"usage: probe library output-directory reference-library [smoothing-policy]");alarm(120);auto device=MTLCreateSystemDefaultDevice();require(device!=nil,"device");NSError* error=nil;
 auto library=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[1])] error:&error];require(library!=nil,"library");auto reference=[device newLibraryWithURL:[NSURL fileURLWithPath:@(argv[3])] error:&error];require(reference!=nil,"reference");auto queue=[device newCommandQueue];
 const uint32_t policy=argc==5?unsigned(std::stoul(argv[4])):1;require(policy<=1,"smoothing policy");
 auto pipeline=[&](id<MTLLibrary> source,NSString* name){NSError* e=nil;auto values=[MTLFunctionConstantValues new];uint32_t id=0x80000002u,zero=0;for(unsigned k=10;k<16;++k)[values setConstantValue:&id type:MTLDataTypeUInt atIndex:k];for(unsigned k=16;k<21;++k){const uint32_t v=k==17?policy:zero;[values setConstantValue:&v type:MTLDataTypeUInt atIndex:k];}auto fn=[source newFunctionWithName:name constantValues:values error:&e];require(fn!=nil,"function");auto p=[device newComputePipelineStateWithFunction:fn error:&e];if(!p)throw std::runtime_error(std::string(source==reference?"reference:":"candidate:")+name.UTF8String+": "+e.localizedDescription.UTF8String);return p;};
 auto abi=pipeline(library,@"abi"),reference_abi=pipeline(reference,@"abi");
 auto alloc=[&](size_t n){auto b=[device newBufferWithLength:n options:MTLResourceStorageModeShared];require(b!=nil,"allocation");memset(b.contents,0,n);return b;};
 auto run=[&](id<MTLComputePipelineState> p,unsigned n,auto bind){auto c=[queue commandBuffer];auto e=[c computeCommandEncoder];[e setComputePipelineState:p];bind(e);[e dispatchThreads:MTLSizeMake(n,1,1) threadsPerThreadgroup:MTLSizeMake(std::min(n,32u),1,1)];[e endEncoding];[c commit];[c waitUntilCompleted];require(c.status==MTLCommandBufferStatusCompleted,"command failed");};
 auto sizes=alloc(16);run(abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});auto a=(uint32_t*)sizes.contents;require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[2]==sizeof(RealState)&&a[3]==RealProbeAbi,"ABI");
 memset(sizes.contents,0,sizes.length);run(reference_abi,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:sizes offset:0 atIndex:0];});require(a[0]==sizeof(Input)&&a[1]==sizeof(Output)&&a[3]==RealProbeAbi,"frozen control ABI");
 auto original=pipeline(reference,@"original"),new_original=pipeline(library,@"original"),advance=pipeline(library,@"advance"),observe=pipeline(library,@"observe");
 const unsigned count=1024;auto inputs=alloc(count*sizeof(Input)),baseline=alloc(count*sizeof(Output)),candidate=alloc(count*sizeof(Output)),states=alloc(count*sizeof(RealState));auto in=(Input*)inputs.contents;
 for(unsigned i=0;i<count;++i){auto& v=in[i];v.program.max_iterations=4;v.program.de_option=i%2?20:0;v.steps=i%7;v.iteration=77+i%13;v.initial_refine=i%5;v.initial_de=i%9;v.initial_work=i%17;v.depth=float(i%31)/7.f;v.stop=.01f;
 for(unsigned k=0;k<3;++k){v.point[k]=float(int((i*17+k*13)%199)-99)/23.f;v.ray[k]=float(int((i*7+k*5)%19)-9)/11.f;}
 for(unsigned j=0;j<Capacity;++j){float value=0;
 switch(i%8){case 0:value=100.f;break;case 1:value=2.f;break;case 2:value=j%2?std::nextafter(2.f,3.f):2.f;break;case 3:value=j%2?std::nextafter(3.5f,4.f):3.5f;break;case 4:{const float t[]={3.f,2.f,4.f,3.05f};value=t[j%4];break;}case 5:{const float t[]={5.5f,2.5f,6.f,1.25f};value=t[j%4];break;}case 6:{const float t[]={-3.f,6.f,2.75f,3.125f};value=t[j%4];break;}default:value=float(int((i+j*7)%23)-6)/4.f;}
 v.smooth[j]=value;v.iterations[j]=(i+j)%7;}
 if(i%31==0)v.fault_at=1+i%12;if(i%37==0)v.initial_fault=16;if(i%41==0)v.steps=256;if(i%43==0)v.steps=0xffffffffu;if(i%47==0){v.steps=255;v.fault_at=1;v.initial_fault=0;}
 if(i%53==0)v.ray[1]=std::bit_cast<float>(0x7fc00000u);if(i%59==0)v.depth=std::numeric_limits<float>::infinity();if(i%61==0)v.smooth[1]=std::bit_cast<float>(0x7fc00000u);
 auto& p=v.program;p.version=kFormulaAbiVersion;p.driver=0;p.slot_count=6;p.work_limit=64;p.local_work_limit=16;p.radius_stop=16;p.gradient_radius_stop=16;p.stop_scale=1;p.de_scale=1;p.de_offset=.001f;p.de_offset_floor=.0001f;p.log_radius_stop=1.4f;for(auto& n:p.hybrid_counts)n=1;
 for(auto& slot:p.slots){slot.version=kFormulaAbiVersion;slot.namespace_id=InternalFormula;slot.id=2;slot.enabled=1;slot.negative_params[1]=1.f;slot.negative_params[2]=.25f;}
 if(v.fault_at)p.work_limit=1;if(v.steps==255)p.version=0;

 }
 run(original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:baseline offset:0 atIndex:1];});
 auto save=[&](const char* name,id<MTLBuffer> b){std::ofstream f(std::string(argv[2])+"/"+name+".bin",std::ios::binary);f.write((const char*)b.contents,b.length);require(bool(f),"evidence write");};save("input",inputs);save("control",baseline);
 run(new_original,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];});require(!memcmp(baseline.contents,candidate.contents,baseline.length),"new original differs from frozen control");
 run(observe,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];});save("observed",candidate);
 auto expected=(Output*)baseline.contents;auto observed=(Output*)candidate.contents;
 for(unsigned i=0;i<count;++i)require(!memcmp(expected+i,observed+i,offsetof(Output,trace)),"observer differs from frozen control");
 memcpy(baseline.contents,candidate.contents,baseline.length);
 unsigned queries=0,completed=0,guards=0,preexisting=0,work=0;
 for(unsigned i=0;i<count;++i){const auto& v=in[i];const auto& out=expected[i];const auto& t=out.trace;require(!t.overflow&&t.count<=Capacity,"trace capacity");queries+=t.count;completed+=out.success;work+=out.geometry.formula_work-v.initial_work;
 require(out.success==uint32_t(!out.geometry.fault),"terminal fault");
 if(v.steps>255){++guards;require(t.count==0&&out.geometry.fault==1&&out.iteration==v.iteration&&!memcmp(out.point,v.point,sizeof(v.point))&&!memcmp(&out.depth,&v.depth,sizeof(float)),"refinement limit guard");}
 if(v.initial_fault&&v.steps<=255){++preexisting;require(t.count==1&&out.geometry.fault==v.initial_fault,"preexisting fault");}
 require(out.geometry.refine_calls==v.initial_refine+t.count,"refine calls");uint32_t iteration=v.iteration;
 for(unsigned j=0;j<t.count;++j){const auto& q=t.queries[j];require(q.maximum==v.program.max_iterations+(v.program.de_option!=20?1:0)&&q.smooth==1&&q.temporary==1,"temporary maximum contract");
 if(j)require(!memcmp(&q.before,&t.queries[j-1].after,sizeof(q.before)),"carried lane");
 require(q.after_record.formula_work>=q.before_record.formula_work&&q.after_record.de_calls==q.before_record.de_calls+uint32_t(!q.before_record.fault),"child work/accounting");if(!q.after_record.fault)iteration=uint32_t(q.result.state.iteration);
 }
 if(t.count)require(!memcmp(&out.lane,&t.queries[t.count-1].after,sizeof(out.lane)),"final lane");require(out.iteration==iteration,"final iteration");
 }
 require(queries>1000&&completed>100&&guards>10&&preexisting>10&&work>1000,"vacuous real fixture");
 unsigned comparisons=0;const unsigned quanta[]={1,2,3,7,31,1000,0};for(unsigned repeat=0;repeat<3;++repeat)for(auto quantum:quanta){memset(states.contents,0,states.length);memset(candidate.contents,0xa5,candidate.length);
 for(unsigned step=0;step<8192;++step){unsigned q=quantum?quantum:(step%4==0?7:step%4==1?1:step%4==2?3:2);
 run(advance,count,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:inputs offset:0 atIndex:0];[e setBuffer:candidate offset:0 atIndex:1];[e setBuffer:states offset:0 atIndex:2];[e setBytes:&q length:4 atIndex:3];});
 bool all=true;auto st=(RealState*)states.contents;for(unsigned i=0;i<count;++i){all&=st[i].done!=0;if(!st[i].done){auto bytes=(unsigned char*)candidate.contents+i*sizeof(Output);for(unsigned k=0;k<sizeof(Output);++k)require(bytes[k]==0xa5,"pending publication");}}
 if(all)break;require(step!=8191,"unfinished");}
 if(!repeat&&quantum==1)save("candidate-first",candidate);
 require(!memcmp(baseline.contents,candidate.contents,baseline.length),"real refinement state or boundary mismatch");comparisons+=count;
 }
 save("candidate",candidate);
 std::cout<<"{\"scope\":\"real internal-2 Hybrid3D iteration refinement\",\"cases\":"<<count<<",\"comparisons\":"<<comparisons<<",\"queries_one_schedule\":"<<queries<<",\"completed_one_schedule\":"<<completed<<",\"formula_work_one_schedule\":"<<work<<",\"limit_guards\":"<<guards<<",\"preexisting_faults\":"<<preexisting<<",\"differences\":0}\n";
 return 0;}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 2;}}}
