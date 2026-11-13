#include "render/native_metal/native_ao_dispatch.h"
#include "render/native_metal/native_ao_seed_chunk_abi.h"
#include "core/numeric_precision.h"
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdio>
using namespace mb3d;using namespace mb3d_native_ao;
namespace g=mb3d::metal_exact::gpu;namespace c=mb3d_native_ao_chunk;
unsigned checks=0;void check(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
struct Buffers {id<MTLBuffer> lanes,seeds,usage;};
int main(int argc,char**argv){@autoreleasepool{try{
    if(argc!=2)throw std::runtime_error("usage: chunk_test NATIVE_LIBRARY");
    ScopedNumericPrecision precision(PrecisionMode::Native);g::Metal m(argv[1],3,8,0,false,true);
    unsigned knownSeen=0,unknownSeen=0,executedSeen=0;
    for(U32 owners:{1u,3u,6u,32u})for(int failure:{-3,-2,-1,0,130,640,1025})for(U32 repetition=0;repetition<3;++repetition){
        TMandHeader10 h{};h.Width=257;h.Height=owners*5+1;h.dZoom=1;h.dFOVy=45;h.dZstart=-2;h.dZend=2;
        h.bCalc3D=1;h.bCalcAmbShadowAutomatic=9;h.sAmbShadowThreshold=1;h.SSAORcount=2;h.bSSAO24BorderMirrorSize=30;
        auto preparation=PrepareNativeScreenAO(h,h.Width,h.Height,owners);auto p=preparation.params;auto l=preparation.levels.front();
        if(failure==-2){l.minimum_radius=0x1p31f;p.directions[0]=1.f;}
        if(failure==-3)for(auto& d:p.directions)d=0.f;
        const U32 n=p.width*p.height,bytes=n*32u,zero=0,cp=(((p.height+owners-1)/owners)*p.width+127)/128,total=cp*owners;
        mb3d_native_ao_reuse::BaseTable table{};table.version=1;table.count=192;table.params=p;table.level=l;
        auto bundle=m.buffer(sizeof(table),&table);
        // The reused base coordinates must be the actual device-produced table.
        g::dispatchStage(m,m.pipeline(g::ns("native_ao_reuse_table")),32,[&](id<MTLComputeCommandEncoder> e){
            [e setBytes:&p length:sizeof(p) atIndex:0];[e setBytes:&l length:sizeof(l) atIndex:1];[e setBuffer:bundle offset:0 atIndex:2];
        });
        const auto pixel=[&](U32 owner,U32 ordinal){return ((ordinal/p.width)*owners+owner)*p.width+ordinal%p.width;};
        std::vector<Record> records(n);std::vector<unsigned char> cert(bytes,failure<-1?0:1);
        for(U32 i=0;i<n;++i){records[i].version=1;records[i].algorithm=p.algorithm;records[i].active=i%11!=0;}
        // A genuine unknown branch is retained among known chunks. This test
        // checks consumer equivalence; producer truth has separate fixtures.
        const U32 unknown=pixel(0,300);records[unknown].active=1;cert[unknown*32]=0;
        if(failure>=0){const U32 i=pixel(owners-1,U32(failure));records[i].active=1;
            if(failure==640)records[i].samples=p.work_limit;
            else if(failure==1025)cert[i*32+7]=static_cast<unsigned char>(l.samples+1);
            else records[i].fault=Invalid;}
        auto input=m.buffer(n*sizeof(Record),records.data()),certificates=m.buffer(bytes,cert.data());
        const auto fresh=[&]{std::vector<LaneState> lanes(owners);std::vector<SeedRecord> seeds(n);std::vector<U32> usage(n);
            return Buffers{m.buffer(owners*sizeof(LaneState),lanes.data()),m.buffer(n*sizeof(SeedRecord),seeds.data()),m.buffer(n*4,usage.data())};};
        auto reference=fresh(),actual=fresh();auto prefixes=m.buffer(n*sizeof(c::Pixel)),chunks=m.buffer(total*sizeof(c::Chunk)),fault=m.buffer(4,&zero);
        const c::Run prepare{cp,total,0,total};
        g::dispatchStage(m,m.pipeline(g::ns("native_ao_chunk_prepare")),total*128,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:bundle offset:0 atIndex:0];[e setBuffer:input offset:0 atIndex:1];[e setBuffer:certificates offset:0 atIndex:2];
            [e setBuffer:prefixes offset:0 atIndex:3];[e setBuffer:chunks offset:0 atIndex:4];[e setBytes:&prepare length:sizeof(prepare) atIndex:5];[e setBuffer:fault offset:0 atIndex:6];
        },1,128);
        check(*static_cast<const U32*>(fault.contents)==0,"Chunk producer fault");
        for(U32 i=0;i<total;++i){const auto row=static_cast<const c::Chunk*>(chunks.contents)[i];knownSeen+=row.known;unknownSeen+=!row.known;}
        const auto dispatch=[&](Buffers& b,bool fast,U32 reset,U32 batch,U32 count){
            const c::Run run{cp,total,batch,count};
            g::dispatchStage(m,m.pipeline(g::ns(fast?"native_ao_chunk_lanes":"native_ao_seed_lanes_certified")),owners,[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:input offset:0 atIndex:0];[e setBuffer:b.lanes offset:0 atIndex:3];[e setBytes:&reset length:4 atIndex:4];
                [e setBytes:&n length:4 atIndex:5];[e setBytes:&owners length:4 atIndex:6];[e setBuffer:b.seeds offset:0 atIndex:7];
                [e setBuffer:bundle offset:0 atIndex:8];[e setBuffer:certificates offset:0 atIndex:9];[e setBytes:&bytes length:4 atIndex:10];
                [e setBuffer:b.usage offset:0 atIndex:11];[e setBytes:&zero length:4 atIndex:12];[e setBytes:&n length:4 atIndex:13];
                if(fast){[e setBytes:&count length:4 atIndex:14];[e setBuffer:chunks offset:0 atIndex:15];[e setBytes:&run length:sizeof(run) atIndex:16];}
            },1,owners);
            if(fast)g::dispatchStage(m,m.pipeline(g::ns("native_ao_chunk_scatter")),owners*count*128,[&](id<MTLComputeCommandEncoder> e){
                [e setBuffer:bundle offset:0 atIndex:0];[e setBuffer:prefixes offset:0 atIndex:1];[e setBuffer:chunks offset:0 atIndex:2];
                [e setBuffer:b.seeds offset:0 atIndex:3];[e setBuffer:b.usage offset:0 atIndex:4];[e setBytes:&run length:sizeof(run) atIndex:5];[e setBuffer:fault offset:0 atIndex:6];
            });
        };
        const auto failed=[&](Buffers& b){U32 result=0;for(U32 i=0;i<owners;++i)result|=static_cast<const LaneState*>(b.lanes.contents)[i].fault;return result;};
        for(U32 batch=0;batch<cp;batch+=8){const U32 count=std::min(8u,cp-batch);
            for(U32 k=0;k<count;++k){dispatch(reference,false,batch==0&&k==0,batch+k,1);if(failed(reference))break;}
            dispatch(actual,true,batch==0,batch,count);
            check(!std::memcmp(actual.lanes.contents,reference.lanes.contents,owners*sizeof(LaneState)),"Changed chunk cursor/seed/fault frontier");
            check(!std::memcmp(actual.seeds.contents,reference.seeds.contents,n*sizeof(SeedRecord)),"Changed canonical seed plans before/after fault");
            check(!std::memcmp(actual.usage.contents,reference.usage.contents,n*4),"Changed certificate use at frontier");
            check(!std::memcmp(input.contents,records.data(),n*sizeof(Record)),"Chunk producer mutated accumulated records");
            check(*static_cast<const U32*>(fault.contents)==0,"Chunk scatter fault");
            if(failed(reference))break;
        }
        check(bool(failed(actual))==(failure==-2||failure>=0),"Failure fixture did not exercise its intended branch");
        for(U32 i=0;i<total;++i)executedSeen+=static_cast<const c::Chunk*>(chunks.contents)[i].executed;
    }
    check(knownSeen&&unknownSeen&&executedSeen,"Chunk controls did no real work");
    std::printf("{\"status\":\"PASS\",\"checks\":%u,\"known_chunks\":%u,\"unknown_chunks\":%u,\"executed_chunks\":%u}\n",checks,knownSeen,unknownSeen,executedSeen);return 0;
}catch(const std::exception& e){std::fprintf(stderr,"Chunk checks failed: %s\n",e.what());return 1;}}}
