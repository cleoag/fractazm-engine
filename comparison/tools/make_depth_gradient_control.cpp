#include "io/m3p_parser.h"
#include <cstring>
#include <iostream>
int main(int n,char**v){if(n!=3)return 2;auto f=mb3d::LoadM3PFile(v[1]);if(!f.success)return 3;auto h=f.header;h.Light.TBoptions&=0x3fffffffu;if(!mb3d::SaveM3PFile(v[2],h,f.addon))return 4;auto g=mb3d::LoadM3PFile(v[2]);if(!g.success||memcmp(&h,&g.header,sizeof(h))||memcmp(&f.addon,&g.addon,sizeof(f.addon)))return 5;std::cout<<"Only depth-gradient bits changed; decoded header/addon exact\n";}
