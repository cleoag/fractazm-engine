// Resource-only probe: reuse the candidate's production decoder, no render/orbit.
#include "core/lightmap.h"
#include "core/render_context.h"
#include "core/numeric_precision.h"
#include <charconv>
#include <cstring>
#include <iostream>
#include <memory>
int main(int argc,char**argv){
 if(argc<3)return 2;
 auto context=std::make_shared<mb3d::RenderContext>(argv[1]);
 context->precision=mb3d::PrecisionMode::Native;
 mb3d::ScopedRenderContext binding(context);
 std::cout<<"[";
 for(int i=2;i<argc;++i){
  int id=0;const char* end=argv[i]+std::strlen(argv[i]);
  auto parsed=std::from_chars(argv[i],end,id);if(parsed.ec!=std::errc{}||parsed.ptr!=end||id<=0)return 2;
  TLightMap m{};const bool loaded=mb3d::LoadLightMapNr(id,&m);
  if(i!=2)std::cout<<",";
  std::cout<<"{\"id\":"<<id<<",\"loaded\":"<<(loaded?"true":"false")
    <<",\"nonempty\":"<<(m.LMa?"true":"false")<<",\"width\":"<<m.LMWidth<<",\"height\":"<<m.LMHeight<<",\"type\":"<<m.iMapType<<"}";
  mb3d::FreeLightMap(&m);
 }
 std::cout<<"]\n";return 0;
}
