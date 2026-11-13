// Native DOF device controls: CPU integer sort and independent image invariants.
#include "render/native_metal/dof_dispatch.h"
#include "render/native_metal/dof_host.h"
#include "comparison/metal_exact/shadow_abi.h"
#include "comparison/metal_exact/color_abi.h"
#include "core/dof.h"
#include "core/numeric_precision.h"
#include <cstdio>
#include <vector>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <limits>
using namespace mb3d;using namespace mb3d::native_metal;using namespace mb3d_dof;
namespace gpu=mb3d::metal_exact::gpu;
int checks=0;void check(bool v,const char* s){++checks;if(!v)throw std::runtime_error(s);}
template<class F> void rejects(F f,const char* why){bool failed=false;try{f();}catch(const std::exception&){failed=true;}check(failed,why);}
TMandHeader10 header(unsigned mode,unsigned plain,int w=3,int h=3){
    TMandHeader10 p{};p.Width=w;p.Height=h;p.dZoom=1;p.dFOVy=45;p.dZstart=-2;p.dZend=2;p.bCalc3D=1;
    p.bCalcDOFtype=1|(mode<<1)|(plain?8:0);p.sDOFaperture=2;p.sDOFclipR=3;return p;
}
std::vector<ExactColorPixel> colors(size_t n){std::vector<ExactColorPixel> v(n);
    for(size_t i=0;i<n;++i)v[i].rgb=uint32_t((17+i*37)%256)|uint32_t((71+i*29)%256)<<8|uint32_t((199+i*19)%256)<<16;return v;}
std::vector<ExactColorPixel> run(gpu::Metal& m,const Params& p,const std::vector<ExactSurface>& s,
                               const std::vector<ExactColorPixel>& c,const DOFLimits& l={},DOFActivity* activity=nullptr){
    auto surface=m.buffer(s.size()*sizeof(ExactSurface),s.data()),input=m.buffer(c.size()*sizeof(ExactColorPixel),c.data());
    auto out=RunNativeDOF(m,p,surface,input,DOFExecutionPolicy(p,l));std::vector<ExactColorPixel> got(c.size());std::memcpy(got.data(),out.rgb.contents,got.size()*sizeof(got[0]));
    check(std::memcmp(surface.contents,s.data(),s.size()*sizeof(s[0]))==0,"DOF mutated source depth");
    check(std::memcmp(input.contents,c.data(),c.size()*sizeof(c[0]))==0,"DOF mutated source RGB");
    check(out.activity.passes==p.passes&&!out.activity.faults,"DOF pass activity");
    check(out.activity.records==uint64_t(p.width)*p.height*p.passes,"DOF validated record count");
    if(activity)*activity=out.activity;return got;
}
void same(const std::vector<ExactColorPixel>& a,const std::vector<ExactColorPixel>& b,const char* s){check(a.size()==b.size()&&std::memcmp(a.data(),b.data(),a.size()*sizeof(a[0]))==0,s);}
int main(int argc,char** argv){@autoreleasepool{try{
    if(argc!=2)throw std::runtime_error("usage: dof_gpu_test NATIVE_LIBRARY");
    const ScopedNumericPrecision precision(PrecisionMode::Native);
    gpu::Metal m(argv[1],3,8,0,false,true);
    uint64_t active_changes=0,cpu_differences=0;
    for(unsigned plain=0;plain<2;++plain)for(unsigned mode=0;mode<4;++mode){
        auto h=header(mode,plain);const auto original=h;auto p=PrepareNativeDOF(h);
        check(std::memcmp(&h,&original,sizeof(h))==0,"Host preparation mutated header");
        check(p.plain==plain&&p.passes==mode+1,"Host mode flags");
        auto c=colors(9);std::vector<ExactSurface> focus(9);for(auto& s:focus)s.depth_packed=8388352u<<8;
        auto f=run(m,p,focus,c);same(f,c,"Exact-focus invariant");
        std::vector<ExactSurface> active(9);const uint32_t keys[]={8388352,8000000,6000000,0,4000000,6000000,8000000,0xffffff,2000000};
        for(unsigned i=0;i<9;++i)active[i].depth_packed=keys[i]<<8;
        DOFActivity activity;auto a=run(m,p,active,c,{},&activity);auto b=run(m,p,active,c);same(a,b,"Repeated DOF RGB differs");
        DOFLimits slices;slices.source_slice=1;slices.gather_slice=1;slices.sort_slice=1;slices.tile_batch=1;
        auto split=run(m,p,active,c,slices);same(a,split,"Split-slice DOF differs");
        check(activity.positive_radius_sources>0&&activity.contributing_pairs>0&&activity.changed_pixels>0,"Active DOF was silently skipped");active_changes+=activity.changed_pixels;
        std::vector<TsiLight5> cpuDepth(9);std::vector<uint8_t> cpuRGB(27);
        for(unsigned i=0;i<9;++i){std::memcpy(&cpuDepth[i].RoughZposFine,&active[i].depth_packed,4);for(unsigned ch=0;ch<3;++ch)cpuRGB[i*3+ch]=(c[i].rgb>>(8*ch))&255;}
        CalcDepthOfField(cpuRGB.data(),cpuDepth.data(),3,3,3,nullptr,&h);
        for(unsigned i=0;i<9;++i)for(unsigned ch=0;ch<3;++ch)cpu_differences+=cpuRGB[i*3+ch]!=((a[i].rgb>>(8*ch))&255);
    }
    // Multiple batches, tied depths and active blur: scheduling cannot change
    // the source permutation, per-target accumulation order or quantization.
    {auto h=header(0,0,129,65);auto p=PrepareNativeDOF(h);
        std::vector<ExactSurface> depth(129*65);auto c=colors(depth.size());
        for(unsigned i=0;i<depth.size();++i)depth[i].depth_packed=(uint32_t((i*17)%7)*1000000u)<<8;
        DOFLimits small,wide;wide.tile_batch=64;DOFActivity a,b;
        auto x=run(m,p,depth,c,small,&a),y=run(m,p,depth,c,wide,&b);
        same(x,y,"Wide GPU DOF tile batching changed ordered RGB");
        check(a.sort_work==b.sort_work&&a.candidate_checks==b.candidate_checks&&a.contributing_pairs==b.contributing_pairs,"Wide DOF changed work");
        check(b.tile_batches<a.tile_batches&&b.dispatches<a.dispatches,"Wide DOF did not batch independent tiles");
    }
    for(unsigned plain=0;plain<2;++plain)for(unsigned mode=0;mode<4;++mode){
        auto h=header(mode,plain,65,49);auto p=PrepareNativeDOF(h);p.clip_radius=24;
        std::vector<ExactSurface> depth(65*49);auto c=colors(depth.size());
        for(unsigned i=0;i<depth.size();++i)depth[i].depth_packed=((i%17==0)?0xffffffu:((i*73)%9)*900000u)<<8;
        DOFLimits old,fast;old.spatial_candidates=false;old.tile_batch=fast.tile_batch=8;
        DOFActivity a,b;auto x=run(m,p,depth,c,old,&a),y=run(m,p,depth,c,fast,&b);
        same(x,y,"Spatial candidates changed ordered DOF RGB");
        check(a.sort_work==b.sort_work&&a.candidate_checks==b.candidate_checks&&a.contributing_pairs==b.contributing_pairs&&a.changed_pixels==b.changed_pixels,"Spatial DOF changed semantic work");
        check(b.candidate_scan_checks<a.candidate_scan_checks,"Spatial DOF did not reduce scans");
    }
    for(unsigned fault=0;fault<2;++fault){
        auto p=PrepareNativeDOF(header(0,0,2,1));const uint32_t count=2;
        Source sources[2]{};sources[0].index=0;sources[1].index=1;
        Item items[2]={{0,0},{0,fault?2u:0u}};uint32_t initial[2]={~0u,~0u};Global global{};
        auto src=m.buffer(sizeof(sources),sources),order=m.buffer(sizeof(items),items),ranks=m.buffer(sizeof(initial),initial),g=m.buffer(sizeof(global),&global);
        gpu::dispatchStage(m,m.pipeline(@"native_dof_inverse_rank"),2,[&](id<MTLComputeCommandEncoder> e){
            [e setBytes:&p length:sizeof(p) atIndex:0];[e setBuffer:src offset:0 atIndex:1];[e setBuffer:order offset:0 atIndex:2];
            [e setBuffer:ranks offset:0 atIndex:3];[e setBuffer:g offset:0 atIndex:4];[e setBytes:&count length:4 atIndex:5];});
        check(static_cast<Global*>(g.contents)->fault!=0,"Malformed inverse permutation accepted");
    }
    // Literal independently derived equal-radius two-color average. Coarse maxR
    // exceeds3; both modes give equal weights to red and blue, yielding128,0,128.
    for(unsigned plain=0;plain<2;++plain){Params p{1,2,1,plain,1,0,0,0,1.f/256,1,0,0,0,2,1000,0};
        std::vector<ExactSurface> s(2);for(auto& x:s)x.depth_packed=(8388352u-256)<<8;
        auto c=colors(2);c[0].rgb=255;c[1].rgb=255u<<16;
        auto a=run(m,p,s,c);check(a[0].rgb==0x800080&&a[1].rgb==0x800080,"Literal active two-color average");}
    // Two-focus zone: depth0 has zero final radius although its raw radius is
    // negative; depth8 has radius2. Sorted far blue is accumulated before near
    // red: the1.01 cap retains .01 blue, giving(252,0,3). Plain downweights its
    // contribution to the zero-radius foreground to zero.
    for(unsigned plain=0;plain<2;++plain){Params p{1,2,1,plain,1,0,0,0,1.f/256,1,0,0,6,2,1000,0};
        std::vector<ExactSurface> s(2);s[0].depth_packed=8388352u<<8;s[1].depth_packed=(8388352u-512)<<8;
        auto c=colors(2);c[0].rgb=255;c[1].rgb=255u<<16;
        auto a=run(m,p,s,c);check(a[0].rgb==(plain?255u:0x0300fcu)&&a[1].rgb==0xff0000u,"Independent two-focus occlusion control");}
    for(unsigned plain=0;plain<2;++plain){Params p{1,1,1,plain,1,0,0,0,1.f/256,1,0,0,0,2,1000,0};
        std::vector<ExactSurface> s(1);s[0].depth_packed=0x80000000;
        auto c=colors(1);c[0].rgb=0xc08040;DOFActivity activity;
        auto a=run(m,p,s,c,{},&activity);same(a,c,"Radius1000 single-pixel normalization");check(activity.maximum_radius==1000,"Radius1000 was silently reduced");}
    for(unsigned plain=0;plain<2;++plain)for(unsigned mode=0;mode<4;++mode){
        auto h=header(mode,plain,1,1);auto p=PrepareNativeDOF(h);auto c=colors(1);
        std::vector<ExactSurface> s(1);s[0].depth_packed=0x80000000;DOFActivity activity;
        auto a=run(m,p,s,c,{},&activity);same(a,c,"Single-pixel multipass invariant");
        check(activity.sort_work==0,"Single-pixel DOF dispatched unnecessary sort");
        check(activity.candidate_scan_checks==p.passes,"Single-pixel scan counter");
    }
    // Odd integer radius1 has nearest-even bounding radius Round(1.5)=2.
    // An interior16x16 tile can therefore have400 candidates, not324.
    for(unsigned plain=0;plain<2;++plain){Params p{1,40,40,plain,1,0,0,0,1.f/256,1,0,0,0,2.f/3,1,0};
        std::vector<ExactSurface> s(1600);for(auto& x:s)x.depth_packed=(8388352u-256)<<8;
        auto c=colors(1600);for(auto& x:c)x.rgb=0xc08040;DOFActivity activity;
        auto a=run(m,p,s,c,{},&activity);same(a,c,"Odd-radius tile capacity/constant-color invariant");check(activity.maximum_radius==2,"Odd-radius nearest-even bound");}
    // Actual exact sort permutation, including tied keys, against existing CPU
    // implementation whose arithmetic is integer-only and independent of MSL.
    for(auto keys:std::vector<std::vector<int>>{{3,3,3,3,3,3,3},{-1,0,1,2,3,4,5},{5,4,3,2,1,0,-1},{2,-1,2,4,2,0,-1}}){
        std::vector<Item> items;std::vector<TSortItem> expected;
        for(unsigned i=0;i<keys.size();++i){items.push_back({keys[i],i});expected.push_back({keys[i],uint16_t(i),0});}
        QuickSortInt(int(expected.size()),expected.data());
        auto buffer=m.buffer(items.size()*sizeof(Item),items.data());SortState initial{};initial.version=1;initial.count=items.size();initial.stack_size=1;initial.stack_hi[0]=int(items.size())-1;
        auto state=m.buffer(sizeof(initial),&initial);SortLaunch l{uint32_t(items.size()),uint32_t(items.size()),1,0,100000};
        auto kernel=m.pipeline(@"native_dof_sort");SortState got{};
        do{gpu::dispatchStage(m,kernel,2,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:buffer offset:0 atIndex:0];[e setBuffer:state offset:0 atIndex:1];[e setBytes:&l length:sizeof(l) atIndex:2];});std::memcpy(&got,state.contents,sizeof(got));check(!got.fault,"Exact sort fault");}while(!got.done);
        const auto* actual=static_cast<const Item*>(buffer.contents);for(unsigned i=0;i<items.size();++i)check(actual[i].source==expected[i].wX&&actual[i].key==expected[i].iZ,"CPU QuickSort permutation mismatch");
    }
    // Plain's32767+fine boundary must not use sorted key's background remap.
    Params p{1,4,1,1,1,0,0,0,1.f/256,1,0,0,0,2,8,0};
    std::vector<ExactSurface> s(5);const uint32_t packed[]={0x7fff0000,0x7fff0100,0x7fffff00,0x80000000};
    for(unsigned i=0;i<4;++i)s[i].depth_packed=packed[i];
    auto surface=m.buffer(s.size()*sizeof(s[0]),s.data());std::vector<Source> source(5);std::memset(&source.back(),0xa5,sizeof(Source));
    auto sources=m.buffer(source.size()*sizeof(Source),source.data());std::vector<Item> items(5);items.back()={123,456};auto order=m.buffer(items.size()*sizeof(Item),items.data());
    Global initial{32768,0,0,0,0,0,0,0};auto global=m.buffer(sizeof(initial),&initial);Pass passInit{};auto pass=m.buffer(sizeof(Pass),&passInit);const uint32_t allocation=4,index=0;
    gpu::dispatchStage(m,m.pipeline(@"native_dof_depth"),5,[&](id<MTLComputeCommandEncoder> e){[e setBytes:&p length:sizeof(p) atIndex:0];[e setBuffer:surface offset:0 atIndex:1];[e setBuffer:sources offset:0 atIndex:2];[e setBuffer:order offset:0 atIndex:3];[e setBuffer:global offset:0 atIndex:4];[e setBytes:&allocation length:4 atIndex:5];});
    gpu::dispatchStage(m,m.pipeline(@"native_dof_pass"),1,[&](id<MTLComputeCommandEncoder> e){[e setBytes:&p length:sizeof(p) atIndex:0];[e setBuffer:global offset:0 atIndex:1];[e setBuffer:pass offset:0 atIndex:2];[e setBytes:&index length:4 atIndex:3];});
    gpu::dispatchStage(m,m.pipeline(@"native_dof_radii"),5,[&](id<MTLComputeCommandEncoder> e){[e setBytes:&p length:sizeof(p) atIndex:0];[e setBuffer:pass offset:0 atIndex:1];[e setBuffer:sources offset:0 atIndex:2];[e setBuffer:global offset:0 atIndex:3];[e setBytes:&allocation length:4 atIndex:4];});
    const auto* v=static_cast<const Source*>(sources.contents);check(v[0].radius==0&&v[1].radius<0&&v[2].radius<0&&v[3].radius==8,"Plain depth boundary classification");
    check(std::memcmp(&v[4],&source[4],sizeof(Source))==0,"Radius overdispatch canary");
    check(static_cast<Global*>(global.contents)->fault==0,"Depth boundary fault");
    // Bypass host validation: malformed GPU descriptors cannot mutate arrays.
    for(unsigned mode=0;mode<7;++mode){auto bad=p;
        if(mode==0)bad.version=2;if(mode==1)bad.width=5;if(mode==2)bad.aperture=NAN;
        if(mode==3)bad.passes=0;if(mode==4)bad.plain=2;if(mode==5)bad.reserved1=1;if(mode==6)bad.inverse_z_corr=INFINITY;
        std::memcpy(global.contents,&initial,sizeof(initial));std::memset(sources.contents,0xa5,sources.length);std::memset(order.contents,0x5a,order.length);
        std::vector<unsigned char> before(sources.length,0xa5),beforeOrder(order.length,0x5a);
        gpu::dispatchStage(m,m.pipeline(@"native_dof_depth"),5,[&](id<MTLComputeCommandEncoder> e){[e setBytes:&bad length:sizeof(bad) atIndex:0];[e setBuffer:surface offset:0 atIndex:1];[e setBuffer:sources offset:0 atIndex:2];[e setBuffer:order offset:0 atIndex:3];[e setBuffer:global offset:0 atIndex:4];[e setBytes:&allocation length:4 atIndex:5];});
        check(static_cast<Global*>(global.contents)->fault!=0,"Malformed device descriptor accepted");
        check(std::memcmp(before.data(),sources.contents,before.size())==0&&std::memcmp(beforeOrder.data(),order.contents,beforeOrder.size())==0,"Malformed descriptor wrote source/order arrays");
    }
    // All input scalars are finite, but depth*FOV overflows a denominator.
    // Sorted fails during coarse-radius preparation; plain during per-pixel radius.
    for(unsigned plain=0;plain<2;++plain){auto overflow=p;overflow.plain=plain;overflow.inverse_z_mul=1;overflow.inverse_z_corr=1;
        overflow.fov_factor=std::numeric_limits<float>::max();overflow.aperture=1;overflow.clip_radius=1;
        std::memcpy(global.contents,&initial,sizeof(initial));std::memset(sources.contents,0,sources.length);
        gpu::dispatchStage(m,m.pipeline(@"native_dof_depth"),4,[&](id<MTLComputeCommandEncoder> e){[e setBytes:&overflow length:sizeof(overflow) atIndex:0];[e setBuffer:surface offset:0 atIndex:1];[e setBuffer:sources offset:0 atIndex:2];[e setBuffer:order offset:0 atIndex:3];[e setBuffer:global offset:0 atIndex:4];[e setBytes:&allocation length:4 atIndex:5];});
        gpu::dispatchStage(m,m.pipeline(@"native_dof_pass"),1,[&](id<MTLComputeCommandEncoder> e){[e setBytes:&overflow length:sizeof(overflow) atIndex:0];[e setBuffer:global offset:0 atIndex:1];[e setBuffer:pass offset:0 atIndex:2];[e setBytes:&index length:4 atIndex:3];});
        gpu::dispatchStage(m,m.pipeline(@"native_dof_radii"),4,[&](id<MTLComputeCommandEncoder> e){[e setBytes:&overflow length:sizeof(overflow) atIndex:0];[e setBuffer:pass offset:0 atIndex:1];[e setBuffer:sources offset:0 atIndex:2];[e setBuffer:global offset:0 atIndex:3];[e setBytes:&allocation length:4 atIndex:4];});
        check(static_cast<Global*>(global.contents)->fault!=0,"Infinite intermediate was masked as zero blur");
    }
    // Both source pixels overlap one tile, but only one candidate is admitted.
    // Physical trailing storage is a canary, not part of the declared capacity.
    // The second dispatched tile is likewise outside the independent allocation.
    for(unsigned plain=0;plain<2;++plain){
        Params tiny{1,2,1,plain,1,0,0,0,1.f/256,1,0,0,0,2,8,0};
        const Source entries[2]={{0,0,1,0,1,0,0,0},{0,1,1,0,1,0,0,0}};
        const Item permutation[2]={{0,0},{0,1}};
        auto entriesBuffer=m.buffer(sizeof(entries),entries),permutationBuffer=m.buffer(sizeof(permutation),permutation);
        auto tileBuffer=m.buffer(2*sizeof(Tile));auto candidateBuffer=m.buffer(2*sizeof(uint32_t));
        const Launch launch{1,1,1,1,0,2,2,0};
        for(unsigned malformed=0;malformed<2;++malformed){
            Tile tiles[2]{};tiles[0]={0,0,2,1,0,0,0,malformed};std::memset(&tiles[1],0xa5,sizeof(Tile));
            const uint32_t candidates[2]={0xdeadbeefu,0xabcdef01u};Global fresh{};
            std::memcpy(tileBuffer.contents,tiles,sizeof(tiles));std::memcpy(candidateBuffer.contents,candidates,sizeof(candidates));
            std::memcpy(global.contents,&fresh,sizeof(fresh));
            gpu::dispatchStage(m,m.pipeline(@"native_dof_candidates"),2,[&](id<MTLComputeCommandEncoder> e){
                [e setBytes:&tiny length:sizeof(tiny) atIndex:0];[e setBytes:&launch length:sizeof(launch) atIndex:1];
                [e setBuffer:entriesBuffer offset:0 atIndex:2];[e setBuffer:permutationBuffer offset:0 atIndex:3];
                [e setBuffer:tileBuffer offset:0 atIndex:4];[e setBuffer:candidateBuffer offset:0 atIndex:5];[e setBuffer:global offset:0 atIndex:6];});
            const auto* actualTiles=static_cast<const Tile*>(tileBuffer.contents);
            const auto* actualCandidates=static_cast<const uint32_t*>(candidateBuffer.contents);
            check(static_cast<Global*>(global.contents)->fault==16,"Invalid candidate list did not report its device fault");
            check(std::memcmp(&actualTiles[1],&tiles[1],sizeof(Tile))==0,"Candidate tile overdispatch wrote outside allocation");
            check(actualCandidates[1]==candidates[1],"Candidate overflow wrote outside declared list capacity");
            check(std::memcmp(entriesBuffer.contents,entries,sizeof(entries))==0&&
                  std::memcmp(permutationBuffer.contents,permutation,sizeof(permutation))==0,"Candidate rejection mutated source/order input");
            if(malformed){
                check(std::memcmp(actualTiles,tiles,sizeof(tiles))==0&&
                      std::memcmp(actualCandidates,candidates,sizeof(candidates))==0,"Reserved tile field was not rejected before writes");
            }else{
                check(actualTiles[0].fault==2&&actualTiles[0].count==1&&actualTiles[0].cursor==2&&actualCandidates[0]==0,
                      "Candidate capacity failure lost its bounded partial-list record");
            }
        }
    }
    auto good=PrepareNativeDOF(header(0,0));auto c=colors(9);std::vector<ExactSurface> d(9);for(auto& q:d)q.depth_packed=0;
    DOFLimits low;low.sort_work_limit=1;rejects([&]{run(m,good,d,c,low);},"Sort work exhaustion accepted");
    low={};low.gather_work_limit=1;rejects([&]{run(m,good,d,c,low);},"Gather work exhaustion accepted");
    low={};low.candidate_scan_work_limit=1;rejects([&]{run(m,good,d,c,low);},"Candidate scan work exhaustion accepted");
    auto bad=good;bad.aperture=NAN;rejects([&]{run(m,bad,d,c);},"NaN parameter accepted");
    bad=good;bad.version=2;rejects([&]{run(m,bad,d,c);},"Invalid version accepted");
    bad=good;bad.reserved0=1;rejects([&]{run(m,bad,d,c);},"Reserved parameter accepted");
    auto input=m.buffer(c.size()*sizeof(c[0]),c.data()),dep=m.buffer(d.size()*sizeof(d[0]),d.data());
    const DOFExecutionPolicy execution(good);
    rejects([&]{RunNativeDOF(m,good,dep,input,execution,[]{throw std::runtime_error("cancel control");});},"Cancellation callback ignored");
    auto altered=good;altered.passes=good.passes==1?2:1;
    unsigned polls=0;bool mismatch=false;
    try{RunNativeDOF(m,altered,dep,input,execution,[&]{++polls;});}
    catch(const std::invalid_argument& e){mismatch=std::string(e.what())=="Native DOF execution policy input mismatch";}
    check(mismatch&&!polls,"Stale DOF policy reached device");
    check(std::memcmp(input.contents,c.data(),c.size()*sizeof(c[0]))==0,"Canceled DOF mutated input");
    std::printf("{\"status\":\"PASS\",\"checks\":%d,\"active_changes\":%llu,\"cpu_channel_differences\":%llu,\"parity_claim\":false}\n",checks,(unsigned long long)active_changes,(unsigned long long)cpu_differences);return 0;
}catch(const std::exception& e){std::fprintf(stderr,"DOF GPU test failed: %s (checks%d)\n",e.what(),checks);return 1;}}}
