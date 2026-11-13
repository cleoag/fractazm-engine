#include "navigation_preview.h"
#include "comparison/metal_exact/metal_device.h"
#include "comparison/metal_exact/scene_host.h"
#include "core/numeric_precision.h"
#include <cstring>
namespace mb3d::native_metal {
namespace gpu=metal_exact::gpu;
struct NavigationPreviewSession::Impl {
    std::string library,key;
    std::unique_ptr<gpu::Metal> metal;
    metal_exact::PreparedExactScene scene;
    id<MTLComputePipelineState> pipeline;
    id<MTLBuffer> rgb,records,columns,rows;
    uint32_t nextPhase=0;
    uint64_t covered=0,devices=0,preparations=0;
    bool active=false;
    explicit Impl(std::string path):library(std::move(path)){}
    id<MTLBuffer> buffer(id<MTLBuffer> previous,size_t size,const void* data=nullptr) {
        if(!previous||previous.length!=size)previous=metal->buffer(size);
        if(data)std::memcpy(previous.contents,data,size);
        return previous;
    }
};
NavigationPreviewSession::NavigationPreviewSession(std::string path):impl(std::make_unique<Impl>(std::move(path))){}
NavigationPreviewSession::~NavigationPreviewSession()=default;
uint64_t NavigationPreviewSession::coveredPixels() const{return impl->covered;}
uint64_t NavigationPreviewSession::deviceCreations() const{return impl->devices;}
uint64_t NavigationPreviewSession::scenePreparations() const{return impl->preparations;}
NativeMetalAttempt NavigationPreviewSession::render(const TMandHeader10& header,const THeaderCustomAddon& addon,
    const RenderOptions& options,const NavigationPreviewOptions& preview,bool accumulate) {
    @autoreleasepool {
        auto& state=*impl;
        NativeMetalAttempt result;result.library=state.library;
        const auto oldDevices=state.devices;
        const uint64_t oldHits=state.metal?state.metal->pipelineArchiveHits:0,oldMisses=state.metal?state.metal->pipelineArchiveMisses:0;
        try {
            const unsigned w=options.width_override>0?options.width_override:header.Width;
            const unsigned h=options.height_override>0?options.height_override:header.Height;
            const unsigned block=preview.pattern==NavigationPattern::Dense?1:preview.block_size;
            if(options.precision!=PrecisionMode::Native||!w||!h||w>1600||h>1600||
               (block!=1&&block!=2&&block!=4)||unsigned(preview.pattern)>2||options.mc_samples||options.control||
               (accumulate&&preview.phase>=block*block))
                throw std::invalid_argument("Unsupported navigation preview request");
            // Private in-process identity, not a wire format or persistent hash.
            std::string key(reinterpret_cast<const char*>(&header),sizeof(header));
            key.append(reinterpret_cast<const char*>(&addon),sizeof(addon));
            const uint32_t config[]={w,h,block,uint32_t(preview.pattern),preview.seed};
            key.append(reinterpret_cast<const char*>(config),sizeof(config));key+=options.resource_root;
            ScopedNumericPrecision precision(PrecisionMode::Native);
            if(accumulate&&preview.phase) {
                if(!state.active||state.key!=key||state.nextPhase!=preview.phase)
                    throw std::invalid_argument("Accumulation camera/options changed or phase is not consecutive; restart at phase 0");
            } else {
                state.active=false;state.covered=0;
                auto scene=metal_exact::PrepareLoadedExactScene(header,addon,w,h,options.resource_root);
                ++state.preparations;
                if(!state.metal||state.metal->formulaKind!=scene.device.formula_kind||
                   state.metal->bulbPower!=scene.device.bulb.power||state.metal->capabilityId!=scene.device.capability_id) {
                    state.rgb=nil;state.records=nil;state.columns=nil;state.rows=nil;state.pipeline=nil;
                    state.metal=std::make_unique<gpu::Metal>(state.library,scene.device.formula_kind,scene.device.bulb.power,scene.device.capability_id,false,true);
                    auto abi=state.metal->buffer(3*sizeof(uint32_t));
                    gpu::dispatchStage(*state.metal,state.metal->pipeline(@"navigation_preview_abi"),1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:abi offset:0 atIndex:0];});
                    const auto* words=static_cast<const uint32_t*>(abi.contents);
                    gpu::require(words[0]==2&&words[1]==9&&words[2]==1,"Unsupported navigation accumulation ABI");
                    state.pipeline=state.metal->pipeline(@"navigation_points");++state.devices;
                }
                state.scene=std::move(scene);state.key=std::move(key);
                state.columns=state.buffer(state.columns,state.scene.columns.size()*sizeof(ExactTrig),state.scene.columns.data());
                state.rows=state.buffer(state.rows,state.scene.rows.size()*sizeof(ExactTrig),state.scene.rows.data());
            }
            auto& metal=*state.metal;
            result.device=metal.device.name.UTF8String;
            const uint32_t count=((w+block-1)/block)*((h+block-1)/block);
            uint32_t cfg[]={w,h,block,count,uint32_t(preview.pattern),preview.phase,preview.seed,0,uint32_t(accumulate)};
            state.rgb=state.buffer(state.rgb,size_t(w)*h*4);
            state.records=state.buffer(state.records,size_t(count)*16);
            constexpr uint32_t batch=262144;
            for(uint32_t begin=0;begin<count;begin+=batch) {
                cfg[7]=begin;
                gpu::dispatchStage(metal,state.pipeline,std::min(batch,count-begin),[&](id<MTLComputeCommandEncoder> encoder){
                    [encoder setBuffer:state.rgb offset:0 atIndex:0];[encoder setBuffer:state.records offset:0 atIndex:1];
                    [encoder setBuffer:state.columns offset:0 atIndex:2];[encoder setBuffer:state.rows offset:0 atIndex:3];
                    [encoder setBytes:&state.scene.device length:sizeof(state.scene.device) atIndex:4];
                    [encoder setBytes:cfg length:sizeof(cfg) atIndex:5];
                });
            }
            uint64_t samples=0;
            const auto* words=static_cast<const uint32_t*>(state.records.contents);
            for(uint32_t i=0;i<count;++i) {
                const uint32_t cx=i%((w+block-1)/block),cy=i/((w+block-1)/block);
                const uint32_t area=std::min(block,w-cx*block)*std::min(block,h-cy*block);
                const uint32_t executed=(!accumulate||preview.phase<area)?1:0;
                gpu::require(words[i*4]==0&&words[i*4+3]==executed,"Navigation primary sample fault at sample "+std::to_string(i));
                gpu::require(words[i*4+1]>=cx*block&&words[i*4+1]<std::min(w,(cx+1)*block)&&
                    words[i*4+2]>=cy*block&&words[i*4+2]<std::min(h,(cy+1)*block),"Navigation sample ownership fault");
                samples+=executed;
            }
            state.covered=accumulate?state.covered+samples:samples;
            state.nextPhase=preview.phase+1;state.active=accumulate;
            auto& frame=result.result;frame.width=w;frame.height=h;frame.image.resize(size_t(w)*h*3);
            const auto* pixels=static_cast<const uint32_t*>(state.rgb.contents);
            for(size_t i=0;i<size_t(w)*h;++i)for(unsigned c=0;c<3;++c)frame.image[i*3+c]=uint8_t(pixels[i]>>(8*c));
            frame.success=true;frame.status=RenderStatus::Completed;result.status=NativeMetalStatus::Completed;
            result.execution.engine="navigation_points";result.execution.validated=true;
            result.execution.primary_records=samples;result.execution.color_records=samples;
            result.execution.pipeline_archive_hits=metal.pipelineArchiveHits-(state.devices==oldDevices?oldHits:0);
            result.execution.pipeline_archive_misses=metal.pipelineArchiveMisses-(state.devices==oldDevices?oldMisses:0);
        } catch(const std::invalid_argument& e) {
            state.active=false;state.covered=0;
            result.status=NativeMetalStatus::Unsupported;result.reason=e.what();result.result.error=e.what();
        } catch(const std::exception& e) {
            state.active=false;state.covered=0;
            state.pipeline=nil;state.rgb=nil;state.records=nil;state.columns=nil;state.rows=nil;state.metal.reset();
            result.status=NativeMetalStatus::Failed;result.reason=e.what();result.result.error=e.what();
            result.result.image.clear();result.result.success=false;
        }
        return result;
    }
}
NativeMetalAttempt RenderNavigationPreview(const TMandHeader10& header,const THeaderCustomAddon& addon,
    const RenderOptions& options,const NavigationPreviewOptions& preview,const std::string& library) {
    NavigationPreviewSession session(library);
    return session.render(header,addon,options,preview,false);
}
}
