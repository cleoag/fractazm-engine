#include "presentation.h"
#include <cassert>
#include <iostream>
int main(){
 using viewer::shouldPresent;
 assert(shouldPresent(3,3,2,1,1,false));
 assert(shouldPresent(3,5,2,1,1,false)); // continuous motion must not starve display
 assert(!shouldPresent(2,5,3,1,1,false));
 assert(!shouldPresent(3,5,2,1,2,false)); // Stop/load/settings invalidate old work
 assert(!shouldPresent(3,5,2,1,1,true));
 assert(shouldPresent(5,5,3,1,1,true));
 assert(shouldPresent(5,5,5,1,1,true)); // idle refinement replaces same-camera draft
 assert(!shouldPresent(6,5,3,1,1,false));
 using viewer::Quality;using viewer::nextPass;
 for(auto quality:{Quality::Points,Quality::Grid,Quality::Full}) {
  auto first=nextPass(quality,false,0,0);assert(first.phase==0&&first.quality!=Quality::Full);
  const unsigned total=quality==Quality::Grid?4:16;
  for(unsigned phase=1;phase<total;++phase){auto p=nextPass(quality,true,phase,total);assert(p.phase==phase&&p.quality==first.quality);}
  auto full=nextPass(quality,true,total,total);assert(full.quality==Quality::Full&&full.phase==0);
  auto moved=nextPass(quality,false,total,total);assert(moved.quality==first.quality&&moved.phase==0);
 }
 assert(nextPass(Quality::Draft,true,1,1).quality==Quality::Full);
 assert(nextPass(Quality::Points,true,1,1).quality==Quality::Full); // unsupported sparse draft
 for(unsigned total:{65u,1024u}){
  for(unsigned phase=1;phase<total;++phase){auto p=nextPass(Quality::Points,true,phase,total);assert(p.phase==phase&&p.quality==Quality::Points);}
  assert(nextPass(Quality::Points,true,total,total).quality==Quality::Full);
  assert(nextPass(Quality::FastGeometry,false,0,total).quality==Quality::FastGeometry);
  assert(nextPass(Quality::FastGeometry,true,1,total).quality==Quality::FastGeometry);
  assert(nextPass(Quality::FastGeometry,true,total,total).quality==Quality::Full);
 }
 for(unsigned motion=0;motion<200;++motion)assert(!viewer::cancelIdleNavigation(false,false,true));
 assert(viewer::cancelIdleNavigation(false,true,true));
 assert(!viewer::cancelIdleNavigation(false,true,false));
 assert(!viewer::cancelIdleNavigation(true,true,true));
 for(auto q:{Quality::Draft,Quality::Grid,Quality::Points,Quality::Full,Quality::FastGeometry}){
  const auto settled=viewer::previewPass(q,true,7,16,240,640);
  assert(settled.phase==0&&settled.quality==(q==Quality::Full?Quality::Points:q));
 }
 viewer::MotionResolution resolution;
 assert(resolution.edge()==320);
 resolution.observe(10,true);assert(resolution.edge()==320); // Ignore cold compilation.
 for(int i=0;i<3;++i)resolution.observe(.02,false);
 assert(resolution.edge()==320); // Avoid changing resolution on every fast reply.
 resolution.observe(.02,false);assert(resolution.edge()==480);
 for(int i=0;i<10;++i)resolution.observe(.2,false);
 assert(resolution.edge()==160);
 resolution.observe(0,false);assert(resolution.edge()==160);
 std::cout<<"Presentation, complete refinement schedules, cancellation, resolution reset and adaptive motion checks passed\n";
}
