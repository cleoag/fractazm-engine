#include "render/native_metal/seed_scan_dispatch.h"
#include "render/native_metal/native_ao_readback.h"
#include <cstdio>
#include <tuple>
namespace g=mb3d::metal_exact::gpu;
namespace s=mb3d_seed_scan;
using namespace mb3d::native_metal;
unsigned checks=0;
void check(bool condition,const char* message){++checks;if(!condition)throw std::runtime_error(message);}
template<class F>void rejects(F f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}check(rejected,"Malformed seed scan accepted");}
void de_weight_controls(g::Metal& m){
    using namespace mb3d_native_ao;
    DEParams p{};p.version=mb3d_native_ao::Version;p.width=p.full_width=3;p.height=p.full_height=513;
    p.task_rows=37;p.task_count=(p.height+p.task_rows-1)/p.task_rows;p.first_step_random=1;
    p.max_steps=1024;p.work_limit=65536;p.base_stop=p.step_width=p.max_distance_base=1;
    p.z_corr=p.z_mul=p.correction_weight=p.angular_spacing=1;p.step_multiplier=2;
    const U32 n=p.width*p.height;std::vector<ExactSurface> surface(n);
    for(U32 i=0;i<n;++i){surface[i].depth_packed=i%7?100u:0x80000000u;surface[i].si_gradient=i%11?0u:32768u;}
    auto input=m.buffer(n*sizeof(ExactSurface),surface.data());
    const Launch launch{0,n,n,0};const auto pipeline=m.pipeline(g::ns("native_seed_de_weights"));
    for(U32 quality=0;quality<=3;++quality)for(U32 dither=0;dither<=255;++dither)for(int top:{-256,0}){
        p.quality=quality;p.dither=dither;p.top=top;
        for(U32 row=1;row<=3;++row)p.row_counts[row]=row<=quality?1u<<(row+1u):0u;
        check(valid_de(p),"Invalid DE weight fixture");
        std::vector<U32> initial(n+p.task_count,0xa5a5a5a5u);U32 zero=0;
        auto words=m.buffer(initial.size()*4,initial.data()),fault=m.buffer(4,&zero);
        g::dispatchStage(m,pipeline,n,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:input offset:0 atIndex:0];[e setBuffer:words offset:0 atIndex:1];
            [e setBytes:&p length:sizeof(p) atIndex:2];[e setBytes:&launch length:sizeof(launch) atIndex:3];
            [e setBuffer:fault offset:0 atIndex:4];});
        check(*static_cast<const U32*>(fault.contents)==0,"DE weight device fault");
        const auto* values=static_cast<const U32*>(words.contents);
        bool same=true;for(U32 i=0;i<n;++i)same&=values[i]==(de_active(surface[i])?de_expected_rays(p,i):0u);
        check(same,"GPU weight differs from original host ray count");
        check(!std::memcmp(values+n,initial.data()+n,p.task_count*4),"DE weights overwrote owner exit slots");
    }
    check(!std::memcmp(input.contents,surface.data(),n*sizeof(ExactSurface)),"DE weights mutated surface");
}
int main(int argc,char** argv){@autoreleasepool{try{
    g::require(argc==2,"usage: seed-scan-test NATIVE_LIBRARY");g::Metal m(argv[1],3,8,0,false,true);
    de_weight_controls(m);
    for(auto dimensions:{std::pair{3u,17u},std::pair{257u,33u},std::pair{769u,129u},std::pair{769u,513u}})
    for(uint32_t requested:{1u,3u,32u})for(uint32_t mapping:{s::Interleaved,s::Contiguous})
    for(uint32_t kind:{s::PrimaryFlags,s::DrawCounts}){
        const auto [width,height]=dimensions;const uint32_t rows=(height+std::min(height,requested)-1)/std::min(height,requested);
        const uint32_t owners=mapping==s::Interleaved?std::min(height,requested):(height+rows-1)/rows;
        const auto plan=PlanSeedScan(width,height,owners,mapping,rows,kind,0xfedcba98u);
        std::vector<uint32_t> input(plan.words,0xa5a5a5a5u),expected(plan.words);uint64_t draws=0;
        for(uint32_t i=0;i<width*height;++i)input[i]=(i*137u+5u)%(kind==s::PrimaryFlags?3u:34u);
        for(uint32_t owner=0;owner<owners;++owner){
            uint32_t seed=kind==s::PrimaryFlags?(owner+2u)*0x324594a1u+0x24563487u:0xfedcba98u;
            for(uint32_t r=0;r<rows;++r){const uint32_t y=mapping==s::Interleaved?r*owners+owner:owner*rows+r;
                if(y>=height)continue;
                for(uint32_t x=0;x<width;++x){const uint32_t i=y*width+x;expected[i]=seed;
                    const uint32_t n=kind==s::PrimaryFlags?uint32_t(input[i]==1):input[i];draws+=n;
                    // Deliberately use scalar recurrence, not the GPU jump or scan.
                    for(uint32_t j=0;j<n;++j)seed=214013u*seed+2531011u;
                }
            }
            expected[width*height+owner]=seed;
        }
        for(unsigned repeat=0;repeat<3;++repeat){
            auto words=m.buffer(input.size()*4,input.data());unsigned polls=0;
            const auto result=RunSeedScan(m,plan,words,plan.scratch_bytes,[&]{++polls;});
            check(!std::memcmp(words.contents,expected.data(),expected.size()*4),"GPU scan seed/owner-exit differs from scalar oracle");
            check(result.draws==draws,"GPU scan lost draw counts");
            check(result.commands>=2*plan.levels.size()+1&&polls==2*result.commands,"GPU scan omitted stage/poll");
        }
    }
    const auto plan=PlanSeedScan(37,17,3,s::Interleaved,6,s::PrimaryFlags);
    std::vector<uint32_t> input(plan.words,0);auto words=m.buffer(input.size()*4,input.data());unsigned polls=0;
    rejects([&]{RunSeedScan(m,plan,words,plan.scratch_bytes-1,[&]{++polls;});});
    check(!polls,"Under-budget scan reached GPU");
    auto altered=plan;altered.levels[0].reserved=1;
    rejects([&]{RunSeedScan(m,altered,words,plan.scratch_bytes,[]{});});
    auto shortBuffer=m.buffer(4);rejects([&]{RunSeedScan(m,plan,shortBuffer,plan.scratch_bytes,[]{});});
    rejects([&]{PlanSeedScan(UINT32_MAX,2,1,s::Interleaved,2,s::PrimaryFlags);});
    rejects([&]{PlanSeedScan(37,17,3,s::Contiguous,9,s::PrimaryFlags);});
    input[19]=3;auto invalid=m.buffer(input.size()*4,input.data());
    rejects([&]{RunSeedScan(m,plan,invalid,plan.scratch_bytes,[]{});});
    auto cancelled=m.buffer(input.size()*4,input.data());
    rejects([&]{RunSeedScan(m,plan,cancelled,plan.scratch_bytes,[]{throw std::runtime_error("cancel");});});
    std::printf("{\"status\":\"PASS_SCOPED\",\"checks\":%u}\n",checks);return 0;
}catch(const std::exception& e){std::fprintf(stderr,"Seed scan: %s\n",e.what());return 1;}}}
