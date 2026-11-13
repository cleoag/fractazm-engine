#pragma once
// Shared offline Metal device and bounded dispatch infrastructure. No oracle,
// diagnostic kernels, image encoding or renderer process launch is included.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include <unordered_map>
#include <CommonCrypto/CommonDigest.h>
#include "bounded_pipeline_cache.h"
#include "work_meter.h"
#include "scene_abi.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <TargetConditionals.h>
namespace mb3d::metal_exact::gpu {
using Clock=std::chrono::steady_clock;
inline double elapsed(Clock::time_point t) { return std::chrono::duration<double,std::milli>(Clock::now()-t).count(); }
inline NSString* ns(const std::string& s) { return [NSString stringWithUTF8String:s.c_str()]; }
inline void require(bool condition,const std::string& reason) { if(!condition) throw std::runtime_error(reason); }
struct CachedPipeline { id<MTLComputePipelineState> value; };
struct CachedNativeLibrary {
    id<MTLLibrary> value;
    std::string sha256;
    BoundedPipelineCache<CachedPipeline,64> pipelines;
};
inline BoundedPipelineCache<CachedNativeLibrary,4>& nativeLibraries() {
    static BoundedPipelineCache<CachedNativeLibrary,4> cache;return cache;
}
inline std::shared_ptr<CachedNativeLibrary> nativeLibrary(id<MTLDevice> device,const std::filesystem::path& path) {
    // Read/hash/load the same bounded immutable bytes, never a path/mtime key.
    std::ifstream input(path,std::ios::binary|std::ios::ate);
    require(bool(input),"native library open failed");
    const auto size=input.tellg();
    require(size>0&&size<=64u*1024u*1024u,"native library exceeds cache input bound");
    NSMutableData* data=[NSMutableData dataWithLength:static_cast<NSUInteger>(size)];
    require(data!=nil,"native library snapshot allocation failed");
    input.seekg(0);input.read(static_cast<char*>(data.mutableBytes),size);
    require(bool(input)&&input.peek()==std::char_traits<char>::eof(),"native library changed while reading");
    unsigned char hash[CC_SHA256_DIGEST_LENGTH];CC_SHA256(data.bytes,(CC_LONG)data.length,hash);
    std::string key=std::to_string(device.registryID)+":native-fp32:";
    constexpr char hex[]="0123456789abcdef";
    for(auto v:hash){key+=hex[v>>4];key+=hex[v&15];}
    return nativeLibraries().get(key,[=]{
        auto snapshot=dispatch_data_create(data.bytes,data.length,dispatch_get_global_queue(QOS_CLASS_DEFAULT,0),^{(void)data;});
        NSError* error=nil;auto library=[device newLibraryWithData:snapshot error:&error];
        require(library!=nil,error?error.description.UTF8String:"offline native library unavailable");
        auto result=std::make_shared<CachedNativeLibrary>();result->value=library;
        result->sha256=key.substr(key.rfind(':')+1);return result;
    });
}
// MB3D_METAL_PIPELINE_CACHE names the persistent pipeline archive directory;
// "off" disables it. The default is the user's cache directory. The iOS and
// visionOS simulators never use one: Metal aborts the process there when a
// binary archive is serialized ("Target device architecture is nil").
inline std::filesystem::path pipelineArchiveDirectory() {
#if TARGET_OS_SIMULATOR
    return {};
#endif
    if(const char* value=std::getenv("MB3D_METAL_PIPELINE_CACHE"))
        return std::string(value)=="off"?std::filesystem::path{}:std::filesystem::path(value);
    NSArray<NSString*>* roots=NSSearchPathForDirectoriesInDomains(NSCachesDirectory,NSUserDomainMask,YES);
    return roots.count?std::filesystem::path(roots.firstObject.UTF8String)/"mb3d"/"metal-pipelines":std::filesystem::path{};
}
// Keeps the newest archives within kPipelineArchiveBytes (a hit refreshes its
// time; all 77 package scenes take about 0.3 GB) and drops temporary files a
// stopped render left for over an hour. Another process may lose an archive
// it is about to load; that is an ordinary miss.
constexpr uintmax_t kPipelineArchiveBytes=uintmax_t(2)<<30;
inline void prunePipelineArchives(const std::filesystem::path& directory,uintmax_t limit=kPipelineArchiveBytes) {
    struct Entry {std::filesystem::file_time_type time;uintmax_t size;std::filesystem::path path;};
    std::error_code ec;std::vector<Entry> archives;
    const auto stale=std::filesystem::file_time_type::clock::now()-std::chrono::hours(1);
    for(std::filesystem::directory_iterator it(directory,ec),end;!ec&&it!=end;it.increment(ec)) {
        const auto time=it->last_write_time(ec);if(ec){ec.clear();continue;}
        const auto size=it->file_size(ec);if(ec){ec.clear();continue;}
        if(it->path().extension()==".metalarchive")archives.push_back({time,size,it->path()});
        else if(it->path().extension()==".tmp"&&time<stale)std::filesystem::remove(it->path(),ec);
    }
    std::sort(archives.begin(),archives.end(),[](const Entry& a,const Entry& b){return a.time>b.time;});
    uintmax_t total=0;
    for(const auto& a:archives)if((total+=a.size)>limit)std::filesystem::remove(a.path,ec);
}
// Device state an application renderer keeps between renders of one shader
// library: the device, one queue, the loaded library and the setup checks
// that already passed on them. Never shared by two renders at the same time.
struct SharedMetal {
    id<MTLDevice> device;
    id<MTLCommandQueue> queue;
    std::shared_ptr<CachedNativeLibrary> library;
    std::array<uint32_t,3> arithmeticProfile{};
    bool profileVerified=false;
    // Setup markers that matched: pipeline key plus the expected bytes.
    std::unordered_set<std::string> verifiedMarkers;
};
struct DispatchObserver {
    std::function<void(uint64_t)> before;
    std::function<void()> after;
};
struct Metal {
    id<MTLDevice> device;
    id<MTLLibrary> library;
    id<MTLCommandQueue> queue;
    uint32_t formulaKind,bulbPower,capabilityId;
    bool divisionBaseline;
    bool hasNativeFormulaIds=false;
    std::array<uint32_t,6> nativeFormulaIds{};
    uint32_t nativeOrbitDriver=0xffffffffu;
    uint32_t nativeSmoothingPolicy=1;
    uint32_t nativeWrapperPolicy=1;
    // DE combination (FormulaType) and the second part's orbit driver. Set
    // before the first formula pipeline; 0 keeps both constants undefined.
    uint32_t nativeFormulaType=0;
    uint32_t nativeOrbitDriver2=0xffffffffu;
    // 1: every formula DE call site calls one outlined device function
    // (general_de_outlined) instead of inlining the formula program.
    uint32_t nativeOutlineDE=0;
    // 1: primary pipelines compile the RayMarch first-step jitter
    // (function constant 22); set only around those pipelines.
    uint32_t nativePrimaryRandom=0;
    std::array<uint32_t,3> arithmeticProfile{};
    double pipelineCreationMs=0;
    uint64_t pipelineArchiveHits=0,pipelineArchiveMisses=0;
    // GPU execution time and count of the commands this driver waited for.
    double gpuMs=0;uint64_t gpuCommands=0;
    void noteCommand(double ms){gpuMs+=ms;++gpuCommands;}
    DispatchObserver observer;
    std::shared_ptr<CachedNativeLibrary> reusableLibrary;
    SharedMetal* shared=nullptr;
    void beforeDispatch(uint64_t units) {
        if(mb3d::renderCancelRequested())throw mb3d::RenderCancelRequested();
        if(mb3d::render_work_meter)mb3d::render_work_meter->fetch_add(units,std::memory_order_relaxed);
        if(observer.before)observer.before(units);
    }
    void afterDispatch() { if(observer.after)observer.after(); }
    explicit Metal(const std::filesystem::path& path,uint32_t formula,uint32_t power,uint32_t capability,bool restoring,bool native=false,
                   const std::array<uint32_t,6>* nativeIds=nullptr,uint32_t nativeDriver=0xffffffffu,DispatchObserver callbacks={},uint32_t smoothingPolicy=1,bool reuseNative=true,uint32_t wrapperPolicy=1,
                   SharedMetal* sharedState=nullptr)
      :formulaKind(formula),bulbPower(power),capabilityId(capability),divisionBaseline(restoring),observer(std::move(callbacks)),shared(sharedState) {
        if(nativeIds){require(native,"formula specialization requires Native arithmetic");nativeFormulaIds=*nativeIds;hasNativeFormulaIds=true;}
        nativeOrbitDriver=nativeDriver;nativeSmoothingPolicy=smoothingPolicy;
        nativeWrapperPolicy=wrapperPolicy;
        require(nativeWrapperPolicy<=1,"invalid Native wrapper policy");
        require(nativeSmoothingPolicy<=1,"invalid Native smoothing policy");
        if(shared) {
            // The owner loaded and hashed the library once; `path` is not read again.
            require(native&&reuseNative,"shared Metal state requires the reusable Native library");
            device=shared->device;reusableLibrary=shared->library;queue=shared->queue;
            require(device!=nil&&reusableLibrary&&queue!=nil,"shared Metal state is incomplete");
            library=reusableLibrary->value;
            if(shared->profileVerified){arithmeticProfile=shared->arithmeticProfile;return;}
        } else {
        device=MTLCreateSystemDefaultDevice();require(device!=nil,"Metal device unavailable");
        NSError* error=nil;
        if(native&&reuseNative){reusableLibrary=nativeLibrary(device,path);library=reusableLibrary->value;}
        else library=[device newLibraryWithURL:[NSURL fileURLWithPath:ns(std::filesystem::absolute(path).string())] error:&error];
        require(library!=nil,error?error.description.UTF8String:"offline library unavailable");
        queue=[device newCommandQueue];require(queue!=nil,"Metal queue unavailable");
        }
        // Query the actual offline shader, not its filename. This setup-only
        // dispatch prevents either mode from silently running the other library.
        auto identity=buffer(sizeof(arithmeticProfile));
        std::memset(identity.contents,0,sizeof(arithmeticProfile));
        auto marker=pipeline(@"exact_arithmetic_profile");
        beforeDispatch(1);
        auto command=[queue commandBuffer];auto encoder=[command computeCommandEncoder];
        require(command&&encoder,"arithmetic profile dispatch unavailable");
        [encoder setComputePipelineState:marker];[encoder setBuffer:identity offset:0 atIndex:0];
        [encoder dispatchThreads:MTLSizeMake(1,1,1) threadsPerThreadgroup:MTLSizeMake(1,1,1)];
        [encoder endEncoding];[command commit];[command waitUntilCompleted];
        require(command.status==MTLCommandBufferStatusCompleted,"arithmetic profile dispatch failed");
        std::memcpy(arithmeticProfile.data(),identity.contents,sizeof(arithmeticProfile));
        const std::array<uint32_t,3> expected={native?2u:1u,4u,native?32u:80u};
        require(arithmeticProfile==expected,"offline library precision/profile mismatch");
        afterDispatch();
        if(shared){shared->arithmeticProfile=arithmeticProfile;shared->profileVerified=true;}
    }
    // Every specialization field, including namespaced IDs, integer orbit
    // driver and smoothing; no mutable scene state here.
    std::string pipelineKey(NSString* name) const {
        std::string key=name.UTF8String;
        auto add=[&](uint32_t x){key+=':';key+=std::to_string(x);};
        add(formulaKind);add(bulbPower);add(capabilityId);add(divisionBaseline);add(hasNativeFormulaIds);
        for(auto id:nativeFormulaIds)add(id);
        add(nativeOrbitDriver);add(nativeSmoothingPolicy);
        add(nativeWrapperPolicy);add(nativeFormulaType);add(nativeOrbitDriver2);add(nativeOutlineDE);add(nativePrimaryRandom);
        return key;
    }
    // Kernel names of the pipelines handed out, for the text of a failed command.
    std::unordered_map<const void*,std::string> pipelineNames;
    id<MTLComputePipelineState> pipeline(NSString* name) {
        const auto start=Clock::now();
        if(reusableLibrary) {
            std::string key=pipelineKey(name);
            auto result=reusableLibrary->pipelines.get(key,[&]{auto r=std::make_shared<CachedPipeline>();r->value=compilePipeline(name,&key);return r;});
            pipelineCreationMs+=elapsed(start);pipelineNames[(__bridge const void*)result->value]=name.UTF8String;return result->value;
        }
        auto result=compilePipeline(name);pipelineCreationMs+=elapsed(start);pipelineNames[(__bridge const void*)result]=name.UTF8String;return result;
    }
    id<MTLComputePipelineState> compilePipeline(NSString* name,const std::string* archiveKey=nullptr) {
        NSError* error=nil;id<MTLFunction> function=[library newFunctionWithName:name];
        require(function!=nil,"missing offline kernel "+std::string(name.UTF8String));
        const bool specialized=function.functionConstantsDictionary.count!=0;
        if(specialized) {
            MTLFunctionConstantValues* values=[MTLFunctionConstantValues new];
            for(MTLFunctionConstant* constant in function.functionConstantsDictionary.allValues) {
                if(constant.index==4&&constant.type==MTLDataTypeBool) {
                    [values setConstantValue:&divisionBaseline type:MTLDataTypeBool atIndex:4];
                } else if(constant.index>=10&&constant.index<16&&constant.type==MTLDataTypeUInt) {
                    require(hasNativeFormulaIds,"missing native formula specialization");
                    const uint32_t value=nativeFormulaIds[constant.index-10];
                    [values setConstantValue:&value type:MTLDataTypeUInt atIndex:constant.index];
                } else if(constant.index==21&&constant.type==MTLDataTypeUInt) {
                    require(nativeOutlineDE<=1,"invalid Native DE outlining policy");
                    [values setConstantValue:&nativeOutlineDE type:MTLDataTypeUInt atIndex:21];
                } else if(constant.index==22&&constant.type==MTLDataTypeUInt) {
                    require(nativePrimaryRandom<=1,"invalid Native primary jitter policy");
                    [values setConstantValue:&nativePrimaryRandom type:MTLDataTypeUInt atIndex:22];
                } else if(constant.index==18&&constant.type==MTLDataTypeUInt) {
                    [values setConstantValue:&nativeWrapperPolicy type:MTLDataTypeUInt atIndex:18];
                } else if(constant.index==17&&constant.type==MTLDataTypeUInt) {
                    [values setConstantValue:&nativeSmoothingPolicy type:MTLDataTypeUInt atIndex:17];
                } else if(constant.index==16&&constant.type==MTLDataTypeUInt) {
                    require(hasNativeFormulaIds&&nativeOrbitDriver<=9,"missing native orbit driver specialization");
                    [values setConstantValue:&nativeOrbitDriver type:MTLDataTypeUInt atIndex:16];
                } else if((constant.index==19||constant.index==20)&&constant.type==MTLDataTypeUInt) {
                    require(nativeFormulaType<=6&&(!nativeFormulaType||(hasNativeFormulaIds&&nativeOrbitDriver2<=9)),
                            "invalid native DE combination specialization");
                    if(nativeFormulaType)[values setConstantValue:constant.index==19?&nativeFormulaType:&nativeOrbitDriver2
                                                             type:MTLDataTypeUInt atIndex:constant.index];
                } else {
                    require(constant.index<=2&&constant.type==MTLDataTypeUInt,
                            "unexpected exact scene specialization constant");
                    const uint32_t value=constant.index==0?formulaKind:constant.index==1?bulbPower:capabilityId;
                    [values setConstantValue:&value type:MTLDataTypeUInt atIndex:constant.index];
                }
            }
            function=[library newFunctionWithName:name constantValues:values error:&error];
            require(function!=nil,error?error.description.UTF8String:"scene specialization failed");
        }
        // A kernel without function constants is the same binary for every
        // scene, so its archive key is its name alone.
        auto p=archiveKey&&reusableLibrary?archivedPipeline(function,specialized?*archiveKey:std::string(name.UTF8String),error):
            [device newComputePipelineStateWithFunction:function error:&error];
        require(p!=nil,std::string(name.UTF8String)+": "+(error?error.description.UTF8String:"pipeline creation failed"));
        return p;
    }
    // Persistent pipeline cache. The system shader cache evicts large
    // pipelines (a reflection pipeline recompiled for minutes on every run),
    // so each specialized native pipeline is kept as its own MTLBinaryArchive.
    // One file per key: concurrent renders never rewrite each other's entries,
    // and a file is published by rename only after it is complete. The key
    // holds the library bytes, the device, the OS and every specialization
    // field. An unreadable, stale or foreign archive is a miss: the pipeline
    // is compiled as before and its archive replaced. The archive stores the
    // compiler's own binary, so a hit runs the same machine code. A failed
    // compile returns nil with the compiler's error in failure.
    id<MTLComputePipelineState> archivedPipeline(id<MTLFunction> function,const std::string& key,NSError* __strong& failure) {
        MTLComputePipelineDescriptor* descriptor=[MTLComputePipelineDescriptor new];descriptor.computeFunction=function;
        NSError* error=nil;
        auto compile=[&]{error=nil;auto p=[device newComputePipelineStateWithDescriptor:descriptor options:MTLPipelineOptionNone reflection:nil error:&error];
            failure=error;return p;};
        const auto directory=pipelineArchiveDirectory();
        if(directory.empty())return compile();
        std::string full=reusableLibrary->sha256+"|"+device.name.UTF8String+"|"+
            NSProcessInfo.processInfo.operatingSystemVersionString.UTF8String+"|"+key;
        unsigned char hash[CC_SHA256_DIGEST_LENGTH];CC_SHA256(full.data(),(CC_LONG)full.size(),hash);
        std::string file;constexpr char hex[]="0123456789abcdef";
        for(auto v:hash){file+=hex[v>>4];file+=hex[v&15];}
        const auto path=directory/(file+".metalarchive");
        std::error_code ec;
        if(std::filesystem::exists(path,ec)) {
            MTLBinaryArchiveDescriptor* stored=[MTLBinaryArchiveDescriptor new];stored.url=[NSURL fileURLWithPath:ns(path.string())];
            if(id<MTLBinaryArchive> archive=[device newBinaryArchiveWithDescriptor:stored error:&error]) {
                descriptor.binaryArchives=@[archive];
                if(auto p=[device newComputePipelineStateWithDescriptor:descriptor options:MTLPipelineOptionFailOnBinaryArchiveMiss reflection:nil error:&error]) {
                    // A hit refreshes the file time that bounds the directory.
                    std::filesystem::last_write_time(path,std::filesystem::file_time_type::clock::now(),ec);
                    ++pipelineArchiveHits;return p;
                }
                descriptor.binaryArchives=nil;
            }
        }
        ++pipelineArchiveMisses;
        // Compile once into a new archive, publish it, and load from it.
        id<MTLBinaryArchive> archive=[device newBinaryArchiveWithDescriptor:[MTLBinaryArchiveDescriptor new] error:&error];
        // Adding the function is the backend compile. Only an archive fault
        // falls back to a plain compile: a pipeline the compiler cannot build
        // is not compiled again, which doubled rank 6's failing attempts.
        if(archive&&![archive addComputePipelineFunctionsWithDescriptor:descriptor error:&error]) {
            if(!error||![error.domain isEqualToString:MTLBinaryArchiveDomain]||error.code==MTLBinaryArchiveErrorCompilationFailure){
                failure=error;return nil;}
            archive=nil;
        }
        if(archive) {
            descriptor.binaryArchives=@[archive];
            std::filesystem::create_directories(directory,ec);
            const auto temporary=directory/(file+"."+std::to_string(getpid())+".tmp");
            if([archive serializeToURL:[NSURL fileURLWithPath:ns(temporary.string())] error:&error])
                std::filesystem::rename(temporary,path,ec);
            if(ec||std::filesystem::exists(temporary))std::filesystem::remove(temporary,ec);
            prunePipelineArchives(directory);
        }
        return compile();
    }
    id<MTLBuffer> buffer(size_t size,const void* data=nullptr) {
        auto b=data?[device newBufferWithBytes:data length:size options:MTLResourceStorageModeShared]:
                    [device newBufferWithLength:size options:MTLResourceStorageModeShared];
        require(b!=nil,"Metal buffer allocation failed");return b;
    }
    double run(id<MTLComputePipelineState> pipeline,id<MTLBuffer> a,id<MTLBuffer> b,
               const void* params,size_t paramSize,uint32_t width,uint32_t height=1) {
        beforeDispatch(uint64_t(width)*height);
        auto command=[queue commandBuffer];auto encoder=[command computeCommandEncoder];
        require(command!=nil&&encoder!=nil,"command/encoder unavailable");
        [encoder setComputePipelineState:pipeline];[encoder setBuffer:a offset:0 atIndex:0];
        if(b) [encoder setBuffer:b offset:0 atIndex:1];
        [encoder setBytes:params length:paramSize atIndex:b?2:1];
        NSUInteger tw=pipeline.threadExecutionWidth;
        NSUInteger th=height==1?1:std::min<NSUInteger>(4,pipeline.maxTotalThreadsPerThreadgroup/tw);
        [encoder dispatchThreads:MTLSizeMake(width,height,1) threadsPerThreadgroup:MTLSizeMake(tw,th,1)];
        [encoder endEncoding];[command commit];[command waitUntilCompleted];
        require(command.status==MTLCommandBufferStatusCompleted,command.error?command.error.description.UTF8String:"GPU command failed");
        require(command.GPUEndTime>=command.GPUStartTime&&command.GPUStartTime>0,"GPU timestamps unavailable");
        afterDispatch();
        noteCommand((command.GPUEndTime-command.GPUStartTime)*1000);
        return (command.GPUEndTime-command.GPUStartTime)*1000;
    }
};

// Included after the shared Metal driver and its timestamp/report helpers.
// Launch extents come from allocations, independently of the scene declaration.
// This also makes malformed-dimension controls visit every allocated record.
// Eight columns use an 8x4 tile at 32 threads; explicit zero selects one row.
static double runScene(Metal& m, id<MTLComputePipelineState> pipeline,
                       id<MTLBuffer> output, id<MTLBuffer> cols,
                       id<MTLBuffer> rows, const ExactScene& scene,
                       unsigned rowBatch = 8, unsigned groupWidth = 32,
                       double* maxCommandMs = nullptr, unsigned tileWidth = 8) {
    require(pipeline != nil && output != nil && cols != nil && rows != nil,
            "primary dispatch requires a pipeline and allocated buffers");
    require(cols.length % sizeof(ExactTrig) == 0 &&
            rows.length % sizeof(ExactTrig) == 0,
            "primary camera allocation is not an exact trig record array");
    const size_t width = cols.length / sizeof(ExactTrig);
    const size_t height = rows.length / sizeof(ExactTrig);
    require(width > 0 && width <= 4096 && height > 0 && height <= 4096,
            "primary camera allocation dimensions are outside 1..4096");
    require(output.length == width * height * sizeof(ExactPixel),
            "primary output allocation does not match camera allocations");
    require(output != cols && output != rows,
            "primary output must not alias camera allocations");
    const NSUInteger simdWidth = pipeline.threadExecutionWidth;
    require(rowBatch > 0 && rowBatch <= 4096,
            "primary rows per command must be in 1..4096");
    require(simdWidth > 0 && groupWidth > 0 &&
            groupWidth % simdWidth == 0 &&
            groupWidth <= pipeline.maxTotalThreadsPerThreadgroup,
            "primary threadgroup must be a legal positive SIMD-width multiple");
    const unsigned tile = tileWidth == 0 ? groupWidth : tileWidth;
    require(tile > 0 && tile <= groupWidth && groupWidth % tile == 0,
            "primary tile width must be zero or a positive factor of its thread count");
    const unsigned tileHeight = groupWidth / tile;
    if (maxCommandMs)
        require(std::isfinite(*maxCommandMs) && *maxCommandMs >= 0,
                "primary maximum command timestamp must be initialized");

    double total = 0;
    for (uint32_t y = 0; y < height; y += rowBatch) {
        m.beforeDispatch(uint64_t(width)*std::min<size_t>(rowBatch,height-y));
        auto command = [m.queue commandBuffer];
        auto encoder = [command computeCommandEncoder];
        require(command != nil && encoder != nil,
                "cannot encode GPU primary stage");
        [encoder setComputePipelineState:pipeline];
        [encoder setBuffer:output offset:0 atIndex:0];
        [encoder setBuffer:cols offset:0 atIndex:1];
        [encoder setBuffer:rows offset:0 atIndex:2];
        const uint32_t launch[]{y, uint32_t(width), uint32_t(height)};
        [encoder setBytes:&scene length:sizeof(scene) atIndex:3];
        [encoder setBytes:launch length:sizeof(launch) atIndex:4];
        [encoder dispatchThreads:MTLSizeMake(width, std::min<size_t>(rowBatch, height - y), 1)
           threadsPerThreadgroup:MTLSizeMake(tile, tileHeight, 1)];
        [encoder endEncoding];
        [command commit];
        [command waitUntilCompleted];
        require(command.status == MTLCommandBufferStatusCompleted,
                command.error ? command.error.description.UTF8String : "GPU primary stage failed");
        const double ms = (command.GPUEndTime - command.GPUStartTime) * 1000;
        require(command.GPUStartTime > 0 && std::isfinite(ms) && ms > 0,
                "GPU primary timestamp unavailable");
        m.afterDispatch();
        total += ms;
        if (maxCommandMs) *maxCommandMs = std::max(*maxCommandMs, ms);
    }
    return total;
}


// The production CPU oracle runs separately. Only scene constants/tables are
// uploaded: no CPU pixel, surface, shadow, AO or color result enters this path.
static double dispatchStage(Metal& m,id<MTLComputePipelineState> pipeline,
                            unsigned count,const std::function<void(id<MTLComputeCommandEncoder>)>& bind,unsigned height=1,unsigned groupWidth=0,double* timestamps=nullptr) {
    m.beforeDispatch(uint64_t(count)*height);
    auto c=[m.queue commandBuffer];auto e=[c computeCommandEncoder];
    require(c&&e,"cannot encode shading stage");[e setComputePipelineState:pipeline];bind(e);
    [e dispatchThreads:MTLSizeMake(count,height,1) threadsPerThreadgroup:MTLSizeMake(groupWidth?groupWidth:pipeline.threadExecutionWidth,1,1)];
    [e endEncoding];
    const auto submitted=std::chrono::steady_clock::now();
    [c commit];[c waitUntilCompleted];
    if(c.status!=MTLCommandBufferStatusCompleted){
        // Which command the device gave up on, and after how long: the system
        // stops commands that hold the GPU for too long.
        const double wall=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-submitted).count();
        const auto named=m.pipelineNames.find((__bridge const void*)pipeline);
        std::string text="[";text+=named!=m.pipelineNames.end()?named->second:"unnamed pipeline";
        text+=", "+std::to_string(uint64_t(count)*height)+" threads, "+std::to_string(int(wall))+" ms] ";
        text+=c.error?c.error.description.UTF8String:"shading GPU command failed";
        require(false,text);
    }
    require(c.GPUStartTime>0&&c.GPUEndTime>=c.GPUStartTime,"shading timestamp missing");
    if(timestamps){timestamps[0]=c.GPUStartTime;timestamps[1]=c.GPUEndTime;}
    m.afterDispatch();
    m.noteCommand((c.GPUEndTime-c.GPUStartTime)*1000);
    return (c.GPUEndTime-c.GPUStartTime)*1000;
}

// A setup marker reads constants of the shader library and of one pipeline
// specialization. It is dispatched and compared as before; with shared state a
// marker that already matched for the same pipeline key and expected bytes is
// not dispatched again, because neither the library nor the device changed.
static void verifyMarker(Metal& m,NSString* name,const void* expected,size_t bytes,const std::string& reason) {
    std::string key;
    if(m.shared) {
        key=m.pipelineKey(name);key+='=';key.append(static_cast<const char*>(expected),bytes);
        if(m.shared->verifiedMarkers.count(key))return;
    }
    auto pipeline=m.pipeline(name);auto identity=m.buffer(bytes);
    std::memset(identity.contents,0,identity.length);
    dispatchStage(m,pipeline,1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:identity offset:0 atIndex:0];});
    require(std::memcmp(identity.contents,expected,bytes)==0,reason);
    if(m.shared)m.shared->verifiedMarkers.insert(std::move(key));
}

} // namespace mb3d::metal_exact::gpu
