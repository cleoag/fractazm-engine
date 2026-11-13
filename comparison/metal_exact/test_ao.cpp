#include "ao_host.h"
#include "ao_math.h"
#include "shadow_host.h"
#include "scene_host_internal.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>

int main(int argc,char** argv) {
    if (argc != 2) { std::fprintf(stderr,"usage: test_ao pinned.m3p\n"); return 2; }
    unsigned checks = 0, failures = 0, changed = 0;
    auto check = [&](bool ok,const char* what,unsigned pixel = 0) {
        ++checks;
        if (!ok && ++failures <= 20) std::fprintf(stderr,"AO pixel %u: %s\n",pixel,what);
    };
    try {
        // Synthetic packed Z supplies discontinuities, borders, background and
        // every interior quartet lane to the independent production AO oracle.
        for (const auto size : {std::pair{17u,13u},std::pair{64u,48u},std::pair{199u,149u}}) {
            auto scene = mb3d::metal_exact::PrepareExactScene(argv[1],size.first,size.second);
            auto ao = mb3d::metal_exact::PrepareExactAO(scene);
            std::vector<ExactSurface> input(size_t(size.first)*size.second);
            uint32_t state = 0x65347a13;
            for (unsigned pixel = 0; pixel < input.size(); ++pixel) {
                state = state*1664525u+1013904223u;
                auto& p = input[pixel];
                p.normal[0] = int(pixel%65536)-32768; p.normal[1] = -713; p.normal[2] = 29009;
                p.depth_packed = (state&0x7fffffffU) | ((pixel%7 == 0 || size.first == 17) ? 0x80000000U : 0u);
                p.si_gradient = pixel&65535; p.o_trap = (pixel*31)&65535;
                p.shadow = (pixel*37)&65535; p.amb_shadow = 5000;
            }
            auto expected = mb3d::metal_exact::ReferenceAO(scene,input,4);
            auto actual = mb3d::metal_exact::SoftwareAO(ao,input);
            check(actual.size()==expected.size(),"pixel count");
            for (unsigned pixel = 0; pixel < actual.size(); ++pixel) {
                check(actual[pixel].amb_shadow==expected[pixel].amb_shadow,"production AO integer",pixel);
                if (actual[pixel].amb_shadow != input[pixel].amb_shadow) ++changed;
                expected[pixel].amb_shadow = input[pixel].amb_shadow;
                actual[pixel].amb_shadow = input[pixel].amb_shadow;
                check(std::memcmp(&actual[pixel],&input[pixel],sizeof(ExactSurface))==0,"other device surface fields preserved",pixel);
                check(std::memcmp(&expected[pixel],&input[pixel],sizeof(ExactSurface))==0,"other reference surface fields preserved",pixel);
            }
            auto invalid = ao;
            invalid.device.version = 2;
            bool rejected = false;
            try { (void)mb3d::metal_exact::SoftwareAO(invalid,input); }
            catch (const std::invalid_argument&) { rejected = true; }
            check(rejected,"unknown AO ABI rejected");
            invalid = ao; invalid.levels[0].index = 0; rejected = false;
            try { (void)mb3d::metal_exact::SoftwareAO(invalid,input); }
            catch (const std::invalid_argument&) { rejected = true; }
            check(rejected,"invalid wavelet level rejected");
            invalid = ao; invalid.device.directions[7] = 0x7fc00001u; rejected = false;
            try { (void)mb3d::metal_exact::SoftwareAO(invalid,input); }
            catch (const std::invalid_argument&) { rejected = true; }
            check(rejected,"NaN direction rejected");
            invalid = ao; invalid.device.directions[8] = 0xff800000u; rejected = false;
            try { (void)mb3d::metal_exact::SoftwareAO(invalid,input); }
            catch (const std::invalid_argument&) { rejected = true; }
            check(rejected,"infinite direction rejected");
            for (const uint32_t bad : {0u,0x80000000u,0xbf800000u,0x7fc00001u,0x7f800000u}) {
                invalid = ao; invalid.levels[0].z_threshold = bad; rejected = false;
                try { (void)mb3d::metal_exact::SoftwareAO(invalid,input); }
                catch (const std::invalid_argument&) { rejected = true; }
                check(rejected,"invalid denominator threshold rejected");
                invalid = ao; invalid.levels[0].reciprocal_radius[0] = bad; rejected = false;
                try { (void)mb3d::metal_exact::SoftwareAO(invalid,input); }
                catch (const std::invalid_argument&) { rejected = true; }
                check(rejected,"invalid reciprocal sample radius rejected");
            }
            invalid = ao; invalid.levels[0].sit = 0; rejected = false;
            try { (void)mb3d::metal_exact::SoftwareAO(invalid,input); }
            catch (const std::invalid_argument&) { rejected = true; }
            check(rejected,"zero angular scale rejected");
            invalid = ao; invalid.levels[0].sum_multiplier = 0x7fc00000u; rejected = false;
            try { (void)mb3d::metal_exact::SoftwareAO(invalid,input); }
            catch (const std::invalid_argument&) { rejected = true; }
            check(rejected,"NaN sum multiplier rejected");
            invalid = ao; invalid.levels[0].min_radius = 0; rejected = false;
            try { (void)mb3d::metal_exact::SoftwareAO(invalid,input); }
            catch (const std::invalid_argument&) { rejected = true; }
            check(rejected,"zero minimum sample radius rejected");
            input[0].fault = 1; rejected = false;
            try { (void)mb3d::metal_exact::ReferenceAO(scene,input); }
            catch (const std::invalid_argument&) { rejected = true; }
            check(rejected,"faulted geometry rejected");
        }
        // Real production SphereIFS surface is a separate observable input.
        auto scene = mb3d::metal_exact::PrepareExactScene(argv[1],64,48);
        auto input = mb3d::metal_exact::ReferenceSurface(scene,4);
        auto expected = mb3d::metal_exact::ReferenceAO(scene,input,4);
        auto actual = mb3d::metal_exact::SoftwareAO(mb3d::metal_exact::PrepareExactAO(scene),input);
        for (unsigned pixel = 0; pixel < actual.size(); ++pixel)
            check(std::memcmp(&actual[pixel],&expected[pixel],sizeof(ExactSurface))==0,"real SphereIFS production AO",pixel);
        scene.host->file.header.bCalcAmbShadowAutomatic = 23;
        bool rejected = false;
        try { (void)mb3d::metal_exact::PrepareExactAO(scene); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected,"unsupported AO branch rejected");
        check(changed>0,"discriminating nonconstant AO witness");
    } catch (const std::exception& error) {
        std::fprintf(stderr,"%s\n",error.what()); return 2;
    }
    std::printf("exact HiQ AO: %u checks, %u failures, %u changed synthetic pixels\n",checks,failures,changed);
    return failures ? 1 : 0;
}
