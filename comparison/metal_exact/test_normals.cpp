#include "scene_host.h"
#include "scene_math.h"
#include <array>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stdexcept>

namespace {
using namespace mb3d_metal_exact;
struct Checks {
    std::size_t count=0,failures=0,records=0,hits=0,misses=0,minus_witnesses=0;
    void operator()(bool pass,const char* message,std::size_t pixel=0) {
        ++count;
        if(!pass&&++failures<=24)
            std::fprintf(stderr,"pixel %zu: %s\n",pixel,message);
    }
};

ExactNormalSamples samples_for(ExactPixel pixel,ExactScene scene,unsigned order) {
    ExactNormalSamples samples;
    std::memset(&samples,0xa5,sizeof(samples));
    // These orders model unrelated GPU tasks completing independently. Every
    // sample word and the trap/reserved words must overwrite stale allocation.
    constexpr unsigned orders[][6]={{0,1,2,3,4,5},{5,3,1,4,2,0},{2,5,0,4,1,3}};
    for(unsigned probe:orders[order%3]) {
        const auto value=normal_probe(pixel,scene,probe);
        samples.distance[probe]=value.distance.bits;
        if(probe==5) {samples.orbit_trap=value.trap.bits;samples.reserved=0;}
    }
    return samples;
}
bool same(const ExactPixel& a,const ExactPixel& b) {
    return std::memcmp(&a,&b,sizeof(a))==0;
}

void test_scene(Checks& check,const std::filesystem::path& path) {
    const auto prepared=mb3d::metal_exact::PrepareExactScene(path.string(),16,12);
    const auto reference=mb3d::metal_exact::ReferencePrimary(prepared,2);
    for(unsigned y=0;y<prepared.device.height;++y)
        for(unsigned x=0;x<prepared.device.width;++x) {
            const std::size_t index=y*prepared.device.width+x;
            const auto original=render_primary(prepared.device,prepared.columns[x],prepared.rows[y],x,y);
            const auto geometry=render_primary_geometry(prepared.device,prepared.columns[x],prepared.rows[y],x,y);
            const auto samples=samples_for(geometry,prepared.device,unsigned(index));
            const auto result=finish_normal(geometry,samples);
            check(same(original,result),"staged primary differs from complete inline record",index);
            check(same(reference[index],result),"staged primary differs from independent production CPU record",index);
            check(result.fault==0,"valid scene produced a fault",index);
            check(samples.reserved==0,"normal sample reserved word was not overwritten",index);
            check(geometry.orbit_trap==0&&geometry.raw_normal[0]==0&&geometry.raw_normal[1]==0&&
                  geometry.raw_normal[2]==0&&geometry.normal[0]==0&&geometry.normal[1]==0&&geometry.normal[2]==0,
                  "geometry pass calculated or leaked normal state",index);
            ++check.records;
            if(geometry.rm_result&&!geometry.start_inside) {
                ++check.hits;
                check(samples.orbit_trap==reference[index].orbit_trap,
                      "Y-minus retained orbit trap differs from production",index);
                const Vec p={D(geometry.position[0]),D(geometry.position[1]),D(geometry.position[2])};
                const F64 off=(S(prepared.device.de_stop)<F64(1)?S(prepared.device.de_stop):F64(1))*
                    (F64(1)+D(geometry.depth)*S(prepared.device.de_stop_factor))*D(0x3fc3333333333333ULL);
                for(unsigned axis=0;axis<3;++axis) {
                    const Vec direction=column(prepared.device,int(axis));
                    const Vec plus=add(p,mul(direction,off));
                    const Vec minus=add(plus,mul(direction,F64(-2)*off));
                    const Vec alternative=add(p,mul(direction,-off));
                    if(minus.x.bits!=alternative.x.bits||minus.y.bits!=alternative.y.bits||
                       minus.z.bits!=alternative.z.bits)++check.minus_witnesses;
                }
                // A geometry fault must not suppress the original normal
                // writes. The final record retains the fault bit as well.
                auto faulted=geometry;faulted.fault|=4;
                auto expected=result;expected.fault|=4;
                check(same(finish_normal(faulted,samples_for(faulted,prepared.device,2)),expected),
                      "existing fault suppressed active normal calculations",index);
            } else {
                ++check.misses;
                const ExactNormalSamples zero{};
                check(std::memcmp(&samples,&zero,sizeof(zero))==0,
                      "inactive probes retained stale data",index);
                ExactNormalSamples poison;
                std::memset(&poison,0xff,sizeof(poison));
                check(same(finish_normal(geometry,poison),geometry),
                      "inactive finish consumed undefined samples",index);
            }
        }
}

void test_controls(Checks& check,const std::filesystem::path& path) {
    const auto prepared=mb3d::metal_exact::PrepareExactScene(path.string(),16,12);
    for(unsigned control=0;control<8;++control) {
        auto scene=prepared.device;
        auto tx=prepared.columns[8],ty=prepared.rows[6];
        unsigned x=8,y=6;
        if(control==0)scene.version=0;
        if(control==1)scene.max_steps=1;
        if(control==2)tx.sin_value=0x7ff8000000000000ULL;
        if(control==3)ty.cos_value=0x7ff0000000000000ULL;
        if(control==4)x=scene.width;
        if(control==5)y=scene.height;
        if(control==6)scene.y_start[0]=0x7fefffffffffffffULL;
        if(control==7)scene.formula_kind=0;
        const auto original=render_primary(scene,tx,ty,x,y);
        const auto geometry=render_primary_geometry(scene,tx,ty,x,y);
        const auto result=finish_normal(geometry,samples_for(geometry,scene,control));
        check(same(result,original),"rejected or bounded primary changed during split",control);
        check(result.fault!=0,"fault control failed to reject",control);
    }
    // Direct normal probes need no trace history: test an inside record and
    // degenerate gradient independently of the primary scene's visible hits.
    ExactPixel inside{};inside.rm_result=1;inside.start_inside=1;inside.fault=4;
    check(same(finish_normal(inside,samples_for(inside,prepared.device,0)),inside),
          "inside hit was changed by normal pass");
    ExactPixel hit{};hit.rm_result=1;hit.final_stop=prepared.device.de_stop;
    hit.position[0]=F64(-3).bits;
    auto scene=prepared.device;
    for(auto& value:scene.vgrads)value=0;
    const auto actual=finish_normal(hit,samples_for(hit,scene,1));
    check(actual.raw_normal[0]==0&&actual.raw_normal[1]==0&&actual.raw_normal[2]==0&&
          actual.normal[0]==0&&actual.normal[1]==0&&actual.normal[2]==-32767,
          "zero gradient lost the original packed-Z convention");
    auto samples=samples_for(hit,prepared.device,1);
    for(unsigned source=0;source<7;++source) {
        auto invalid=samples;
        if(source<6)invalid.distance[source]=0x7ff8000000000000ULL;
        else invalid.orbit_trap=0x7ff0000000000000ULL;
        check((finish_normal(hit,invalid).fault&4)!=0,
              "nonfinite normal/trap did not retain a numerical fault",source);
    }
}
}

int main(int argc,char** argv) {
#if !defined(__APPLE__) || !defined(__aarch64__)
    std::fprintf(stderr,"Exact staged normal contract requires native Apple ARM64.\n");
    return 2;
#else
    if(argc!=2) {std::fprintf(stderr,"usage: test_normals repository-root\n");return 2;}
    Checks check;
    try {
        const auto root=std::filesystem::path(argv[1]);
        const char* scenes[]={
            "comparison/scenes/extall_dIFS_SphereIFS.m3p",
            "comparison/scenes/extall_dIFS_boxIFS.m3p",
            "comparison/scenes/ext_16_boxifs.m3p",
            "comparison/metal_exact/fixtures/box_asymmetric.m3p",
            "comparison/metal_exact/fixtures/box_camera.m3p",
            "comparison/metal_exact/fixtures/sphere_varied.m3p",
            "comparison/metal_exact/fixtures/sphere_apply0_orbit.m3p",
            "comparison/scenes/test_01_intpow2.m3p",
            "comparison/metal_exact/fixtures/integer_power3.m3p",
            "comparison/metal_exact/fixtures/integer_power4.m3p",
            "comparison/scenes/test_02_intpow5.m3p",
            "comparison/metal_exact/fixtures/integer_power6.m3p",
            "comparison/metal_exact/fixtures/integer_power7.m3p",
            "comparison/scenes/test_03_intpow8.m3p"
        };
        for(const auto* scene:scenes)test_scene(check,root/scene);
        test_controls(check,root/scenes[0]);
        check(check.hits>0&&check.misses>0,"scene corpus did not exercise both active and inactive normals");
        check(check.minus_witnesses>0,"scene corpus did not distinguish rounded-plus minus construction");
        std::printf("Exact staged normals: %zu checks, %zu primary records, %zu hits, %zu inactive, "
                    "%zu minus-order witnesses, %zu failures.\n",
                    check.count,check.records,check.hits,check.misses,check.minus_witnesses,check.failures);
        return check.failures?1:0;
    } catch(const std::exception& error) {
        std::fprintf(stderr,"Exact staged normals FAILED: %s\n",error.what());
        return 1;
    }
#endif
}
