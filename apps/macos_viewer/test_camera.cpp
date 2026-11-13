#include "camera.h"
#include "io/m3p_parser.h"
#include <cassert>
#include <iostream>
using viewer::Vec;
static double dist(Vec a, Vec b) { return std::hypot(a[0]-b[0],a[1]-b[1],a[2]-b[2]); }
int main(int argc,char** argv) {
    assert(argc==3);
    auto scene=mb3d::LoadM3PFile(argv[1]);assert(scene.success);
    viewer::Camera base(scene.header), orbit=base,fly=base,pan=base;
    orbit.rotate(.35,.15,false);assert(dist(orbit.center,base.center)==0);assert(dist(orbit.forward,base.forward)>.1);
    fly.rotate(.35,.15,true);assert(dist(fly.eye(),base.eye())<1e-12);
    pan.move(.1,.2,.3);auto h=scene.header;pan.write(h);
    assert(std::abs((h.dZstart-h.dZmid)-base.offset)<1e-12);
    assert(std::abs((h.dZend-h.dZmid)-base.farOffset)<1e-12);
    assert(dist(pan.center,base.center)>.1*base.span);
    auto accumulated=base;
    for(int n=0;n<10000;++n)accumulated.rotate(.001,-.0005,false);
    double dot=0;for(int i=0;i<3;++i)dot+=accumulated.right[i]*accumulated.up[i];assert(std::abs(dot)<1e-10);
    auto invalid=scene.header;std::memset(invalid.hVGrads,0,sizeof(invalid.hVGrads));
    bool rejected=false;try{viewer::Camera c(invalid);}catch(...){rejected=true;}assert(rejected);
    for(auto entry: {std::pair{"orbit",orbit},std::pair{"fly",fly},std::pair{"pan",pan}}){
        auto changed=scene.header;entry.second.write(changed);
        assert(mb3d::SaveM3PFile(std::string(argv[2])+"/"+entry.first+".m3p",changed,scene.addon));
    }
    std::cout<<"6 camera invariant groups passed; orbit/fly/pan fixtures written\n";
}
