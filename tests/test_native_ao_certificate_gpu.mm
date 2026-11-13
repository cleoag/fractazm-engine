#include "render/native_metal/native_ao_dispatch.h"
#include "core/numeric_precision.h"
#include <array>
#include <vector>
#include <cstring>
#include <cstdio>
using namespace mb3d;using namespace mb3d_native_ao;
namespace gpu=mb3d::metal_exact::gpu;
unsigned checks=0;
void check(bool b,const char* s){++checks;if(!b)throw std::runtime_error(s);}
template<class F>void reject(F f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}check(rejected,"AO certificate malformed control accepted");}
void collective_seed_controls(gpu::Metal& m) {
    using Table=mb3d_native_ao_reuse::BaseTable;
    for(U32 workers:{1u,3u,6u,32u}) {
        TMandHeader10 h{};h.Width=257;h.Height=workers*3+1;h.dZoom=1;h.dFOVy=45;
        h.dZstart=-2;h.dZend=2;h.bCalc3D=1;h.bCalcAmbShadowAutomatic=9;
        h.sAmbShadowThreshold=1;h.SSAORcount=2;h.bSSAO24BorderMirrorSize=30;
        auto preparation=PrepareNativeScreenAO(h,h.Width,h.Height,workers);
        const auto p=preparation.params;const auto l=preparation.levels.front();
        const U32 n=p.width*p.height,bytes=n*32u,unknown=1,chunks=8;
        if(workers==32){
            auto many=PrepareNativeScreenAO(h,h.Width,h.Height,33);
            check(native_metal::ProductionScreenAOExecutionPolicy(many).seed_continuation_chunks()==1,"Large owner set entered bounded collective");
            auto plain=h;plain.bCalcAmbShadowAutomatic=5;
            auto noRandom=PrepareNativeScreenAO(plain,plain.Width,plain.Height,3);
            check(!random_mode(noRandom.params.algorithm)&&native_metal::ProductionScreenAOExecutionPolicy(noRandom).seed_continuation_chunks()==1,"Nonrandom AO entered seed collective");
        }
        check(p.logical_lanes==workers,"Collective fixture owner count");
        Table table{};table.version=1;table.count=192;table.params=p;table.level=l;
        auto bundle=m.buffer(sizeof(table),&table);
        std::vector<U32> zeros(n*8,0);auto certificates=m.buffer(bytes,zeros.data());
        struct Buffers {id<MTLBuffer> lanes,seeds,usage;};
        const auto fresh=[&]{
            std::vector<LaneState> lanes(workers);std::vector<SeedRecord> seeds(n);
            return Buffers{m.buffer(lanes.size()*sizeof(LaneState),lanes.data()),
                m.buffer(seeds.size()*sizeof(SeedRecord),seeds.data()),m.buffer(n*4,zeros.data())};
        };
        for(int faultCase:{-1,0,1,2}) {
            std::vector<Record> records(n);
            for(U32 i=0;i<n;++i){records[i].version=1;records[i].algorithm=p.algorithm;records[i].active=i%11!=0;}
            if(faultCase>=0){
                const U32 owner=workers-1,ordinal=faultCase==0?0: faultCase==1?130:640;
                const U32 pixel=(ordinal/p.width*workers+owner)*p.width+ordinal%p.width;
                records[pixel].active=1;
                if(faultCase==2)records[pixel].samples=p.work_limit;
                else records[pixel].fault=Invalid;
            }
            auto input=m.buffer(n*sizeof(Record),records.data());
            const auto dispatch=[&](Buffers& b,bool collective,U32 reset,U32 quota=8,U32 extra=0,U32 width=0){
                auto pipeline=m.pipeline(gpu::ns(collective?"native_ao_seed_lanes_collective":"native_ao_seed_lanes_certified"));
                gpu::dispatchStage(m,pipeline,workers+extra,[&](id<MTLComputeCommandEncoder> e){
                    [e setBuffer:input offset:0 atIndex:0];[e setBuffer:b.lanes offset:0 atIndex:3];
                    [e setBytes:&reset length:4 atIndex:4];[e setBytes:&n length:4 atIndex:5];
                    [e setBytes:&workers length:4 atIndex:6];[e setBuffer:b.seeds offset:0 atIndex:7];
                    [e setBuffer:bundle offset:0 atIndex:8];[e setBuffer:certificates offset:0 atIndex:9];
                    [e setBytes:&bytes length:4 atIndex:10];[e setBuffer:b.usage offset:0 atIndex:11];
                    [e setBytes:&unknown length:4 atIndex:12];[e setBytes:&n length:4 atIndex:13];
                    if(collective)[e setBytes:&quota length:4 atIndex:14];
                },1,width?width:workers);
            };
            auto reference=fresh();bool failed=false;
            for(U32 i=0;i<chunks;++i){dispatch(reference,false,i==0);
                const auto* lanes=static_cast<const LaneState*>(reference.lanes.contents);
                for(U32 owner=0;owner<workers;++owner)failed|=lanes[owner].fault!=0;
                if(failed)break;
            }
            check(failed==(faultCase>=0),"First-fault fixture did not exercise its intended path");
            for(U32 repeat=0;repeat<3;++repeat){auto actual=fresh();dispatch(actual,true,1);
                check(!std::memcmp(actual.lanes.contents,reference.lanes.contents,workers*sizeof(LaneState)),"Collective changed first-fault owner cursors/seeds");
                check(!std::memcmp(actual.seeds.contents,reference.seeds.contents,n*sizeof(SeedRecord)),"Collective changed seed plans or advanced after fault");
                check(!std::memcmp(actual.usage.contents,reference.usage.contents,n*4),"Collective changed certificate usage");
                check(!std::memcmp(input.contents,records.data(),n*sizeof(Record)),"Collective mutated geometry records");
                if(faultCase==-1&&workers==32){
                    auto continued=fresh();for(U32 i=0;i<chunks+1;++i)dispatch(continued,false,i==0);
                    dispatch(actual,true,0,1);
                    check(!std::memcmp(actual.lanes.contents,continued.lanes.contents,workers*sizeof(LaneState)),"Collective reset-free continuation changed cursors");
                    check(!std::memcmp(actual.seeds.contents,continued.seeds.contents,n*sizeof(SeedRecord)),"Collective reset-free continuation changed plans");
                }
            }
            if(faultCase==-1){
                uint64_t draws=0;const auto* plans=static_cast<const SeedRecord*>(reference.seeds.contents);
                for(U32 i=0;i<n;++i)draws+=plans[i].draws;
                check(draws>0,"Collective fixture performed no random planning");
                for(U32 malformed:{0u,1u,2u,3u}){auto actual=fresh();
                    dispatch(actual,true,1,malformed==0?0:malformed==1?9:8,malformed==2?1:0,malformed==3&&workers>1?1:workers);
                    if(malformed==3&&workers==1)continue;
                    const auto* lanes=static_cast<const LaneState*>(actual.lanes.contents);
                    for(U32 owner=0;owner<workers;++owner)check(lanes[owner].fault!=0,"Malformed collective launch accepted");
                    auto empty=fresh();
                    check(!std::memcmp(actual.seeds.contents,empty.seeds.contents,n*sizeof(SeedRecord)),"Malformed launch wrote seed plans");
                }
            }
        }
    }
}
void certified_prefix_fault_controls(gpu::Metal& m) {
    TMandHeader10 h{};h.Width=37;h.Height=17;h.dZoom=1;h.dFOVy=45;
    h.dZstart=-2;h.dZend=2;h.bCalc3D=1;h.bCalcAmbShadowAutomatic=9;
    h.sAmbShadowThreshold=1;h.SSAORcount=2;h.bSSAO24BorderMirrorSize=30;
    auto preparation=PrepareNativeScreenAO(h,h.Width,h.Height,1);
    for(int faultAt:{0,2,31,-1})for(U32 repeat=0;repeat<3;++repeat){
        auto p=preparation.params;const auto l=preparation.levels.front();
        if(faultAt<0)p.work_limit=9;
        const U32 n=p.width*p.height,bytes=n*32u,workers=1,reset=1,unknown=0;
        mb3d_native_ao_reuse::BaseTable table{};table.version=1;table.count=192;table.params=p;table.level=l;
        std::vector<Record> records(n);records[0].version=1;records[0].algorithm=p.algorithm;records[0].active=1;
        if(faultAt<0)records[0].samples=p.work_limit-1;
        std::vector<unsigned char> cert(bytes,0);
        if(faultAt<0){cert[0]=1;cert[1]=1;}
        else{for(int d=0;d<faultAt;++d)cert[d]=1;cert[faultAt]=static_cast<unsigned char>(l.samples+1);}
        std::vector<LaneState> lanes(workers);std::vector<SeedRecord> plans(n);std::vector<U32> usage(n);
        auto input=m.buffer(n*sizeof(Record),records.data()),laneBuffer=m.buffer(sizeof(LaneState),lanes.data());
        auto seeds=m.buffer(n*sizeof(SeedRecord),plans.data()),used=m.buffer(n*4,usage.data());
        auto bundle=m.buffer(sizeof(table),&table),certificates=m.buffer(bytes,cert.data());
        gpu::dispatchStage(m,m.pipeline(gpu::ns("native_ao_seed_lanes_certified")),1,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:input offset:0 atIndex:0];[e setBuffer:laneBuffer offset:0 atIndex:3];
            [e setBytes:&reset length:4 atIndex:4];[e setBytes:&n length:4 atIndex:5];
            [e setBytes:&workers length:4 atIndex:6];[e setBuffer:seeds offset:0 atIndex:7];
            [e setBuffer:bundle offset:0 atIndex:8];[e setBuffer:certificates offset:0 atIndex:9];
            [e setBytes:&bytes length:4 atIndex:10];[e setBuffer:used offset:0 atIndex:11];
            [e setBytes:&unknown length:4 atIndex:12];[e setBytes:&n length:4 atIndex:13];
        },1,1);
        const U32 draws=faultAt<0?1u:U32(faultAt);U32 expected=(2u*0x324594a1u)+0x24563487u;
        for(U32 i=0;i<draws;++i)expected=214013u*expected+2531011u;
        const auto& actual=*static_cast<const LaneState*>(laneBuffer.contents);
        const auto& plan=*static_cast<const SeedRecord*>(seeds.contents);
        check(actual.seed==expected&&plan.end==expected&&plan.draws==draws,"Certified prefix lost seed advances before fault");
        check(actual.fault==(faultAt<0?WorkExhausted:Invalid)&&plan.fault==actual.fault,"Certified prefix changed first fault");
        check(actual.x==0&&actual.y==0&&!actual.finished,"Certified prefix advanced past faulted pixel");
        check(!std::memcmp(input.contents,records.data(),n*sizeof(Record)),"Certified prefix mutated inputs");
    }
}
int main(int argc,char**argv){@autoreleasepool{try{
    if(argc!=2)throw std::runtime_error("usage: ao_certificate_test NATIVE_LIBRARY");
    ScopedNumericPrecision precision(PrecisionMode::Native);gpu::Metal m(argv[1],3,8,0,false,true);
    collective_seed_controls(m);
    certified_prefix_fault_controls(m);
    using Policy=native_metal::ScreenAOExecutionPolicy;using Geometry=Policy::SeedGeometry;
    uint64_t allKnown=0,allUsed=0,allActive=0;
    for(unsigned mode:{4u,5u,8u,9u})for(unsigned width:{3u,37u,129u}){
        TMandHeader10 h{};h.Width=width;h.Height=width==129?97:17;h.dZoom=1;h.dFOVy=45;h.dZstart=-2;h.dZend=2;h.bCalc3D=1;
        h.bCalcAmbShadowAutomatic=1|((mode>=6?mode-4:mode)<<1);h.bPlanarOptic=mode>=6?2:0;
        h.sAmbShadowThreshold=1;h.SSAORcount=2;h.bSSAO24BorderMirrorSize=30;
        auto p=PrepareNativeScreenAO(h,width+1,h.Height,3);const auto n=p.params.output_count;
        std::vector<ExactSurface> input(n);
        for(unsigned i=0;i<n;++i){input[i].depth_packed=(i%13==0?0x80000000u:((i*12791)%8388352u)<<8);input[i].amb_shadow=(i*17)%16000;}
        auto buffer=m.buffer(n*sizeof(ExactSurface),input.data());
        const Policy ordered(p),enabled=native_metal::ProductionScreenAOExecutionPolicy(p);
        const Policy unknown(p,true,Policy::LaneGrouping::LegacySimd,native_metal::NativeImageCommandPolicy{},Geometry::ForcedUnknown);
        check(enabled.uses_certificates()&&!ordered.uses_certificates()&&unknown.force_unknown(),"AO policy selection");
        check(screen_scratch_bytes(p.params,true)-screen_scratch_bytes(p.params)==3540ull+40ull*p.params.width*p.params.height,"AO certificate scratch closure");
        auto old=RunNativeScreenAO(m,p,buffer,1ull<<30,ordered,[]{});
        std::vector<ExactSurface> reference(n);std::memcpy(reference.data(),old.surface.contents,n*sizeof(ExactSurface));
        std::array<uint64_t,4> repeatCounts{};
        for(unsigned repeat=0;repeat<3;++repeat)for(bool forced:{false,true}){
            auto result=RunNativeScreenAO(m,p,buffer,1ull<<30,forced?unknown:enabled,[]{});
            check(!std::memcmp(reference.data(),result.surface.contents,n*sizeof(ExactSurface)),"Certificate changed packed surface");
            check(!std::memcmp(&old.totals,&result.totals,sizeof(Totals)),"Certificate changed semantic work");
            check(result.seed_planning_draws==old.seed_planning_draws&&result.seed_shortcuts==old.seed_shortcuts,"Certificate changed seeds/legacy shortcuts");
            check(result.certificate_version==1&&result.certificate_levels==p.levels.size()&&result.certificate_pixels==uint64_t(p.params.width)*p.params.height*p.levels.size(),"Missing actual certificate levels/pixels");
            const uint64_t extra=2+p.levels.size()*(1+(p.params.width*p.params.height+16383)/16384);
            const uint64_t calls=(((p.params.height+p.params.logical_lanes-1)/p.params.logical_lanes)*p.params.width+127)/128;
            const uint64_t chunks=(forced?unknown:enabled).seed_continuation_chunks();
            const uint64_t saved=p.levels.size()*(calls-(calls+chunks-1)/chunks);
            const bool prefix=(forced?unknown:enabled).uses_chunk_prefix();
            const uint64_t prefixExtra=prefix?1+p.levels.size()*((calls*p.params.logical_lanes+1023)/1024+(calls+chunks-1)/chunks):0;
            check(result.dispatches==old.dispatches+extra-saved+prefixExtra&&result.commands==old.commands+extra-saved+prefixExtra,"Certificate command accounting");
            check(result.certificate_used<=result.certificate_known&&(!forced||result.certificate_used==0),"Certificate forced-unknown control");
            const std::array<uint64_t,4> counts={result.certificate_levels,result.certificate_pixels,result.certificate_known,result.certificate_used};
            if(!forced){if(repeat==0)repeatCounts=counts;else check(counts==repeatCounts,"Certificate repeat work");
                allKnown+=result.certificate_known;allUsed+=result.certificate_used;allActive+=result.totals.changed;}
            else check(result.certificate_known==repeatCounts[2],"Forced unknown changed classification");
            check(!std::memcmp(input.data(),buffer.contents,n*sizeof(ExactSurface)),"Certificate mutated input");
        }
        unsigned polls=0;
        reject([&]{RunNativeScreenAO(m,p,buffer,screen_scratch_bytes(p.params,true)-1,enabled,[&]{++polls;});});
        check(polls==0,"Certificate under-budget dispatch reached GPU");
        reject([&]{RunNativeScreenAO(m,p,buffer,1ull<<30,enabled,[&]{if(++polls==9)throw std::runtime_error("cancel");});});
        auto limited=p;limited.params.work_limit=1;
        reject([&]{RunNativeScreenAO(m,limited,buffer,1ull<<30,native_metal::ProductionScreenAOExecutionPolicy(limited),[]{});});
        const auto level=p.levels.front();const U32 tag=(1u<<28)|(level.repeat_index<<16)|(level.index<<8);
        reject([&]{certificate_count(tag|33u,level);});reject([&]{certificate_count(tag^(1u<<8),level);});
        reject([&]{validate_certificate_usage(tag|2u,1,level,false,false,false);});
        reject([&]{validate_certificate_usage(tag|1u,1,level,true,false,false);});
        reject([&]{validate_certificate_usage(tag|1u,1,level,false,true,false);});
        reject([&]{validate_certificate_usage(tag|1u,1,level,false,false,true);});
        mb3d_native_ao_reuse::BaseTable table{};table.version=1;table.count=192;table.params=p.params;table.level=level;
        validate_certificate_table(table,p.params,level);
        table.params.directions[0]=-table.params.directions[0];
        reject([&]{validate_certificate_table(table,p.params,level);});
    }
    check(allKnown>0&&allUsed>0&&allActive>0,"Certificate controls inactive");
    std::printf("{\"status\":\"PASS\",\"checks\":%u,\"classified\":%llu,\"used\":%llu,\"active\":%llu}\n",checks,(unsigned long long)allKnown,(unsigned long long)allUsed,(unsigned long long)allActive);return 0;
}catch(const std::exception&e){std::fprintf(stderr,"AO certificate failed: %s\n",e.what());return 1;}}}
