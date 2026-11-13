#include "general_navigation.h"
#include <CommonCrypto/CommonDigest.h>
#include <chrono>
#include "resource_requirements.h"
#include "src/render/native_metal/general_scene_host.h"
#include "src/render/native_metal/pixel_independence.h"
#include "src/render/native_metal/pixel_state_contract.h"
#include "src/render/native_metal/general_normals_host.h"
#include "src/render/native_metal/native_inside_host.h"
#include "src/render/native_metal/wrapper_policy.h"
#include "comparison/metal_exact/metal_device.h"
#include "comparison/metal_exact/scene_host_internal.h"
namespace mb3d::native_metal {
namespace gpu=metal_exact::gpu;
struct GeneralNavigationSession::Impl {
 double gpuSeconds=0,maxCommandSeconds=0,prepareSeconds=0,compileSeconds=0,gpuBegin=0,gpuEnd=0;
 std::array<double,4> preparationTimes{};
 std::string resourceKey,resourceIdentity; uint64_t uploads=0; bool resourcesValid=false;
 std::string path,key;PreparedGeneralScene scene;std::unique_ptr<gpu::Metal> gpu;
 id<MTLComputePipelineState> pipeline=nil,displayPipeline=nil;
 id<MTLBuffer> descriptor=nil,program=nil,maps=nil,pixels=nil,columns=nil,rows=nil,states=nil,records=nil,rgb=nil,display=nil;
 mb3d_native::NativeNormalParams normal{};mb3d_inside::Params inside{};
 unsigned next=0,block=32,workers=4,samplesPerPhase=1;bool independent=false,balanced=false,forceOrdered=false;uint64_t covered=0,range=0,devices=0,preparations=0;bool valid=false;
 Impl(std::string p,bool ordered):path(std::move(p)),forceOrdered(ordered){}
};
GeneralNavigationSession::GeneralNavigationSession(std::string p,bool ordered):impl(std::make_unique<Impl>(std::move(p),ordered)){}
GeneralNavigationSession::~GeneralNavigationSession()=default;
void GeneralNavigationSession::reset(){impl->valid=false;}
bool GeneralNavigationSession::active()const{return impl->valid;}
uint64_t GeneralNavigationSession::coveredPixels()const{return impl->covered;}
double GeneralNavigationSession::gpuSeconds()const{return impl->gpuSeconds;}
double GeneralNavigationSession::maxCommandSeconds()const{return impl->maxCommandSeconds;}
uint64_t GeneralNavigationSession::deviceCreations()const{return impl->devices;}
uint64_t GeneralNavigationSession::scenePreparations()const{return impl->preparations;}
uint64_t GeneralNavigationSession::resourceUploads()const{return impl->uploads;}
std::string GeneralNavigationSession::resourceIdentity()const{return impl->resourceIdentity;}
std::array<double,4> GeneralNavigationSession::preparationTimes()const{return impl->preparationTimes;}
double GeneralNavigationSession::prepareSeconds()const{return impl->prepareSeconds;}
double GeneralNavigationSession::compileSeconds()const{return impl->compileSeconds;}
double GeneralNavigationSession::gpuBeginSeconds()const{return impl->gpuBegin;}
double GeneralNavigationSession::gpuEndSeconds()const{return impl->gpuEnd;}
unsigned GeneralNavigationSession::phases()const{return 1+(impl->block*impl->block-1+impl->samplesPerPhase-1)/impl->samplesPerPhase;}
NativeMetalAttempt GeneralNavigationSession::render(const TMandHeader10& h,const THeaderCustomAddon& a,const RenderOptions& opts,unsigned phase,const std::function<bool()>& superseded,bool fast_step,uint64_t scene_revision,bool reuse_preparation){
 @autoreleasepool {
 NativeMetalAttempt out;auto& s=*impl;s.gpuSeconds=s.maxCommandSeconds=s.prepareSeconds=s.compileSeconds=s.gpuBegin=s.gpuEnd=0;out.library=s.path;s.preparationTimes={};
 bool controlStarted=false;
 try{
  auto poll=[&]{
   if(superseded&&superseded()){
    if(opts.control)opts.control->cancel();else throw RenderStopped(RenderStatus::Cancelled);
   }
   if(opts.control)opts.control->poll();
  };
  if(opts.control){opts.control->start();controlStarted=true;}
  poll();
  ScopedNumericPrecision precision(PrecisionMode::Native);
  uint64_t hits=s.gpu?s.gpu->pipelineArchiveHits:0,misses=s.gpu?s.gpu->pipelineArchiveMisses:0;
  const unsigned w=opts.width_override>0?opts.width_override:h.Width,ht=opts.height_override>0?opts.height_override:h.Height;
  gpu::require(opts.precision==PrecisionMode::Native&&w&&ht&&w<=1600&&ht<=1600&&!opts.mc_samples,"Invalid general navigation request");
  std::string key(reinterpret_cast<const char*>(&h),sizeof(h));key.append(reinterpret_cast<const char*>(&a),sizeof(a));
  key.append(reinterpret_cast<const char*>(&opts.thread_count),sizeof(opts.thread_count));key+=opts.resource_root;key.append(reinterpret_cast<const char*>(&w),sizeof(w));key.append(reinterpret_cast<const char*>(&ht),sizeof(ht));
  key.push_back(fast_step?'1':'0');key.append(reinterpret_cast<const char*>(&scene_revision),sizeof(scene_revision));
  if(phase){gpu::require(s.valid&&key==s.key&&phase==s.next,"General navigation snapshot/phase mismatch");}
  else{
   s.valid=false;s.covered=s.range=0;
   auto header=h;header.Iterations=std::min(header.Iterations,64);header.MinimumIterations=std::min<unsigned>(header.MinimumIterations,header.Iterations);header.bColorOnIt=0;header.bVolLightNr=0;
   // Optional Pascal Navigator non-HiQual step. Ordinary Points retains authored
   // marching settings: the aggressive step can introduce extra range exits.
   if(fast_step){
    header.mZstepDiv=.7f;header.sRaystepLimiter=std::max(header.sRaystepLimiter,.35f);
    header.sDEstop=std::max(header.sDEstop,.01f);header.bStepsafterDEStop=4;
   }
   s.workers=std::clamp(unsigned(opts.thread_count?opts.thread_count:4),1u,64u);
   // Only decoded map data is reused. All MCT/program/camera/marching values
   // are regenerated, including step-width-dependent DE scales and offsets.
   auto immutable=header;
   immutable.dXmid=immutable.dYmid=immutable.dZmid=immutable.dZstart=immutable.dZend=0;
   memset(immutable.hVGrads,0,sizeof(immutable.hVGrads));
   std::string resourceKey(reinterpret_cast<const char*>(&immutable),sizeof(immutable));
   resourceKey.append(reinterpret_cast<const char*>(&a),sizeof(a));
   resourceKey.append(reinterpret_cast<const char*>(&scene_revision),sizeof(scene_revision));
   resourceKey+=opts.resource_root;
   const bool retained=reuse_preparation&&scene_revision&&s.resourcesValid&&resourceKey==s.resourceKey&&
       CaptureRenderContext()->maps.sequences.Count()==0&&CaptureRenderContext()->maps.frame_number==1;
   auto prepareStart=std::chrono::steady_clock::now();
   s.preparationTimes[0]=std::chrono::duration<double>(prepareStart.time_since_epoch()).count();
   auto prepared=PrepareGeneralSceneWithMaps(header,a,w,ht,opts.resource_root,retained?&s.scene.formulas:nullptr);
   s.prepareSeconds=std::chrono::duration<double>(std::chrono::steady_clock::now()-prepareStart).count();
   s.preparationTimes[1]=s.preparationTimes[0]+s.prepareSeconds;
   if(!retained) {
       ++s.preparations;
       CC_SHA256_CTX digest;CC_SHA256_Init(&digest);
       const auto& f=prepared.formulas;
       CC_SHA256_Update(&digest,f.maps.data(),CC_LONG(f.maps.size()*sizeof(mb3d_native::NativeMapDescriptor)));
       CC_SHA256_Update(&digest,f.map_pixels.data(),CC_LONG(f.map_pixels.size()*sizeof(float)));
       unsigned char bytes[CC_SHA256_DIGEST_LENGTH];CC_SHA256_Final(bytes,&digest);
       const char* hex="0123456789abcdef";s.resourceIdentity.clear();
       for(auto b:bytes){s.resourceIdentity+=hex[b>>4];s.resourceIdentity+=hex[b&15];}
   }
   if(fast_step){
    const unsigned refinements=prepared.formulas.device.de_option==20?7:4;
    prepared.base.host->mct.iDEAddSteps=refinements;prepared.base.device.refinement_steps=refinements;
   }
   gpu::require(prepared.base.device.width==w&&prepared.base.device.height==ht,"Tiled navigation not implemented");
   std::array<uint32_t,6> ids{};
   for(unsigned i=0;i<6;++i){auto& f=prepared.formulas.device.slots[i];ids[i]=mb3d_native::FormulaPipelineSelector(f.namespace_id,f.id,f.enabled);}
   auto& p=prepared.formulas.device;
   bool reuse=s.gpu&&s.gpu->nativeFormulaIds==ids&&s.gpu->nativeOrbitDriver==p.driver&&s.gpu->nativeFormulaType==p.formula_type&&
    s.gpu->nativeWrapperPolicy==mb3d_native::ProductionWrapperPolicy(p)&&
    (!p.formula_type||s.gpu->nativeOrbitDriver2==prepared.formulas.second.driver);
   auto compileStart=std::chrono::steady_clock::now();
   if(!reuse){
    s.preparationTimes[2]=std::chrono::duration<double>(compileStart.time_since_epoch()).count();
    hits=misses=0;
    s.gpu=std::make_unique<gpu::Metal>(s.path,kGeneralFormulaKind,0,kGeneralCapability,false,true,&ids,p.driver,gpu::DispatchObserver{},1,true,mb3d_native::ProductionWrapperPolicy(p));
    s.gpu->nativeFormulaType=p.formula_type;if(p.formula_type)s.gpu->nativeOrbitDriver2=prepared.formulas.second.driver;
    for(auto id:ids)if(id==10006u||id==10189u||id==10198u)s.gpu->nativeOutlineDE=1;
    auto abi=s.gpu->buffer(16);gpu::dispatchStage(*s.gpu,s.gpu->pipeline(@"general_navigation_abi"),1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:abi offset:0 atIndex:0];});
    const uint32_t expected[]={5,sizeof(mb3d_native::NativeFormulaState),sizeof(ExactScene),sizeof(mb3d_native::NativeFormulaProgram)};
    gpu::require(!memcmp(abi.contents,expected,16),"General navigation shader ABI mismatch");s.pipeline=s.gpu->pipeline(@"general_navigation");
    s.displayPipeline=s.gpu->pipeline(@"general_navigation_display");++s.devices;
   }
   s.compileSeconds=reuse?0:std::chrono::duration<double>(std::chrono::steady_clock::now()-compileStart).count();
   if(!reuse)s.preparationTimes[3]=s.preparationTimes[2]+s.compileSeconds;
   // Account for both retained and incoming CPU/GPU copies during replacement.
   // Commands complete synchronously; published frames own independent RGB bytes.
   const uint64_t resourceBytes=prepared.formulas.maps.size()*sizeof(mb3d_native::NativeMapDescriptor)+prepared.formulas.map_pixels.size()*sizeof(float);
   const uint64_t oldCpu=s.scene.formulas.maps.size()*sizeof(mb3d_native::NativeMapDescriptor)+s.scene.formulas.map_pixels.size()*sizeof(float);
   uint64_t oldGpu=0;
   for(id<MTLBuffer> b:@[s.descriptor?:[NSNull null],s.program?:[NSNull null],s.maps?:[NSNull null],s.pixels?:[NSNull null],s.columns?:[NSNull null],s.rows?:[NSNull null],s.states?:[NSNull null],s.records?:[NSNull null],s.rgb?:[NSNull null],s.display?:[NSNull null]])
       if((id)b!=NSNull.null)oldGpu+=b.length;
   const uint64_t mutableBytes=sizeof(ExactScene)+(prepared.camera.columns.size()+prepared.camera.rows.size())*sizeof(mb3d_camera::Axis)+s.workers*sizeof(mb3d_native::NativeFormulaState)+uint64_t(w)*ht*8+uint64_t((w+s.block-1)/s.block)*((ht+s.block-1)/s.block)*16*16;
   gpu::require(oldCpu+oldGpu+2*resourceBytes+prepared.formulas.program_count()*sizeof(mb3d_native::NativeFormulaProgram)+mutableBytes<=NativeDeviceBudget(s.gpu->device.recommendedMaxWorkingSetSize),"General navigation retained resource budget exceeded");
   s.scene=std::move(prepared);s.key=key;const auto& program=s.scene.formulas.device;
   const auto contract=mb3d_native::AnalyzePrimaryPixelState(program,program.formula_type?&s.scene.formulas.second:nullptr);
   // Reuse the selected engine's proved generic entry contract. Conditional
   // dIFS contracts still require ordered lanes; no speculative frame is shown.
   const bool admitted=mb3d_native::IndependentPrimaryPixels(program)||(contract.independent&&!contract.conditional);
   s.independent=!s.forceOrdered&&admitted;s.balanced=admitted;s.samplesPerPhase=admitted?16:1;
   s.normal=PrepareGeneralNormals(s.scene.base.host->mct,s.scene.formulas.device);s.normal.smoothing=0;
   s.inside=PrepareNativeInside(s.scene.base.host->mct);
   auto buffer=[&](size_t n,const void* data){auto b=s.gpu->buffer(std::max(size_t(4),n));if(n)memcpy(b.contents,data,n);return b;};
   s.descriptor=buffer(sizeof(ExactScene),&s.scene.base.device);
   std::vector<mb3d_native::NativeFormulaProgram> programs{s.scene.formulas.device};if(s.scene.formulas.program_count()==2)programs.push_back(s.scene.formulas.second);
   const auto programBytes=programs.size()*sizeof(programs[0]);
   if(!reuse||!retained||!s.program||s.program.length!=programBytes||memcmp(s.program.contents,programs.data(),programBytes))
       s.program=buffer(programBytes,programs.data());
   if(!reuse||!retained) {
       s.maps=buffer(s.scene.formulas.maps.size()*sizeof(mb3d_native::NativeMapDescriptor),s.scene.formulas.maps.data());
       s.pixels=buffer(s.scene.formulas.map_pixels.size()*sizeof(float),s.scene.formulas.map_pixels.data());
       ++s.uploads;
   }
   s.resourceKey=std::move(resourceKey);s.resourcesValid=true;
   s.columns=buffer(s.scene.camera.columns.size()*sizeof(mb3d_camera::Axis),s.scene.camera.columns.data());
   s.rows=buffer(s.scene.camera.rows.size()*sizeof(mb3d_camera::Axis),s.scene.camera.rows.data());
   s.states=s.gpu->buffer(s.workers*sizeof(mb3d_native::NativeFormulaState));
   s.rgb=s.gpu->buffer(size_t(w)*ht*4);s.display=s.gpu->buffer(size_t(w)*ht*4);s.records=s.gpu->buffer(((w+s.block-1)/s.block)*((ht+s.block-1)/s.block)*16*s.samplesPerPhase);
  }
  gpu::require(phase<phases(),"General navigation phase out of bounds");
  const unsigned cellsX=(w+s.block-1)/s.block,cellsY=(ht+s.block-1)/s.block,cells=cellsX*cellsY;
  uint32_t cfg[]={s.block,0,s.workers,uint32_t(s.independent)|(uint32_t(s.balanced)<<1)};const uint32_t counts[]={uint32_t(s.scene.formulas.maps.size()),uint32_t(s.scene.formulas.map_pixels.size())};
  auto& m=*s.gpu;uint64_t traced=0;
  // Small dispatches preserve each logical lane across command boundaries.
  // Ordered owners each advance at most eight cells. Interleave their commands,
  // never their state: a coarse row still belongs to row % workers throughout.
  const unsigned batchLimit=s.independent?4096:8;
  const unsigned begin=phase?1+(phase-1)*s.samplesPerPhase:0;
  const unsigned end=phase?std::min(begin+s.samplesPerPhase,s.block*s.block):1;
  for(unsigned samplePhase=begin;samplePhase<end;){
  cfg[1]=samplePhase;
  const unsigned passes=s.independent?end-begin:1;
  const unsigned workItems=s.independent?cells*passes:((cellsY+s.workers-1)/s.workers)*cellsX;
  for(unsigned first=0;first<workItems;first+=batchLimit){
   uint32_t batch[]={first,std::min(batchLimit,workItems-first),0,0};
   unsigned admitted=batch[1];
   if(!s.independent){
    admitted=0;
    for(unsigned owner=0;owner<std::min(s.workers,cellsY);++owner){
     const unsigned owned=((cellsY-1-owner)/s.workers+1)*cellsX;
     if(first<owned)admitted+=std::min(batch[1],owned-first);
    }
   }
   poll();if(opts.control)opts.control->consume(admitted);
   double timestamps[2]{};
   const double seconds=gpu::dispatchStage(m,s.pipeline,s.independent?batch[1]:s.workers,[&](id<MTLComputeCommandEncoder> e){
    const id<MTLBuffer> buffers[]={s.rgb,s.records,s.descriptor,s.program,s.states};
    for(unsigned i=0;i<5;++i)[e setBuffer:buffers[i] offset:0 atIndex:i];
    [e setBytes:batch length:sizeof(batch) atIndex:5];[e setBuffer:s.maps offset:0 atIndex:6];[e setBuffer:s.pixels offset:0 atIndex:7];
    [e setBytes:cfg length:sizeof(cfg) atIndex:8];[e setBytes:counts length:sizeof(counts) atIndex:9];
    [e setBytes:&s.normal length:sizeof(s.normal) atIndex:10];[e setBytes:&s.inside length:sizeof(s.inside) atIndex:11];
    [e setBytes:&s.scene.camera.device length:sizeof(s.scene.camera.device) atIndex:12];
    [e setBuffer:s.columns offset:0 atIndex:13];[e setBuffer:s.rows offset:0 atIndex:14];
   },1,s.independent?32:1,timestamps)/1000.;
   // Separate stateful owners into different groups, as the production host does.
   // No lane is split or reordered; independent pixels retain SIMD groups.
   if(!s.gpuBegin)s.gpuBegin=timestamps[0];s.gpuEnd=timestamps[1];
   s.gpuSeconds+=seconds;s.maxCommandSeconds=std::max(s.maxCommandSeconds,seconds);
   ++out.execution.primary_dispatches;
   poll();
  }
  for(unsigned pass=0;pass<passes;++pass){
  uint64_t sampleTraced=0;
  const auto* records=static_cast<const uint32_t*>(s.records.contents)+pass*cells*4;
  for(unsigned i=0;i<cells;++i){gpu::require(!records[i*4],"General navigation GPU fault "+std::to_string(records[i*4]));sampleTraced+=records[i*4+2];out.execution.formula_work+=records[i*4+3];s.range+=records[i*4+1];}
  traced+=sampleTraced;s.covered+=sampleTraced;gpu::require(s.range*100<=s.covered,"General navigation FP32 range exceeds 1% of traced pixels");
  }
  samplePhase+=passes;
  }
  // Only interpolate unmeasured pixels for presentation. Counts, sampled RGB,
  // formula work and carried state remain unchanged; completion is an exact copy.
  poll();const uint32_t displayCfg[]={w,ht,s.block,end|(uint32_t(s.balanced)<<31)};double displayTimes[2]{};
  const double displaySeconds=gpu::dispatchStage(m,s.displayPipeline,w*ht,[&](id<MTLComputeCommandEncoder> e){
   [e setBuffer:s.rgb offset:0 atIndex:0];[e setBuffer:s.display offset:0 atIndex:1];
   [e setBytes:displayCfg length:sizeof(displayCfg) atIndex:2];
  },1,32,displayTimes)/1000.;
  s.gpuSeconds+=displaySeconds;s.maxCommandSeconds=std::max(s.maxCommandSeconds,displaySeconds);s.gpuEnd=displayTimes[1];
  s.next=phase+1;s.valid=true;
  auto& r=out.result;r.width=w;r.height=ht;r.image.resize(size_t(w)*ht*3);
  const auto* rgb=static_cast<const uint8_t*>(s.display.contents);for(size_t i=0;i<size_t(w)*ht;++i)memcpy(r.image.data()+i*3,rgb+i*4,3);
  poll();if(opts.control)opts.control->complete();
  r.success=true;r.status=RenderStatus::Completed;out.status=NativeMetalStatus::Completed;
  out.execution.engine=s.independent?"general_navigation_independent":"general_navigation_ordered";out.execution.validated=true;out.execution.primary_records=traced;out.execution.color_records=traced;
  out.execution.pipeline_archive_hits=m.pipelineArchiveHits-hits;out.execution.pipeline_archive_misses=m.pipelineArchiveMisses-misses;
 }catch(const RenderStopped& e){s.valid=false;out.status=NativeMetalStatus::Failed;out.result.status=e.status();out.result.error=e.what();out.reason=e.what();out.result.image.clear();
 }catch(const std::exception& e){if(controlStarted)opts.control->fail(std::current_exception());s.valid=false;out.status=NativeMetalStatus::Failed;out.result.error=e.what();out.reason=e.what();out.result.image.clear();s.gpu.reset();s.resourcesValid=false;}
 return out;
 }
}
}
