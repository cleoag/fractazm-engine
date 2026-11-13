#include "render/native_metal/navigation_preview.h"
#include "io/m3p_parser.h"
#include <cassert>
#include <iostream>
static uint32_t mix(uint32_t x){x^=x>>16;x*=0x7feb352d;x^=x>>15;x*=0x846ca68b;return x^(x>>16);}
int main(int argc,char**argv){
 assert(argc==3);auto scene=mb3d::LoadM3PFile(argv[1]);assert(scene.success);
 mb3d::RenderOptions o;o.precision=mb3d::PrecisionMode::Native;o.width_override=65;o.height_override=49;o.thread_count=4;
 mb3d::native_metal::NavigationPreviewOptions n;n.pattern=mb3d::native_metal::NavigationPattern::Dense;
 auto dense=mb3d::native_metal::RenderNavigationPreview(scene.header,scene.addon,o,n,argv[2]);assert(dense.result.success);
 n.pattern=mb3d::native_metal::NavigationPattern::Stratified;
 for(unsigned phase=0;phase<16;++phase){
  n.phase=phase;auto sparse=mb3d::native_metal::RenderNavigationPreview(scene.header,scene.addon,o,n,argv[2]);assert(sparse.result.success);
  assert(sparse.execution.primary_records==17*13);
  for(unsigned y=0;y<49;y+=4)for(unsigned x=0;x<65;x+=4){
   unsigned cw=std::min(4u,65-x),ch=std::min(4u,49-y),cell=(y/4)*17+x/4;
   unsigned slot=(mix(cell^n.seed)%(cw*ch)+phase%(cw*ch))%(cw*ch),sx=x+slot%cw,sy=y+slot/cw;
   for(unsigned yy=y;yy<y+ch;++yy)for(unsigned xx=x;xx<x+cw;++xx)for(unsigned c=0;c<3;++c)
    assert(sparse.result.image[(yy*65+xx)*3+c]==dense.result.image[(sy*65+sx)*3+c]);
  }
 }
 n.block_size=0;auto bad=mb3d::native_metal::RenderNavigationPreview(scene.header,scene.addon,o,n,argv[2]);assert(!bad.result.success&&bad.result.image.empty());
 mb3d::BackendOptions b;b.requested=mb3d::RenderBackend::Metal;b.allow_cpu_fallback=false;o.no_shadows=true;
 auto denied=mb3d::RenderWithBackend(scene.header,scene.addon,o,b);
 assert(!denied.render.success&&denied.render.image.empty()&&!denied.backend.fallback&&denied.backend.effective==mb3d::RenderBackend::Metal);
 std::cout<<"16 sparse phases match dense full-camera samples including clipped edge cells; invalid preview and strict no-CPU fallback checks passed\n";
}
