#include "core/renderer.h"
#include "io/m3p_parser.h"
#include "mb3d/mb3d.h"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <iterator>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
namespace fs = std::filesystem;
constexpr int kWidth = 48;
constexpr int kHeight = 36;
int checks = 0;

void check(bool value, const std::string& message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}

struct Scene {
    TMandHeader10 header;
    THeaderCustomAddon addon;
};

Scene load(const fs::path& path) {
    const auto loaded = mb3d::LoadM3PFile(path.string());
    check(loaded.success, "Cannot load tracked scene " + path.string() + ": " + loaded.error);
    Scene scene{loaded.header, loaded.addon};
    // These are bounded concurrency controls, not new Pascal parity images.
    scene.header.Width = kWidth;
    scene.header.Height = kHeight;
    return scene;
}

struct Scratch {
    fs::path path;
    explicit Scratch(const fs::path& parent) {
        fs::create_directories(parent);
        const auto id = std::chrono::steady_clock::now().time_since_epoch().count();
        for (int attempt = 0; attempt != 100; ++attempt) {
            path = fs::absolute(parent) /
                ("render-isolation-" + std::to_string(id) + "-" + std::to_string(attempt));
            if (fs::create_directory(path)) return;
        }
        throw std::runtime_error("Cannot create independent resource roots");
    }
    ~Scratch() {
        std::error_code error;
        fs::remove_all(path, error);
    }
};

void writeMap(const fs::path& fixture, const fs::path& root, bool invert) {
    std::ifstream input(fixture, std::ios::binary);
    check(static_cast<bool>(input), "Cannot read tracked map fixture");
    std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(input)), {});
    check(bytes.size() >= 54 && bytes[0] == 'B' && bytes[1] == 'M',
          "Map control must be a BMP with an explicit pixel offset");
    const uint32_t offset = uint32_t(bytes[10]) | (uint32_t(bytes[11]) << 8) |
                            (uint32_t(bytes[12]) << 16) | (uint32_t(bytes[13]) << 24);
    check(offset >= 54 && offset < bytes.size(), "Invalid map fixture pixel offset");
    // Preserve the BMP header/dimensions; change only the pixel payload.
    if (invert)
        for (size_t i = offset; i < bytes.size(); ++i) bytes[i] = uint8_t(255 - bytes[i]);
    fs::create_directories(root / "M3Maps");
    std::ofstream output(root / "M3Maps/9001.bmp", std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
    check(static_cast<bool>(output), "Cannot write isolated map fixture");
}

mb3d::RenderOptions options(int workers, const fs::path& root) {
    mb3d::RenderOptions opts;
    opts.thread_count = workers;
    opts.deterministic_mode = true;
    opts.no_shadows = opts.no_ao = opts.no_dof = opts.no_reflections = true;
    opts.resource_root = root.string();
    return opts;
}

mb3d::RenderResult render(Scene scene, const mb3d::RenderOptions& opts) {
    return mb3d::Render(scene.header, scene.addon, opts);
}

mb3d::RenderResult paint(Scene scene, const std::vector<TsiLight5>& surface,
                         const mb3d::RenderOptions& opts) {
    return mb3d::PaintFromSiLight(scene.header, scene.addon, surface, opts);
}

void successful(const mb3d::RenderResult& result, const std::string& label) {
    check(result.success && result.error.empty(), label + ": " + result.error);
    check(result.width == kWidth && result.height == kHeight &&
              result.image.size() == kWidth * kHeight * 3 &&
              result.silight.size() == kWidth * kHeight,
          label + ": incomplete output");
}

bool sameSurface(const mb3d::RenderResult& lhs, const mb3d::RenderResult& rhs) {
    return lhs.silight.size() == rhs.silight.size() &&
        std::memcmp(lhs.silight.data(), rhs.silight.data(),
                    lhs.silight.size() * sizeof(TsiLight5)) == 0;
}

void identical(const mb3d::RenderResult& actual, const mb3d::RenderResult& serial,
               const std::string& label) {
    successful(actual, label);
    check(actual.image == serial.image, label + ": RGB differs from same-worker serial control");
    check(sameSurface(actual, serial), label + ": packed siLight differs from same-worker serial control");
    check(actual.parity_trace_lines == serial.parity_trace_lines,
          label + ": trace configuration or output differs from serial control");
}

void nonconstant(const mb3d::RenderResult& result, const std::string& label) {
    bool differs = false;
    for (size_t i = 3; i < result.image.size(); i += 3)
        differs |= std::memcmp(result.image.data(), result.image.data() + i, 3) != 0;
    check(differs, label + ": degenerate constant RGB control");
}

// Both callers reach this barrier before either enters the public renderer.
// Each job receives its own header/addon copy, with shared read-only siLight.
// No process cwd, environment or resource files change while jobs are running.
class StartGate {
    std::mutex mutex_;
    std::condition_variable ready_;
    int arrived_ = 0;
public:
    void arrive() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (++arrived_ == 2) ready_.notify_all();
        else ready_.wait(lock, [&] { return arrived_ == 2; });
    }
};

template<class Left, class Right>
std::pair<mb3d::RenderResult, mb3d::RenderResult> concurrent(Left left, Right right) {
    StartGate gate;
    auto a = std::async(std::launch::async, [&] { gate.arrive(); return left(); });
    auto b = std::async(std::launch::async, [&] { gate.arrive(); return right(); });
    return {a.get(), b.get()};
}

template<class Left, class Right>
void overlap(const std::string& label, Left left, Right right,
             const mb3d::RenderResult& leftSerial, const mb3d::RenderResult& rightSerial) {
    // Repetition exercises cold per-job allocation and destruction as well as
    // differently scheduled worker phases. It is not a race-detector substitute.
    for (int repeat = 0; repeat != 3; ++repeat) {
        const auto actual = concurrent(left, right);
        identical(actual.first, leftSerial, label + " left " + std::to_string(repeat));
        identical(actual.second, rightSerial, label + " right " + std::to_string(repeat));
    }
}

void setDiffuseMap(Scene& scene) {
    constexpr unsigned mapNumber = 9001;
    scene.header.Light.bColorMap = uint8_t(mapNumber & 255);
    scene.header.Light.Lights[1].AdditionalByteEx = uint8_t(mapNumber >> 8);
    scene.header.Light.Lights[1].FreeByte &= uint8_t(~3u);
    scene.header.Light.Lights[2].AdditionalByteEx = 30;
}

mb3d::RenderResult apiRender(const fs::path& scene, int workers) {
    MB3DRenderOptions opts{};
    opts.width = kWidth;
    opts.height = kHeight;
    opts.thread_count = workers;
    opts.no_shadows = opts.no_ao = opts.no_dof = opts.no_reflections = 1;
    struct Owner {
        MB3DRenderResult value;
        ~Owner() { mb3d_free(value.pixels); mb3d_free(value.zbuffer); }
    } api{mb3d_render_file(scene.string().c_str(), &opts)};
    mb3d::RenderResult result;
    result.success = api.value.success && api.value.pixels && api.value.zbuffer;
    result.error = api.value.error;
    if (!result.success) return result;
    result.width = api.value.width;
    result.height = api.value.height;
    const size_t count = size_t(result.width) * result.height;
    result.image.assign(api.value.pixels, api.value.pixels + count * 3);
    result.silight.resize(count);
    // Only the public packed depth is present in the C API result. The other
    // lanes are zero in both C API controls and are not claimed as tested.
    for (size_t i = 0; i < count; ++i) result.silight[i].Zpos = api.value.zbuffer[i];
    return result;
}

void reflectionTraceRanges(const fs::path& source, const fs::path& resourceRoot,
                           const fs::path& scratch) {
#ifdef MB3D_ENABLE_PARITY_TRACE
    const auto scene = load(source /
        "comparison/scenes_phase3_reflections/phase3_refl_intpow2_r2.m3p");
    check((scene.header.bCalcSRautomatic & 1) != 0,
          "Reflection trace control must enable secondary rays");
    std::vector<std::string> serialEnd;
    std::vector<std::string> serialStart;
    for (int workers : {1, 4}) {
        auto opts = options(workers, resourceRoot);
        opts.no_reflections = false;
        opts.parity_trace_dump_path = (scratch / "reflection-range.trace").string();
        opts.diag_points.push_back({kWidth / 2, kHeight / 2});
        opts.parity_trace_end_stage = "mct_ready";
        const auto ended = render(scene, opts);
        successful(ended, "reflection trace end-range control");
        check(!ended.parity_trace_lines.empty() &&
              ended.parity_trace_lines.back().find("stage=mct_ready ") == 0,
              "Reflection worker resumed a trace already ended before ray marching");
        for (const auto& line : ended.parity_trace_lines)
            check(line.find("stage=sr_") != 0,
                  "Reflection worker emitted after the selected trace end stage");

        // The start stage is inside the late pass, after earlier reflection
        // setup/lighting emitters. A fresh worker cursor would log those too.
        opts.parity_trace_start_stage = "sr_primary";
        opts.parity_trace_end_stage = "sr_primary";
        const auto started = render(scene, opts);
        successful(started, "reflection trace start-range control");
        check(started.parity_trace_lines.size() == 1 &&
              started.parity_trace_lines.front().find("stage=sr_primary ") == 0,
              "Reflection worker ignored the selected trace start/end window");
        if (workers == 1) {
            serialEnd = ended.parity_trace_lines;
            serialStart = started.parity_trace_lines;
        } else {
            check(ended.parity_trace_lines == serialEnd,
                  "Trace ending before reflections differs between serial and parallel execution");
            check(started.parity_trace_lines == serialStart,
                  "Trace starting inside reflections differs between serial and parallel execution");
        }
    }
#endif
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 3)
            throw std::runtime_error("usage: test_render_isolation <source-root> <scratch-dir>");
        const fs::path source = fs::absolute(argv[1]);
        Scratch scratch(argv[2]);
        const auto rootA = scratch.path / "a";
        const auto rootB = scratch.path / "b";
        const auto fixture = source / "comparison/fixtures/M3Maps/9001.bmp";
        writeMap(fixture, rootA, false);
        writeMap(fixture, rootB, true);

        const auto constantA = load(source / "comparison/scenes/ext_09_aboxmod1.m3p");
        auto constantB = constantA;
        bool changed = false;
        for (auto& formula : constantB.addon.Formulas) {
            if (std::strncmp(reinterpret_cast<const char*>(formula.CustomFname), "ABoxMod1", 8) == 0) {
                check(formula.iOptionCount > 0, "External constant control has no Scale option");
                formula.dOptionValue[0] *= 1.125;
                changed = true;
            }
        }
        check(changed, "External constant control must select ABoxMod1");
        const auto heightmap = load(source / "comparison/scenes_validation/heightmap_real_asset.m3p");
        auto diffuse = load(source / "comparison/scenes/test_01_intpow2.m3p");
        setDiffuseMap(diffuse);
        auto background = load(source / "comparison/scenes/test_01_intpow2.m3p");
        std::memset(background.header.Light.BGbmp, 0, sizeof(background.header.Light.BGbmp));
        const char bgName[] = "M3Maps/9001.bmp";
        std::memcpy(background.header.Light.BGbmp, bgName, sizeof(bgName));
        const auto volume = load(source / "comparison/scenes_phase3_vollight/phase3_vl_intpow2_medium.m3p");
        check((volume.header.bVolLightNr & 7) != 0, "Volume control does not request a volume map");

        // Run before any serial AO call, so the direction-table initialization
        // is actually contested. Controls are captured afterward in this process.
        auto aoA = load(source / "comparison/scenes_phase3_hiqao/phase3_hiqao_intpow2_threshold_medium.m3p");
        auto aoB = load(source / "comparison/scenes_phase3_hiqao/phase3_hiqao_amazingbox_threshold_medium.m3p");
        aoA.header.bCalcAmbShadowAutomatic = aoB.header.bCalcAmbShadowAutomatic = 9;
        auto aoOpts = options(4, rootA);
        aoOpts.no_ao = false;
        const auto coldAO = concurrent([&] { return render(aoA, aoOpts); },
                                       [&] { return render(aoB, aoOpts); });
        const auto warmA = render(aoA, aoOpts);
        const auto warmB = render(aoB, aoOpts);
        successful(warmA, "warm HiQ AO A");
        successful(warmB, "warm HiQ AO B");
        identical(coldAO.first, warmA, "cold HiQ AO A");
        identical(coldAO.second, warmB, "cold HiQ AO B");

        reflectionTraceRanges(source, rootA, scratch.path);

        for (int workers : {1, 2, 4, 16}) {
            const std::string tag = std::to_string(workers) + " workers";
            const auto optsA = options(workers, rootA);
            const auto optsB = options(workers, rootB);
            const auto serialA = render(constantA, optsA);
            const auto serialB = render(constantB, optsB);
            successful(serialA, tag + " constants A");
            successful(serialB, tag + " constants B");
            nonconstant(serialA, tag + " constants A");
            check(serialA.image != serialB.image && !sameSurface(serialA, serialB),
                  tag + ": distinct external constants must change geometry and RGB");
            overlap(tag + " different external constants", [&] { return render(constantA, optsA); },
                    [&] { return render(constantB, optsB); }, serialA, serialB);

            const auto mapA = render(heightmap, optsA);
            const auto mapB = render(heightmap, optsB);
            successful(mapA, tag + " heightmap A");
            successful(mapB, tag + " heightmap B");
            nonconstant(mapA, tag + " heightmap A");
            check(mapA.image != mapB.image && !sameSurface(mapA, mapB),
                  tag + ": same-number map bytes must change geometry and RGB");
            overlap(tag + " heightmap resource roots", [&] { return render(heightmap, optsA); },
                    [&] { return render(heightmap, optsB); }, mapA, mapB);

            const auto surface = render(diffuse, optsA);
            successful(surface, tag + " diffuse surface");
            const auto paintA = paint(diffuse, surface.silight, optsA);
            const auto paintB = paint(diffuse, surface.silight, optsB);
            successful(paintA, tag + " diffuse paint A");
            successful(paintB, tag + " diffuse paint B");
            check(paintA.image != paintB.image,
                  tag + ": paint-only map controls must consume different map pixels");
            overlap(tag + " paint resource roots", [&] { return paint(diffuse, surface.silight, optsA); },
                    [&] { return paint(diffuse, surface.silight, optsB); }, paintA, paintB);
            overlap(tag + " render and paint", [&] { return render(constantB, optsB); },
                    [&] { return paint(diffuse, surface.silight, optsA); }, serialB, paintA);

            const auto bgA = paint(background, surface.silight, optsA);
            const auto bgB = paint(background, surface.silight, optsB);
            successful(bgA, tag + " background A");
            successful(bgB, tag + " background B");
            check(bgA.image != bgB.image, tag + ": background controls must consume root-specific bytes");
            overlap(tag + " background map borrowed view", [&] { return paint(background, surface.silight, optsA); },
                    [&] { return paint(background, surface.silight, optsB); }, bgA, bgB);

            const auto apiSceneA = source / "comparison/scenes/ext_09_aboxmod1.m3p";
            const auto apiSceneB = source / "comparison/scenes/ext_01_bristorbrot.m3p";
            const auto apiA = apiRender(apiSceneA, workers);
            const auto apiB = apiRender(apiSceneB, workers);
            successful(apiA, tag + " C API A");
            successful(apiB, tag + " C API B");
            check(apiA.image != apiB.image, tag + ": C API controls must differ");
            overlap(tag + " C API jobs", [&] { return apiRender(apiSceneA, workers); },
                    [&] { return apiRender(apiSceneB, workers); }, apiA, apiB);

#ifdef MB3D_ENABLE_PARITY_TRACE
            auto checkpointOpts = optsA;
            checkpointOpts.parity_trace_dump_path = (scratch.path / "checkpoint.trace").string();
            checkpointOpts.parity_trace_schema_mode = true;
            // Reuse a CLI-style continuation handle across jobs. Render must
            // clone its mutable cursor, not mutate options shared by callers.
            checkpointOpts.parity_trace_stage_state =
                std::make_shared<mb3d::ParityTraceStageRangeState>();
            checkpointOpts.parity_trace_stage_state->end_stage = "formula_contract_out";
            checkpointOpts.diag_points.push_back({kWidth / 2, kHeight / 2});
            auto detailOpts = checkpointOpts;
            detailOpts.parity_trace_dump_path = (scratch.path / "detail.trace").string();
            detailOpts.parity_trace_level = "all";
            auto legacyOpts = checkpointOpts;
            legacyOpts.parity_trace_dump_path = (scratch.path / "legacy.trace").string();
            legacyOpts.parity_trace_schema_mode = false;
            const auto checkpoint = render(constantA, checkpointOpts);
            const auto detail = render(constantA, detailOpts);
            const auto legacy = render(constantA, legacyOpts);
            successful(checkpoint, tag + " checkpoint trace");
            successful(detail, tag + " detailed trace");
            successful(legacy, tag + " legacy trace");
            check(!checkpoint.parity_trace_lines.empty() && !detail.parity_trace_lines.empty() &&
                      !legacy.parity_trace_lines.empty(), tag + ": trace controls must emit records");
            check(checkpoint.parity_trace_lines != detail.parity_trace_lines &&
                      checkpoint.parity_trace_lines != legacy.parity_trace_lines,
                  tag + ": trace format/detail controls must differ");
            overlap(tag + " checkpoint and detailed trace", [&] { return render(constantA, checkpointOpts); },
                    [&] { return render(constantA, detailOpts); }, checkpoint, detail);
            overlap(tag + " schema and legacy trace", [&] { return render(constantA, checkpointOpts); },
                    [&] { return render(constantA, legacyOpts); }, checkpoint, legacy);
            check(!checkpointOpts.parity_trace_stage_state->end_reached &&
                      checkpointOpts.parity_trace_stage_state->pending_stage.empty(),
                  tag + ": public renders mutated the caller's shared trace cursor");
#endif

            // Each worker-count case starts with a truly missing map. Missing
            // resources preserve the existing fallback contract, not an error.
            const auto missingRoot = scratch.path / ("missing-" + std::to_string(workers));
            fs::create_directories(missingRoot);
            const auto missingOpts = options(workers, missingRoot);
            const auto missing = render(heightmap, missingOpts);
            successful(missing, tag + " missing-map fallback");
            check(!sameSurface(missing, mapA), tag + ": missing-map control did not exercise fallback");
            overlap(tag + " missing and valid map", [&] { return render(heightmap, missingOpts); },
                    [&] { return render(heightmap, optsA); }, missing, mapA);
            writeMap(fixture, missingRoot, false);
            identical(render(heightmap, missingOpts), mapA, tag + " map recovery in new job");

            auto volumeOpts = optsA;
            volumeOpts.no_shadows = false;
            const auto volumeSerial = render(volume, volumeOpts);
            successful(volumeSerial, tag + " volume");
            nonconstant(volumeSerial, tag + " volume");
            overlap(tag + " volume and ordinary render", [&] { return render(volume, volumeOpts); },
                    [&] { return render(constantB, optsB); }, volumeSerial, serialB);
            // Re-enter after other contexts are destroyed: no dangling formula,
            // lighting-map or volume-map storage may survive the public call.
            identical(render(constantA, optsA), serialA, tag + " final cold constants recovery");
            identical(render(volume, volumeOpts), volumeSerial, tag + " final cold volume recovery");
        }
        std::printf("Render/Paint/C API job isolation: %d checks passed at 1/2/4/16 workers; "
                    "same-platform concurrency evidence, not Pascal parity.\n", checks);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Render/Paint job isolation failed: %s\n", error.what());
        return 1;
    }
}
