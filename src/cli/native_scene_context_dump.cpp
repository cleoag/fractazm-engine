// Host-only inventory of original-size/settings scene preparation. No renderer,
// device creation or shader dispatch is linked/called by this diagnostic.
#include "io/m3p_parser.h"
#include "src/render/native_metal/general_scene_host.h"
#include "src/render/native_metal/general_normals_host.h"
#include "src/render/native_metal/native_inside_host.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {
void bytes(const std::filesystem::path& path, const void* data, std::size_t size) {
    std::ofstream out(path, std::ios::binary);
    if (size) out.write(static_cast<const char*>(data), size);
    if (!out) throw std::runtime_error("Cannot write context artifact");
}
template<class T> void record(const std::filesystem::path& path, const T& value) {
    bytes(path, &value, sizeof(value));
}
template<class T> void array(const std::filesystem::path& path, const std::vector<T>& value) {
    bytes(path, value.data(), value.size()*sizeof(T));
}
void program(const std::filesystem::path& output, const std::string& name,
             const mb3d_native::NativeFormulaProgram& value) {
    record(output/(name+".bin"), value);
    // This parameter payload is the complete slot suffix, including typed raw
    // aliases/negative offsets/constants. It is NOT the point probe input ABI.
    std::ofstream params(output/(name+"-parameters.bin"), std::ios::binary);
    for (unsigned i=0; i<6; ++i) {
        const auto& slot=value.slots[i];
        const auto offset=offsetof(mb3d_native::NativeFormulaSlot, params);
        const auto* data=reinterpret_cast<const char*>(&slot)+offset;
        const auto size=sizeof(slot)-offset;
        params.write(data,size);
        bytes(output/(name+"-slot"+std::to_string(i)+"-parameters.bin"),data,size);
    }
    if (!params) throw std::runtime_error("Cannot write parameter payload");
}
void describe(const mb3d_native::NativeFormulaProgram& p) {
    std::cout << "{\"driver\":" << p.driver << ",\"de_option\":" << p.de_option
        << ",\"max_iterations\":" << p.max_iterations << ",\"formula_type\":" << p.formula_type
        << ",\"start_slot\":" << p.start_slot << ",\"repeat_slot\":" << p.repeat_slot
        << ",\"end_slot\":" << p.end_slot << ",\"calc_smooth\":" << p.calc_smooth << ",\"slots\":[";
    for (unsigned i=0; i<6; ++i) {
        if(i) std::cout << ',';
        const auto& s=p.slots[i];
        std::cout << "{\"slot\":" << i << ",\"enabled\":" << s.enabled
            << ",\"namespace\":" << s.namespace_id << ",\"id\":" << s.id
            << ",\"count\":" << p.hybrid_counts[i] << ",\"de_option\":" << s.de_option
            << ",\"param_count\":" << s.param_count << ",\"traits\":" << s.traits
            << ",\"wrapper_flags\":" << s.wrapper_flags << ",\"difs_style\":" << s.difs_style
            << ",\"internal_power\":" << s.internal_power << '}';
    }
    std::cout << "]}";
}
}

int main(int argc, char** argv) { try {
    if (argc!=4) throw std::runtime_error("scene.m3p approved-resource-root new-output-directory");
    const std::filesystem::path output=argv[3];
    if (std::filesystem::exists(output)) throw std::runtime_error("New output directory required");
    const auto file=mb3d::LoadM3PFile(argv[1]);
    if (!file.success) throw std::runtime_error(file.error);
    const mb3d::ScopedNumericPrecision precision(mb3d::PrecisionMode::Native);
    auto scene=mb3d::native_metal::PrepareGeneralSceneWithMaps(file.header,file.addon,
        file.header.Width,file.header.Height,argv[2],nullptr);
    mb3d::ScopedRenderContext binding(scene.base.host->context);
    const auto normal=mb3d::native_metal::PrepareGeneralNormals(scene.base.host->mct,scene.formulas.device);
    const auto inside=mb3d::native_metal::PrepareNativeInside(scene.base.host->mct);
    std::filesystem::create_directories(output);
    record(output/"loaded-header.bin",file.header); record(output/"loaded-addon.bin",file.addon);
    record(output/"prepared-header.bin",scene.base.host->file.header);
    record(output/"prepared-addon.bin",scene.base.host->file.addon);
    program(output,"primary",scene.formulas.device);
    if (scene.formulas.program_count()==2) program(output,"secondary",scene.formulas.second);
    record(output/"scene.bin",scene.base.device); record(output/"normal.bin",normal);
    record(output/"inside.bin",inside); record(output/"volume.bin",scene.volume);
    record(output/"camera.bin",scene.camera.device);
    array(output/"camera-columns.bin",scene.camera.columns); array(output/"camera-rows.bin",scene.camera.rows);
    array(output/"maps.bin",scene.formulas.maps); array(output/"map-pixels.bin",scene.formulas.map_pixels);
    std::cout << "{\"schema\":\"mb3d.native-scene-context-dump.v1\",\"host_only\":true,"
        "\"settings_overrides\":[],\"precision\":\"native\",\"width\":" << file.header.Width
        << ",\"height\":" << file.header.Height << ",\"original_iterations\":" << file.header.Iterations
        << ",\"original_secondary_iterations\":" << file.header.iMaxItsF2
        << ",\"program_count\":" << scene.formulas.program_count()
        << ",\"persistent_lane\":" << scene.formulas.requires_persistent_lane
        << ",\"normal_smoothing\":" << normal.smoothing << ",\"map_count\":" << scene.formulas.maps.size()
        << ",\"programs\":[";
    describe(scene.formulas.device);
    if(scene.formulas.program_count()==2) { std::cout << ','; describe(scene.formulas.second); }
    std::cout << "]}\n";
    return 0;
} catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 2; } }
