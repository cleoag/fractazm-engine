// Separate preview artifact, built against the explicitly selected engine snapshot.
#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/general_geometry.metal.h"
#include "src/render/native_metal/primary_inside_bridge.metal.h"
using namespace metal;
using namespace mb3d_native;
kernel void exact_arithmetic_profile(device uint* p [[buffer(0)]]){p[0]=2;p[1]=4;p[2]=32;}
kernel void general_navigation_abi(device uint* p [[buffer(0)]]){
 p[0]=5;p[1]=sizeof(NativeFormulaState);p[2]=sizeof(ExactScene);p[3]=sizeof(NativeFormulaProgram);
}
inline uint2 navigation_spatial_slot(uint phase){
 uint2 p=0;
 for(uint b=0;b<5;++b){p.x|=((phase>>(2*b))&1u)<<(4-b);p.y|=((phase>>(2*b+1))&1u)<<(4-b);}
 return p;
}
inline uint navigation_spatial_phase(uint2 p){
 uint phase=0;
 for(uint b=0;b<5;++b){phase|=((p.x>>(4-b))&1u)<<(2*b);phase|=((p.y>>(4-b))&1u)<<(2*b+1);}
 return phase;
}
// Each owner traverses its coarse rows in order. State survives chunks and
// phases; changing camera starts a new navigation traversal, never a Full seed.
kernel void general_navigation(device uint* rgb [[buffer(0)]],device uint4* records [[buffer(1)]],
 constant ExactScene& scene [[buffer(2)]],constant NativeFormulaProgram& program [[buffer(3)]],
 device NativeFormulaState* states [[buffer(4)]],constant uint4& batch [[buffer(5)]],
 device const NativeMapDescriptor* maps [[buffer(6)]],device const float* pixels [[buffer(7)]],
 constant uint4& cfg [[buffer(8)]],constant uint2& counts [[buffer(9)]],
 constant NativeNormalParams& normals [[buffer(10)]],constant mb3d_inside::Params& inside [[buffer(11)]],
 constant mb3d_camera::Params& camera [[buffer(12)]],device const mb3d_camera::Axis* columns [[buffer(13)]],
 device const mb3d_camera::Axis* rows [[buffer(14)]],uint owner [[thread_position_in_grid]]) {
 const uint block=cfg.x,basePhase=cfg.y,workers=cfg.z,cellsX=(scene.width+block-1)/block;
 const uint cells=cellsX*((scene.height+block-1)/block);
 const bool independent=(cfg.w&1u)!=0,balanced=(cfg.w&2u)!=0;
 auto lane=(independent||(basePhase==0&&batch.x==0))?native_initial_state(float3(0),as_type<float>(scene.de_stop),program):states[owner];
 for(uint ordinal=batch.x+(independent?owner:0);ordinal<batch.x+batch.y;ordinal+=independent?batch.y:1){
  // Ordered batches address the owner's local sequence, not global cells.
  // This is exactly the former ascending-cell order filtered by row % workers,
  // but every owner can advance in the same command. Tail owners may be empty.
  const uint i=independent?ordinal%cells:((ordinal/cellsX)*workers+owner)*cellsX+ordinal%cellsX;
  if(i>=cells)continue;
  const uint phase=basePhase+(independent?ordinal/cells:0),record=independent?ordinal:i;
  uint cy=i/cellsX;
  uint ox=(i%cellsX)*block,oy=cy*block,cw=min(block,scene.width-ox),ch=min(block,scene.height-oy),area=cw*ch;
  if(phase>=area){records[record]=uint4(0);continue;}
  // Coprime cyclic permutation gives disjoint complete coverage, including edges.
  uint slot=(phase+(i*2654435761u)%area)%area,x=ox+slot%cw,y=oy+slot/cw;
  // Only proved-independent pixels use a spatially balanced dyadic prefix.
  // Ordered/stateful cells and clipped edges retain their original traversal.
  if(balanced&&cw==32&&ch==32){uint2 p=navigation_spatial_slot(phase^((i*2654435761u)&1023u));x=ox+p.x;y=oy+p.y;}
  NativeGeometryRecord rec{};rec.version=kGeneralGeometryVersion;
  NativeNormalRecord nr{};nr.version=kGeneralNormalVersion;NativeNormalResult normal{};
  mb3d_inside::Record ir{};
  NativeResources resources{maps,pixels,counts.x,counts.y,nullptr};
  auto ray=mb3d_camera::camera_ray(camera,columns[x],rows[y],x,y,general_double(scene.max_ray_length));
  auto value=general_primary_inside(scene,program,inside,ray,x,y,resources,rec,lane,ir,normals,nr,normal);
  uint fault=rec.fault|value.fault|nr.fault|normal.fault|ray.fault;
  records[record]=uint4(fault,rec.fp32_range_exits!=0,1,rec.formula_work);
  if(fault)continue;
  float light=value.rm_result?(.15f+.85f*abs(float(value.normal[2])/32767.f)):0.f;
  uint v=uint(clamp(light,0.f,1.f)*255.f);uint color=value.rm_result?(v|(uint(v*.82f)<<8)|(uint(v*.6f)<<16)):0x231a13;
  if(!phase)for(uint yy=oy;yy<oy+ch;++yy)for(uint xx=ox;xx<ox+cw;++xx)rgb[yy*scene.width+xx]=color;
  else rgb[y*scene.width+x]=color;
 }
 if(!independent)states[owner]=lane;
}

// Display reconstruction never feeds the orbit state or the exact sample buffer.
// Completed samples occupy a cyclic contiguous interval in each clipped cell.
// Its nearest point is on an end row or in the rectangle of complete middle rows.
inline int navigation_distance(int2 a,int2 b){int2 d=a-b;return d.x*d.x+d.y*d.y;}
inline int2 navigation_nearest(int first,int last,int width,int2 p){
 int r0=first/width,r1=last/width;
 int2 best= int2(clamp(p.x,first%width,r0==r1?last%width:width-1),r0);
 int2 q=int2(clamp(p.x,r0==r1?first%width:0,last%width),r1);
 if(navigation_distance(q,p)<navigation_distance(best,p))best=q;
 if(r1>r0+1){q=int2(clamp(p.x,0,width-1),clamp(p.y,r0+1,r1-1));
  if(navigation_distance(q,p)<navigation_distance(best,p))best=q;}
 return best;
}
kernel void general_navigation_display(device const uint* samples [[buffer(0)]],
 device uint* display [[buffer(1)]],constant uint4& cfg [[buffer(2)]],
 uint index [[thread_position_in_grid]]) {
 const uint width=cfg.x,height=cfg.y,block=cfg.z,end=cfg.w&0x7fffffffu;
 const bool balanced=(cfg.w>>31)!=0;
 if(index>=width*height)return;
 const uint x=index%width,y=index/width,cellsX=(width+block-1)/block,cellsY=(height+block-1)/block;
 const uint cx=x/block,cy=y/block,cw=min(block,width-cx*block),ch=min(block,height-cy*block),area=cw*ch;
 const uint start=((cy*cellsX+cx)*2654435761u)%area,slot=(y%block)*cw+x%block;
 const uint samplePhase=balanced&&cw==32&&ch==32?
  navigation_spatial_phase(uint2(x%32,y%32))^(((cy*cellsX+cx)*2654435761u)&1023u):(slot+area-start)%area;
 if(samplePhase<min(end,area)){display[index]=samples[index];return;}
 float3 color=0.f;float weights=0.f;
 for(int yy=max(0,int(cy)-1);yy<=min(int(cellsY)-1,int(cy)+1);++yy)
 for(int xx=max(0,int(cx)-1);xx<=min(int(cellsX)-1,int(cx)+1);++xx){
  uint ox=uint(xx)*block,oy=uint(yy)*block,w=min(block,width-ox),h=min(block,height-oy),a=w*h;
  uint first=((uint(yy)*cellsX+uint(xx))*2654435761u)%a,count=min(end,a),last=min(first+count,a)-1;
  int2 p=int2(x,y)-int2(ox,oy),q=navigation_nearest(int(first),int(last),int(w),p);
  if(balanced&&w==32&&h==32){
   uint step=32,complete=1;while(complete*4<=count){complete*=4;step/=2;}
   int2 offset=int2(navigation_spatial_slot(((uint(yy)*cellsX+uint(xx))*2654435761u)&1023u)&uint2(step-1));
   q=offset+clamp((p-offset+int(step/2))/int(step),int2(0),(int2(31)-offset)/int(step))*int(step);
  }else if(first+count>a){int2 other=navigation_nearest(0,int(first+count-a-1),int(w),p);
   if(navigation_distance(other,p)<navigation_distance(q,p))q=other;}
  const float distance=float(navigation_distance(q,p))+1.f,weight=1.f/(distance*distance);
  const uint v=samples[(oy+uint(q.y))*width+ox+uint(q.x)];
  color+=float3(v&255u,(v>>8)&255u,(v>>16)&255u)*weight;weights+=weight;
 }
 uint3 c=uint3(clamp(color/weights+0.5f,0.f,255.f));display[index]=c.x|(c.y<<8)|(c.z<<16);
}
