#pragma once
#include "seed_scan_plan.h"
#include "comparison/metal_exact/metal_device.h"
#include <limits>

namespace mb3d::native_metal {
struct SeedScanResult {uint64_t draws=0,commands=0;double gpu_ms=0,max_command_ms=0;};
// Input/output contains per-pixel flags/counts followed by owner exit slots.
// Only small per-owner totals return to the host; pixel seed generation is GPU.
inline SeedScanResult RunSeedScan(metal_exact::gpu::Metal& metal,const SeedScanPlan& plan,
    id<MTLBuffer> words,uint64_t budget,const std::function<void()>& poll) {
    namespace g=metal_exact::gpu;using mb3d_seed_scan::Params;
    g::require(!plan.levels.empty(),"Empty seed scan plan");const auto& first=plan.levels.front();
    const auto checked=PlanSeedScan(first.width,first.height,first.owners,first.mapping,
        first.rows_per_owner,first.input_kind,first.initial_seed);
    g::require(checked.words==plan.words&&checked.scratch_bytes==plan.scratch_bytes&&
        checked.levels.size()==plan.levels.size()&&
        !std::memcmp(checked.levels.data(),plan.levels.data(),plan.levels.size()*sizeof(Params)),"Changed seed scan plan");
    g::require(words&&words.length==plan.words*4&&budget>=plan.scratch_bytes&&bool(poll),"Seed scan allocation/poll");
    g::require(metal.arithmeticProfile==std::array<uint32_t,3>{2,4,32},"Seed scan profile");
    std::vector<id<MTLBuffer>> data{words},totals;
    for(const auto& p:plan.levels){auto buffer=metal.buffer(uint64_t(p.owners)*p.groups*4);totals.push_back(buffer);data.push_back(buffer);}
    uint32_t zero=0;auto fault=metal.buffer(4,&zero),identity=metal.buffer(12);
    SeedScanResult result;
    const auto run=[&](const char* name,uint32_t count,const std::function<void(id<MTLComputeCommandEncoder>)>& bind,uint32_t group=0){
        poll();const double ms=g::dispatchStage(metal,metal.pipeline(g::ns(name)),count,bind,1,group);
        g::require(std::isfinite(ms)&&ms>=0,"Seed scan timestamp");
        ++result.commands;result.gpu_ms+=ms;result.max_command_ms=std::max(result.max_command_ms,ms);
        g::require(*static_cast<const uint32_t*>(fault.contents)==0,"Seed scan GPU fault");poll();
    };
    run("native_seed_scan_abi",1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:identity offset:0 atIndex:0];});
    const uint32_t expected[]={mb3d_seed_scan::Version,sizeof(Params),mb3d_seed_scan::Group};
    g::require(!std::memcmp(identity.contents,expected,sizeof(expected)),"Seed scan ABI");
    const auto bind=[&](id<MTLComputeCommandEncoder> e,const Params& p,id<MTLBuffer> a,id<MTLBuffer> b){
        [e setBuffer:a offset:0 atIndex:0];[e setBuffer:b offset:0 atIndex:1];
        [e setBytes:&p length:sizeof(p) atIndex:2];[e setBuffer:fault offset:0 atIndex:3];
    };
    for(size_t i=0;i<plan.levels.size();++i){auto p=plan.levels[i];
        for(uint32_t base=0;base<p.owners*p.groups;base+=1024){p.offset=base;p.count=std::min(1024u,p.owners*p.groups-base);
            run("native_seed_scan_blocks",p.count*mb3d_seed_scan::Group,
                [&](id<MTLComputeCommandEncoder> e){bind(e,p,data[i],totals[i]);},mb3d_seed_scan::Group);}
    }
    for(size_t i=plan.levels.size()-1;i>0;--i){auto p=plan.levels[i-1];
        for(uint32_t base=0;base<p.owners*p.items;){p.offset=base;p.count=std::min(262144u,p.owners*p.items-base);
            run("native_seed_scan_add",p.count,[&](id<MTLComputeCommandEncoder> e){bind(e,p,data[i-1],data[i]);});base+=p.count;}
    }
    for(uint32_t base=0;base<first.owners*first.items;){auto p=first;p.offset=base;p.count=std::min(262144u,p.owners*p.items-base);
        run("native_seed_scan_finish",p.count,[&](id<MTLComputeCommandEncoder> e){bind(e,p,words,totals.back());});base+=p.count;}
    const auto* counts=static_cast<const uint32_t*>(totals.back().contents);
    for(uint32_t owner=0;owner<first.owners;++owner)result.draws+=counts[owner];
    g::require(result.draws<=uint64_t(first.width)*first.height*(first.input_kind==mb3d_seed_scan::PrimaryFlags?1u:33u),"Seed scan total exceeds bound");
    return result;
}
}
