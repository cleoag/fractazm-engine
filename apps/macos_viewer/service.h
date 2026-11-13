#pragma once
// Private same-binary transport. Scene structures are copied, never borrowed.
// One request in flight; length bounds are checked before allocation.
#include <cerrno>
#include <vector>
// The signal handler only advances a process-lifetime epoch. It never borrows
// a job pointer; cancellation is polled on the owning thread between commands.
static_assert(std::atomic<unsigned>::is_always_lock_free);
static std::atomic<unsigned> generalCancelEpoch{0};
static bool generalPreviewEnabled(){const char* value=getenv("MB3D_VIEWER_GENERAL_PREVIEW");return value&&std::string(value)=="1";}
static void cancelGeneralSignal(int){generalCancelEpoch.fetch_add(1,std::memory_order_relaxed);}
static bool transfer(int fd,void* data,size_t size,bool writing) {
    auto* p=static_cast<uint8_t*>(data);
    while(size){ssize_t n=writing?write(fd,p,size):read(fd,p,size);if(n<0&&errno==EINTR)continue;if(n<=0)return false;p+=n;size-=n;}
    return true;
}
static bool sendPacket(int fd,NSDictionary* info,const std::vector<uint8_t>& rgb={}) {
    NSData* data=[NSJSONSerialization dataWithJSONObject:info options:0 error:nil];
    if(!data||data.length>1024*1024||rgb.size()>1600*1600*3)return false;
    uint32_t sizes[]={0x4d425633,uint32_t(data.length),uint32_t(rgb.size())};
    return transfer(fd,sizes,sizeof(sizes),true)&&transfer(fd,(void*)data.bytes,data.length,true)&&
        transfer(fd,(void*)rgb.data(),rgb.size(),true);
}
static NSDictionary* receivePacket(int fd,std::vector<uint8_t>& rgb) {
    uint32_t sizes[3];if(!transfer(fd,sizes,sizeof(sizes),false))return nil;
    if(sizes[0]!=0x4d425633||sizes[1]>1024*1024||sizes[2]>1600*1600*3)return nil;
    NSMutableData* data=[NSMutableData dataWithLength:sizes[1]];rgb.resize(sizes[2]);
    if(!transfer(fd,data.mutableBytes,data.length,false)||!transfer(fd,rgb.data(),rgb.size(),false))return nil;
    id value=[NSJSONSerialization JSONObjectWithData:data options:0 error:nil];
    return [value isKindOfClass:NSDictionary.class]?value:nil;
}
static NSDictionary* sceneRequest(const TMandHeader10& h,const THeaderCustomAddon& a,
                                  NSString* root,NSString* backend,NSString* mode,int edge,unsigned phase) {
    return @{@"protocol":@1,@"request_id":@1,@"scene_revision":@1,@"camera_revision":@1,@"quality_revision":@1,@"epoch":@1,@"input_seconds":@0,@"submit_seconds":@0, @"camera_supported":@(viewer::Camera::supports(h)),@"header":[[NSData dataWithBytes:&h length:sizeof(h)] base64EncodedStringWithOptions:0],
        @"addon":[[NSData dataWithBytes:&a length:sizeof(a)] base64EncodedStringWithOptions:0],
        @"root":root,@"backend":backend,@"mode":mode,@"edge":@(edge),@"phase":@(phase)};
}
static NSDictionary* renderRequestBody(NSDictionary* request,mb3d::native_metal::NavigationPreviewSession* session,
                                   std::vector<uint8_t>& rgb,mb3d::native_metal::GeneralNavigationSession* general=nullptr) {
    auto start=std::chrono::steady_clock::now();rgb.clear();
    try {
        for(NSString* key in @[@"header",@"addon",@"root",@"backend",@"mode"])
            if(![request[key] isKindOfClass:NSString.class])throw std::runtime_error("Invalid render request field");
        for(NSString* key in @[@"edge",@"phase"])
            if(![request[key] isKindOfClass:NSNumber.class])throw std::runtime_error("Invalid render request number");
        TMandHeader10 header;THeaderCustomAddon addon;
        NSData* h=[[NSData alloc] initWithBase64EncodedString:request[@"header"]?:@"" options:0];
        NSData* a=[[NSData alloc] initWithBase64EncodedString:request[@"addon"]?:@"" options:0];
        if(h.length!=sizeof(header)||a.length!=sizeof(addon))throw std::runtime_error("Invalid scene snapshot");
        memcpy(&header,h.bytes,sizeof(header));memcpy(&addon,a.bytes,sizeof(addon));
        int edge=[request[@"edge"] intValue];
        if(edge<32||edge>1600||header.Width<=0||header.Height<=0)throw std::runtime_error("Invalid preview size");
        NSString* mode=request[@"mode"];
        if(![@[@"full",@"draft",@"grid",@"points",@"geometry",@"noao",@"fastgeometry"] containsObject:mode])throw std::runtime_error("Invalid quality mode");
        NSString* backendName=request[@"backend"];
        if(![@[@"cpu",@"metal"] containsObject:backendName])throw std::runtime_error("Invalid backend");
        bool fastStep=[mode isEqual:@"fastgeometry"];
        if(fastStep&&(!general||![backendName isEqual:@"metal"]))throw std::runtime_error("Fast geometry requires experimental Metal preview");
        mb3d::RenderOptions opts;opts.precision=mb3d::PrecisionMode::Native;opts.thread_count=request[@"threads"]?[request[@"threads"] intValue]:4;
        if(opts.thread_count<1||opts.thread_count>64)throw std::runtime_error("Invalid worker count");
        double scale=double(edge)/std::max(header.Width,header.Height);
        opts.width_override=std::max(2,int(header.Width*scale));opts.height_override=std::max(2,int(header.Height*scale));
        opts.resource_root=[request[@"root"] UTF8String];
        mb3d::BackendOptions backend;backend.requested=[backendName isEqual:@"cpu"]?mb3d::RenderBackend::CPU:mb3d::RenderBackend::Metal;
        backend.metal_library=resource(@"metal_native_scene.metallib").UTF8String;backend.allow_cpu_fallback=false;
        NSString* effectiveMode=mode;bool sparseUnsupported=false;
        mb3d::BackendRenderResult out;
        bool points=fastStep||[mode isEqual:@"grid"]||[mode isEqual:@"points"]||[mode isEqual:@"geometry"];
        unsigned phase=[request[@"phase"] unsignedIntValue],phases=1;
        uint64_t covered=0;bool usedGeneral=false;
        if(points&&backend.requested==mb3d::RenderBackend::Metal) {
            mb3d::native_metal::NavigationPreviewOptions p;
            p.pattern=[mode isEqual:@"points"]?mb3d::native_metal::NavigationPattern::Stratified:
                ([mode isEqual:@"geometry"]?mb3d::native_metal::NavigationPattern::Dense:mb3d::native_metal::NavigationPattern::Grid);
            p.block_size=[mode isEqual:@"grid"]?2:4;p.phase=phase;
            if(general&&phase==0)general->reset();
            bool generalPath=fastStep||(general&&phase&&general->active());
            auto renderGeneral=[&]{
                opts.control=std::make_shared<mb3d::RenderControl>();
                const unsigned epoch=generalCancelEpoch.load(std::memory_order_relaxed);
                return general->render(header,addon,opts,phase,[epoch]{return generalCancelEpoch.load(std::memory_order_relaxed)!=epoch;},fastStep,[request[@"scene_revision"] unsignedLongLongValue],![request[@"disable_reuse"] boolValue]);
            };
            auto attempt=generalPath?renderGeneral():session?session->render(header,addon,opts,p):
                mb3d::native_metal::RenderNavigationPreview(header,addon,opts,p,resource(@"navigation_preview.metallib").UTF8String);
            if(attempt.status==mb3d::NativeMetalStatus::Unsupported&&phase==0&&general){generalPath=true;attempt=renderGeneral();}
            if(attempt.status==mb3d::NativeMetalStatus::Unsupported&&phase==0){sparseUnsupported=true;points=false;}
            else {
                out.render=std::move(attempt.result);out.backend.effective=mb3d::RenderBackend::Metal;out.backend.execution=attempt.execution;
                phases=p.pattern==mb3d::native_metal::NavigationPattern::Dense?1:p.block_size*p.block_size;
                covered=session?session->coveredPixels():out.backend.execution.primary_records;
                if(generalPath){usedGeneral=true;phases=general->phases();covered=general->coveredPixels();effectiveMode=fastStep?@"experimental fast geometry; step 0.7; max 64 iterations; headlight":@"experimental points; authored step; max 64 iterations; headlight";}
            }
        } else points=false;
        if(!points) {
            if(phase)throw std::runtime_error("This mode has no accumulation phases");
            if([mode isEqual:@"noao"])opts.no_ao=true;
            else if(![mode isEqual:@"full"]) {
                opts.no_ao=true;opts.no_dof=true;opts.no_reflections=true;
                header.bCalculateHardShadow &= ~1u;header.bHScalculated=0;backend.prefer_general_metal=true;
                effectiveMode=sparseUnsupported?@"draft GPU (sparse unsupported); effects off":@"draft: AO / shadows / reflections / DOF off";
            }
            out=mb3d::RenderWithBackend(header,addon,opts,backend);
        }
        if(out.render.status==mb3d::RenderStatus::Cancelled)return @{@"status":@"cancelled",@"pid":@(getpid()),@"error":@"Navigation superseded"};
        if(!out.render.success)throw std::runtime_error(out.render.error);
        rgb=std::move(out.render.image);
        auto times=usedGeneral?general->preparationTimes():std::array<double,4>{};
        return @{@"status":@"completed",@"backend":@(mb3d::RenderBackendName(out.backend.effective)),
            @"fallback":@(out.backend.fallback_code.c_str()),@"reason":@(out.backend.fallback_reason.c_str()),
            @"preview_mode":effectiveMode,@"samples":@(out.backend.execution.primary_records),@"ao_samples":@(out.backend.execution.ao_samples),
            @"ao_active":@(out.backend.execution.ao_active),@"faults":@(out.backend.execution.faults),@"engine":@(out.backend.execution.engine.c_str()),
            @"prepare_begin_steady_seconds":times[0]?@(times[0]):NSNull.null,@"prepare_end_steady_seconds":times[1]?@(times[1]):NSNull.null,
            @"compile_begin_steady_seconds":times[2]?@(times[2]):NSNull.null,@"compile_end_steady_seconds":times[3]?@(times[3]):NSNull.null,
            @"resource_uploads":@(usedGeneral?general->resourceUploads():0),@"resource_identity":@(usedGeneral?general->resourceIdentity().c_str():""),
            @"prepare_seconds":@(usedGeneral?general->prepareSeconds():0),@"compile_seconds":@(usedGeneral?general->compileSeconds():0),
            @"gpu_begin_seconds":usedGeneral&&general->gpuBeginSeconds()>0?@(general->gpuBeginSeconds()):NSNull.null,
            @"gpu_end_seconds":usedGeneral&&general->gpuEndSeconds()>0?@(general->gpuEndSeconds()):NSNull.null,
            @"general_gpu_seconds":@(usedGeneral?general->gpuSeconds():0),@"general_max_command_seconds":@(usedGeneral?general->maxCommandSeconds():0),
            @"formula_work":@(out.backend.execution.formula_work),@"primary_dispatches":@(out.backend.execution.primary_dispatches),
            @"seconds":@(std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count()),
            @"width":@(out.render.width),@"height":@(out.render.height),@"phase":@(phase),@"phases":@(phases),@"covered":@(covered),
            @"device_creations":@(usedGeneral?general->deviceCreations():session?session->deviceCreations():0),@"scene_preparations":@(usedGeneral?general->scenePreparations():session?session->scenePreparations():0),
            @"pid":@(getpid()),@"archive_hits":@(out.backend.execution.pipeline_archive_hits),@"archive_misses":@(out.backend.execution.pipeline_archive_misses),
            @"surface":@"internal RGB preview; not final export",@"precision":@"native",@"threads":@(opts.thread_count)};
    }catch(const std::exception& e){rgb.clear();return @{@"status":@"failed",@"error":@(e.what()),@"pid":@(getpid())};}
}
static NSDictionary* renderRequest(NSDictionary* request,mb3d::native_metal::NavigationPreviewSession* session,
    std::vector<uint8_t>& rgb,mb3d::native_metal::GeneralNavigationSession* general=nullptr) {
    NSMutableDictionary* identity=[NSMutableDictionary new];
    for(NSString* k in @[@"request_id",@"scene_revision",@"camera_revision",@"quality_revision",@"epoch"])
        if([request[k] isKindOfClass:NSNumber.class])identity[k]=request[k];
    identity[@"protocol"]=@1;identity[@"presentation"]=@"unpresented";
    for(NSString* k in @[@"prepare_begin_steady_seconds",@"prepare_end_steady_seconds",@"compile_begin_steady_seconds",@"compile_end_steady_seconds",@"gpu_begin_seconds",@"gpu_end_seconds",@"present_seconds",@"input_seconds",@"submit_seconds",@"service_begin_seconds",@"publish_seconds"])identity[k]=NSNull.null;
    for(NSString* k in @[@"protocol",@"request_id",@"scene_revision",@"camera_revision",@"quality_revision",@"epoch",@"input_seconds",@"submit_seconds"]){
        bool number=[request[k] isKindOfClass:NSNumber.class];
        double value=number?[request[k] doubleValue]:-1;
        bool timestamp=[k hasSuffix:@"_seconds"];
        if(!number||CFGetTypeID((__bridge CFTypeRef)request[k])==CFBooleanGetTypeID()||!std::isfinite(value)||value<0||
           (!timestamp&&(value<1||value>9007199254740991.||floor(value)!=value))||
           ([k isEqual:@"protocol"]&&value!=1)||
           ([k isEqual:@"submit_seconds"]&&value<[request[@"input_seconds"] doubleValue])) {
            rgb.clear();identity[@"status"]=@"failed";identity[@"error"]=@"Private protocol/version/identity mismatch";identity[@"pid"]=@(getpid());return identity;
        }
    }
    for(NSString* k in @[@"threads",@"disable_reuse",@"disable_telemetry",@"diagnostic_timeout"]){
        id field=request[k];if(!field)continue;
        bool number=[field isKindOfClass:NSNumber.class];double value=number?[field doubleValue]:-1;
        bool workers=[k isEqual:@"threads"];
        if(!number||!std::isfinite(value)||floor(value)!=value||
           (workers?(value<1||value>64||CFGetTypeID((__bridge CFTypeRef)field)==CFBooleanGetTypeID()):(value!=0&&value!=1))){
            rgb.clear();identity[@"status"]=@"failed";identity[@"error"]=@"Invalid private protocol optional field";identity[@"pid"]=@(getpid());return identity;
        }
    }
    double begin=CACurrentMediaTime();
    NSDictionary* body=[request[@"diagnostic_timeout"] boolValue]?@{@"status":@"failed",@"error":@"Accumulation exceeded 180 s diagnostic threshold",@"pid":@(getpid())}:renderRequestBody(request,session,rgb,general);
    [identity addEntriesFromDictionary:body];
    identity[@"presentation"]=@"unpresented";
    identity[@"requested_mode"]=request[@"mode"]?:@"unknown";
    identity[@"approximate"]=@(![request[@"mode"] isEqual:@"full"]);
    identity[@"partial"]=@([body[@"phases"] unsignedIntValue]>[body[@"phase"] unsignedIntValue]+1);
    if([request[@"disable_telemetry"] boolValue])return identity;
    identity[@"preparation_clock_domain"]=@"cpp_steady_seconds";
    identity[@"input_seconds"]=request[@"input_seconds"];
    identity[@"submit_seconds"]=request[@"submit_seconds"];
    identity[@"service_begin_seconds"]=@(begin);identity[@"publish_seconds"]=@(CACurrentMediaTime());
    identity[@"clock_domain"]=@"mach_absolute_host_seconds";
    identity[@"gpu_clock_domain"]=@"metal_host_seconds";
    identity[@"clock_calibration"]=@"cross-domain durations not calculated";
    identity[@"present_seconds"]=NSNull.null;
    return identity;
}
static int service() {
    signal(SIGPIPE,SIG_IGN);signal(SIGUSR1,cancelGeneralSignal);
    int replies=dup(STDOUT_FILENO);dup2(STDERR_FILENO,STDOUT_FILENO); // Library diagnostics cannot corrupt the transport.
    mb3d::native_metal::NavigationPreviewSession session(resource(@"navigation_preview.metallib").UTF8String);
    mb3d::native_metal::GeneralNavigationSession general(resource(@"general_navigation.metallib").UTF8String,
        getenv("MB3D_VIEWER_ORDERED_CONTROL")&&std::string(getenv("MB3D_VIEWER_ORDERED_CONTROL"))=="1");
    bool experimental=generalPreviewEnabled();
    auto frameStart=std::chrono::steady_clock::now();
    for(;;){@autoreleasepool {
        std::vector<uint8_t> rgb;NSDictionary* request=receivePacket(STDIN_FILENO,rgb);
        if(!request||!rgb.empty())break;
        if(![request[@"phase"] isKindOfClass:NSNumber.class]||[request[@"phase"] unsignedIntValue]==0)frameStart=std::chrono::steady_clock::now();
        double age=std::chrono::duration<double>(std::chrono::steady_clock::now()-frameStart).count();
        // The whole accumulation, not each slice separately, has a real guard.
        alarm(unsigned(std::max(1.0,300.0-age)));
        if(age>=180){NSMutableDictionary* expired=[request mutableCopy];expired[@"diagnostic_timeout"]=@YES;request=expired;}
        NSDictionary* result=renderRequest(request,&session,rgb,experimental?&general:nullptr);
        bool sent=sendPacket(replies,result,rgb);alarm(0);if(!sent)break;
    }}
    close(replies);return 0;
}
