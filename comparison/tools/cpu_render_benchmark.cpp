// Full ordinary CPU renders: one fresh-process first frame, then same-process
// repetitions. Each Render still creates and releases its own job resources.
#include "core/formula_registry.h"
#include "core/image_process.h"
#include "core/renderer.h"
#include "io/image_writer.h"
#include "io/m3p_parser.h"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;

double milliseconds(Clock::time_point begin, Clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - begin).count();
}

std::string quoted(const std::string& value) {
    std::ostringstream out;
    out << '"';
    static constexpr char digits[] = "0123456789abcdef";
    for (unsigned char c : value) {
        if (c == '"' || c == '\\') out << '\\' << c;
        else if (c < 0x20) out << "\\u00" << digits[c >> 4] << digits[c & 15];
        else out << c;
    }
    out << '"';
    return out.str();
}

struct Options {
    fs::path scene, output, resource_root;
    mb3d::PrecisionMode precision = mb3d::DefaultPrecisionMode();
    int width = 0, height = 0, threads = 0, warmups = 1, repeats = 3;
};

int integer(const char* value, const std::string& key) {
    int result = 0;
    const char* end = value + std::strlen(value);
    const auto parsed = std::from_chars(value, end, result);
    if (parsed.ec != std::errc{} || parsed.ptr != end)
        throw std::invalid_argument("Invalid integer for " + key);
    return result;
}

Options parse(int argc, char** argv) {
    Options result;
    std::vector<std::string> seen;
    for (int i = 1; i < argc; ++i) {
        const std::string key = argv[i];
        if (std::find(seen.begin(), seen.end(), key) != seen.end())
            throw std::invalid_argument("Repeated option: " + key);
        seen.push_back(key);
        if (i + 1 == argc) throw std::invalid_argument("Missing value for " + key);
        const char* value = argv[++i];
        if (key == "--scene") result.scene = value;
        else if (key == "--output") result.output = value;
        else if (key == "--resource-root") result.resource_root = value;
        else if (key == "--precision") {
            if (!mb3d::ParsePrecisionMode(value, result.precision))
                throw std::invalid_argument("Precision must be compatible or native");
        }
        else if (key == "--width") result.width = integer(value, key);
        else if (key == "--height") result.height = integer(value, key);
        else if (key == "--threads") result.threads = integer(value, key);
        else if (key == "--warmups") result.warmups = integer(value, key);
        else if (key == "--repeats") result.repeats = integer(value, key);
        else throw std::invalid_argument("Unknown option: " + key);
    }
    if (result.scene.empty() || result.output.empty() ||
        result.width < 1 || result.height < 1 || result.width > 16384 ||
        result.height > 16384 || result.threads < 1 || result.warmups < 0 ||
        result.repeats < 3)
        throw std::invalid_argument(
            "Required: --scene FILE --output NEW_DIR --width 1..16384 "
            "--height 1..16384 --threads N [--warmups N>=0] [--repeats N>=3] "
            "[--resource-root DIR]");
    result.scene = fs::absolute(result.scene);
    result.output = fs::absolute(result.output);
    result.resource_root = result.resource_root.empty() ? fs::current_path()
                                                       : fs::absolute(result.resource_root);
    if (!fs::is_regular_file(result.scene))
        throw std::invalid_argument("Scene is not a regular file");
    std::string extension = result.scene.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (extension != ".m3p") throw std::invalid_argument("Only ordinary .m3p input is supported");
    if (!fs::is_directory(result.resource_root))
        throw std::invalid_argument("Resource root is not a directory");
    return result;
}

std::string formula_name(const THAformula& formula) {
    const char* data = reinterpret_cast<const char*>(formula.CustomFname);
    size_t size = 0;
    while (size < sizeof(formula.CustomFname) && data[size]) ++size;
    while (size && data[size - 1] == ' ') --size;
    return std::string(data, size);
}

void reject_deferred(const mb3d::M3PLoadResult& scene) {
    if (scene.header.MCoptions & 1u)
        throw std::invalid_argument("Deferred Monte Carlo scene is excluded");
    for (const auto& formula : scene.addon.Formulas) {
        if (mb3d::IsInternalFormula(formula.iFnr)) continue;
        std::string name = formula_name(formula);
        std::transform(name.begin(), name.end(), name.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        // Reject even dormant saved JIT slots rather than admit a deferred input.
        if (name.find("jit") != std::string::npos)
            throw std::invalid_argument("Deferred JIT scene is excluded");
    }
    // Render performs the existing prepared-parameter and reachable-formula
    // validation before it starts workers. Do not initialize its registry here:
    // cold timing must include the real first Render's preparation.
}

struct Sample {
    std::string phase, image, state = "not_run", error;
    int index = 0, width = 0, height = 0, threads = 0;
    bool compared = false, exact_rgb = false;
    double load_ms = 0, render_ms = 0, readback_ms = 0;
    double render_and_readback_ms = 0, png_encode_write_ms = 0;
    double end_to_end_ms = 0, cold_total_ms = 0, cold_end_to_end_ms = 0;
    mb3d::RenderProfile profile;
    mb3d::RenderStageTiming final_surface, save;
};

struct Report {
    Options options;
    std::string status = "FAILED", error;
    mb3d::M3PLoadResult loaded{};
    double load_ms = 0, setup_ms = 0;
    Sample cold;
    std::vector<Sample> warmups, samples;
};

void write_sample(std::ostream& out, const Sample& s) {
    out << "{\"phase\":" << quoted(s.phase) << ",\"index\":" << s.index
        << ",\"image\":" << quoted(s.image) << ",\"state\":" << quoted(s.state)
        << ",\"error\":" << quoted(s.error) << ",\"width\":" << s.width
        << ",\"height\":" << s.height << ",\"threads\":" << s.threads
        << ",\"exact_rgb_matches_cold\":";
    if (s.compared) out << (s.exact_rgb ? "true" : "false");
    else out << "null";
    out << ",\"load_ms\":" << s.load_ms
        << ",\"preparation_ms\":" << s.profile.stages[static_cast<size_t>(mb3d::RenderProfileStage::Preparation)].seconds * 1000.0
        << ",\"render_ms\":" << s.render_ms
        << ",\"readback_ms\":" << s.readback_ms
        << ",\"render_and_readback_ms\":" << s.render_and_readback_ms
        << ",\"png_encode_write_ms\":" << s.png_encode_write_ms
        << ",\"end_to_end_ms\":" << s.end_to_end_ms;
    if (s.phase == "cold")
        out << ",\"cold_total_ms\":" << s.cold_total_ms
            << ",\"cold_end_to_end_ms\":" << s.cold_end_to_end_ms;
    out << ",\"render_profile_total_ms\":" << s.profile.total_seconds * 1000.0
        << ",\"stages\": {";
    for (size_t i = 0; i < s.profile.stages.size(); ++i) {
        if (i) out << ',';
        const auto& stage = s.profile.stages[i];
        out << quoted(mb3d::RenderProfileStageName(static_cast<mb3d::RenderProfileStage>(i)))
            << ":{\"state\":" << quoted(stage.state) << ",\"seconds\":" << stage.seconds << '}';
    }
    out << ",\"load\":{\"state\":" << quoted(s.phase == "cold" ? "completed" : "not_run")
        << ",\"seconds\":" << s.load_ms / 1000.0 << '}'
        << ",\"final_surface\":{\"state\":" << quoted(s.final_surface.state)
        << ",\"seconds\":" << s.final_surface.seconds << '}'
        << ",\"save\":{\"state\":" << quoted(s.save.state)
        << ",\"seconds\":" << s.save.seconds << "}}}";
}

void write_report(const Report& report) {
    const auto& opts = report.options;
    const auto& h = report.loaded.header;
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << std::setprecision(17)
        << "{\n\"schema\":\"mb3d.cpu-render-benchmark.v1\",\"backend\":\"cpu\",\"status\":"
        << quoted(report.status) << ",\"error\":" << quoted(report.error)
        << ",\"scene\":" << quoted(opts.scene.string())
        << ",\"resource_root\":" << quoted(opts.resource_root.string())
#if defined(__aarch64__) || defined(_M_ARM64)
        << ",\"architecture\":\"arm64\""
#elif defined(__x86_64__) || defined(_M_X64)
        << ",\"architecture\":\"x86_64\""
#else
        << ",\"architecture\":\"other\""
#endif
        << ",\n\"settings\":{\"width\":" << opts.width << ",\"height\":" << opts.height
        << ",\"threads\":" << opts.threads << ",\"warmups\":" << opts.warmups
        << ",\"repeats\":" << opts.repeats << '}'
        << ",\"execution\":{\"deterministic\":true,\"profile_timing\":true,\"work_control\":\"none\",\"precision\":"
        << quoted(mb3d::PrecisionModeName(opts.precision)) << '}'
        << ",\n\"timing_contract\":{"
           "\"cold\":\"first Render in a fresh process; OS file caches are not flushed\","
           "\"warm\":\"same process and loaded scene; each Render owns fresh job resources and workers\","
           "\"render_ms\":\"production Render call including preparation and worker teardown\","
           "\"readback_ms\":\"CPU BuildFinalRGBSurface copy at scale 1 and crop 0; no device readback\","
           "\"cold_total_ms\":\"scene load through first final surface; excludes PNG encode/write\","
           "\"cold_end_to_end_ms\":\"scene load through first PNG encode/write\","
           "\"end_to_end_ms\":\"header/addon copy through PNG encode/write; cold includes load\","
           "\"validation\":\"RGB byte comparisons and report serialization happen after timing\"}"
        << ",\n\"setup_ms\":{\"load\":" << report.load_ms
        << ",\"ordinary_input_check\":" << report.setup_ms << '}';
    out << ",\n\"loaded_features\":{\"load_success\":" << (report.loaded.success ? "true" : "false")
        << ",\"source_mand_id\":" << report.loaded.source_mand_id
        << ",\"width\":" << h.Width << ",\"height\":" << h.Height
        << ",\"iterations\":" << h.Iterations
        << ",\"bCalc3D\":" << unsigned(h.bCalc3D) << ",\"effective_bCalc3D\":1"
        << ",\"bCalculateHardShadow\":" << unsigned(h.bCalculateHardShadow)
        << ",\"bCalc1HSsoft\":" << unsigned(h.bCalc1HSsoft)
        << ",\"bNormalsOnDE\":" << unsigned(h.bNormalsOnDE)
        << ",\"bCalcAmbShadowAutomatic\":" << unsigned(h.bCalcAmbShadowAutomatic)
        << ",\"bCalcSRautomatic\":" << unsigned(h.bCalcSRautomatic)
        << ",\"bCalcDOFtype\":" << unsigned(h.bCalcDOFtype)
        << ",\"byCalcNsOnZBufAuto\":" << unsigned(h.byCalcNsOnZBufAuto)
        << ",\"bVolLightNr\":" << unsigned(h.bVolLightNr)
        << ",\"bStereoMode\":" << unsigned(h.bStereoMode)
        << ",\"bPlanarOptic\":" << unsigned(h.bPlanarOptic)
        << ",\"bDFogIt\":" << unsigned(h.bDFogIt)
        << ",\"TilingOptions\":" << h.TilingOptions
        << ",\"bIsJulia\":" << unsigned(h.bIsJulia)
        << ",\"MCoptions\":" << unsigned(h.MCoptions)
        << ",\"hybrid_mode\":" << unsigned(report.loaded.addon.bOptions1 & 3)
        << ",\"formulas\":[";
    for (int i = 0; i < 6; ++i) {
        if (i) out << ',';
        const auto& f = report.loaded.addon.Formulas[i];
        out << "{\"slot\":" << i << ",\"id\":" << f.iFnr
            << ",\"name\":" << quoted(formula_name(f)) << ",\"iterations\":" << f.iItCount << '}';
    }
    out << "]},\n\"cold\":";
    write_sample(out, report.cold);
    out << ",\n\"warmups\":[";
    for (size_t i = 0; i < report.warmups.size(); ++i) {
        if (i) out << ',';
        write_sample(out, report.warmups[i]);
    }
    out << "],\n\"samples\":[";
    for (size_t i = 0; i < report.samples.size(); ++i) {
        if (i) out << ',';
        write_sample(out, report.samples[i]);
    }
    out << "]\n}\n";
    const std::string json = out.str();
    const auto path = opts.output / "report.json";
    std::FILE* file = std::fopen(path.string().c_str(), "wbx");
    if (!file) throw std::runtime_error("Could not exclusively create report.json");
    const bool written = std::fwrite(json.data(), 1, json.size(), file) == json.size();
    const int closed = std::fclose(file);
    if (!written || closed != 0) throw std::runtime_error("Could not write report.json");
}

std::string image_name(const std::string& phase, int index) {
    if (phase == "cold") return "cold.png";
    std::ostringstream out;
    out << phase << '-' << std::setw(3) << std::setfill('0') << index << ".png";
    return out.str();
}

void run_sample(Report& report, Sample& sample, const mb3d::RenderOptions& options,
                const std::string& phase, int index, Clock::time_point cold_start,
                std::vector<uint8_t>& reference, int& reference_width, int& reference_height) {
    sample.phase = phase;
    sample.index = index;
    sample.image = image_name(phase, index);
    sample.state = "interrupted";
    sample.load_ms = phase == "cold" ? report.load_ms : 0;
    const fs::path path = report.options.output / sample.image;
    const auto begin = Clock::now();
    // Render may mutate both records. Never carry those writes into another job.
    auto header = report.loaded.header;
    auto addon = report.loaded.addon;
    header.bCalc3D = 1; // Same full-3D headless convention as mb3d_render.
    const auto render_begin = Clock::now();
    auto rendered = mb3d::Render(header, addon, options);
    const auto render_end = Clock::now();
    sample.render_ms = milliseconds(render_begin, render_end);
    sample.profile = rendered.profile;
    sample.width = rendered.width;
    sample.height = rendered.height;
    sample.threads = rendered.profile.workers;
    if (!rendered.success) {
        sample.error = rendered.error;
        throw std::runtime_error("Render failed: " + rendered.error);
    }
    sample.final_surface.state = "interrupted";
    mb3d::FinalRGBSurface surface;
    const auto surface_begin = Clock::now();
    const bool surface_ok = mb3d::BuildFinalRGBSurface(rendered.image.data(), rendered.width,
                                                      rendered.height, 1, 0, 0, surface);
    const auto surface_end = Clock::now();
    sample.readback_ms = milliseconds(surface_begin, surface_end);
    sample.final_surface.seconds = sample.readback_ms / 1000.0;
    sample.render_and_readback_ms = milliseconds(render_begin, surface_end);
    if (!surface_ok) throw std::runtime_error("BuildFinalRGBSurface failed");
    sample.final_surface.state = "completed";
    sample.save.state = "interrupted";
    const auto save_begin = Clock::now();
    const bool saved = mb3d::WritePNG(path.string(), surface.pixels.data(), surface.width, surface.height, 3);
    const auto save_end = Clock::now();
    sample.png_encode_write_ms = milliseconds(save_begin, save_end);
    sample.save.seconds = sample.png_encode_write_ms / 1000.0;
    sample.end_to_end_ms = milliseconds(begin, save_end);
    if (phase == "cold") {
        sample.cold_total_ms = milliseconds(cold_start, surface_end);
        sample.cold_end_to_end_ms = milliseconds(cold_start, save_end);
        sample.end_to_end_ms = sample.cold_end_to_end_ms;
    }
    if (!saved) throw std::runtime_error("PNG encode/write failed");
    sample.save.state = "completed";
    // No byte comparisons, hashing, test dispatch or independent oracle enters
    // any timed region. Retain only the cold RGB, not all repeated frame buffers.
    if (phase == "cold") {
        reference = std::move(surface.pixels);
        reference_width = surface.width;
        reference_height = surface.height;
        sample.exact_rgb = true;
    } else {
        sample.exact_rgb = surface.width == reference_width && surface.height == reference_height &&
                           surface.pixels == reference;
    }
    sample.compared = true;
    sample.state = "completed";
    if (!sample.exact_rgb) throw std::runtime_error("Repeated RGB bytes differ from cold frame");
}
} // namespace

int main(int argc, char** argv) {
    if (argc == 2 && std::strcmp(argv[1], "--help") == 0) {
        std::puts("mb3d_cpu_render_benchmark --scene FILE --output NEW_DIR --width N --height N "
                  "--threads N [--warmups N>=0] [--repeats N>=3] [--resource-root DIR]");
        return 0;
    }
    Report report;
    bool created = false;
    try {
        report.options = parse(argc, argv);
        if (!fs::create_directory(report.options.output))
            throw std::invalid_argument("Output directory must be new");
        created = true;
        mb3d::RenderOptions options;
        options.precision = report.options.precision;
        options.width_override = report.options.width;
        options.height_override = report.options.height;
        options.thread_count = report.options.threads;
        options.resource_root = report.options.resource_root.string();
        options.deterministic_mode = true;
        options.profile_timing = true;
        report.warmups.reserve(static_cast<size_t>(report.options.warmups));
        report.samples.reserve(static_cast<size_t>(report.options.repeats));
        const auto cold_start = Clock::now();
        report.loaded = mb3d::LoadM3PFile(report.options.scene.string());
        const auto loaded_at = Clock::now();
        report.load_ms = milliseconds(cold_start, loaded_at);
        if (!report.loaded.success) throw std::runtime_error("Scene load failed: " + report.loaded.error);
        reject_deferred(report.loaded);
        report.setup_ms = milliseconds(loaded_at, Clock::now());
        std::vector<uint8_t> reference;
        int reference_width = 0, reference_height = 0;
        run_sample(report, report.cold, options, "cold", 0, cold_start,
                   reference, reference_width, reference_height);
        for (int i = 0; i < report.options.warmups; ++i) {
            report.warmups.emplace_back();
            run_sample(report, report.warmups.back(), options, "warmup", i, cold_start,
                       reference, reference_width, reference_height);
        }
        for (int i = 0; i < report.options.repeats; ++i) {
            report.samples.emplace_back();
            run_sample(report, report.samples.back(), options, "warm", i, cold_start,
                       reference, reference_width, reference_height);
        }
        report.status = "COMPLETE";
        write_report(report);
        std::printf("CPU benchmark complete: %s\n", (report.options.output / "report.json").string().c_str());
        return 0;
    } catch (const std::exception& error) {
        report.status = "FAILED";
        report.error = error.what();
        std::fprintf(stderr, "CPU benchmark failed: %s\n", error.what());
        if (created && !fs::exists(report.options.output / "report.json")) {
            try { write_report(report); }
            catch (const std::exception& write_error) {
                std::fprintf(stderr, "Failure report could not be saved: %s\n", write_error.what());
            }
        }
        return 1;
    }
}
