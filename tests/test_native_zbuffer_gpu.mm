// Task-local Native stage controls. Independent literals and the existing CPU
// implementation are the oracles; CPU/GPU numerical differences are reported.
#include "render/native_metal/zbuffer_host.h"
#include "render/native_metal/zbuffer_dispatch.h"
#include "comparison/metal_exact/shadow_abi.h"
#include "core/zbuffer_normals.h"
#include "core/numeric_precision.h"
#include <cstdio>
#include <cstring>
#include <vector>
#include <limits>
#include <cmath>
using namespace mb3d;using namespace mb3d::native_metal;using namespace mb3d_zn;
namespace gpu=mb3d::metal_exact::gpu;
unsigned checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
template<class F>void rejects(F f,const char* why){bool failed=false;try{f();}catch(const std::exception&){failed=true;}check(failed,why);}
Params descriptor(unsigned w,unsigned h,unsigned mode=0){
    Params p{};p.version=Version;p.width=p.full_width=w;p.height=p.full_height=h;
    p.mode=mode;p.step=1;p.aspect=1;p.optic_z=1;p.inverse_z_mul=1.f/256;p.depth_correction=1;p.count=w*h;return p;
}
ExactSurface pixel(unsigned i=0){ExactSurface s{};s.normal[0]=123;s.normal[1]=-456;s.normal[2]=789;
    s.depth_packed=8388352u<<8;s.si_gradient=17+i;s.o_trap=291+i;s.shadow=518+i;s.amb_shadow=473+i;return s;}
struct Capture{std::vector<ExactSurface> output;std::vector<Coordinate> positions;std::vector<Record> records;uint32_t fault;};
Capture execute(gpu::Metal& m,const Params& p,const std::vector<ExactSurface>& source,
                const std::vector<Coordinate>* custom=nullptr,const Launch* overrideLaunch=nullptr){
    const unsigned n=unsigned(source.size());check(n>0,"Empty test input");
    std::vector<ExactSurface> padded=source;padded.push_back({});std::memset(&padded.back(),0xa5,sizeof(ExactSurface));
    auto in=m.buffer(padded.size()*sizeof(ExactSurface),padded.data());
    auto pos=m.buffer((n+1)*sizeof(Coordinate)),out=m.buffer((n+1)*sizeof(ExactSurface)),records=m.buffer((n+1)*sizeof(Record)),status=m.buffer(4);
    std::memset(pos.contents,0x5a,pos.length);std::memset(out.contents,0xa5,out.length);std::memset(records.contents,0x5a,records.length);
    *static_cast<uint32_t*>(status.contents)=0;
    Launch l{};l.count=l.source_count=l.coordinate_count=l.output_count=l.record_count=n;if(overrideLaunch)l=*overrideLaunch;
    if(custom){check(custom->size()==n,"Custom positions size");std::memcpy(pos.contents,custom->data(),n*sizeof(Coordinate));}
    else gpu::dispatchStage(m,m.pipeline(@"native_zn_reconstruct"),n+3,[&](id<MTLComputeCommandEncoder> e){
        [e setBytes:&p length:sizeof(p) atIndex:0];[e setBytes:&l length:sizeof(l) atIndex:1];
        [e setBuffer:in offset:0 atIndex:2];[e setBuffer:pos offset:0 atIndex:3];[e setBuffer:status offset:0 atIndex:4];});
    // A failed first stage must never be consumed by the next one.
    if(!*static_cast<uint32_t*>(status.contents))gpu::dispatchStage(m,m.pipeline(@"native_zn_normals"),n+3,[&](id<MTLComputeCommandEncoder> e){
        [e setBytes:&p length:sizeof(p) atIndex:0];[e setBytes:&l length:sizeof(l) atIndex:1];
        [e setBuffer:in offset:0 atIndex:2];[e setBuffer:pos offset:0 atIndex:3];[e setBuffer:out offset:0 atIndex:4];
        [e setBuffer:records offset:0 atIndex:5];[e setBuffer:status offset:0 atIndex:6];});
    Capture c;c.fault=*static_cast<uint32_t*>(status.contents);c.output.resize(n);c.positions.resize(n);c.records.resize(n);
    std::memcpy(c.output.data(),out.contents,n*sizeof(ExactSurface));std::memcpy(c.positions.data(),pos.contents,n*sizeof(Coordinate));
    std::memcpy(c.records.data(),records.contents,n*sizeof(Record));
    check(!std::memcmp(in.contents,padded.data(),padded.size()*sizeof(ExactSurface)),"Input/canary mutation");
    ExactSurface outputCanary;Coordinate positionCanary;Record recordCanary;
    std::memset(&outputCanary,0xa5,sizeof(outputCanary));std::memset(&positionCanary,0x5a,sizeof(positionCanary));std::memset(&recordCanary,0x5a,sizeof(recordCanary));
    check(!std::memcmp(static_cast<ExactSurface*>(out.contents)+n,&outputCanary,sizeof(outputCanary)),"Output overdispatch");
    check(!std::memcmp(static_cast<Coordinate*>(pos.contents)+n,&positionCanary,sizeof(positionCanary)),"Coordinate overdispatch");
    check(!std::memcmp(static_cast<Record*>(records.contents)+n,&recordCanary,sizeof(recordCanary)),"Record overdispatch");
    if(!c.fault)for(unsigned i=0;i<n;++i){
        const auto& r=c.records[i];check(r.version==Version&&r.index==i&&!r.fault&&!r.reserved[0]&&!r.reserved[1],"Invalid normal record");
        auto before=source[i],after=c.output[i];std::memset(before.normal,0,sizeof(before.normal));std::memset(after.normal,0,sizeof(after.normal));
        check(!std::memcmp(&before,&after,sizeof(before)),"Stage changed non-normal surface fields");
        check(r.foreground<=1&&r.changed<=1&&r.neighbors<=4,"Invalid normal activity");
    }
    return c;
}
void same(const Capture& a,const Capture& b){
    check(a.fault==b.fault&&a.output.size()==b.output.size(),"Repeat shape/fault");
    check(!std::memcmp(a.output.data(),b.output.data(),a.output.size()*sizeof(ExactSurface)),"Repeat surface differs");
    check(!std::memcmp(a.positions.data(),b.positions.data(),a.positions.size()*sizeof(Coordinate)),"Repeat positions differ");
    check(!std::memcmp(a.records.data(),b.records.data(),a.records.size()*sizeof(Record)),"Repeat work differs");
}
TMandHeader10 header(unsigned w,unsigned h,unsigned mode){
    TMandHeader10 h0{};h0.Width=w;h0.Height=h;h0.bCalc3D=1;h0.byCalcNsOnZBufAuto=1;
    h0.bPlanarOptic=mode;h0.dZoom=1;h0.dFOVy=45;h0.dZstart=-2;h0.dZend=2;h0.dStepWidth=.125;return h0;
}
int main(int argc,char** argv){@autoreleasepool{try{
    if(argc!=2)throw std::runtime_error("usage: zbuffer_gpu_test NATIVE_LIBRARY_WITH_ZBUFFER");
    const ScopedNumericPrecision precision(PrecisionMode::Native);gpu::Metal m(argv[1],3,8,0,false,true);
    auto marker=m.buffer(5*sizeof(uint32_t));std::memset(marker.contents,0,marker.length);
    gpu::dispatchStage(m,m.pipeline(@"native_zn_abi"),1,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:marker offset:0 atIndex:0];});
    const uint32_t expected[]={Version,sizeof(Params),sizeof(Coordinate),sizeof(Record),sizeof(Launch)};
    check(!std::memcmp(marker.contents,expected,sizeof(expected)),"ABI marker mismatch");
    // Orthogonal literal plane: ty=-Y,tx=-X for ordinary neighborhoods.
    // A one-pixel dimension uses the positive fallback tangent instead.
    for(auto extent:std::vector<std::pair<unsigned,unsigned>>{{1,1},{1,4},{4,1},{4,3}}){
        const auto [w,h]=extent;auto p=descriptor(w,h);std::vector<ExactSurface> s(w*h);for(unsigned i=0;i<s.size();++i)s[i]=pixel(i);
        auto a=execute(m,p,s);check(!a.fault,"Literal plane fault");same(a,execute(m,p,s));same(a,execute(m,p,s));
        const int nz=((w==1) != (h==1))?32767:-32767;
        for(unsigned i=0;i<s.size();++i){check(a.output[i].normal[0]==0&&a.output[i].normal[1]==0&&a.output[i].normal[2]==nz,"Literal plane normal");
            check(a.positions[i].x==float(i%w)&&a.positions[i].y==float(i/w+1)&&a.positions[i].z==0&&a.positions[i].depth==0,"Literal plane position");}
    }
    // The first two reconstructed rows omit tile top in their geometric origin.
    // Pixel depth is exactly3 from the independently chosen packed key.
    {auto p=descriptor(2,3);p.left=2;p.top=7;std::vector<ExactSurface> s(6,pixel());for(auto& v:s)v.depth_packed=(8388352u-256)<<8;
        auto a=execute(m,p,s);check(!a.fault,"Tiled literal fault");
        for(unsigned i=0;i<6;++i)check(a.positions[i].x==float(i%2+2)&&a.positions[i].y==float(i/2+1+(i/2<2?0:7))&&a.positions[i].z==3&&a.positions[i].depth==3,"Tile origin/depth literal");}
    // An isolated foreground has two fallback tangents; background and interior
    // preserve all fields, even when they have neighboring foreground normals.
    {auto p=descriptor(3,3);std::vector<ExactSurface> s(9,pixel());for(auto& v:s)v.depth_packed=0x80000000;
        s[4]=pixel();s[1].si_gradient=32768;s[1].depth_packed=0;
        auto a=execute(m,p,s);check(!a.fault&&a.output[4].normal[2]==-32767,"Isolated foreground normal");
        for(unsigned i=0;i<9;++i)if(i!=4)check(!std::memcmp(&s[i],&a.output[i],sizeof(ExactSurface))&&!a.records[i].foreground&&!a.records[i].changed,"Background/interior mutated");}
    // Existing CPU implementation is used without changing its arithmetic or
    // imposing a compatibility tolerance on this Native GPU stage.
    uint64_t cpuDifferences=0,cpuCompared=0;int maximumPackedDifference=0;
    for(unsigned mode=0;mode<4;++mode)for(auto extent:std::vector<std::pair<unsigned,unsigned>>{{1,1},{1,4},{4,1},{4,3}}){
        auto [w,h]=extent;auto h0=header(w,h,mode),before=h0;auto p=PrepareNativeZNormals(h0);
        check(!std::memcmp(&before,&h0,sizeof(h0)),"Host mutated header");check(p.version==Version&&p.mode==mode&&p.count==w*h,"Host descriptor mode/extent");
        std::vector<ExactSurface> s(w*h);std::vector<TsiLight5> cpu(w*h);
        for(unsigned i=0;i<s.size();++i){s[i]=pixel(i);s[i].depth_packed=(8388352u-256u*(i+1))<<8;
            auto& v=cpu[i];v.NormalX=s[i].normal[0];v.NormalY=s[i].normal[1];v.NormalZ=s[i].normal[2];
            std::memcpy(&v.RoughZposFine,&s[i].depth_packed,4);v.SIgradient=s[i].si_gradient;v.OTrap=s[i].o_trap;v.Shadow=s[i].shadow;v.AmbShadow=s[i].amb_shadow;}
        RecalculateNormalsFromZBuffer(h0,cpu);auto a=execute(m,p,s);check(!a.fault,"CPU comparison GPU fault");same(a,execute(m,p,s));same(a,execute(m,p,s));
        for(unsigned i=0;i<s.size();++i){int values[]={cpu[i].NormalX,cpu[i].NormalY,cpu[i].NormalZ};for(unsigned k=0;k<3;++k){++cpuCompared;const int delta=std::abs(values[k]-a.output[i].normal[k]);cpuDifferences+=delta!=0;maximumPackedDifference=std::max(maximumPackedDifference,delta);}}
    }
    // Custom coordinates distinguish a neighbor exactly at the sentinel
    // threshold from one below it: CPU's strict < and > are asymmetric.
    {auto p=descriptor(3,1);std::vector<ExactSurface> s(3,pixel());
        std::vector<Coordinate> v(3);v[0].x=-1e19f;v[1].x=0;v[2].x=-1e20f;
        for(unsigned i=0;i<3;++i){v[i].index=i;v[i].foreground=v[i].x> -1e19f;}
        auto a=execute(m,p,s,&v);check(!a.fault&&a.output[1].normal[2]==32767,"Strict sentinel equality changed tangent");
        v[0].x=-1e20f;a=execute(m,p,s,&v);check(!a.fault&&a.output[1].normal[2]==-32767,"Missing-neighbor fallback control");
        v[0].x=v[1].x=v[2].x=0;for(auto& c:v)c.foreground=1;
        a=execute(m,p,s,&v);check(!a.fault&&a.output[1].normal[0]==0&&a.output[1].normal[1]==0&&a.output[1].normal[2]==0,"Collapsed zero normal");}
    auto p=descriptor(2,2);std::vector<ExactSurface> s(4,pixel());
    {
        const ZNormalsExecutionPolicy execution(p);
        auto input=m.buffer(s.size()*sizeof(ExactSurface),s.data());auto actual=RunNativeZNormals(m,p,input,execution);auto reference=execute(m,p,s);
        check(actual.activity.records==4&&actual.activity.foreground==4&&actual.activity.changed==4&&actual.activity.neighbors==8&&actual.activity.dispatches==3&&!actual.activity.faults,"Wrapped activity");
        check(!std::memcmp(actual.surface.contents,reference.output.data(),reference.output.size()*sizeof(ExactSurface)),"Wrapped/raw results differ");
        check(!std::memcmp(input.contents,s.data(),s.size()*sizeof(ExactSurface)),"Wrapped source mutated");
        check(NativeZNormalsAllocationBytes(p)==4*(32+48+32)+24,"Workspace byte accounting");
        Params disabled{};auto skipped=RunNativeZNormals(m,disabled,input,ZNormalsExecutionPolicy(disabled));check(skipped.surface==input&&!skipped.activity.dispatches&&!NativeZNormalsAllocationBytes(disabled),"Disabled wrapper mutated output");
        disabled.count=4;rejects([&]{RunNativeZNormals(m,disabled,input,ZNormalsExecutionPolicy(disabled));},"Malformed disabled descriptor accepted");
        auto shortInput=m.buffer(sizeof(ExactSurface));rejects([&]{RunNativeZNormals(m,p,shortInput,execution);},"Short source allocation accepted");
        for(unsigned stop:{1u,5u,7u}){unsigned polls=0;bool returned=false;
            rejects([&]{auto canceled=RunNativeZNormals(m,p,input,execution,[&]{if(++polls==stop)throw std::runtime_error("control cancellation");});returned=true;},"Cancellation swallowed");
            check(!returned&&polls==stop,"Canceled stage published output/continued");}
        auto bad=s;bad[1].fault=1;auto faulty=m.buffer(bad.size()*sizeof(ExactSurface),bad.data());
        rejects([&]{RunNativeZNormals(m,p,faulty,execution);},"Wrapper published GPU-faulted surface");
        auto altered=p;altered.mode=p.mode?0:1;unsigned polls=0;bool mismatch=false;
        try{RunNativeZNormals(m,altered,input,execution,[&]{++polls;});}
        catch(const std::invalid_argument& e){mismatch=std::string(e.what())=="Native Z normals execution policy input mismatch";}
        check(mismatch&&!polls&&!std::memcmp(input.contents,s.data(),s.size()*sizeof(ExactSurface)),"Stale Z policy reached device/mutated input");
    }
    for(unsigned bad=0;bad<19;++bad){auto malformed=p;Launch l{};l.count=l.source_count=l.coordinate_count=l.output_count=l.record_count=4;
        switch(bad){case 0:malformed.version=2;break;case 1:malformed.width=4097;break;case 2:malformed.count=3;break;
        case 3:malformed.mode=4;break;case 4:malformed.step=NAN;break;case 5:malformed.fov=INFINITY;break;
        case 6:malformed.aspect=0;break;case 7:malformed.optic_z=0;break;case 8:malformed.inverse_z_mul=0;break;
        case 9:malformed.depth_correction=0;break;case 10:malformed.stop_factor=NAN;break;case 11:malformed.reserved_float=1;break;
        case 12:malformed.reserved[1]=1;break;case 13:l.source_count=3;break;case 14:l.coordinate_count=3;break;
        case 15:l.output_count=3;break;case 16:l.record_count=3;break;case 17:l.reserved[0]=1;break;case 18:malformed.left=4097;break;}
        auto a=execute(m,malformed,s,nullptr,&l);check(a.fault==1,"Malformed descriptor/capacity accepted");
        std::vector<unsigned char> untouched(a.output.size()*sizeof(ExactSurface),0xa5);check(!std::memcmp(a.output.data(),untouched.data(),untouched.size()),"Malformed descriptor wrote output");}
    for(unsigned bad=0;bad<5;++bad){auto corrupted=s;
        if(bad==0)corrupted[0].fault=1;if(bad==1)corrupted[0].reserved[2]=1;if(bad==2)corrupted[0].normal[1]=32768;
        if(bad==3)corrupted[0].si_gradient=65536;if(bad==4)corrupted[0].shadow=65536;
        check(execute(m,p,corrupted).fault==2,"Invalid surface accepted");}
    auto base=execute(m,p,s);
    for(unsigned bad=0;bad<7;++bad){auto v=base.positions;
        if(bad==0)v[1].index=9;if(bad==1)v[1].reserved=1;if(bad==2)v[1].fault=1;if(bad==3)v[1].x=NAN;
        if(bad==4)v[1].depth=INFINITY;if(bad==5)v[1].foreground=2;if(bad==6)v[1].foreground=0;
        check(execute(m,p,s,&v).fault==8,"Invalid neighbor coordinate accepted");}
    {auto overflow=p;overflow.inverse_z_mul=std::numeric_limits<float>::max();auto source=s;source[0].depth_packed=0;
        check(execute(m,overflow,source).fault==4,"Nonfinite reconstruction intermediate accepted");}
    {auto overflow=p;overflow.mode=1;overflow.fov=1e30f;
        check(execute(m,overflow,s).fault==4,"Infinite view norm masked as finite zero vector");}
    {auto v=base.positions;v[1].y=std::numeric_limits<float>::max();v[3].y=-std::numeric_limits<float>::max();
        check(execute(m,p,s,&v).fault!=0,"Nonfinite tangent intermediate accepted");}
    {auto projection=descriptor(3,1);std::vector<ExactSurface> source(3,pixel());std::vector<Coordinate> v(3);
        for(unsigned i=0;i<3;++i){v[i].index=i;v[i].foreground=1;}v[0].x=1e30f;
        check(execute(m,projection,source,&v).fault==16,"Infinite squared tangent norm masked as zero weight");}
    auto h0=header(2,2,0);h0.byCalcNsOnZBufAuto=0;check(PrepareNativeZNormals(h0).version==0,"Disabled host stage admitted");
    h0=header(2,2,0);h0.dStepWidth=0;rejects([&]{PrepareNativeZNormals(h0);},"Zero host step accepted");
    h0=header(2,2,0);h0.dStepWidth=std::numeric_limits<double>::infinity();rejects([&]{PrepareNativeZNormals(h0);},"Nonfinite host step accepted");
    std::printf("{\"status\":\"PASS\",\"checks\":%u,\"cpu_normal_components\":%llu,\"cpu_gpu_different_components\":%llu,\"maximum_packed_delta\":%d,\"repeat_count\":3}\n",checks,(unsigned long long)cpuCompared,(unsigned long long)cpuDifferences,maximumPackedDifference);return 0;
}catch(const std::exception& e){std::fprintf(stderr,"FAIL after %u checks: %s\n",checks,e.what());return 1;}}}
