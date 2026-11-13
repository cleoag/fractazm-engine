#include "src/render/native_metal/restricted_primary_admission.h"
#include <cassert>
#include <limits>
#include <iostream>
int main(){
 ExactPixel p[2]{};ExactNormalSamples n[2]{};
 auto check=[&]{return mb3d::native_metal::RequiresGeneralInitialInside(p,n,2);};
 assert(!check());p[0].rm_result=1;p[0].start_inside=1;assert(check());
 p[1].fault=4;assert(!check());p[1].fault=0;
 p[1].raw_normal[0]=std::bit_cast<uint64_t>(std::numeric_limits<double>::quiet_NaN());assert(!check());p[1].raw_normal[0]=0;
 n[1].reserved=1;assert(!check());n[1].reserved=0;
 n[1].distance[3]=std::bit_cast<uint64_t>(std::numeric_limits<double>::quiet_NaN());assert(!check());n[1].distance[3]=0;
 n[1].orbit_trap=std::bit_cast<uint64_t>(std::numeric_limits<double>::infinity());assert(!check());n[1].orbit_trap=0;
 p[1].reserved_tail=1;assert(!check());p[1].reserved_tail=0;
 p[1].start_inside=2;assert(!check());p[1].start_inside=0;
 p[0].rm_result=0;assert(!check());
 std::cout<<"10 admission checks passed\n";
}
