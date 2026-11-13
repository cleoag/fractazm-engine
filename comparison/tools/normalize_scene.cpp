// Use the real loader/migration path for corpus preparation; never reinterpret old headers.
#include "io/m3p_parser.h"
#include <cstdio>
int main(int argc,char **argv) {
  if(argc!=3) return 2;
  auto data=mb3d::LoadM3PFile(argv[1]);
  if(!data.success){std::fprintf(stderr,"%s\n",data.error.c_str());return 1;}
  if(!mb3d::SaveM3PFile(argv[2],data.header,data.addon)) return 3;
  std::printf("source_mand_id=%d\n",data.source_mand_id);
  return 0;
}
