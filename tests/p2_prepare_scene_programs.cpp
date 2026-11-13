// Read-only P2 diagnostic: use the frozen P1 preparation archives, no GPU work.
#include "src/render/native_metal/general_scene_host.h"
#include "src/render/native_metal/general_normals_host.h"
#include "src/render/native_metal/native_inside_host.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace mb3d;
template<class T> void read(const std::string& p,T& v){
 if(std::filesystem::file_size(p)!=sizeof(v))throw std::runtime_error("input ABI");
 std::ifstream f(p,std::ios::binary);f.read(reinterpret_cast<char*>(&v),sizeof(v));if(!f)throw std::runtime_error("input read");
}
template<class T> void write(const std::string& p,const T& v){
 std::ofstream f(p,std::ios::binary);f.write(reinterpret_cast<const char*>(&v),sizeof(v));if(!f)throw std::runtime_error("output write");
}
template<class T> void writeArray(const std::string& p,const std::vector<T>& values){
 std::ofstream f(p,std::ios::binary);if(!values.empty())f.write(reinterpret_cast<const char*>(values.data()),values.size()*sizeof(T));if(!f)throw std::runtime_error("array write");
}
int main(int argc,char** argv){try{
 if(argc!=6)throw std::runtime_error("header addon resource-root output-dir fast[0/1]");
 TMandHeader10 h{};THeaderCustomAddon a{};read(argv[1],h);read(argv[2],a);
 const std::string output=argv[4];const bool fast=std::string(argv[5])=="1";
 if(std::string(argv[5])!="0"&&!fast)throw std::runtime_error("profile");
 if(std::filesystem::exists(output))throw std::runtime_error("new output required");
 h.Iterations=std::min(h.Iterations,64);h.MinimumIterations=std::min<unsigned>(h.MinimumIterations,h.Iterations);h.bColorOnIt=0;h.bVolLightNr=0;
 if(fast){h.mZstepDiv=.7f;h.sRaystepLimiter=std::max(h.sRaystepLimiter,.35f);h.sDEstop=std::max(h.sDEstop,.01f);h.bStepsafterDEStop=4;}
 // Dimensions are the P1 request dimensions already produced by --request.
 const ScopedNumericPrecision precision(PrecisionMode::Native);
 auto scene=native_metal::PrepareGeneralSceneWithMaps(h,a,h.Width,h.Height,argv[3],nullptr);
 ScopedRenderContext binding(scene.base.host->context);
 auto normal=native_metal::PrepareGeneralNormals(scene.base.host->mct,scene.formulas.device);normal.smoothing=0;
 auto inside=native_metal::PrepareNativeInside(scene.base.host->mct);
 std::filesystem::create_directory(output);
 write(output+"/scene.bin",scene.base.device);write(output+"/program.bin",scene.formulas.device);
 if(scene.formulas.program_count()==2)write(output+"/second.bin",scene.formulas.second);
 write(output+"/normal.bin",normal);write(output+"/inside.bin",inside);
 write(output+"/camera.bin",scene.camera.device);
 writeArray(output+"/camera-columns.bin",scene.camera.columns);writeArray(output+"/camera-rows.bin",scene.camera.rows);
 writeArray(output+"/maps.bin",scene.formulas.maps);writeArray(output+"/map-pixels.bin",scene.formulas.map_pixels);

 auto& p=scene.formulas.device;
 std::cout<<"{\"driver\":"<<p.driver<<",\"second_driver\":"<<scene.formulas.second.driver<<",\"program_count\":"<<scene.formulas.program_count()<<",\"persistent_lane\":"<<scene.formulas.requires_persistent_lane<<",\"normals_on_de\":"<<normal.normals_on_de<<",\"map_count\":"<<scene.formulas.maps.size()<<",\"width\":"<<h.Width<<",\"height\":"<<h.Height<<",\"max_iterations\":"<<p.max_iterations<<",\"de_option\":"<<p.de_option<<",\"slots\":[";
 for(unsigned i=0;i<6;++i){auto& s=p.slots[i];if(i)std::cout<<',';std::cout<<"{\"enabled\":"<<s.enabled<<",\"namespace\":"<<s.namespace_id<<",\"id\":"<<s.id<<",\"count\":"<<p.hybrid_counts[i]<<"}";}
 std::cout<<"]}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}}
