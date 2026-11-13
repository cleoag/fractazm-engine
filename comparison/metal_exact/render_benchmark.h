#pragma once
#include "io/image_writer.h"

// Render-only measurements use the same offline kernels and dispatch helpers
// as qualification, but create no oracle, control or comparison pipelines.
// Correctness qualification belongs to a separate process with the same binary,
// library, scene and options. This mode checks faults and exact repeatability.
#include "render_dispatch.h"
using mb3d::metal_exact::gpu::RenderBenchmarkPipelines;

static void renderBenchmark(const Options& o,const mb3d::metal_exact::PreparedExactScene& prepared,
                            double scenePreparationMs,Clock::time_point coldStart) {
    using namespace mb3d::metal_exact;
    const auto scene=prepared.device;
    const uint32_t count=scene.width*scene.height;
    auto t=Clock::now();
    const auto packing=PrepareSurface(prepared);
    const auto shadow=PrepareShadow(prepared);
    const auto ao=PrepareExactAO(prepared,o.threads);
    const auto color=PrepareExactColor(prepared);
    const double shadingPreparationMs=elapsed(t);
    const bool shadows=shadow.enabled!=0;
    const bool split=shadows&&o.shadowMode!="legacy",parallel=shadows&&o.shadowMode=="parallel";
    const bool parallelNormals=o.normalMode=="parallel";
    require(std::filesystem::create_directory(o.output),"cannot exclusively create benchmark output directory");
    t=Clock::now();
    Metal m(o.library,scene.formula_kind,scene.bulb.power,scene.capability_id,o.divisionMode=="restoring",o.precision==mb3d::PrecisionMode::Native);
    const double deviceSetupMs=elapsed(t);
    t=Clock::now();RenderBenchmarkPipelines p(m,o,shadows);const double pipelineMs=elapsed(t);
    t=Clock::now();
    uint64_t deviceBufferBytes=0;
    auto buffer=[&](size_t bytes,const void* data=nullptr) {
        deviceBufferBytes+=bytes;return m.buffer(bytes,data);
    };
    auto cols=buffer(prepared.columns.size()*sizeof(ExactTrig),prepared.columns.data());
    auto rows=buffer(prepared.rows.size()*sizeof(ExactTrig),prepared.rows.data());
    auto pixels=buffer(size_t(count)*sizeof(ExactPixel));
    auto surface=buffer(size_t(count)*sizeof(ExactSurface));
    auto shaded=buffer(size_t(count)*sizeof(ExactSurface));
    id<MTLBuffer> normalSamples=parallelNormals?buffer(size_t(count)*sizeof(ExactNormalSamples)):nil;
    id<MTLBuffer> rays=split?buffer(size_t(count)*sizeof(ExactShadowRay)):nil;
    id<MTLBuffer> traces=parallel?buffer(size_t(count)*sizeof(ExactShadowTrace)):nil;
    auto depth=buffer(size_t(count)*4),temporary=buffer(size_t(count)*4),angles=buffer(size_t(count)*64);
    auto atan=buffer(ao.atan_terms.size()*8,ao.atan_terms.data());
    auto colorScene=buffer(sizeof(color.device),&color.device);
    auto colorCols=buffer(color.columns.size()*sizeof(ExactTrig),color.columns.data());
    auto colorRows=buffer(color.rows.size()*sizeof(ExactTrig),color.rows.data());
    auto rgb=buffer(size_t(count)*sizeof(ExactColorPixel));
    // The normal RGB8 destination is allocated as part of setup, reused for
    // every frame, and populated inside the measured final readback boundary.
    std::vector<uint8_t> image(size_t(count)*3);
    const double allocationMs=elapsed(t);
    const double setupTotalMs=elapsed(coldStart);
    const char* stageNames[]={"primary","surface_packing","shadow_preparation","shadow_traversal",
                              "hard_shadows","ambient_occlusion","final_color","surface_copy"};
    const bool stageActive[]={true,true,split,parallel,shadows,true,true,!shadows};
    double maxPrimaryCommandMs=0,maxShadowCommandMs=0;
    std::vector<ExactPixel> firstPrimary;
    std::vector<ExactSurface> firstSurface,firstShaded;
    std::vector<ExactColorPixel> firstColors;
    std::vector<ExactShadowRay> firstRays;
    std::vector<ExactShadowTrace> firstTraces;
    std::vector<ExactNormalSamples> firstNormals;
    std::vector<uint8_t> firstImage;
    bool allRGBExact=true,allRecordsExact=true;
    NSMutableArray* warmupReports=[NSMutableArray array];
    NSMutableArray* measuredReports=[NSMutableArray array];
    std::vector<double> gpuTimes,renderTimes,endToEndTimes;

    const mb3d::metal_exact::gpu::RenderStageBuffers renderBuffers{cols,rows,pixels,surface,shaded,normalSamples,rays,traces,depth,temporary,angles,atan,colorScene,colorCols,colorRows,rgb};

    auto frame=[&](NSString* phase,unsigned index,bool cold) {
        const auto frameStart=Clock::now();
        const auto dispatch=mb3d::metal_exact::gpu::RunRenderStages(m,o,scene,packing,shadow,ao,p,
            renderBuffers,maxPrimaryCommandMs,maxShadowCommandMs);
        const auto& gpu=dispatch.gpu;const auto& wall=dispatch.wall;const auto& normalGPU=dispatch.normalGPU;
        const double renderMs=elapsed(frameStart);
        const auto readStart=Clock::now();
        const auto* actualColors=static_cast<const ExactColorPixel*>(rgb.contents);
        for(uint32_t i=0;i<count;++i)for(unsigned c=0;c<3;++c)
            image[size_t(i)*3+c]=uint8_t(actualColors[i].rgb>>(c*8));
        const double readbackMs=elapsed(readStart),renderAndReadbackMs=elapsed(frameStart);
        const double coldTotalMs=cold?elapsed(coldStart):0;
        // From here onward no comparison, fault scan, allocation for retained
        // validation records or image encoding is included in frame timing.
        const auto validationStart=Clock::now();
        const auto* actualPrimary=static_cast<const ExactPixel*>(pixels.contents);
        const auto* actualSurface=static_cast<const ExactSurface*>(surface.contents);
        const auto* actualShaded=static_cast<const ExactSurface*>(shaded.contents);
        uint64_t primaryFaults=0,surfaceFaults=0,finalSurfaceFaults=0,colorFaults=0,rayFaults=0,traceFaults=0;
        uint64_t normalFaults=0,hits=0,shadowChanges=0,aoChanges=0,latentTraceFaults=0;
        uint64_t nonfinitePrimary=0;
        for(uint32_t i=0;i<count;++i) {
            if(actualPrimary[i].fault && primaryFaults<5)
                fprintf(stderr,"primary fault pixel=%u code=%u depth=%.9g de=%.9g normal=(%d,%d,%d)\n",i,
                    actualPrimary[i].fault,bd(actualPrimary[i].depth),bd(actualPrimary[i].final_de),
                    actualPrimary[i].normal[0],actualPrimary[i].normal[1],actualPrimary[i].normal[2]);
            primaryFaults+=actualPrimary[i].fault!=0||actualPrimary[i].reserved_tail!=0;
            const auto& pixel=actualPrimary[i];
            auto finiteDouble=[&](uint64_t value){nonfinitePrimary+=!std::isfinite(bd(value));};
            for(unsigned axis=0;axis<3;++axis) {
                finiteDouble(pixel.ray[axis]);finiteDouble(pixel.start[axis]);
                finiteDouble(pixel.position[axis]);finiteDouble(pixel.raw_normal[axis]);
            }
            for(auto value:{pixel.depth,pixel.axial_depth,pixel.final_de,pixel.orbit_trap})finiteDouble(value);
            for(auto value:{pixel.step_count,pixel.smooth_iterations,pixel.final_stop})
                nonfinitePrimary+=!std::isfinite(std::bit_cast<float>(value));
            const auto& s=actualSurface[i];const auto& a=actualShaded[i];const auto& c=actualColors[i];
            surfaceFaults+=s.fault!=0||s.reserved[0]||s.reserved[1]||s.reserved[2];
            finalSurfaceFaults+=a.fault!=0||a.reserved[0]||a.reserved[1]||a.reserved[2];
            colorFaults+=c.fault!=0||c.reserved0||c.reserved1;
            hits+=actualPrimary[i].rm_result!=0;
            // Shadow kernels retain amb_shadow, and AO retains shadow.
            shadowChanges+=s.shadow!=a.shadow;aoChanges+=s.amb_shadow!=a.amb_shadow;
            if(split) {const auto& r=static_cast<const ExactShadowRay*>(rays.contents)[i];rayFaults+=r.fault!=0||r.reserved[0]||r.reserved[1];}
            if(parallel) {const auto& r=static_cast<const ExactShadowTrace*>(traces.contents)[i];traceFaults+=r.fault!=0||r.reserved[0]||r.reserved[1]||r.reserved[2];latentTraceFaults+=r.terminal_fault!=0;}
            if(parallelNormals)normalFaults+=static_cast<const ExactNormalSamples*>(normalSamples.contents)[i].reserved!=0;
        }
        require(primaryFaults+surfaceFaults+finalSurfaceFaults+colorFaults+rayFaults+traceFaults+normalFaults+nonfinitePrimary==0,
                "benchmark frame contains a faulted or invalid stage record: primary="+std::to_string(primaryFaults)+
                " surface="+std::to_string(surfaceFaults)+" final="+std::to_string(finalSurfaceFaults)+
                " color="+std::to_string(colorFaults)+" rays="+std::to_string(rayFaults)+
                " traces="+std::to_string(traceFaults)+" normals="+std::to_string(normalFaults)+
                " nonfinite_primary="+std::to_string(nonfinitePrimary));
        bool recordsExact=true,rgbExact=true;
        if(cold) {
            firstPrimary=readBuffer<ExactPixel>(pixels);firstSurface=readBuffer<ExactSurface>(surface);
            firstShaded=readBuffer<ExactSurface>(shaded);firstColors=readBuffer<ExactColorPixel>(rgb);firstImage=image;
            if(split)firstRays=readBuffer<ExactShadowRay>(rays);
            if(parallel)firstTraces=readBuffer<ExactShadowTrace>(traces);
            if(parallelNormals)firstNormals=readBuffer<ExactNormalSamples>(normalSamples);
        } else {
            recordsExact=std::memcmp(pixels.contents,firstPrimary.data(),pixels.length)==0&&
                std::memcmp(surface.contents,firstSurface.data(),surface.length)==0&&
                std::memcmp(shaded.contents,firstShaded.data(),shaded.length)==0&&
                std::memcmp(rgb.contents,firstColors.data(),rgb.length)==0;
            if(split)recordsExact&=std::memcmp(rays.contents,firstRays.data(),rays.length)==0;
            if(parallel)recordsExact&=std::memcmp(traces.contents,firstTraces.data(),traces.length)==0;
            if(parallelNormals)recordsExact&=std::memcmp(normalSamples.contents,firstNormals.data(),normalSamples.length)==0;
            rgbExact=image==firstImage;allRecordsExact&=recordsExact;allRGBExact&=rgbExact;
        }
        bool nonconstant=false;
        for(size_t i=3;i<image.size();i+=3) if(image[i]!=image[0]||image[i+1]!=image[1]||image[i+2]!=image[2]) {nonconstant=true;break;}
        const double validationMs=elapsed(validationStart);
        char filename[64];
        if(cold)std::snprintf(filename,sizeof(filename),"cold.png");
        else std::snprintf(filename,sizeof(filename),"%s-%03u.png",[phase UTF8String],index);
        const auto saveStart=Clock::now();
        require(mb3d::WritePNG((o.output/filename).string(),image.data(),int(scene.width),int(scene.height),3),"benchmark PNG write failed");
        const double saveMs=elapsed(saveStart);
        require(recordsExact&&rgbExact,"benchmark frame differs from the cold frame");
        NSMutableDictionary* stages=[NSMutableDictionary dictionary],*gpuStages=[NSMutableDictionary dictionary];
        double gpuTotal=0;
        for(unsigned k=0;k<8;++k) {
            gpuTotal+=gpu[k];gpuStages[ns(stageNames[k])]=@(gpu[k]);
            stages[ns(stageNames[k])]=@{@"state":stageActive[k]?@"completed":@"not_run",@"seconds":@(wall[k]/1000),@"gpu_ms":@(gpu[k])};
        }
        NSMutableDictionary* result=[@{@"phase":phase,@"index":@(index),@"image":ns(filename),
            @"width":@(scene.width),@"height":@(scene.height),@"render_ms":@(renderMs),
            @"readback_ms":@(readbackMs),@"render_and_readback_ms":@(renderAndReadbackMs),
            @"gpu_timestamp_ms":@(gpuTotal),@"png_encode_write_ms":@(saveMs),
            @"end_to_end_ms":@(renderAndReadbackMs+saveMs),@"validation_ms":@(validationMs),
            @"exact_rgb_matches_cold":@(rgbExact),@"exact_stage_records_match_cold":@(recordsExact),
            @"nonfinite_primary_values":@(nonfinitePrimary),
            @"stages":stages,@"stages_gpu_ms":gpuStages,
            @"normal_pipeline_gpu_ms":@{@"geometry":@(normalGPU[0]),@"probes":@(normalGPU[1]),@"finish":@(normalGPU[2])},
            @"activity":@{@"hits":@(hits),@"misses":@(count-hits),@"nonconstant_rgb":@(nonconstant),
                @"shadow_changed_pixels":@(shadowChanges),@"ao_changed_pixels":@(aoChanges)},
            @"faults":@{@"primary":@(primaryFaults),@"surface":@(surfaceFaults),@"final_surface":@(finalSurfaceFaults),
                @"color":@(colorFaults),@"shadow_rays":@(rayFaults),@"shadow_traces":@(traceFaults),@"normal_scratch":@(normalFaults),
                @"latent_shadow_terminal_faults":@(latentTraceFaults)}} mutableCopy];
        if(cold) {result[@"cold_total_ms"]=@(coldTotalMs);result[@"cold_end_to_end_ms"]=@(coldTotalMs+saveMs);}
        else if([phase isEqualToString:@"warm"]) {
            gpuTimes.push_back(gpuTotal);renderTimes.push_back(renderAndReadbackMs);endToEndTimes.push_back(renderAndReadbackMs+saveMs);
        }
        return result;
    };
    auto cold=frame(@"cold",0,true);
    for(unsigned i=0;i<o.warmups;++i)[warmupReports addObject:frame(@"warmup",i,false)];
    for(unsigned i=0;i<o.repeats;++i)[measuredReports addObject:frame(@"warm",i,false)];
    NSDictionary* report=@{@"schema":@"mb3d.metal-render-benchmark.v1",@"status":@"COMPLETE",@"backend":@"metal",
        @"precision":ns(mb3d::PrecisionModeName(o.precision)),
        @"execution":@{@"precision":ns(mb3d::PrecisionModeName(o.precision)),
            @"shader_precision":o.precision==mb3d::PrecisionMode::Native?@"hardware_float32":@"software_binary64_extended80",
            @"cpu_preparation_precision":ns(mb3d::PrecisionModeName(o.precision))},
        @"library_precision":ns(mb3d::PrecisionModeName(o.precision)),
        @"library":ns(std::filesystem::absolute(o.library).string()),
        @"library_profile":@{@"profile_id":@(m.arithmeticProfile[0]),@"scene_abi_version":@(m.arithmeticProfile[1]),@"arithmetic_bits":@(m.arithmeticProfile[2])},
        @"profile_validation_dispatches":@1,@"numeric_profile":numericProfile(prepared,true),@"scene_description":sceneDescription(prepared),
        @"scene":ns(std::filesystem::absolute(o.scene).string()),@"device":m.device.name,
        @"native_arm64":@YES,@"translated":@NO,@"cpu_compute_fallback_calls":@0,@"cpu_oracle_renders":@0,@"diagnostic_dispatches":@0,
        @"settings":@{@"width":@(o.width),@"height":@(o.height),@"threads":@(o.threads),@"warmups":@(o.warmups),@"repeats":@(o.repeats),
            @"hard_shadows_override":@(o.hardShadows),@"shadow_mode":ns(o.shadowMode),@"division_mode":ns(o.divisionMode),@"normal_mode":ns(o.normalMode),
            @"primary_rows_per_command":@(o.primaryRows),@"primary_threads":@(o.primaryThreads),@"primary_tile_width":@(o.primaryTileWidth),
            @"normal_threads":@(o.normalThreads),@"shadow_rows_per_command":@(o.shadowRows),@"shadow_prep_threads":@(o.shadowPrepThreads),@"shadow_trace_threads":@(o.shadowTraceThreads)},
        @"loaded_features":@{@"hard_shadows":@(shadows),@"ambient_occlusion":@YES,@"ao_mode":@"HiQ",@"de_normals":@YES,
            @"zbuffer_normals":@NO,@"reflections_transparency":@NO,@"volume":@NO,@"dof":@NO,
            @"formula_slots":@1,@"shadow_light_mask":@(shadow.light_mask),@"ao_levels":@(ao.device.num_levels)},
        @"hardware":@{@"unified_memory":@(m.device.hasUnifiedMemory),@"registry_id":@(m.device.registryID),
            @"max_buffer_length":@(m.device.maxBufferLength),@"recommended_max_working_set_size":@(m.device.recommendedMaxWorkingSetSize)},
        @"setup_ms":@{@"scene_preparation":@(scenePreparationMs),@"shading_preparation":@(shadingPreparationMs),
            @"device_library_queue":@(deviceSetupMs),@"pipeline_creation":@(pipelineMs),@"buffer_allocation":@(allocationMs),@"total":@(setupTotalMs)},
        @"render_pipeline_names":p.names,@"device_buffer_bytes":@(deviceBufferBytes),@"cold":cold,
        @"warmups":warmupReports,@"samples":measuredReports,
        @"warm_medians_ms":@{@"gpu_timestamp":@(median(gpuTimes)),@"render_and_readback":@(median(renderTimes)),@"end_to_end":@(median(endToEndTimes))},
        @"max_command_gpu_ms":@{@"primary":@(maxPrimaryCommandMs),@"hard_shadows":@(maxShadowCommandMs)},
        @"exact_rgb_repeats":@(allRGBExact),@"exact_stage_repeats":@(allRecordsExact),
        @"timing_scope":@"Cold begins before M3P loading/preparation and ends at final RGB8 readback in a fresh process/context; OS/driver caches are not purged. Warm frames reuse prepared tables, render pipelines and buffers. Frame render includes command encoding/submission/waits; readback includes conversion to RGB8. PNG encode/write is measured separately. End-to-end is render/readback plus PNG time (cold additionally includes setup), excluding fault/repeat scans, retained validation snapshots and report serialization. Parent process wall time includes all work.",
        @"validation_scope":@"Fault/reserved fields and byte-exact repeats of primary, packed/final surface, RGB and selected ray/normal records are checked after each frame outside timings. No independent arithmetic, original-fixture or CPU render oracle runs here; compatible requires separate qualification; native permits cross-profile differences and is not exact parity. A setup-only arithmetic-profile marker dispatch binds the requested offline library. Latent shadow trace terminal faults are resolved by the ordered shadow stage and are reported separately.",
        @"memory_scope":@"Device buffer bytes cover render resources only. Process peak RSS additionally includes host preparation, RGB output, cold-frame validation snapshots and PNG/report work."};
    writeJSON(o.output/"report.json",report);
    fprintf(stderr,"Metal render benchmark: COMPLETE; cold %.3f ms; warm render/readback %.3f ms\n",
            [cold[@"cold_total_ms"] doubleValue],median(renderTimes));
}
