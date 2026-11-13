// Task-local scene preparation audit: no Render, orbit step, CPU pixel or GPU API calls.
#include "render/native_metal/render_plan.h"
#include "render/native_metal/soft_shadow_host.h"
#include "render/native_metal/generic_color_host.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include "comparison/metal_exact/shadow_host.h"
#include "render/native_metal/native_ao_host.h"
#include "render/native_metal/native_ao_readback.h"
#include "render/native_metal/native_inside_host.h"
#include "comparison/metal_exact/native_reflection_host.h"
#include "core/numeric_precision.h"
#include "render/native_metal/dof_host.h"
#include "render/native_metal/general_normals_host.h"
#include "render/native_metal/zbuffer_host.h"
#include "io/m3p_parser.h"
#include "render/native_metal/dof_dispatch.h"
#include "render/native_metal/zbuffer_dispatch.h"
#include "comparison/metal_exact/shadow_abi.h"
#include "comparison/metal_exact/color_abi.h"
#include <cmath>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
// Allocation planning is shared with runtime; this tool performs no Metal calls.
using namespace mb3d;
using namespace mb3d::native_metal;
using namespace mb3d::metal_exact;
std::string q(const std::string& in){
 std::string out="\"";for(unsigned char c:in){if(c=='"'||c=='\\'){out+='\\';out+=char(c);}
 else if(c<32){const char* h="0123456789abcdef";out+="\\u00";out+=h[c>>4];out+=h[c&15];}else out+=char(c);}
 return out+'\"';
}
int main(int argc,char** argv){try{
 if(argc!=4 && !(argc==5&&std::string(argv[4])=="--original-size"))throw std::runtime_error("usage: admission INPUT RESOURCE_ROOT NEW_JSON [--original-size]");
 const bool originalSize=argc==5;
 const std::string input=argv[1],root=argv[2],output=argv[3];
 if(std::filesystem::exists(output))throw std::runtime_error("Preserve existing output");
 auto loaded=LoadM3PFile(input);PreparedRenderPlan plan;auto& stages=plan.stages;
 const unsigned requestedWidth=originalSize?unsigned(loaded.header.Width):64;
 const unsigned requestedHeight=originalSize?unsigned(loaded.header.Height):48;
 std::string status="LOADER_REJECTED",deferred;bool preparedOk=false,allOk=false;
 bool shadows=false,ambient=false,reflections=false;
 auto& prepared=plan.scene;unsigned aoAlgorithm=0,workers=0;uint64_t aoBytes=0,dofBytes=0,znormalBytes=0;
 if(loaded.success){
  // Guard all six loaded names, including disabled and stale internal slots,
  // before any GetMCT/constant/map preparation is reachable.
  if(loaded.header.MCoptions&1u)deferred="enabled_excluded_mode";
  for(const auto& f:loaded.addon.Formulas){
   const char* first=reinterpret_cast<const char*>(f.CustomFname);
   std::string name(first,std::find(first,first+sizeof(f.CustomFname),'\0'));
   std::transform(name.begin(),name.end(),name.begin(),[](unsigned char c){return char(std::tolower(c));});
   if(name.find("jit")!=std::string::npos)deferred="excluded_named_formula";
  }
  if(!deferred.empty())status="DEFERRED_REJECTED_BEFORE_PREPARATION";
  else {
   const auto originalHeader=loaded.header;const auto originalAddon=loaded.addon;
   RenderPlanRequest request;
   request.width=requestedWidth;request.height=requestedHeight;request.logical_workers=6;
   request.resource_root=root;request.deterministic=true;request.force_calc3d=true;
   plan=PrepareNativeRenderPlan(loaded.header,loaded.addon,request,PlanDiagnostics::CollectIndependentStages);
   preparedOk=!stages.empty()&&stages.front().status=="ACCEPTED";
   shadows=plan.shadows;ambient=plan.ambient;reflections=plan.reflections;
   workers=plan.logical_workers;aoAlgorithm=plan.ao_algorithm;aoBytes=plan.ao_bytes;
   dofBytes=plan.dof_bytes;znormalBytes=plan.z_normal_bytes;
   if(std::memcmp(&originalHeader,&loaded.header,sizeof(originalHeader))||std::memcmp(&originalAddon,&loaded.addon,sizeof(originalAddon)))
    throw std::runtime_error("Preparation mutated caller input");
   allOk=plan.ready();
   status=allOk?"PREPARATION_ACCEPTED":"PREPARATION_REJECTED";
  }
 }
 std::ofstream out(output);if(!out)throw std::runtime_error("Cannot create output");
 out<<"{\"schema\":\"mb3d.native-general-scene-admission.v1\",\"status\":"<<q(status)
    <<",\"input\":"<<q(input)<<",\"resource_root\":"<<q(root)
    <<",\"profile\":\"native_cpu_scene_preparation\",\"preparation_revision\":\"shared_plan02_requested_lighting\",\"width\":"<<requestedWidth<<",\"height\":"<<requestedHeight<<",\"threads\":6,\"deterministic\":true"
    <<",\"main_cli_calc3d_override\":true"
    <<",\"render_executed\":false,\"gpu_executed\":false,\"formula_point_execution\":false"
    <<",\"loader_success\":"<<(loaded.success?"true":"false")<<",\"loader_error\":"<<q(loaded.error)
    <<",\"deferred_guard\":"<<q(deferred)<<",\"loaded_six_names_checked\":"<<(loaded.success?"true":"false")
    <<",\"general_preparation_attempted\":"<<(!stages.empty()?"true":"false");
 if(loaded.success){const auto& h=loaded.header;
  out<<",\"loaded_features\":{\"width\":"<<h.Width<<",\"height\":"<<h.Height<<",\"calc3d\":"<<unsigned(h.bCalc3D)
   <<",\"tiling\":"<<unsigned(h.TilingOptions)<<",\"stereo\":"<<unsigned(h.bStereoMode)
   <<",\"post_zbuffer_normals\":"<<unsigned(h.byCalcNsOnZBufAuto)<<",\"dof\":"<<unsigned(h.bCalcDOFtype)
   <<",\"volume\":"<<unsigned(h.bVolLightNr)<<",\"ao\":"<<unsigned(h.bCalcAmbShadowAutomatic)
   <<",\"shadow\":"<<unsigned(h.bCalculateHardShadow)<<",\"reflection\":"<<unsigned(h.bCalcSRautomatic)
   <<",\"hybrid_selector\":"<<unsigned(loaded.addon.bOptions1)<<",\"inside_analytic_selector\":"<<unsigned(loaded.addon.bOptions2)
   <<",\"de_combination_selector\":"<<unsigned(loaded.addon.bOptions3)<<"}";
 }
 out<<",\"requested_lighting_resources\":[";
 for(size_t i=0;i<plan.lighting_requirements.size();++i){
  if(i)out<<',';const auto& r=plan.lighting_requirements[i];
  out<<"{\"kind\":"<<q(r.kind)<<",\"original_light_index\":"<<r.light_index
     <<",\"map_id\":"<<r.map_id<<",\"filename\":"<<q(r.filename)<<",\"gpu_supported\":false}";
 }
 out<<"]";
 std::string firstReject;bool reachable=true;
 out<<",\"stages\":[";for(size_t i=0;i<stages.size();++i){if(i)out<<',';const auto& s=stages[i];
  out<<"{\"name\":"<<q(s.name)<<",\"status\":"<<q(s.status)<<",\"reason\":"<<q(s.reason)
     <<",\"reachable_from_main\":"<<(reachable?"true":"false")<<"}";
  if(s.status=="REJECTED"){if(firstReject.empty())firstReject=s.name;reachable=false;}}
 out<<"],\"first_rejected_stage\":"<<q(firstReject);
 if(preparedOk){const auto& p=prepared.formulas.device;const auto& m=prepared.base.host->mct;
  out<<",\"required_stages\":{\"hard_shadow\":"<<(shadows?"true":"false")
     <<",\"ambient_occlusion\":"<<(ambient?"true":"false")<<",\"reflection\":"<<(reflections?"true":"false")<<"}";
  out<<",\"prepared\":{\"driver\":"<<p.driver<<",\"de_option\":"<<p.de_option<<",\"max_iterations\":"<<p.max_iterations
     <<",\"requires_persistent_lane\":"<<(prepared.formulas.requires_persistent_lane?"true":"false")
     <<",\"color_option\":"<<unsigned(m.ColorOption)<<",\"color_on_iteration\":"<<unsigned(m.ColorOnIt)
     <<",\"actual_width\":"<<prepared.base.device.width<<",\"actual_height\":"<<prepared.base.device.height<<",\"logical_workers\":"<<workers
     <<",\"ao_algorithm\":"<<aoAlgorithm<<",\"ao_scratch_bytes\":"<<aoBytes<<",\"dof_bytes\":"<<dofBytes<<",\"z_normal_bytes\":"<<znormalBytes<<",\"slots\":[";
  for(unsigned i=0;i<6;++i){if(i)out<<',';const auto& s=p.slots[i];
   out<<"{\"slot\":"<<i<<",\"namespace\":"<<s.namespace_id<<",\"id\":"<<s.id<<",\"enabled\":"<<s.enabled<<",\"traits\":"<<s.traits<<"}";}
  out<<"],\"map_elements\":"<<prepared.formulas.map_pixels.size()<<",\"maps\":[";
  for(size_t i=0;i<prepared.formulas.maps.size();++i){if(i)out<<',';const auto& d=prepared.formulas.maps[i];
   out<<"{\"id\":"<<d.map_id<<",\"width\":"<<d.width<<",\"height\":"<<d.height<<",\"elements\":"<<d.element_count<<",\"flags\":"<<d.flags<<"}";}
  out<<"]}";
 }
 out<<",\"meaning\":\"Scene-only shared-plan admission at the reported requested dimensions; no device allocation, numerical, render, resource-closure or parity qualification. Later stage preparations after a rejection are diagnostic only; first rejected stage is the main preflight cause.\"}\n";
 out.close();if(!out)throw std::runtime_error("Output write failed");return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 2;}}
