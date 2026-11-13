#pragma once
#include "seed_scan_abi.h"
#include <cstdint>
#include <vector>
#include <stdexcept>
namespace mb3d::native_metal {
struct SeedScanPlan {
    std::vector<mb3d_seed_scan::Params> levels;
    uint64_t words=0,scratch_bytes=16;
};
inline SeedScanPlan PlanSeedScan(uint32_t width,uint32_t height,uint32_t owners,
    uint32_t mapping,uint32_t rows,uint32_t kind,uint32_t initialSeed=0) {
    namespace s=mb3d_seed_scan;
    const auto require=[](bool condition,const char* text){if(!condition)throw std::invalid_argument(text);};
    const uint64_t n=uint64_t(width)*height;
    require(width&&height&&owners&&owners<=height&&rows&&rows<=height&&
        (mapping==s::Interleaved||mapping==s::Contiguous)&&
        (kind==s::PrimaryFlags||kind==s::DrawCounts),"Seed scan shape");
    require(mapping==s::Interleaved?rows==(height+owners-1)/owners:
        owners==(height+rows-1)/rows,"Seed scan task ownership");
    const uint64_t items=uint64_t(rows)*width;
    require(n<=UINT32_MAX-owners&&n*(kind==s::PrimaryFlags?1:33)<=UINT32_MAX&&items<=0xffffff00u&&
        items*owners<=UINT32_MAX,"Seed scan count overflow");
    SeedScanPlan result;result.words=n+owners;
    s::Params p{s::Version,width,height,owners,rows,mapping,kind,initialSeed,uint32_t(items),0,1,0,0,0};
    for(;;){p.groups=(p.items+s::Group-1)/s::Group;
        require(uint64_t(owners)*p.groups*s::Group<=UINT32_MAX,"Seed scan dispatch overflow");
        result.levels.push_back(p);result.scratch_bytes+=uint64_t(owners)*p.groups*sizeof(uint32_t);
        if(p.groups==1)break;p.items=p.groups;p.first=0;
    }
    return result;
}
}
