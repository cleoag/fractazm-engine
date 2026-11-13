#include "scene_host.h"
#include "core/math3d.h"
#include "core/renderer.h"
#include "io/m3p_parser.h"
#include <bit>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <unistd.h>

int main(int argc, char** argv) {
    const mb3d::ScopedNumericPrecision precision(mb3d::PrecisionMode::Compatible);
    if (argc != 3) { std::fprintf(stderr, "usage: test scene.m3p unsupported.m3p\n"); return 2; }
    unsigned checks = 0, failures = 0, hits = 0;
    auto check = [&](bool success, const char* what, unsigned pixel = 0) {
        ++checks;
        if (!success && ++failures <= 20) std::fprintf(stderr, "pixel %u: %s\n", pixel, what);
    };
    try {
        for (const unsigned threads : {1u, 4u}) {
            auto scene = mb3d::metal_exact::PrepareExactScene(argv[1], 64, 48);
            if(threads==1) {
                const mb3d::ScopedNumericPrecision caller(mb3d::PrecisionMode::Native);
                const auto fromNative=mb3d::metal_exact::PrepareExactScene(argv[1],64,48);
                check(std::memcmp(&scene.device,&fromNative.device,sizeof(scene.device))==0 &&
                      scene.columns.size()==fromNative.columns.size() &&
                      scene.rows.size()==fromNative.rows.size() &&
                      std::memcmp(scene.columns.data(),fromNative.columns.data(),scene.columns.size()*sizeof(ExactTrig))==0 &&
                      std::memcmp(scene.rows.data(),fromNative.rows.data(),scene.rows.size()*sizeof(ExactTrig))==0,
                      "exact scene preparation pins Compatible under Native caller");
                check(mb3d::NativePrecisionEnabled(),"exact preparation restores Native caller");
                const auto native=mb3d::metal_exact::PrepareExactScene(argv[1],64,48,65536,false,
                    mb3d::PrecisionMode::Native);
                check(native.precision==mb3d::PrecisionMode::Native,"explicit native preparation selected");
                bool nativeTrig=true;
                for(const auto* table:{&native.columns,&native.rows})for(const auto& t:*table)
                    nativeTrig &= t.sin_significand==0 && t.cos_significand==0 &&
                        t.sin_sign_exponent==0 && t.cos_sign_exponent==0 &&
                        t.reserved[0]==0 && t.reserved[1]==0 &&
                        std::isfinite(std::bit_cast<double>(t.sin_value)) &&
                        std::isfinite(std::bit_cast<double>(t.cos_value));
                check(nativeTrig,"native camera uses finite scalar tables without extended words");
                bool rejected=false;
                try { (void)mb3d::metal_exact::ReferencePrimary(native,1); }
                catch(const std::invalid_argument&) { rejected=true; }
                check(rejected,"native cannot enter exact CPU primary oracle");
                check(mb3d::NativePrecisionEnabled(),"native preparation restores caller");
            }
            const auto* capability = mb3d::metal_exact::FindExactSceneCapability(scene.source_sha256);
            check(scene.device.version == 4, "prepared scene ABI version");
            if (capability) {
                check(scene.scene_id == capability->scene_id && scene.formula_name == capability->formula_name,
                      "prepared scene/formula identity");
                check(scene.device.capability_id == capability->capability_id &&
                      scene.device.formula_kind == capability->formula_kind &&
                      scene.device.max_iterations == capability->max_iterations &&
                      scene.device.slot_count == capability->slot_count,
                      "prepared device capability identity");
                check(mb3d::metal_exact::FindExactSceneCapabilityById(scene.device.capability_id) == capability,
                      "catalog hash/id resolve the same capability");
            } else {
                check((scene.device.capability_id == mb3d_metal_exact::kRuntimeSphereCapability &&
                       scene.device.formula_kind == 1 && scene.scene_id == "sphereifs_runtime" &&
                       scene.formula_name == "SphereIFS") ||
                      (scene.device.capability_id == mb3d_metal_exact::kRuntimeBoxCapability &&
                       scene.device.formula_kind == 2 && scene.scene_id == "boxifs_runtime" &&
                       scene.formula_name == "boxIFS") ||
                      (scene.device.capability_id == mb3d_metal_exact::kRuntimeBulbCapability &&
                       scene.device.formula_kind == 3 && scene.scene_id == "integer_power_runtime" &&
                       scene.formula_name == "IntegerPower"), "prepared runtime formula identity");
            }
            check(mb3d_metal_exact::valid_formula_scene(scene.device), "prepared shared formula contract");
            for (unsigned i = 0; i < 13; ++i) {
                if (capability)
                    check(scene.device.params[i] == capability->params[i], "prepared coefficient bits", i);
                auto invalid = scene.device;
                if (capability) invalid.params[i] ^= 1;
                else invalid.params[i] = 0x7ff8000000000000ULL;
                check(!mb3d_metal_exact::valid_formula_scene(invalid), "unsupported coefficient rejected", i);
            }
            if (capability) for (const auto& other : mb3d::metal_exact::kExactSceneCapabilities) {
                if (other.capability_id == capability->capability_id) continue;
                auto invalid = scene.device;
                invalid.capability_id = other.capability_id;
                check(!mb3d_metal_exact::valid_formula_scene(invalid), "cross-capability record rejected");
            }
            for (unsigned control = 0; control < 8; ++control) {
                auto invalid = scene.device;
                if (control == 0) invalid.capability_id = 0;
                if (control == 1) invalid.formula_kind = 0;
                if (control == 2) invalid.max_iterations = 0;
                if (control == 3) invalid.slot_count = 0;
                if (control == 4) ++invalid.min_iterations;
                if (control == 5) --invalid.refinement_steps;
                if (control == 6) invalid.color_on_it = 1;
                if (control == 7) invalid.color_radius_stop = 1;
                check(!mb3d_metal_exact::valid_formula_scene(invalid), "malformed formula profile rejected", control);
            }
            if (scene.device.formula_kind == 3) {
                for (unsigned control = 0; control < 9; ++control) {
                    auto invalid = scene.device;
                    if (control == 0) invalid.bulb.power = 9;
                    if (control == 1) invalid.bulb.bailout = 0;
                    if (control == 2) invalid.bulb.gradient_bailout = 0;
                    if (control == 3) invalid.bulb.de_offset = 0;
                    if (control == 4) invalid.bulb.de_offset006 = 0;
                    if (control == 5) invalid.bulb.smooth_hln = 0;
                    if (control == 6) invalid.bulb.smooth_lnrstop = 0x7fc00000u;
                    if (control == 7) invalid.bulb.reserved = 1;
                    if (control == 8) invalid.params[1] ^= 1;
                    check(!mb3d_metal_exact::valid_formula_scene(invalid),
                          "malformed internal formula profile rejected", control);
                }
            } else {
                auto invalid = scene.device;
                invalid.bulb.power = 8;
                check(!mb3d_metal_exact::valid_formula_scene(invalid),
                      "unused internal formula metadata rejected");
            }
            auto reference = mb3d::metal_exact::ReferencePrimary(scene, threads);
            auto loaded = mb3d::LoadM3PFile(argv[1]);
            mb3d::RenderOptions options;options.precision=mb3d::PrecisionMode::Compatible;
            options.width_override = 64; options.height_override = 48;
            options.thread_count = threads;
            for (int y = 0; y < 48; ++y)
                for (int x = 0; x < 64; ++x) options.diag_points.push_back({x,y});
            auto production = mb3d::Render(loaded.header, loaded.addon, options);
            if (!production.success) throw std::runtime_error(production.error);
            check(reference.size() == production.diagnostics.size(), "record count");
            for (unsigned i = 0; i < reference.size(); ++i) {
                const auto& a = reference[i]; const auto& b = production.diagnostics[i];
                check(a.rm_result == unsigned(b.rm_result), "hit classification", i);
                check(a.start_inside == unsigned(b.start_inside), "inside classification", i);
                check(a.depth == std::bit_cast<uint64_t>(b.z_stepped), "depth bits", i);
                check(a.axial_depth == std::bit_cast<uint64_t>(b.act_zpos), "axial depth bits", i);
                // Render diagnostics.ms_destop is captured at ray entry, not
                // after marching. Final-stop exactness is independently checked
                // by GPU/CPU primary records and full-Render orbit-color packing.
                const float final_stop = std::bit_cast<float>(a.final_stop);
                check(std::isfinite(final_stop) && final_stop > 0, "finite positive final stop", i);
                check(a.step_count == std::bit_cast<uint32_t>(b.step_count), "step count bits", i);
                for (unsigned axis = 0; axis < 3; ++axis) {
                    check(a.ray[axis] == std::bit_cast<uint64_t>(b.view_vec[axis]), "ray bits", i);
                    check(a.start[axis] == std::bit_cast<uint64_t>(b.start_pos[axis]), "start bits", i);
                    check(a.position[axis] == std::bit_cast<uint64_t>(b.hit_pos[axis]), "position bits", i);
                }
                if (a.rm_result > 0 && !a.start_inside) {
                    ++hits;
                    check(a.final_de == std::bit_cast<uint64_t>(b.final_de), "pre-refinement DE bits", i);
                    check(a.normal[0] == b.normal_x, "packed normal x", i);
                    check(a.normal[1] == b.normal_y, "packed normal y", i);
                    check(a.normal[2] == b.normal_z, "packed normal z", i);
                    TVec3D normal{};
                    for (unsigned axis = 0; axis < 3; ++axis)
                        normal[axis] = std::bit_cast<double>(a.raw_normal[axis]);
                    int16_t packed[3]; mb3d::MakeWNormalsFromDVec(packed, normal);
                    if (packed[0] == 0 && packed[1] == 0) packed[2] = packed[2] > 0 ? 32767 : -32767;
                    for (unsigned axis = 0; axis < 3; ++axis)
                        check(packed[axis] == a.normal[axis], "raw normal matches production packed normal", i);
                } else {
                    check(a.final_de == 0, "undefined final DE canonical zero", i);
                }
            }
        }
        bool rejected = false;
        try { (void)mb3d::metal_exact::PrepareExactScene(argv[2], 64, 48); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "unsupported source rejected");
        rejected = false;
        try { (void)mb3d::metal_exact::PrepareExactScene(argv[1], 0, 48); }
        catch (const std::invalid_argument&) { rejected = true; }
        check(rejected, "zero dimensions rejected");
        for (const auto dimensions : {std::pair{64u, 0u}, std::pair{4097u, 48u}, std::pair{64u, 4097u}}) {
            rejected = false;
            try { (void)mb3d::metal_exact::PrepareExactScene(argv[1], dimensions.first, dimensions.second); }
            catch (const std::invalid_argument&) { rejected = true; }
            check(rejected, "unsupported dimensions rejected");
        }
        for (const unsigned limit : {0u, 65537u}) {
            rejected = false;
            try { (void)mb3d::metal_exact::PrepareExactScene(argv[1], 64, 48, limit); }
            catch (const std::invalid_argument&) { rejected = true; }
            check(rejected, "unsupported primary work limit rejected");
        }
        auto enabled = mb3d::metal_exact::PrepareExactScene(argv[1], 16, 12, 1, true);
        check(enabled.device.max_steps == 1 && mb3d_metal_exact::valid_formula_scene(enabled.device),
              "explicit hard-shadow and bounded-work overrides retain the formula profile");

        // Runtime admission follows prepared capabilities rather than source
        // hashes. A camera edit is supported; an unimplemented scene pass is
        // rejected even when the rest of the input came from a known fixture.
        std::ifstream input(argv[1], std::ios::binary);
        std::vector<char> data((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        if (!input || data.size() < sizeof(TMandHeader10))
            throw std::runtime_error("Cannot read source for runtime admission controls");
        std::string pattern = (std::filesystem::temp_directory_path() / "mb3d-exact-scene-XXXXXX").string();
        std::vector<char> name(pattern.begin(), pattern.end()); name.push_back('\0');
        const int fd = mkstemp(name.data());
        if (fd < 0) throw std::runtime_error("Cannot create temporary scene identity witness");
        struct TemporaryScene {
            std::filesystem::path path;
            ~TemporaryScene() { std::error_code ignored; std::filesystem::remove(path, ignored); }
        } temporary{std::filesystem::path(name.data())};
        close(fd);
        auto write = [&](const std::vector<char>& bytes) {
            std::ofstream output(temporary.path, std::ios::binary | std::ios::trunc);
            output.write(bytes.data(), bytes.size());
            if (!output) throw std::runtime_error("Cannot write temporary scene admission witness");
        };
        auto changed = data;
        double midpoint;
        std::memcpy(&midpoint, changed.data() + offsetof(TMandHeader10, dXmid), sizeof(midpoint));
        midpoint += 0.125;
        std::memcpy(changed.data() + offsetof(TMandHeader10, dXmid), &midpoint, sizeof(midpoint));
        write(changed);
        const auto original = mb3d::metal_exact::PrepareExactScene(argv[1], 64, 48);
        const auto moved = mb3d::metal_exact::PrepareExactScene(temporary.path.string(), 64, 48);
        check(moved.source_sha256 != original.source_sha256, "camera input has independent source identity");
        check(moved.device.capability_id == (moved.device.formula_kind == 1 ?
              mb3d_metal_exact::kRuntimeSphereCapability : (moved.device.formula_kind == 2 ?
              mb3d_metal_exact::kRuntimeBoxCapability : mb3d_metal_exact::kRuntimeBulbCapability)),
              "changed camera admitted by runtime profile");
        check(moved.device.y_start[0] != original.device.y_start[0],
              "changed camera reaches prepared GPU origin");
        const std::pair<size_t, unsigned char> unsupported[] = {
            {offsetof(TMandHeader10, bCalc3D), 0},
            {offsetof(TMandHeader10, bStereoMode), 1},
            {offsetof(TMandHeader10, byCalcNsOnZBufAuto), 1},
            {offsetof(TMandHeader10, bCalcSRautomatic), 1},
            {offsetof(TMandHeader10, bCalcDOFtype), 1},
            {offsetof(TMandHeader10, bVolLightNr), 1},
            {offsetof(TMandHeader10, TilingOptions), 1},
        };
        for (const auto& [offset, value] : unsupported) {
            changed = data; changed[offset] = char(value); write(changed);
            rejected = false;
            try { (void)mb3d::metal_exact::PrepareExactScene(temporary.path.string(), 64, 48); }
            catch (const std::invalid_argument&) { rejected = true; }
            check(rejected, "unsupported runtime scene feature rejected", unsigned(offset));
        }
        if (original.device.formula_kind == 3) {
            constexpr size_t power_offset = sizeof(TMandHeader10) +
                offsetof(THeaderCustomAddon, Formulas) + offsetof(THAformula, dOptionValue);
            for (const double power : {1.0, 9.0, 2.5,
                    std::numeric_limits<double>::quiet_NaN(),
                    std::numeric_limits<double>::infinity()}) {
                changed = data;
                std::memcpy(changed.data() + power_offset, &power, sizeof(power));
                write(changed);
                rejected = false;
                try { (void)mb3d::metal_exact::PrepareExactScene(temporary.path.string(), 64, 48); }
                catch (const std::invalid_argument&) { rejected = true; }
                check(rejected, "unsupported raw internal power rejected before preparation");
            }
        }
        check(mb3d::metal_exact::FindExactSceneCapability(std::string(64, '0')) == nullptr,
              "unknown source hash is not relabelled as a pinned fixture");
        check(mb3d::metal_exact::FindExactSceneCapabilityById(0) == nullptr,
              "unknown capability id rejected");
        check(hits > 0, "discriminating surface pixels");
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what()); return 2;
    }
    std::printf("exact scene CPU oracle: %u checks, %u failures, %u hit observations\n", checks, failures, hits);
    return failures ? 1 : 0;
}
