// Device admission/bounds controls for the independent DE AO entry.
#include "render/native_metal/native_ao_dispatch.h"
#include "render/native_metal/general_scene_host.h"
#include "render/native_metal/formula_selector.h"
#include "render/native_metal/native_camera_abi.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include "io/m3p_parser.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
using namespace mb3d;using namespace mb3d_native_ao;
namespace gpu=mb3d::metal_exact::gpu;
unsigned checks=0;void check(bool b,const char*s){++checks;if(!b)throw std::runtime_error(s);}
template<class F>void reject(F f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}check(rejected,"Guard failed");}
int main(int argc,char**argv){@autoreleasepool{try{
 if(argc!=4)throw std::runtime_error("usage: de_pixels LIBRARY SCENE RESOURCE_ROOT");
 ScopedNumericPrecision precision(PrecisionMode::Native);
 auto loaded=LoadM3PFile(argv[2]);check(loaded.success,"load");
 loaded.header.bCalc3D=1;loaded.header.iOptions&=~1;loaded.header.bCalcAmbShadowAutomatic&=uint8_t(~128u);
 auto prepared=native_metal::PrepareGeneralScene(loaded.header,loaded.addon,64,48,argv[3]);
 auto p=PrepareNativeDistanceAO(loaded.header,prepared.base.host->mct,64,48,6);
 auto program=prepared.formulas.device;const auto&scene=prepared.base.device;
 std::array<uint32_t,6> ids{};for(unsigned i=0;i<6;++i){auto&s=program.slots[i];ids[i]=mb3d_native::FormulaPipelineSelector(s.namespace_id,s.id,s.enabled);}
 gpu::Metal m(argv[1],scene.formula_kind,0,scene.capability_id,false,true,&ids,program.driver);
 auto marker=m.buffer(20);gpu::dispatchStage(m,m.pipeline(@"native_ao_de_pixels_abi"),1,[&](id<MTLComputeCommandEncoder>e){[e setBuffer:marker offset:0 atIndex:0];});
 const uint32_t expected[]={5,sizeof(Launch),sizeof(DEParams),sizeof(DERecord),kDEPixelBatch};check(!memcmp(marker.contents,expected,20),"ABI");
 constexpr unsigned n=3072;ExactSurface input[n]{};mb3d_camera::Ray rays[n]{};
 for(unsigned i=0;i<n;++i){input[i].depth_packed=0x80000000;input[i].amb_shadow=100+i;rays[i].version=mb3d_camera::Version;}
 auto a=m.buffer(sizeof(input),input),c=m.buffer(sizeof(rays),rays),maps=m.buffer(16),pixels=m.buffer(16);
 auto output=m.buffer((n+1)*sizeof(ExactSurface)),records=m.buffer((n+1)*sizeof(DERecord));
 auto kernel=m.pipeline(@"native_ao_de_pixels");DERecord reference[n]{};
 const bool closedScene=mb3d_native::IndependentDepthScenePixels(program);
 // Valid modes: 0 the closed depth chain (proof 1); host proofs 11 state
 // contract (5), 12 closed primary chain (6), 13 conditional contract (7);
 // 15 a random first step with planned seeds (background pixels draw none).
 std::vector<uint32_t> plannedSeeds(n+p.task_count,p.initial_seed);
 auto seedWords=m.buffer(plannedSeeds.size()*sizeof(uint32_t),plannedSeeds.data());
 for(unsigned mode=0;mode<17;++mode)for(unsigned repeat=0;repeat<(mode?1:3);++repeat){
  auto q=p;auto pr=program;Launch launch{0,n,n,0};uint32_t allocation[]={n,p.task_count,0,0};
  uint32_t proof=closedScene?1:5; // mode 0: the device-checked chain where the scene has one
  uint32_t seeded=0;
  const bool valid=mode==0||(mode>=11&&mode<=13)||mode==15;
  if(mode==1)q.first_step_random=1; // a random sequence without planned seeds
  if(mode==15){q.first_step_random=1;seeded=1;}
  if(mode==16)seeded=1; // planned seeds without a random sequence
  if(mode==2){pr.slots[0].traits|=8;if(!closedScene)proof=1;} // proof 1 must re-derive the chain
  if(mode==3){pr.slots[0].namespace_id=0;if(!closedScene)proof=1;}
  if(mode==4)launch.reserved=1;
  if(mode==5)launch.allocated_pixels=n-1;
  if(mode==6)launch.count=kDEPixelBatch+1;
  if(mode==7)q.version=0;
  if(mode==8)pr.driver=0;
  if(mode==9)proof=0;
  if(mode==10)proof=2;
  if(mode==11)proof=5;
  if(mode==12)proof=6;
  if(mode==13)proof=7;
  if(mode==14)proof=8;
  memset(output.contents,0xa5,output.length);memset(records.contents,0xa5,records.length);
  auto descriptor=m.buffer(sizeof(pr),&pr);
  gpu::dispatchStage(m,kernel,n+4,[&](id<MTLComputeCommandEncoder>e){
   [e setBytes:&scene length:sizeof(scene) atIndex:0];[e setBuffer:descriptor offset:0 atIndex:1];[e setBytes:&q length:sizeof(q) atIndex:2];
   [e setBuffer:c offset:0 atIndex:3];[e setBuffer:a offset:0 atIndex:4];[e setBuffer:output offset:0 atIndex:5];[e setBuffer:records offset:0 atIndex:6];
   [e setBuffer:maps offset:0 atIndex:8];[e setBuffer:pixels offset:0 atIndex:9];[e setBytes:allocation length:sizeof(allocation) atIndex:10];[e setBytes:&launch length:sizeof(launch) atIndex:11];
   [e setBytes:&proof length:sizeof(proof) atIndex:12];[e setBuffer:seedWords offset:0 atIndex:13];
   [e setBytes:&seeded length:sizeof(seeded) atIndex:14];});
  auto r=static_cast<DERecord*>(records.contents);auto o=static_cast<ExactSurface*>(output.contents);
  for(unsigned i=0;i<n;++i){
   check(bool(r[i].common.fault)==!valid,("GPU guard status, mode "+std::to_string(mode)).c_str());
   if(valid){check(r[i].completed==1&&o[i].amb_shadow==0&&!o[i].fault&&!r[i].difs_carried_reads,"Background processing");if(repeat||mode)check(!memcmp(&reference[i],&r[i],sizeof(DERecord)),"Repeat records");else reference[i]=r[i];}
  }
  const auto*end=reinterpret_cast<const unsigned char*>(r+n);for(unsigned i=0;i<sizeof(DERecord);++i)check(end[i]==0xa5,"Output overrun");
  check(!memcmp(input,a.contents,sizeof(input)),"Input mutated");
 }
 const mb3d::native_metal::DistanceAOExecutionPolicy execution(program,p);
 auto fast=RunNativeDistanceAO(m,p,scene,program,nullptr,c,a,maps,pixels,0,0,1ull<<30,execution,[]{});
 check(fast.dispatches==4&&fast.totals.records==n&&fast.totals.changed==n,"Actual independent host pass");
 reject([&]{RunNativeDistanceAO(m,p,scene,program,nullptr,c,a,maps,pixels,0,0,de_scratch_bytes(p)-1,execution,[]{});});
 reject([&]{RunNativeDistanceAO(m,p,scene,program,nullptr,c,a,maps,pixels,0,0,1ull<<30,execution,[]{throw std::runtime_error("cancel");});});
 auto different=p;different.first_step_random=1;
 unsigned polls=0;bool mismatch=false;
 try{RunNativeDistanceAO(m,different,scene,program,nullptr,c,a,maps,pixels,0,0,1ull<<30,execution,[&]{++polls;});}
 catch(const std::invalid_argument& e){mismatch=std::string(e.what())=="Native DE AO execution policy input mismatch";}
 check(mismatch&&!polls&&!memcmp(input,a.contents,sizeof(input)),"Stale DE AO policy reached device/mutated input");
 // Order control on active pixels: the scene's policy against the ordered
 // tasks. Far ray origins put DE queries beyond any dIFS entry minimum, so a
 // conditional (dIFS) scene counts carried reads and must fall back to them.
 std::vector<ExactSurface> active(n);std::vector<mb3d_camera::Ray> far(n);
 for(unsigned i=0;i<n;++i){auto&s=active[i];s.depth_packed=i%4==3?0x80000000u:(40000u+i*7u)<<8;s.normal[2]=-32767;s.amb_shadow=9;
  auto&r=far[i];r.version=mb3d_camera::Version;r.direction[0]=1.f;r.origin[0]=i&1?4.e5f:0.f;r.origin[1]=float(i%64)*.01f;}
 auto activeInput=m.buffer(n*sizeof(ExactSurface),active.data()),farRays=m.buffer(n*sizeof(mb3d_camera::Ray),far.data());
 const mb3d::native_metal::DistanceAOExecutionPolicy ordered(program,p,nullptr,true);
 auto attempt=[&](const mb3d::native_metal::DistanceAOExecutionPolicy&e,NativeAOResult&out)->std::string{
  try{out=RunNativeDistanceAO(m,p,scene,program,nullptr,farRays,activeInput,maps,pixels,0,0,1ull<<30,e,[]{});return "";}
  catch(const std::exception&x){return x.what();}};
 NativeAOResult pixelPass,orderedPass;const auto ea=attempt(execution,pixelPass),eb=attempt(ordered,orderedPass);
 printf("order control: policy %s/%s, ordered %s; carried %llu in %llu pixels, rerun %llu, active %llu, DE queries %llu\n",
  execution.independent()?"pixels":"tasks",execution.conditional_state()?"conditional":"unconditional",eb.empty()?"ok":eb.c_str(),
  (unsigned long long)orderedPass.totals.difs_carried_reads,(unsigned long long)orderedPass.totals.difs_carried_pixels,(unsigned long long)pixelPass.difs_rerun_carried_reads,
  (unsigned long long)orderedPass.totals.active,(unsigned long long)orderedPass.totals.de_queries);
 check(ea==eb,"Order control outcome");
 if(ea.empty()){
  check(!memcmp(pixelPass.surface.contents,orderedPass.surface.contents,n*sizeof(ExactSurface))&&!memcmp(&pixelPass.totals,&orderedPass.totals,sizeof(pixelPass.totals)),"Order control surfaces/work");
  check(!execution.conditional_state()||!orderedPass.totals.difs_carried_reads||pixelPass.difs_rerun_carried_reads,"Conditional pass kept carried reads");
 }
 // Random first step: planned per-pixel seeds against the ordered tasks.
 auto randomParams=p;randomParams.first_step_random=1;
 const mb3d::native_metal::DistanceAOExecutionPolicy randomExecution(program,randomParams),randomOrdered(program,randomParams,nullptr,true);
 auto randomAttempt=[&](const mb3d::native_metal::DistanceAOExecutionPolicy&e,NativeAOResult&out)->std::string{
  try{out=RunNativeDistanceAO(m,randomParams,scene,program,nullptr,farRays,activeInput,maps,pixels,0,0,1ull<<30,e,[]{});return "";}
  catch(const std::exception&x){return x.what();}};
 NativeAOResult seededPass,randomOrderedPass;
 const auto ra=randomAttempt(randomExecution,seededPass),rb=randomAttempt(randomOrdered,randomOrderedPass);
 printf("random control: policy %s, ordered %s; planned draws %llu, advances %llu\n",
  randomExecution.seeded()?"seeded pixels":"tasks",rb.empty()?"ok":rb.c_str(),
  (unsigned long long)seededPass.seed_planning_draws,(unsigned long long)randomOrderedPass.totals.rng_advances);
 check(ra==rb&&randomExecution.seeded()==execution.independent()&&!randomOrdered.seeded(),"Random control outcome");
 if(ra.empty()){
  check(!memcmp(seededPass.surface.contents,randomOrderedPass.surface.contents,n*sizeof(ExactSurface))&&!memcmp(&seededPass.totals,&randomOrderedPass.totals,sizeof(seededPass.totals)),"Random control surfaces/work");
  // A conditional pass with carried reads returns the unplanned ordered rerun.
  check(!randomExecution.seeded()||(seededPass.difs_rerun_carried_reads?!seededPass.seed_planning_draws:
   seededPass.seed_planning_draws==randomOrderedPass.totals.rng_advances&&randomOrderedPass.totals.rng_advances),"Random control draws");
 }
 check(!memcmp(active.data(),activeInput.contents,n*sizeof(ExactSurface)),"Order control input mutated");
 printf("PASS %u checks; 3 exact repeats; proof-5/6/7 entries; seeded entry; 12 malformed GPU controls; bounds, budget, cancellation; order and random controls\n",checks);return 0;
}catch(const std::exception&e){fprintf(stderr,"FAIL %s\n",e.what());return 1;}}}
