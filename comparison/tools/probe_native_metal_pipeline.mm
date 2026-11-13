// Read-only device pipeline preparation probe. No source compilation at runtime,
// image rendering or CPU pixel work. Formula/driver values are explicit inputs.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <array>
using Clock=std::chrono::steady_clock;
int main(int argc,char**argv){@autoreleasepool{
 if(argc!=10){fprintf(stderr,"usage: probe LIB KERNEL DRIVER ID0 ID1 ID2 ID3 ID4 ID5\n");return 2;}
 auto begin=Clock::now();auto emit=[&](const char*phase){printf("{\"phase\":\"%s\",\"seconds\":%.9f}\n",phase,std::chrono::duration<double>(Clock::now()-begin).count());fflush(stdout);};
 uint32_t driver=strtoul(argv[3],nullptr,0);std::array<uint32_t,6> ids{};
 for(unsigned i=0;i<6;++i)ids[i]=strtoul(argv[4+i],nullptr,0);
 NSError*error=nil;auto device=MTLCreateSystemDefaultDevice();if(!device)return 3;
 emit("load_library_begin");auto lib=[device newLibraryWithURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:argv[1]]] error:&error];
 if(!lib){fprintf(stderr,"%s\n",error.description.UTF8String);return 4;}
 emit("lookup_begin");NSString*name=[NSString stringWithUTF8String:argv[2]];auto fn=[lib newFunctionWithName:name];if(!fn)return 5;
 if(fn.functionConstantsDictionary.count){auto values=[MTLFunctionConstantValues new];
  for(MTLFunctionConstant*c in fn.functionConstantsDictionary.allValues){uint32_t v=0;bool b=false;
   if(c.index==4&&c.type==MTLDataTypeBool){[values setConstantValue:&b type:MTLDataTypeBool atIndex:c.index];continue;}
   if(c.type!=MTLDataTypeUInt)return 6;
   if(c.index>=10&&c.index<16)v=ids[c.index-10];else if(c.index==16)v=driver;else if(c.index==17)v=1;
   else if(c.index==0)v=4;else if(c.index==1)v=0;else if(c.index==2)v=0x4e415431u;else return 7;
   [values setConstantValue:&v type:MTLDataTypeUInt atIndex:c.index];}
  emit("specialize_begin");fn=[lib newFunctionWithName:name constantValues:values error:&error];
  if(!fn){fprintf(stderr,"%s\n",error.description.UTF8String);return 8;}}
 emit("pipeline_begin");auto pipeline=[device newComputePipelineStateWithFunction:fn error:&error];
 if(!pipeline){fprintf(stderr,"%s\n",error.description.UTF8String);return 9;}
 emit("complete");return 0;
}}
