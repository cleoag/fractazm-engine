#include "comparison/metal_exact/metal_device.h"
#include <atomic>
#include <future>
#include <iostream>
#include <thread>
#include <unistd.h>
using namespace mb3d::metal_exact::gpu;
int main(int argc,char**argv) { @autoreleasepool { try {
    require(argc==2,"native library argument required");
    // Persistent archives go to a private directory for this process.
    const auto archives=std::filesystem::temp_directory_path()/("mb3d-native-archives-"+std::to_string(getpid()));
    struct RemoveAll {std::filesystem::path p;~RemoveAll(){std::error_code e;std::filesystem::remove_all(p,e);}} archiveCleanup{archives};
    setenv("MB3D_METAL_PIPELINE_CACHE",archives.c_str(),1);
    BoundedPipelineCache<int,2> cache;
    std::atomic<int> compiled{0};std::promise<void> release;
    auto ready=release.get_future().share();
    auto work=[&]{return cache.get("same",[&]{++compiled;ready.wait();return std::make_shared<int>(42);});};
    auto a=std::async(std::launch::async,work);
    while(compiled==0)std::this_thread::yield();
    auto b=std::async(std::launch::async,work);
    while(cache.metrics().waits==0)std::this_thread::yield();
    // A different key must complete while the first compilation is held.
    auto independent=cache.get("other",[]{return std::make_shared<int>(7);});
    release.set_value();auto first=a.get();auto second=b.get();
    require(compiled==1&&first==second&&*independent==7,"single-flight failed");
    for(int i=0;i<10;++i)cache.get(std::to_string(i),[&]{return std::make_shared<int>(i);});
    require(*first==42&&cache.metrics().peak<=2&&cache.metrics().evictions>0,"eviction/lifetime failed");
    try{cache.get("retry",[]()->std::shared_ptr<int>{throw std::runtime_error("injected");});require(false,"fault not raised");}
    catch(const std::runtime_error& e){require(std::string(e.what())=="injected","wrong failure");}
    require(*cache.get("retry",[]{return std::make_shared<int>(9);})==9,"failure poisoned cache");
    std::promise<void> failRelease;auto failReady=failRelease.get_future().share();
    const auto waits=cache.metrics().waits;std::atomic<bool> entered{false};
    auto failing=[&]{try{cache.get("failed-flight",[&]()->std::shared_ptr<int>{entered=true;failReady.wait();throw std::runtime_error("shared failure");});return false;}catch(const std::runtime_error&e){return std::string(e.what())=="shared failure";}};
    auto fa=std::async(std::launch::async,failing);while(!entered)std::this_thread::yield();
    auto fb=std::async(std::launch::async,failing);while(cache.metrics().waits==waits)std::this_thread::yield();
    failRelease.set_value();require(fa.get()&&fb.get(),"waiting client lost compile failure");
    require(*cache.get("failed-flight",[]{return std::make_shared<int>(11);})==11,"shared failure poisoned retry");
    auto make=[&](uint32_t power=2,bool reuse=true){return std::make_unique<Metal>(argv[1],0,power,0,false,true,nullptr,0xffffffffu,DispatchObserver{},1,reuse);};
    auto x=make();auto p=x->pipeline(@"exact_arithmetic_profile");
    auto before=x->reusableLibrary->pipelines.metrics();
    auto y=make();require(y->pipeline(@"exact_arithmetic_profile")==p,"native pipeline not reused");
    auto after=y->reusableLibrary->pipelines.metrics();
    require(after.misses==before.misses&&after.hits>=before.hits+2,"warm cache counters wrong");
    auto changed=make(3);require(changed->reusableLibrary->pipelines.metrics().misses==after.misses+1,"specialization key ignored");
    auto off=make(2,false);require(!off->reusableLibrary&&off->arithmeticProfile==x->arithmeticProfile,"uncached GPU marker differs");
    auto copied=std::filesystem::temp_directory_path()/("mb3d-native-cache-"+std::to_string(getpid())+".metallib");
    struct Remove {std::filesystem::path p;~Remove(){std::error_code e;std::filesystem::remove(p,e);}} cleanup{copied};
    std::filesystem::copy_file(argv[1],copied,std::filesystem::copy_options::overwrite_existing);
    Metal alias(copied,0,2,0,false,true);require(alias.reusableLibrary==x->reusableLibrary,"content-addressed reuse failed");
    [@"invalid replacement" writeToFile:ns(copied.string()) atomically:YES encoding:NSUTF8StringEncoding error:nullptr];
    bool rejected=false;try{Metal bad(copied,0,2,0,false,true);}catch(...){rejected=true;}
    require(rejected,"same-path changed library reused");
    rejected=false;try{Metal wrong(argv[1],0,2,0,false,false);}catch(...){rejected=true;}
    require(rejected,"wrong arithmetic profile accepted");
    // A miss publishes one archive; compiling the key again loads it, and the
    // loaded pipeline runs the same marker. A corrupt archive is a miss that
    // replaces the file; "off" never touches the directory.
    auto z=make();const std::string archiveKey="exact_arithmetic_profile:archive-test";
    auto archived=[&]{return z->compilePipeline(@"exact_arithmetic_profile",&archiveKey);};
    auto files=[&]{std::vector<std::filesystem::path> v;for(auto& e:std::filesystem::directory_iterator(archives))
        if(e.path().extension()==".metalarchive")v.push_back(e.path());return v;};
    // Earlier renders archived this constant-free kernel under its name.
    for(const auto& f:files())std::filesystem::remove(f);
    require(archived()&&z->pipelineArchiveMisses==1&&z->pipelineArchiveHits==0,"archive miss not counted");
    auto published=files();require(published.size()==1,"archive not published");
    auto loaded=archived();require(loaded&&z->pipelineArchiveHits==1&&z->pipelineArchiveMisses==1,"archive not loaded");
    auto identity=z->buffer(sizeof(z->arithmeticProfile));std::memset(identity.contents,0,identity.length);
    auto command=[z->queue commandBuffer];auto encoder=[command computeCommandEncoder];
    [encoder setComputePipelineState:loaded];[encoder setBuffer:identity offset:0 atIndex:0];
    [encoder dispatchThreads:MTLSizeMake(1,1,1) threadsPerThreadgroup:MTLSizeMake(1,1,1)];
    [encoder endEncoding];[command commit];[command waitUntilCompleted];
    require(command.status==MTLCommandBufferStatusCompleted&&
            std::memcmp(identity.contents,z->arithmeticProfile.data(),identity.length)==0,"archived pipeline differs");
    for(const auto& f:files())[@"corrupt archive" writeToFile:ns(f.string()) atomically:YES encoding:NSUTF8StringEncoding error:nullptr];
    require(archived()&&z->pipelineArchiveMisses==2,"corrupt archive accepted");
    require(archived()&&z->pipelineArchiveHits==2,"corrupt archive not replaced");
    // The directory keeps the newest archives within its byte bound and
    // drops stale temporaries.
    const auto pruned=archives/"prune";std::filesystem::create_directories(pruned);
    const auto now=std::filesystem::file_time_type::clock::now();
    for(int i=0;i<6;++i) {
        const auto f=pruned/(std::to_string(i)+".metalarchive");std::ofstream(f)<<std::string(100,'x');
        std::filesystem::last_write_time(f,now-std::chrono::minutes(10-i));
    }
    const auto oldTemporary=pruned/"old.tmp",newTemporary=pruned/"new.tmp";
    std::ofstream(oldTemporary)<<0;std::ofstream(newTemporary)<<0;
    std::filesystem::last_write_time(oldTemporary,now-std::chrono::hours(2));
    prunePipelineArchives(pruned,350);
    size_t kept=0;for(auto& e:std::filesystem::directory_iterator(pruned))kept+=e.path().extension()==".metalarchive";
    require(kept==3&&!std::filesystem::exists(pruned/"2.metalarchive")&&std::filesystem::exists(pruned/"3.metalarchive")&&
            std::filesystem::exists(pruned/"5.metalarchive")&&!std::filesystem::exists(oldTemporary)&&std::filesystem::exists(newTemporary),
            "archive directory bound failed");
    setenv("MB3D_METAL_PIPELINE_CACHE","off",1);
    const auto count=files().size();
    require(archived()&&z->pipelineArchiveHits==2&&z->pipelineArchiveMisses==2&&files().size()==count,"disabled archive used");
    std::cout<<"PASS cache: concurrency, eviction, live handles, retry, real GPU reuse, specialization, content replacement, profile guard, "
               "persistent archive publish/load/replace/bound/off\n";
    return 0;
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;} } }
