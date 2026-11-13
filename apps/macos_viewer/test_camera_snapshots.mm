#import <Foundation/Foundation.h>
#include "camera.h"
#include <cassert>
#include <iostream>
static double distance(viewer::Vec a,viewer::Vec b){return std::hypot(a[0]-b[0],a[1]-b[1],a[2]-b[2]);}
int main(int argc,char** argv){@autoreleasepool{
 assert(argc==3||(argc==4&&std::string(argv[3])=="--single-fixture"));NSString* source=@(argv[1]);NSString* dest=@(argv[2]);unsigned count=0;
 [[NSFileManager defaultManager] createDirectoryAtPath:dest withIntermediateDirectories:YES attributes:nil error:nil];
 for(NSString* name in [[NSFileManager defaultManager] contentsOfDirectoryAtPath:source error:nil]){
  if(![name.pathExtension isEqual:@"json"])continue;
  NSMutableDictionary* q=[[NSJSONSerialization JSONObjectWithData:[NSData dataWithContentsOfFile:[source stringByAppendingPathComponent:name]] options:NSJSONReadingMutableContainers error:nil] mutableCopy];
  NSData* bytes=[[NSData alloc] initWithBase64EncodedString:q[@"header"] options:0];assert(bytes.length==sizeof(TMandHeader10));
  TMandHeader10 h;memcpy(&h,bytes.bytes,sizeof(h));viewer::Camera base(h);auto unchanged=h;base.write(unchanged);assert(!memcmp(&h,&unchanged,sizeof(h)));
  for(unsigned i=0;i<3;++i){auto cam=base;auto changed=h;const char* label=i==0?"orbit":i==1?"fly":"pan";
   if(i==2)cam.move(.01,.005,-.005);else cam.rotate(.01,.005,i==1);
   if(i==0)assert(distance(cam.center,base.center)==0);
   if(i==1)assert(distance(cam.eye(),base.eye())<1e-10*std::max(1.,std::hypot(base.eye()[0],base.eye()[1],base.eye()[2])));
   cam.write(changed);assert(changed.bPlanarOptic==h.bPlanarOptic&&changed.bStereoMode==h.bStereoMode);
   q[@"header"]=[[NSData dataWithBytes:&changed length:sizeof(changed)] base64EncodedStringWithOptions:0];q[@"phase"]=@0;q[@"edge"]=@640;
   NSString* filename=[NSString stringWithFormat:@"%@-%s.json",name.stringByDeletingPathExtension,label];
   assert([[NSJSONSerialization dataWithJSONObject:q options:0 error:nil] writeToFile:[dest stringByAppendingPathComponent:filename] atomically:YES]);
  }
  NSString* series=[dest stringByAppendingPathComponent:@"p1-series"];
  [[NSFileManager defaultManager] createDirectoryAtPath:series withIntermediateDirectories:YES attributes:nil error:nil];
  for(unsigned i=0;i<20;++i){auto cam=base;auto changed=h;cam.rotate(.0001*(i+1),.00005*(i+1),false);cam.write(changed);
    q[@"header"]=[[NSData dataWithBytes:&changed length:sizeof(changed)] base64EncodedStringWithOptions:0];q[@"camera_revision"]=@(i+2);
    NSString* name20=[NSString stringWithFormat:@"%@-%02u.json",name.stringByDeletingPathExtension,i];
    assert([[NSJSONSerialization dataWithJSONObject:q options:0 error:nil] writeToFile:[series stringByAppendingPathComponent:name20] atomically:YES]);}
  ++count;
 }
 assert(count==(argc==4?1:77));std::cout<<count<<" unchanged snapshots and "<<count*3<<" camera transformations passed\n";
}}
