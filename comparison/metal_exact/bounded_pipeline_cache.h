#pragma once
#include <algorithm>
#include <condition_variable>
#include <cstdint>
#include <exception>
#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace mb3d::metal_exact::gpu {
// Immutable objects only. A shared handle outlives eviction. Compiles for
// different keys run outside the lock; same-key callers share one attempt.
template<class T, size_t Limit> class BoundedPipelineCache {
    static_assert(Limit>0);
    struct Entry { std::shared_ptr<T> value; std::exception_ptr error; bool busy=true; uint64_t stamp=0; };
    std::mutex mutex;
    std::condition_variable changed;
    std::map<std::string,std::shared_ptr<Entry>> entries;
    uint64_t tick=0;
public:
    struct Metrics { uint64_t hits=0,misses=0,waits=0,evictions=0; size_t peak=0; };
private:
    Metrics counts;
public:
    Metrics metrics() { std::lock_guard lock(mutex);return counts; }
    template<class Factory> std::shared_ptr<T> get(const std::string& key,Factory factory) {
        std::shared_ptr<Entry> entry;
        {
            std::unique_lock lock(mutex);
            for(;;) {
                auto found=entries.find(key);
                if(found!=entries.end()) {
                    entry=found->second;++counts.hits;entry->stamp=++tick;
                    if(entry->busy)++counts.waits;
                    changed.wait(lock,[&]{return !entry->busy;});
                    if(entry->error)std::rethrow_exception(entry->error);
                    return entry->value;
                }
                if(entries.size()<Limit)break;
                auto oldest=entries.end();
                for(auto i=entries.begin();i!=entries.end();++i)
                    if(!i->second->busy&&(oldest==entries.end()||i->second->stamp<oldest->second->stamp))oldest=i;
                if(oldest!=entries.end()){entries.erase(oldest);++counts.evictions;break;}
                changed.wait(lock);
            }
            entry=std::make_shared<Entry>();entry->stamp=++tick;entries.emplace(key,entry);
            ++counts.misses;counts.peak=std::max(counts.peak,entries.size());
        }
        try {
            auto value=factory();
            {std::lock_guard lock(mutex);entry->value=std::move(value);entry->busy=false;}
            changed.notify_all();return entry->value;
        } catch(...) {
            {std::lock_guard lock(mutex);entry->error=std::current_exception();entry->busy=false;entries.erase(key);}
            changed.notify_all();throw;
        }
    }
};
}
