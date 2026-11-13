#include "core/ray_marcher.h"
#include "core/numerical_de.h"
#include "core/types.h"
#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
extern "C" void BeginFixtureX87(void*);
extern "C" void EndFixtureX87(void*);
extern "C" void Rout3_compiled_pas_sse2(double*, const double*);
struct FixtureX87Environment {
    alignas(16) unsigned char saved[108];
    FixtureX87Environment() { BeginFixtureX87(saved); }
    ~FixtureX87Environment() { EndFixtureX87(saved); }
    void restart() { EndFixtureX87(saved); BeginFixtureX87(saved); }
};
#endif
#include "core/math3d.h"
#include "core/header_trafos.h"
#include "core/ambient_occlusion.h"
#include "core/colorizer.h"
#include "core/lightmap.h"
#include "core/formulas.h"
#include "core/image_process.h"
#include "core/renderer.h"
#include "core/render_context.h"
#include "core/reflection.h"
#include "core/formula_registry.h"
#include "core/external_formulas.h"
#include <map>
#include "io/m3p_parser.h"
#include "io/image_writer.h"
#include "io/buffer_export.h"
#include "windows_console_crash_guard.h"
#include "fixtures/four_d_original_code.h"
#include "fixtures/pinehedron_reference.h"
#include "fixtures/smooth_fold_reference.h"
#include "fixtures/vsshapes_reference.h"
#include "fixtures/formula_regressions.h"
#include "fixtures/smooth_iterations_reference.h"
#include "fixtures/integer_power8_reference.h"
#include "fixtures/visible_light_math_direct.h"
#include "fixtures/map_preprocessing.h"
#include "fixtures/map_sampling.h"
#include "fixtures/volume_sampling.h"
#include "fixtures/transmission_predicate.h"
#include "fixtures/transmission_power.h"
#include "fixtures/radius_de_reference.h"
namespace room_triplex_reference {
#include "fixtures/room_triplex_regressions.h"
}
namespace fold_cut_reference {
#include "fixtures/fold_cut_regressions.h"
}
namespace box_tetra_pine_reference {
#include "fixtures/box_tetra_pine_regressions.h"
}
namespace iq_hex_golden_reference {
#include "fixtures/iq_hex_golden_regressions.h"
}
namespace iq_hex_boundary_reference {
#include "fixtures/iq_hex_boundary_regressions.h"
}
namespace fuzzy_radius_reference {
#include "fixtures/fuzzy_radius_regressions.h"
}
namespace vary2_counter_reference {
#include "fixtures/vary2_counter_regressions.h"
}
namespace slice_coord_golden_reference {
#include "fixtures/slice_coord_golden_regressions.h"
}
namespace slice_coord_boundary_reference {
#include "fixtures/slice_coord_boundary_regressions.h"
}
namespace fold_kali_tube_golden_reference {
#include "fixtures/fold_kali_tube_golden_regressions.h"
}
namespace fold_kali_tube_boundary_reference {
#include "fixtures/fold_kali_tube_boundary_regressions.h"
}
namespace fold_kali_tube_rout_reference {
#include "fixtures/fold_kali_tube_rout_regressions.h"
}
namespace logsp_full_options_reference {
#include "fixtures/logsp_full_options_regressions.h"
}
namespace varyscale_pair_parameters_reference {
#include "fixtures/varyscale_pair_parameters_regressions.h"
}
namespace varyscale_pair_boundary_reference {
#include "fixtures/varyscale_pair_boundary_regressions.h"
}
namespace amazingbox_sse2_parameters_reference {
#include "fixtures/amazingbox_sse2_parameters_regressions.h"
}
namespace amazingbox_sse2_boundary_reference {
#include "fixtures/amazingbox_sse2_boundary_regressions.h"
}
namespace devil_citrus_parameters_reference {
#include "fixtures/devil_citrus_parameters_regressions.h"
}
namespace devil_citrus_boundary_reference {
#include "fixtures/devil_citrus_boundary_regressions.h"
}
namespace magnet_bicomplex_parameters_reference {
#include "fixtures/magnet_bicomplex_parameters_regressions.h"
}
namespace magnet_bicomplex_boundary_reference {
#include "fixtures/magnet_bicomplex_boundary_regressions.h"
}
namespace fold_poincare_reference {
#include "fixtures/fold_poincare_regressions.h"
}
namespace last_image_reference {
#include "fixtures/last_image_regressions.h"
}
namespace quatpow_exp_boundary_reference {
#include "fixtures/quatpow_exp_boundary_regressions.h"
}
namespace remaining_points_parameters_reference {
#include "fixtures/remaining_points_parameters_regressions.h"
}
namespace remaining_points_exceptional_reference {
#include "fixtures/remaining_points_exceptional_regressions.h"
}
namespace cyl_torus_parameters_reference {
#include "fixtures/cyl_torus_parameters_regressions.h"
}
namespace cyl_torus_exceptional_reference {
#include "fixtures/cyl_torus_exceptional_regressions.h"
}
namespace trifox_reference {
#include "fixtures/trifox_regressions.h"
}
namespace scalar_map_parameters_reference {
#include "fixtures/scalar_map_parameters_regressions.h"
}
namespace scalar_map_exceptional_reference {
#include "fixtures/scalar_map_exceptional_regressions.h"
}
namespace knots_weird_parameters_reference {
#include "fixtures/knots_weird_parameters_regressions.h"
}
namespace knots_weird_boundary_reference {
#include "fixtures/knots_weird_boundary_regressions.h"
}
namespace fold_order_parameters_reference {
#include "fixtures/fold_order_parameters_regressions.h"
}
namespace fold_order_boundary_reference {
#include "fixtures/fold_order_boundary_regressions.h"
}
namespace ifs_apply_parameters_reference {
#include "fixtures/ifs_apply_parameters_regressions.h"
}
namespace ifs_apply_boundary_reference {
#include "fixtures/ifs_apply_boundary_regressions.h"
}
namespace branch_julia_parameters_reference {
#include "fixtures/branch_julia_parameters_regressions.h"
}
namespace branch_julia_boundary_reference {
#include "fixtures/branch_julia_boundary_regressions.h"
}
namespace heightmap_family_planar_parameters_reference {
#include "fixtures/heightmap_family_planar_parameters_regressions.h"
}
namespace heightmap_family_planar_boundary_reference {
#include "fixtures/heightmap_family_planar_boundary_regressions.h"
}
namespace heightmap_family_sphere_parameters_reference {
#include "fixtures/heightmap_family_sphere_parameters_regressions.h"
}
namespace heightmap_family_sphere_boundary_reference {
#include "fixtures/heightmap_family_sphere_boundary_regressions.h"
}
namespace angular_radius_parameters_reference {
#include "fixtures/angular_radius_parameters_regressions.h"
}
namespace angular_radius_boundary_reference {
#include "fixtures/angular_radius_boundary_regressions.h"
}
namespace angular_radius_radius_reference {
#include "fixtures/angular_radius_radius_regressions.h"
}
namespace projection_reciprocal_parameters_reference {
#include "fixtures/projection_reciprocal_parameters_regressions.h"
}
namespace projection_reciprocal_boundary_reference {
#include "fixtures/projection_reciprocal_boundary_regressions.h"
}
namespace mapfold_golden_reference {
#include "fixtures/mapfold_golden_regressions.h"
}
namespace icen_golden_reference {
#include "fixtures/icen_golden_regressions.h"
}
namespace mapfold_boundary_reference {
#include "fixtures/mapfold_boundary_regressions.h"
}
namespace icen_boundary_reference {
#include "fixtures/icen_boundary_regressions.h"
}
namespace icen_counter_reference {
#include "fixtures/icen_counter_regressions.h"
}
namespace axis_wave_box_golden_reference {
#include "fixtures/axis_wave_box_golden_regressions.h"
}
namespace axis_wave_box_boundary_golden_reference {
#include "fixtures/axis_wave_box_boundary_golden_regressions.h"
}
namespace dodeca_wavy_golden_reference {
#include "fixtures/dodeca_wavy_golden_regressions.h"
}
namespace dodeca_wavy_boundary_golden_reference {
#include "fixtures/dodeca_wavy_boundary_golden_regressions.h"
}
namespace wavy_counter_golden_reference {
#include "fixtures/wavy_counter_golden_regressions.h"
}
namespace gnarly_family_reference {
#include "fixtures/gnarly_family_regressions.h"
}
namespace gnarly_family_boundary_reference {
#include "fixtures/gnarly_family_boundary_regressions.h"
}
namespace box_tetra_pine_boundary_reference {
#include "fixtures/box_tetra_pine_boundary_regressions.h"
}
namespace manowar_sse_reference {
#include "fixtures/manowar_sse_regressions.h"
}
namespace kali_reference {
#include "fixtures/kali_regressions.h"
}
namespace aboxvsshapes_reference {
#include "fixtures/aboxvsshapes_regressions.h"
}
namespace aboxvsshapes_counter_reference {
#include "fixtures/aboxvsshapes_counter_regressions.h"
}
namespace sliceex_reference {
#include "fixtures/sliceex_regressions.h"
}
namespace e3dtess_reference {
#include "fixtures/e3dtess_regressions.h"
}
namespace columns_reference {
#include "fixtures/columns_regressions.h"
}
namespace piriform_reference {
#include "fixtures/piriform_regressions.h"
}
namespace helispiral_reference {
#include "fixtures/helispiral_regressions.h"
}
namespace mengerkochv2_reference {
#include "fixtures/mengerkochv2_regressions.h"
}
namespace amazingboxtransform_reference {
#include "fixtures/amazingboxtransform_regressions.h"
}
namespace cubetube_reference {
#include "fixtures/cubetube_regressions.h"
}
namespace maptranslate_reference {
#include "fixtures/maptranslate_regressions.h"
}
namespace julia_state_reference {
#include "fixtures/julia_state_regressions.h"
}
namespace transform_state_reference {
#include "fixtures/transform_state_regressions.h"
}
namespace sine_approx_reference {
#include "fixtures/sine_approx_regressions.h"
}
namespace state_writeback_reference {
#include "fixtures/state_writeback_regressions.h"
}
namespace cayley_reference {
#include "fixtures/cayley_regressions.h"
}
namespace ifs_reference {
#include "fixtures/ifs_regressions.h"
}
namespace discoball_reference {
#include "fixtures/discoball_regressions.h"
}
namespace sphcage_reference {
#include "fixtures/sphcage_regressions.h"
}
namespace riemann2_reference {
#include "fixtures/riemann2_regressions.h"
}
namespace hycinv_reference {
#include "fixtures/hycinv_regressions.h"
}
namespace mag_sign_reference {
#include "fixtures/mag_sign_regressions.h"
}
namespace boxifs_twist_reference {
#include "fixtures/boxifs_twist_regressions.h"
}
namespace bulbox_reference {
#include "fixtures/bulbox_regressions.h"
}
namespace sphereoffset4d_reference {
#include "fixtures/sphereoffset4d_regressions.h"
}
namespace knots_reference {
#include "fixtures/knots_regressions.h"
}
namespace kleinbot_reference {
#include "fixtures/kleinbot_regressions.h"
}
namespace general_reference {
#include "fixtures/general_regressions.h"
}
namespace ifstess_reference {
#include "fixtures/ifstess_regressions.h"
}
namespace cantor_reference {
#include "fixtures/cantor_regressions.h"
}
namespace amazing_surf2_reference {
#include "fixtures/amazing_surf2_regressions.h"
}
namespace surf2_smooth_selector_reference {
#include "fixtures/surf2_smooth_selector_regressions.h"
}
namespace newfold_restore_reference {
#include "fixtures/newfold_restore_regressions.h"
}
namespace amazing_surf_reference {
#include "fixtures/amazing_surf_regressions.h"
}
namespace aexion_reference {
#include "fixtures/aexion_regressions.h"
}
namespace pg_bulb_power_reference {
#include "fixtures/pg_bulb_power_regressions.h"
}
namespace selector_overflow_reference {
#include "fixtures/selector_overflow_regressions.h"
}
namespace cell_conversion_reference {
#include "fixtures/cell_conversion_regressions.h"
}
namespace comma_reference {
#include "fixtures/comma_regressions.h"
}
namespace mengerhyper_reference {
#include "fixtures/mengerhyper_regressions.h"
}
namespace juliasets_reference {
#include "fixtures/juliasets_regressions.h"
}
namespace atetravs_reference {
#include "fixtures/atetravs_regressions.h"
}
namespace spherefold_reference {
#include "fixtures/spherefold_regressions.h"
}
namespace aboxsmooth_reference {
#include "fixtures/aboxsmooth_regressions.h"
}
namespace platinum_b_reference {
#include "fixtures/platinum_b_regressions.h"
}
namespace celtic_reference {
#include "fixtures/celtic_regressions.h"
}
namespace mandalay_reference {
#include "fixtures/mandalay_regressions.h"
}
namespace mandalex_reference {
#include "fixtures/mandalex_regressions.h"
}
namespace menger_original_reference {
#include "fixtures/menger_original_regressions.h"
}
namespace boxifs_reference {
#include "fixtures/boxifs_regressions.h"
}
namespace tiling_rucker_reference {
#include "fixtures/tiling_rucker_regressions.h"
}
namespace hexgrid_family_reference {
#include "fixtures/hexgrid_family_regressions.h"
}
namespace hexgrid_reference {
#include "fixtures/hexgrid_regressions.h"
}
namespace gear2_reference {
#include "fixtures/gear2_regressions.h"
}
namespace gear_reference {
#include "fixtures/gear_regressions.h"
}
namespace logspiral_reference {
#include "fixtures/logspiral_regressions.h"
}
namespace iteration_rotation_reference {
#include "fixtures/iteration_rotation_regressions.h"
}
namespace random_transform_reference {
#include "fixtures/random_transform_regressions.h"
}
namespace foldcut_cancellation_reference {
#include "fixtures/foldcut_cancellation_regressions.h"
}
namespace pinegon_reference {
#include "fixtures/pinegon_regressions.h"
}
namespace setw_cancellation_reference {
#include "fixtures/setw_cancellation_regressions.h"
}
#include "mb3d/mb3d.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <limits>

static int g_passed = 0;
static int g_failed = 0;
static const char* g_formulaFilter = nullptr;
static size_t g_formulaRows = 0;
struct FormulaFixtureStats { size_t rows=0, checks=0, failures=0; };
static std::map<std::string, FormulaFixtureStats> g_formulaStats;
static bool g_verbose = false;
static constexpr bool kVerbosePacketTrace = false;

#define LOG_IF_V(...) do { if (g_verbose) std::printf(__VA_ARGS__); } while(0)

#define CHECK(cond, msg) do { \
    if (cond) { g_passed++; } \
    else { g_failed++; std::printf("[ FAIL ] %s (line %d)\n", msg, __LINE__); } \
} while(0)

#define CHECK_NEAR(a, b, eps, msg) do { \
    double _a = (a), _b = (b), _e = (eps); \
    if (std::fabs(_a - _b) <= _e) { g_passed++; } \
    else { g_failed++; std::printf("FAIL: %s: got %.15g, expected %.15g (line %d)\n", msg, _a, _b, __LINE__); } \
} while(0)

// ============================================================================
// Helper: Get path to reference files
// ============================================================================
static std::string refPath(const char* filename) {
    return (std::filesystem::path(__FILE__).parent_path() / "reference" / filename).string();
}

static std::string comparisonScenePath(const char* filename) {
    return (std::filesystem::path(__FILE__).parent_path().parent_path() /
            "comparison/scenes" / filename).string();
}

static std::string tempPath(const char* filename) {
    std::filesystem::path dir = std::filesystem::temp_directory_path() / "mb3d_cpp_port_tests";
    std::filesystem::create_directories(dir);
    return (dir / filename).string();
}

static std::string customFnameString(const uint8_t fname[32]) {
    std::string s;
    for (int i = 0; i < 32 && fname[i] != 0; ++i) {
        s.push_back(static_cast<char>(fname[i]));
    }
    return s;
}

static int findFormulaSlotByName(const THeaderCustomAddon& addon, const char* name) {
    for (int i = 0; i < 6; ++i) {
        if (customFnameString(addon.Formulas[i].CustomFname) == name) {
            return i;
        }
    }
    return -1;
}

static double readDoubleAt(const void* pvar, int byteOffset) {
    double value = 0.0;
    std::memcpy(&value, static_cast<const uint8_t*>(pvar) + byteOffset, sizeof(value));
    return value;
}

static int32_t readInt32At(const void* pvar, int byteOffset) {
    int32_t value = 0;
    std::memcpy(&value, static_cast<const uint8_t*>(pvar) + byteOffset, sizeof(value));
    return value;
}

static uint64_t readUint64At(const void* pvar, int byteOffset) {
    uint64_t value = 0;
    std::memcpy(&value, static_cast<const uint8_t*>(pvar) + byteOffset, sizeof(value));
    return value;
}

static void otrapHybridSlotTight(double& x, double& y, double& z, double& w, void*) {
    x = 1.0;
    y = 1.0;
    z = 0.0;
    w = 0.0;
}

static void otrapHybridSlotWide(double& x, double& y, double& z, double& w, void*) {
    x = 0.1;
    y = 1.0;
    z = 0.0;
    w = 0.0;
}

static int g_otrapGradientProbeCalls = 0;

static void otrapGradientProbe(TPIteration3D pit) {
    ++g_otrapGradientProbeCalls;
    pit->Rout = pit->C1 * pit->C1 + pit->C2 * pit->C2 + pit->C3 * pit->C3 + 1.0;
    pit->SmoothItD = static_cast<float>(100 + g_otrapGradientProbeCalls);
}

struct ExternalFormulaRunState {
    double sse01[4]{};
    TLMSfunction directMap = nullptr;
    TLMSfunction sphereMap = nullptr;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double w = 0.0;
    double cx = 0.0;
    double cy = 0.0;
    double cz = 0.0;
    double cw = 0.0;
    // Persistent sample position C1..3 is distinct from Julia J1..3.
    double px = std::numeric_limits<double>::quiet_NaN();
    double py = std::numeric_limits<double>::quiet_NaN();
    double pz = std::numeric_limits<double>::quiet_NaN();
    double varyScale = 0.0;
    double rStopD = 0.0;
    double rOld = 0.0;
    double rout = 0.0;
    double dFree1 = 0.0;
    double dFree2 = 0.0;
    double deriv1 = 0.0;
    double deriv2 = 0.0;
    double deriv3 = 0.0;
    int itResultI = 0;
    int bFirstIt = 0;
};

static ExternalFormulaRunState runExternalFormulaByName(const char* formulaName,
                                                        const std::vector<double>& optionValues,
                                                        ExternalFormulaRunState state,
                                                        bool explicitSamplePosition = false) {
    mb3d::InitExternalFormulaRegistry();
    const auto* info = mb3d::FindExternalFormula(formulaName);
    if (!info || !info->wrapperFunc) {
        g_failed++;
        std::printf("FAIL: external formula helper missing wrapper for %s\n", formulaName);
        return state;
    }

    THeaderCustomAddon addon{};
    THAformula& formula = addon.Formulas[0];
    formula.iFnr = 20;
    std::memset(formula.CustomFname, 0, sizeof(formula.CustomFname));
    std::memcpy(formula.CustomFname, formulaName,
                std::min<size_t>(std::strlen(formulaName), sizeof(formula.CustomFname) - 1));
    for (size_t i = 0; i < optionValues.size() && i < 16; ++i) {
        formula.dOptionValue[i] = optionValues[i];
    }

    formula.iOptionCount = static_cast<int>(std::min<size_t>(optionValues.size(), 16));
    mb3d::ApplyExternalFormulaOptionTypes(formula);
    mb3d::BuildFormulaConstBuffer(0, &addon, 0);

    TIteration3Dext ext{};
    ext.x = state.x;
    ext.y = state.y;
    ext.z = state.z;
    ext.w = state.w;
    ext.C1 = !explicitSamplePosition && std::isnan(state.px) ? state.cx : state.px;
    ext.C2 = !explicitSamplePosition && std::isnan(state.py) ? state.cy : state.py;
    ext.C3 = !explicitSamplePosition && std::isnan(state.pz) ? state.cz : state.pz;
    ext.J1 = state.cx;
    ext.J2 = state.cy;
    ext.J3 = state.cz;
    ext.J4 = state.cw;
    ext.PVar = mb3d::GetFormulaConstBuffers().pConstPointer[0];
    ext.ItResultI = state.itResultI;
    ext.VaryScale = state.varyScale;
    ext.RStopD = state.rStopD;
    ext.Rold = state.rOld;
    ext.PMapFunc2 = state.directMap;
    ext.PMapFunc = state.sphereMap;
    ext.Rout = state.rout;
    ext.Dfree1 = state.dFree1;
    ext.Dfree2 = state.dFree2;
    ext.Deriv1 = state.deriv1;
    ext.Deriv2 = state.deriv2;
    ext.Deriv3 = state.deriv3;
    ext.bFirstIt = state.bFirstIt;
    std::memcpy(ext.FormulaSse01, state.sse01, sizeof(state.sse01));
    ext.fHPVar[0] = ext.PVar;

    auto* pit = reinterpret_cast<TPIteration3D>(&ext.C1);
    info->wrapperFunc(ext.x, ext.y, ext.z, ext.w, pit);

    state.x = ext.x;
    state.y = ext.y;
    state.z = ext.z;
    state.w = ext.w;
    state.cx = ext.J1;
    state.cy = ext.J2;
    state.cz = ext.J3;
    state.cw = ext.J4;
    state.px = ext.C1;
    state.py = ext.C2;
    state.pz = ext.C3;
    state.rout = ext.Rout;
    state.varyScale = ext.VaryScale;
    state.rStopD = ext.RStopD;
    state.rOld = ext.Rold;
    state.dFree1 = ext.Dfree1;
    state.dFree2 = ext.Dfree2;
    state.deriv1 = ext.Deriv1;
    state.deriv2 = ext.Deriv2;
    state.deriv3 = ext.Deriv3;
    state.itResultI = ext.ItResultI;
    state.bFirstIt = ext.bFirstIt;
    std::memcpy(state.sse01, ext.FormulaSse01, sizeof(state.sse01));
    return state;
}

// Helper: count non-black pixels in RGB image
static int countNonBlackPixels(const uint8_t* pixels, int width, int height) {
    int count = 0;
    for (int i = 0; i < width * height; i++) {
        if (pixels[i * 3 + 0] != 0 || pixels[i * 3 + 1] != 0 || pixels[i * 3 + 2] != 0) {
            count++;
        }
    }
    return count;
}

// Helper: compute sum of all pixel values
static uint64_t pixelSum(const uint8_t* pixels, int width, int height) {
    uint64_t sum = 0;
    for (int i = 0; i < width * height * 3; i++) {
        sum += pixels[i];
    }
    return sum;
}

static std::vector<uint8_t> readFileBytes(const std::string& path) {
    std::vector<uint8_t> data;
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return data;
    std::fseek(f, 0, SEEK_END);
    long sz = std::ftell(f);
    std::rewind(f);
    if (sz > 0) {
        data.resize(static_cast<size_t>(sz));
        if (std::fread(data.data(), 1, static_cast<size_t>(sz), f) != static_cast<size_t>(sz)) {
            data.clear();
        }
    }
    std::fclose(f);
    return data;
}

static void test_final_rgb_surface() {
    std::printf("  [final_rgb_surface]\n");

    {
        // SdoAA path: verify dimensions/success, not exact edge colors.
        std::vector<uint8_t> src(4 * 4 * 3, 0);
        auto setPixel = [&](int x, int y, uint8_t r, uint8_t g, uint8_t b) {
            size_t i = (static_cast<size_t>(y) * 4 + x) * 3;
            src[i + 0] = r;
            src[i + 1] = g;
            src[i + 2] = b;
        };
        for (int y = 0; y < 2; ++y) for (int x = 0; x < 2; ++x) setPixel(x, y, 255, 0, 0);
        for (int y = 0; y < 2; ++y) for (int x = 2; x < 4; ++x) setPixel(x, y, 0, 255, 0);
        for (int y = 2; y < 4; ++y) for (int x = 0; x < 2; ++x) setPixel(x, y, 0, 0, 255);
        for (int y = 2; y < 4; ++y) for (int x = 2; x < 4; ++x) setPixel(x, y, 255, 255, 0);

        mb3d::FinalRGBSurface out;
        CHECK(mb3d::BuildFinalRGBSurface(src.data(), 4, 4, 2, 0, 0, out),
              "BuildFinalRGBSurface scale2 succeeds");
        CHECK(out.width == 2 && out.height == 2, "BuildFinalRGBSurface scale2 dimensions");
        CHECK(out.pixels.size() == 12, "BuildFinalRGBSurface scale2 pixel count");
    }

    {
        // Generic scale>3 path should be exact box-average.
        std::vector<uint8_t> src(8 * 8 * 3, 0);
        auto setPixel = [&](int x, int y, uint8_t r, uint8_t g, uint8_t b) {
            size_t i = (static_cast<size_t>(y) * 8 + x) * 3;
            src[i + 0] = r;
            src[i + 1] = g;
            src[i + 2] = b;
        };
        for (int y = 0; y < 4; ++y) for (int x = 0; x < 4; ++x) setPixel(x, y, 255, 0, 0);
        for (int y = 0; y < 4; ++y) for (int x = 4; x < 8; ++x) setPixel(x, y, 0, 255, 0);
        for (int y = 4; y < 8; ++y) for (int x = 0; x < 4; ++x) setPixel(x, y, 0, 0, 255);
        for (int y = 4; y < 8; ++y) for (int x = 4; x < 8; ++x) setPixel(x, y, 255, 255, 0);

        mb3d::FinalRGBSurface out;
        CHECK(mb3d::BuildFinalRGBSurface(src.data(), 8, 8, 4, 0, 0, out),
              "BuildFinalRGBSurface scale4 succeeds");
        CHECK(out.width == 2 && out.height == 2, "BuildFinalRGBSurface scale4 dimensions");
        CHECK(out.pixels.size() == 12, "BuildFinalRGBSurface scale4 pixel count");
        if (out.pixels.size() == 12) {
            CHECK(out.pixels[0] == 255 && out.pixels[1] == 0 && out.pixels[2] == 0,
                  "scale4 top-left block preserved");
            CHECK(out.pixels[3] == 0 && out.pixels[4] == 255 && out.pixels[5] == 0,
                  "scale4 top-right block preserved");
            CHECK(out.pixels[6] == 0 && out.pixels[7] == 0 && out.pixels[8] == 255,
                  "scale4 bottom-left block preserved");
            CHECK(out.pixels[9] == 255 && out.pixels[10] == 255 && out.pixels[11] == 0,
                  "scale4 bottom-right block preserved");
        }
    }

    {
        // 4x4 direct crop with scale=1 should remove a 1-pixel border.
        std::vector<uint8_t> src(4 * 4 * 3, 0);
        auto setPixel = [&](int x, int y, uint8_t r, uint8_t g, uint8_t b) {
            size_t i = (static_cast<size_t>(y) * 4 + x) * 3;
            src[i + 0] = r;
            src[i + 1] = g;
            src[i + 2] = b;
        };
        setPixel(1, 1, 10, 20, 30);
        setPixel(2, 1, 40, 50, 60);
        setPixel(1, 2, 70, 80, 90);
        setPixel(2, 2, 100, 110, 120);

        mb3d::FinalRGBSurface out;
        CHECK(mb3d::BuildFinalRGBSurface(src.data(), 4, 4, 1, 0, 1, out),
              "BuildFinalRGBSurface crop succeeds");
        CHECK(out.width == 2 && out.height == 2, "BuildFinalRGBSurface crop dimensions");
        CHECK(out.pixels.size() == 12, "BuildFinalRGBSurface crop pixel count");
        if (out.pixels.size() == 12) {
            CHECK(out.pixels[0] == 10 && out.pixels[1] == 20 && out.pixels[2] == 30,
                  "crop top-left preserved");
            CHECK(out.pixels[3] == 40 && out.pixels[4] == 50 && out.pixels[5] == 60,
                  "crop top-right preserved");
            CHECK(out.pixels[6] == 70 && out.pixels[7] == 80 && out.pixels[8] == 90,
                  "crop bottom-left preserved");
            CHECK(out.pixels[9] == 100 && out.pixels[10] == 110 && out.pixels[11] == 120,
                  "crop bottom-right preserved");
        }
    }
}

// Helper: render a reference file at small size with given options
static mb3d::RenderResult renderRef(const char* filename, int size = 64,
                                     bool no_shadows = true, bool no_ao = true,
                                     bool no_dof = true, bool no_reflections = true,
                                     int threads = 1) {
    auto loadResult = mb3d::LoadM3PFile(refPath(filename));
    mb3d::RenderResult empty;
    empty.success = false;
    if (!loadResult.success) {
        empty.error = "Load failed: " + loadResult.error;
        return empty;
    }

    mb3d::RenderOptions opts;

    opts.precision = mb3d::PrecisionMode::Compatible;
    opts.width_override = size;
    opts.height_override = size;
    opts.thread_count = threads;
    opts.no_shadows = no_shadows;
    opts.no_ao = no_ao;
    opts.no_dof = no_dof;
    opts.no_reflections = no_reflections;
    opts.verbose = false;

    return mb3d::Render(loadResult.header, loadResult.addon, opts);
}

static mb3d::RenderResult renderScene(const std::string& scenePath,
                                      int size = 64,
                                      bool no_shadows = true, bool no_ao = true,
                                      bool no_dof = true, bool no_reflections = true,
                                      int threads = 1) {
    auto loadResult = mb3d::LoadM3PFile(scenePath);
    mb3d::RenderResult empty;
    empty.success = false;
    if (!loadResult.success) {
        empty.error = "Load failed: " + loadResult.error;
        return empty;
    }

    mb3d::RenderOptions opts;

    opts.precision = mb3d::PrecisionMode::Compatible;
    opts.width_override = size;
    opts.height_override = size;
    opts.thread_count = threads;
    opts.no_shadows = no_shadows;
    opts.no_ao = no_ao;
    opts.no_dof = no_dof;
    opts.no_reflections = no_reflections;
    opts.verbose = false;

    return mb3d::Render(loadResult.header, loadResult.addon, opts);
}

static void test_getmct_destop_factor_regression() {
    std::printf("  [getmct_destop_factor_regression]\n");

    auto loadResult = mb3d::LoadM3PFile(refPath("formula_2_Quaternion.m3p"));
    CHECK(loadResult.success, "Quaternion load for DEstop factor test");
    if (!loadResult.success) return;

    auto mct = mb3d::GetMCTparasFromHeader(loadResult.header, &loadResult.addon, 0);
    CHECK(mct.bMCTisValid != 0, "Quaternion MCT valid");
    CHECK(mct.bVaryDEstop != 0, "Quaternion vary DEstop enabled");
    if (mct.bMCTisValid == 0) return;

    double expected = mb3d::GetDEstopFactor(&loadResult.header);
    CHECK(std::isfinite(expected) && expected > 0.0, "Quaternion expected DEstop factor is finite and positive");
    CHECK(std::fabs(static_cast<double>(mct.mctDEstopFactor) - expected) < 1e-7,
          "Quaternion mctDEstopFactor follows GetDEstopFactor");
}

// ============================================================================
// Test: Each formula renders a non-black image at 64x64
// ============================================================================
static void test_formulas_render_nonblack() {
    std::printf("  [formulas_render_nonblack]\n");

    struct FormulaTest {
        const char* file;
        const char* name;
    };

    FormulaTest tests[] = {
        {"formula_0_IntPow2.m3p", "IntPow2"},
        {"formula_0_IntPow3.m3p", "IntPow3"},
        {"formula_0_IntPow4.m3p", "IntPow4"},
        {"formula_0_IntPow5.m3p", "IntPow5"},
        {"formula_0_IntPow6.m3p", "IntPow6"},
        {"formula_0_IntPow7.m3p", "IntPow7"},
        {"formula_0_IntPow8.m3p", "IntPow8"},
        {"formula_1_RealPower.m3p", "RealPower"},
        {"formula_2_Quaternion.m3p", "Quaternion"},
        {"formula_3_Tricorn.m3p", "Tricorn"},
        {"formula_4_AmazingBox.m3p", "AmazingBox"},
        {"formula_5_Bulbox.m3p", "Bulbox"},
        {"formula_6_FoldIntPow.m3p", "FoldIntPow"},
        {"formula_9_AexionC.m3p", "AexionC"},
    };

    for (const auto& t : tests) {
        auto result = renderRef(t.file, 64);

        std::string msg = std::string(t.name) + " renders successfully";
        CHECK(result.success, msg.c_str());
        if (!result.success) {
            std::printf("    error: %s\n", result.error.c_str());
            continue;
        }

        msg = std::string(t.name) + " produces non-black image";
        int nonBlack = countNonBlackPixels(result.image.data(), result.width, result.height);
        CHECK(nonBlack > 0, msg.c_str());
        if (nonBlack == 0) {
            std::printf("    %s: all pixels black\n", t.name);
        }
    }
}

// ============================================================================
// Test: Render with and without shadows produces different output
// ============================================================================
static void test_shadows_affect_output() {
    std::printf("  [shadows_affect_output]\n");

    std::string scene = comparisonScenePath("test_11_shadows.m3p");
    auto loadResult = mb3d::LoadM3PFile(scene);
    CHECK(loadResult.success, "test_11_shadows load for shadow test");
    if (!loadResult.success) return;

    bool hasShadows = (loadResult.header.bCalculateHardShadow & 0xFC) != 0;
    CHECK(hasShadows, "test_11_shadows scene has hard-shadow lights enabled");
    if (!hasShadows) return;

    auto withShadows = renderScene(scene, 64, false, true, true, true);
    CHECK(withShadows.success, "render with shadows");
    if (!withShadows.success) return;

    auto noShadows = renderScene(scene, 64, true, true, true, true);
    CHECK(noShadows.success, "render without shadows");
    if (!noShadows.success) return;

    auto countHardShadowBits = [](const mb3d::RenderResult& r) {
        int count = 0;
        for (const auto& px : r.silight) {
            if ((px.Shadow & 0xFC00u) != 0) count++;
        }
        return count;
    };

    int withShadowBits = countHardShadowBits(withShadows);
    int withoutShadowBits = countHardShadowBits(noShadows);
    CHECK(withShadowBits > 0, "shadow-enabled render sets hard-shadow bits");
    CHECK(withoutShadowBits == 0, "shadow-disabled render clears hard-shadow bits");
    CHECK(withShadowBits != withoutShadowBits, "shadow-enabled and shadow-disabled siLight differ");
}

static void test_scene_native_imagescale_header() {
    std::printf("  [scene_native_imagescale_header]\n");

    auto loadResult = mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    CHECK(loadResult.success, "load scene-native ImageScale test scene");
    if (!loadResult.success) return;

    CHECK(loadResult.header.Width == 2000, "scene-native ImageScale header width preserved");
    CHECK(loadResult.header.Height == 2000, "scene-native ImageScale header height preserved");
    CHECK(loadResult.header.bImageScale == 4, "scene-native ImageScale factor preserved");
}

static void test_m3i_load_path() {
    std::printf("  [m3i_load_path]\n");

    auto loadResult = mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    CHECK(loadResult.success, "load source scene for m3i test");
    if (!loadResult.success) return;

    TMandHeader10 header = loadResult.header;
    header.Width = 4;
    header.Height = 3;
    header.TilingOptions = 0;
    header.MandId = 44;
    std::memset(header.PHCustomF, 0, sizeof(header.PHCustomF));
    header.PCFAddon = 0;

    THeaderCustomAddon addon = loadResult.addon;
    addon.bHCAversion = 16;

    std::vector<TsiLight5> siLight(static_cast<size_t>(header.Width) * header.Height);
    for (size_t i = 0; i < siLight.size(); ++i) {
        siLight[i].NormalX = static_cast<int16_t>(100 + i);
        siLight[i].NormalY = static_cast<int16_t>(-50 - static_cast<int>(i));
        siLight[i].NormalZ = static_cast<int16_t>(200 - static_cast<int>(i));
        siLight[i].RoughZposFine = static_cast<uint16_t>(1000 + i);
        siLight[i].Zpos = static_cast<uint16_t>(2000 + i);
        siLight[i].Shadow = static_cast<uint16_t>(300 + i);
        siLight[i].AmbShadow = static_cast<uint16_t>(400 + i);
        siLight[i].SIgradient = static_cast<uint16_t>(500 + i);
        siLight[i].OTrap = static_cast<uint16_t>(600 + i);
    }

    std::vector<uint32_t> image(siLight.size());
    for (size_t i = 0; i < image.size(); ++i) {
        image[i] = 0xFF000000u | (static_cast<uint32_t>(i) & 0xFFu);
    }

    std::string tmpPath = tempPath("mb3d_test_cache.m3i");
    FILE* f = std::fopen(tmpPath.c_str(), "wb");
    CHECK(f != nullptr, "m3i temp file opened");
    if (!f) return;

    std::fwrite(&header, 1, sizeof(header), f);
    std::fwrite(siLight.data(), sizeof(TsiLight5), siLight.size(), f);
    std::fwrite(&addon, 1, sizeof(addon), f);
    std::fwrite(image.data(), sizeof(uint32_t), image.size(), f);
    std::fclose(f);

    auto m3i = mb3d::LoadM3IFile(tmpPath);
    CHECK(m3i.success, "LoadM3IFile succeeds");
    if (m3i.success) {
        CHECK(m3i.has_silight, "LoadM3IFile exposes siLight");
        CHECK(m3i.has_image, "LoadM3IFile exposes image");
        CHECK(m3i.siLight.size() == siLight.size(), "LoadM3IFile siLight size matches");
        CHECK(m3i.image.size() == image.size(), "LoadM3IFile image size matches");
        if (m3i.siLight.size() == siLight.size()) {
            CHECK(m3i.siLight[0].Zpos == siLight[0].Zpos, "LoadM3IFile keeps first siLight pixel");
            CHECK(m3i.siLight.back().OTrap == siLight.back().OTrap, "LoadM3IFile keeps last siLight pixel");
        }
        if (m3i.image.size() == image.size()) {
            CHECK(m3i.image[0] == image[0], "LoadM3IFile keeps first image pixel");
            CHECK(m3i.image.back() == image.back(), "LoadM3IFile keeps last image pixel");
        }
        CHECK(m3i.params.header.Width == 4 && m3i.params.header.Height == 3,
              "LoadM3IFile preserves header dimensions");
    }

    std::remove(tmpPath.c_str());
}

static void test_external_formula_registry() {
    std::printf("  [external_formula_registry]\n");

    mb3d::InitExternalFormulaRegistry();

    const auto* info = mb3d::FindExternalFormula("psphereifs");
    CHECK(info != nullptr, "external formula lookup is case-insensitive");
    if (!info) return;

    CHECK(info->DEoption == 20, "PSphereIFS DEoption matches registry");
    CHECK(info->wrapperFunc != nullptr, "PSphereIFS wrapper function exists");
    CHECK(info->paramCount == 13, "PSphereIFS param count matches registry");

    THAformula formula{};
    formula.iFnr = 20;
    std::memcpy(formula.CustomFname, "PSphereIFS", 10);
    mb3d::ApplyExternalFormulaOptionTypes(formula);
    CHECK(formula.iOptionCount == 11, "ApplyExternalFormulaOptionTypes expands PSphereIFS option count");
    CHECK(formula.byOptionType[1] == 14, "PSphereIFS option type 1 preserved");
    CHECK(formula.byOptionType[6] == 2, "PSphereIFS option type 6 preserved");
}

static void test_external_legacy_option_defaults() {
    mb3d::InitExternalFormulaRegistry();
    THAformula formula{};
    formula.iFnr = 20;
    std::memcpy(formula.CustomFname, "_Rotate", 8);
    formula.iOptionCount = 1;
    formula.dOptionValue[0] = 45;
    formula.dOptionValue[1] = 30; // stale payload beyond the saved count
    formula.dOptionValue[2] = 90;
    mb3d::ApplyExternalFormulaOptionTypes(formula);
    CHECK(formula.iOptionCount == 3, "legacy Rotate expands three angles");
    CHECK(formula.dOptionValue[0] == 45 && formula.dOptionValue[1] == 0 &&
          formula.dOptionValue[2] == 0, "legacy Rotate fills missing angles from Pascal defaults");
    formula.dOptionValue[1] = 30;
    mb3d::ApplyExternalFormulaOptionTypes(formula);
    CHECK(formula.dOptionValue[1] == 30, "current Rotate preserves explicit angles");
    std::memset(&formula, 0, sizeof(formula));
    formula.iFnr = 20;
    std::memcpy(formula.CustomFname, "ABoxMod1", 9);
    mb3d::ApplyExternalFormulaOptionTypes(formula);
    CHECK(formula.dOptionValue[0] == 2 && formula.dOptionValue[2] == 2,
          "missing ABoxMod1 options use nonzero defaults");
}

#if (defined(_MSC_VER) && defined(_M_X64)) || defined(MB3D_SOFTWARE_EXT80)
#include "fixtures/formula_angle_regressions.h"
static void test_original_formula_angle_loader() {
#if defined(_MSC_VER) && defined(_M_X64)
   FixtureX87Environment environment;
#else
   mb3d::numeric::ExtendedScope environment;
#endif
   for (const auto& row : formulaAngleFixtures) {
       THeaderCustomAddon addon{};
       auto& f=addon.Formulas[0];
       f.iFnr=20; f.iOptionCount=1;
       f.byOptionType[0]=static_cast<uint8_t>(row.type);
       f.dOptionValue[0]=row.angle;
       mb3d::BuildFormulaConstBuffer(0,&addon,0);
       const auto* pvar=static_cast<const unsigned char*>(mb3d::GetFormulaConstBuffers().pConstPointer[0]);
       const size_t size=row.type==3 ? 16 : 8;
       CHECK(std::memcmp(pvar-8-size,row.bytes+16-size,size)==0,
             "original Pascal angle loader bytes, including range boundaries");
   }
}
#endif

static void test_formula_const_buffer_layouts() {
    std::printf("  [formula_const_buffer_layouts]\n");

    {
        auto loadResult = mb3d::LoadM3PFile(comparisonScenePath("test_09_foldintpow.m3p"));
        CHECK(loadResult.success, "foldintpow load success for const-buffer test");
        if (!loadResult.success) return;

        mb3d::BuildFormulaConstBuffer(0, &loadResult.addon, 0);
        const void* pvar = mb3d::GetFormulaConstBuffers().pConstPointer[0];
        CHECK(pvar != nullptr, "foldintpow const-buffer pointer exists");
        if (!pvar) return;

        CHECK_NEAR(readDoubleAt(pvar, -8), 0.5, 1e-12, "foldintpow keeps generic 0.5 center");
        CHECK_NEAR(readDoubleAt(pvar, -16), loadResult.addon.Formulas[0].dOptionValue[1], 1e-12,
                   "foldintpow dZmul stored at PVar[-2]");
        CHECK_NEAR(readDoubleAt(pvar, -24), loadResult.addon.Formulas[0].dOptionValue[2], 1e-12,
                   "foldintpow fold stored at PVar[-3]");
        CHECK_NEAR(readDoubleAt(pvar, -32), 2.0 * loadResult.addon.Formulas[0].dOptionValue[2], 1e-12,
                   "foldintpow 2*fold stored at PVar[-4]");
        CHECK_NEAR(readDoubleAt(pvar, 120), 3.0, 1e-12, "positive const slot +120 keeps Pascal constant 3.0");

        void* func = nullptr;
        std::memcpy(&func, static_cast<const uint8_t*>(pvar) - 56, sizeof(func));
        CHECK(func != nullptr, "foldintpow stores sub-formula function pointer in negative PVar tail");
    }

    {
        auto loadResult = mb3d::LoadM3PFile(comparisonScenePath("test_10_aexionc.m3p"));
        CHECK(loadResult.success, "aexionc load success for const-buffer test");
        if (!loadResult.success) return;

        mb3d::BuildFormulaConstBuffer(0, &loadResult.addon, 0);
        const void* pvar = mb3d::GetFormulaConstBuffers().pConstPointer[0];
        CHECK(pvar != nullptr, "aexionc const-buffer pointer exists");
        if (!pvar) return;

        const THAformula& formula = loadResult.addon.Formulas[0];
        CHECK_NEAR(readDoubleAt(pvar, -8), 0.5, 1e-12, "aexionc keeps generic 0.5 center");
        CHECK_NEAR(readDoubleAt(pvar, -16), formula.dOptionValue[0], 1e-12,
                   "aexionc power stored at PVar[-2]");
        CHECK_NEAR(readDoubleAt(pvar, -24), formula.dOptionValue[1], 1e-12,
                   "aexionc dZmul stored at PVar[-3]");
        CHECK(readInt32At(pvar, -28) == static_cast<int32_t>(std::round(formula.dOptionValue[2])),
              "aexionc rotate-c flag stored as int32 at PVar[-28]");
        CHECK(readInt32At(pvar, -32) == static_cast<int32_t>(std::round(formula.dOptionValue[3])),
              "aexionc cond-phi flag stored as int32 at PVar[-32]");
        CHECK_NEAR(readDoubleAt(pvar, -40), formula.dOptionValue[4], 1e-12,
                   "aexionc power-c stored at PVar[-40]");
        CHECK_NEAR(readDoubleAt(pvar, -48), formula.dOptionValue[5], 1e-12,
                   "aexionc Czmul stored at PVar[-48]");
        CHECK(readInt32At(pvar, -52) == static_cast<int32_t>(std::round(formula.dOptionValue[6])),
              "aexionc powc-dist flag stored as int32 at PVar[-52]");
        CHECK(readInt32At(pvar, -56) == static_cast<int32_t>(std::round(formula.dOptionValue[7])),
              "aexionc mode bits stored as int32 at PVar[-56]");
    }

    {
        auto loadResult = mb3d::LoadM3PFile(comparisonScenePath("extall_dIFS_HeightMap2IFS.m3p"));
        CHECK(loadResult.success, "HeightMap2IFS load success for const-buffer test");
        if (!loadResult.success) return;

        int slot = findFormulaSlotByName(loadResult.addon, "HeightMap2IFS");
        CHECK(slot >= 0, "HeightMap2IFS formula slot found");
        if (slot < 0) return;

        mb3d::BuildFormulaConstBuffer(slot, &loadResult.addon, slot);
        const void* pvar = mb3d::GetFormulaConstBuffers().pConstPointer[slot];
        CHECK(pvar != nullptr, "HeightMap2IFS const-buffer pointer exists");
        if (!pvar) return;

        const auto* info = mb3d::FindExternalFormula("HeightMap2IFS");
        CHECK(info != nullptr, "HeightMap2IFS registry info exists");
        if (!info) return;

        CHECK(info->constantCount > 0, "HeightMap2IFS exposes explicit positive constants");
        for (int i = 0; i < info->constantCount; ++i) {
            CHECK(readUint64At(pvar, i * 8) == info->constants[i].rawBits,
                  "HeightMap2IFS positive constant copied into PVar buffer");
        }
        CHECK_NEAR(readDoubleAt(pvar, -8), 0.5, 1e-12,
                   "HeightMap2IFS still keeps generic 0.5 center after constant copy");
    }
}

static void test_external_formula_chain_semantics() {
    std::printf("  [external_formula_chain_semantics]\n");

    {
        ExternalFormulaRunState state;
        state.x = 1.0;
        state.y = 2.0;
        state.z = 3.0;
        state.cx = 10.0;
        state.cy = 20.0;
        state.cz = 30.0;

        state = runExternalFormulaByName("_Transform",
                                         {2.0, 1.0, -2.0, 0.5, 0.0, 0.0, 0.0, 1.0},
                                         state);
        CHECK_NEAR(state.x, 4.0, 1e-12, "_Transform updates x for chain case");
        CHECK_NEAR(state.y, 0.0, 1e-12, "_Transform updates y for chain case");
        CHECK_NEAR(state.z, 7.0, 1e-12, "_Transform updates z for chain case");
        CHECK_NEAR(state.cx, 4.0, 1e-12, "_Transform copies transformed x into C for downstream slot");
        CHECK_NEAR(state.cy, 0.0, 1e-12, "_Transform copies transformed y into C for downstream slot");
        CHECK_NEAR(state.cz, 7.0, 1e-12, "_Transform copies transformed z into C for downstream slot");

        state = runExternalFormulaByName("_juliax2",
                                         {0.0, 100.0, 101.0, 102.0, -100.0, -101.0, -102.0},
                                         state);
        // Original _juliax2 x86 code preserves XYZ at separation zero and selects right Julia seeds.
        CHECK_NEAR(state.x, 4.0, 1e-12, "_juliax2 preserves transformed x at zero separation");
        CHECK_NEAR(state.y, 0.0, 1e-12, "_Transform -> _juliax2 uses upstream copied Cy");
        CHECK_NEAR(state.z, 7.0, 1e-12, "_juliax2 preserves transformed z");
        CHECK_NEAR(state.cx, -100.0, 1e-12, "_juliax2 selects right Julia x seed");
        CHECK_NEAR(state.cy, -101.0, 1e-12, "_juliax2 selects right Julia y seed");
        CHECK_NEAR(state.cz, -102.0, 1e-12, "_juliax2 selects right Julia z seed");
    }

    {
        ExternalFormulaRunState state;
        state.x = 1.0;
        state.y = 2.0;
        state.z = 3.0;
        state.cx = 10.0;
        state.cy = 20.0;
        state.cz = 30.0;

        state = runExternalFormulaByName("_FlipXYc", {1.0}, state);
        CHECK_NEAR(state.x, 2.0, 1e-12, "_FlipXYc swaps x into downstream chain");
        CHECK_NEAR(state.y, 1.0, 1e-12, "_FlipXYc swaps y into downstream chain");
        CHECK_NEAR(state.z, 3.0, 1e-12, "_FlipXYc preserves z into downstream chain");
        CHECK_NEAR(state.cx, 20.0, 1e-12, "_FlipXYc swaps Cx for downstream slot");
        CHECK_NEAR(state.cy, 10.0, 1e-12, "_FlipXYc swaps Cy for downstream slot");
        CHECK_NEAR(state.cz, 30.0, 1e-12, "_FlipXYc preserves Cz for downstream slot");

        state = runExternalFormulaByName("_flipC-xyz", {}, state);
        CHECK_NEAR(state.x, 20.0, 1e-12, "_FlipXYc -> _flipC-xyz consumes swapped Cx");
        CHECK_NEAR(state.y, 10.0, 1e-12, "_FlipXYc -> _flipC-xyz consumes swapped Cy");
        CHECK_NEAR(state.z, 30.0, 1e-12, "_FlipXYc -> _flipC-xyz consumes swapped Cz");
        CHECK_NEAR(state.cx, 2.0, 1e-12, "_flipC-xyz writes previous x back into Cx");
        CHECK_NEAR(state.cy, 1.0, 1e-12, "_flipC-xyz writes previous y back into Cy");
        CHECK_NEAR(state.cz, 3.0, 1e-12, "_flipC-xyz writes previous z back into Cz");
    }
}

static void test_flipxyc_scene_schema() {
    std::printf("  [flipxyc_scene_schema]\n");

    auto loadResult = mb3d::LoadM3PFile(comparisonScenePath("extall_Ads__FlipXYc.m3p"));
    CHECK(loadResult.success, "load extall_Ads__FlipXYc scene for schema test");
    if (!loadResult.success) return;

    auto mct = mb3d::GetMCTparasFromHeader(loadResult.header, &loadResult.addon, 0);

    LOG_IF_V("    iFCount=%d bHybOpt1=%d bHybOpt2=%d bOptions1=%d bOptions2=%d bOptions3=%d\n",
                static_cast<int>(loadResult.addon.iFCount),
                static_cast<int>(loadResult.addon.bHybOpt1),
                static_cast<int>(loadResult.addon.bHybOpt2),
                static_cast<int>(loadResult.addon.bOptions1),
                static_cast<int>(loadResult.addon.bOptions2),
                static_cast<int>(loadResult.addon.bOptions3));
    std::printf("    normalized wEndTo=%d StartFrom2=%d iEnd2=%d RepeatFrom1=%d RepeatFrom2=%d bMCTisValid=%d\n",
                mct.wEndTo,
                mct.StartFrom2,
                mct.iEnd2,
                mct.RepeatFrom1,
                mct.RepeatFrom2,
                mct.bMCTisValid);
    std::printf("    runtime RepeatFrom1=%d StartFrom2=%d RepeatFrom2=%d nHybrid=[%d,%d,%d,%d,%d,%d]\n",
                mct.RepeatFrom1,
                mct.StartFrom2,
                mct.RepeatFrom2,
                mct.nHybrid[0],
                mct.nHybrid[1],
                mct.nHybrid[2],
                mct.nHybrid[3],
                mct.nHybrid[4],
                mct.nHybrid[5]);

    for (int slot = 0; slot < 6; ++slot) {
        const auto& f = loadResult.addon.Formulas[slot];
        const char* extName = reinterpret_cast<const char*>(f.CustomFname);
        const auto* extInfo = mb3d::FindExternalFormula(extName);
        LOG_IF_V("    slot=%d name=%s iFnr=%d iItCount=%d iOptionCount=%d opt0=%.12g\n",
                    slot,
                    customFnameString(f.CustomFname).c_str(),
                    static_cast<int>(f.iFnr),
                    static_cast<int>(f.iItCount),
                    static_cast<int>(f.iOptionCount),
                    f.dOptionValue[0]);
        if (f.iFnr >= 20) {
            std::printf("      registry lookup=%s deoption=%d\n",
                        extInfo ? extInfo->name : "<null>",
                        extInfo ? extInfo->DEoption : -999);
        }
    }

    CHECK(findFormulaSlotByName(loadResult.addon, "_FlipXYc") >= 0,
          "extall_Ads__FlipXYc scene contains _FlipXYc");
}

static bool traceHasSlot(const std::vector<std::string>& lines, int slot) {
    const std::string needle = "slot=" + std::to_string(slot);
    for (const auto& line : lines) {
        if (line.find(needle) != std::string::npos) {
            return true;
        }
    }
    return false;
}

static bool traceHasStageAndSlot(const std::vector<std::string>& lines, const char* stage, int slot) {
    const std::string stageNeedle = std::string("stage=") + stage;
    const std::string slotNeedle = "slot=" + std::to_string(slot);
    for (const auto& line : lines) {
        if (line.find(stageNeedle) != std::string::npos && line.find(slotNeedle) != std::string::npos) {
            return true;
        }
    }
    return false;
}

static void printFirstSlotLines(const std::vector<std::string>& lines, int slot, int maxLines) {
    if (!kVerbosePacketTrace) return;
    const std::string needle = "slot=" + std::to_string(slot);
    int printed = 0;
    for (const auto& line : lines) {
        if (line.find(needle) == std::string::npos) continue;
        std::printf("    %s\n", line.c_str());
        ++printed;
        if (printed >= maxLines) break;
    }
}

static std::string traceFieldValue(const std::string& line, const char* key) {
    std::string needle = std::string(key) + "=";
    std::size_t start = line.find(needle);
    if (start == std::string::npos) return std::string();
    start += needle.size();
    std::size_t end = line.find(' ', start);
    return line.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

struct FormulaContractProbe {
    bool found = false;
    int x = 0;
    int y = 0;
    std::string inLine;
    std::string outLine;
};

static FormulaContractProbe findFirstFormulaContractProbe(TMandHeader10 header,
                                                         THeaderCustomAddon addon,
                                                         const char* traceFileName,
                                                         const char* formulaToken) {
    auto tryProbe = [&](int x, int y) {
        FormulaContractProbe probe{};
        mb3d::RenderOptions opts{};
        opts.precision = mb3d::PrecisionMode::Compatible;
        opts.thread_count = 1;
        opts.deterministic_mode = true;
        opts.diag_only = true;
        opts.diag_points.push_back({x, y});
        opts.parity_trace_dump_path = tempPath(traceFileName);
        opts.parity_trace_start_stage = "formula_pre";
        opts.parity_trace_end_stage = "formula_post";

        auto render = mb3d::Render(header, addon, opts);
        if (!render.success) return probe;

        for (const auto& line : render.parity_trace_lines) {
            if (line.find("stage=formula_contract_in") != std::string::npos &&
                line.find(formulaToken) != std::string::npos) {
                probe.inLine = line;
            }
            if (line.find("stage=formula_contract_out") != std::string::npos &&
                line.find(formulaToken) != std::string::npos) {
                probe.outLine = line;
            }
        }
        if (!probe.inLine.empty() && !probe.outLine.empty()) {
            probe.found = true;
            probe.x = x;
            probe.y = y;
        }
        return probe;
    };

    FormulaContractProbe first = tryProbe(0, 0);
    if (first.found) return first;

    auto tryStep = [&](int step, int maxProbes) {
        FormulaContractProbe probe{};
        int probes = 0;
        for (int y = 0; y < header.Height && !probe.found; y += step) {
            for (int x = 0; x < header.Width && !probe.found; x += step) {
                probe = tryProbe(x, y);
                if (probe.found) {
                    return probe;
                }
                if (++probes >= maxProbes) return probe;
            }
        }
        return probe;
    };

    FormulaContractProbe coarse = tryStep(16, 256);
    if (coarse.found) return coarse;
    return tryStep(4, 512);
}

static void applyCondItcountSwitch(std::array<int, 7>& itcount,
                                   int& execCounter,
                                   int cVal,
                                   int formulaNr,
                                   int icLo,
                                   int icEq,
                                   int icHi) {
    ++execCounter;
    cVal = std::max(1, std::min(100, cVal));
    formulaNr = std::max(1, std::min(6, formulaNr));

    int val = 0;
    if (execCounter < cVal) {
        val = icLo;
    } else if (execCounter == cVal) {
        val = icEq;
    } else {
        val = icHi;
    }
    val = std::max(0, std::min(100, val));
    if (itcount[formulaNr] < 0) {
        val |= int(0x80000000);
    }
    itcount[formulaNr] = val;
}

static void test_flipxyc_runtime_trace_slot_presence() {
    std::printf("  [flipxyc_runtime_trace_slot_presence]\n");

    auto loadResult = mb3d::LoadM3PFile(comparisonScenePath("extall_Ads__FlipXYc.m3p"));
    CHECK(loadResult.success, "load extall_Ads__FlipXYc scene for runtime trace test");
    if (!loadResult.success) return;

    auto runTraceAt = [&](int x, int y) {
        mb3d::RenderOptions opts{};
        opts.precision = mb3d::PrecisionMode::Compatible;
        opts.thread_count = 1;
        opts.deterministic_mode = true;
        opts.diag_only = true;
        opts.diag_points.push_back({x, y});
        opts.parity_trace_dump_path = tempPath("flipxyc_runtime_trace.txt");
        opts.parity_trace_start_stage = "formula_pre";
        opts.parity_trace_end_stage = "hit_after_refine";
        return mb3d::Render(loadResult.header, loadResult.addon, opts);
    };

    auto hotspot = runTraceAt(148, 76);
    CHECK(hotspot.success, "FlipXYc hotspot render succeeds for runtime trace test");
    LOG_IF_V("    hotspot trace lines=%zu slot0=%d slot1=%d\n",
                hotspot.parity_trace_lines.size(),
                traceHasSlot(hotspot.parity_trace_lines, 0) ? 1 : 0,
                traceHasSlot(hotspot.parity_trace_lines, 1) ? 1 : 0);
    std::printf("    hotspot formula_pre slot0=%d slot1=%d\n",
                traceHasStageAndSlot(hotspot.parity_trace_lines, "formula_pre", 0) ? 1 : 0,
                traceHasStageAndSlot(hotspot.parity_trace_lines, "formula_pre", 1) ? 1 : 0);
    if (traceHasSlot(hotspot.parity_trace_lines, 1)) {
        printFirstSlotLines(hotspot.parity_trace_lines, 1, 8);
    }

    auto slot1Probe = runTraceAt(81, 33);
    CHECK(slot1Probe.success, "known FlipXYc slot1 probe render succeeds");
    if (!slot1Probe.success) return;
    std::printf("    known slot1 pixel=81,33 trace lines=%zu formula_pre slot1=%d\n",
                slot1Probe.parity_trace_lines.size(),
                traceHasStageAndSlot(slot1Probe.parity_trace_lines, "formula_pre", 1) ? 1 : 0);
    if (traceHasStageAndSlot(slot1Probe.parity_trace_lines, "formula_pre", 1)) {
        printFirstSlotLines(slot1Probe.parity_trace_lines, 1, 12);
    }
}

static void printTraceIfFailed(const std::vector<std::string>& lines, int prevFailed, const char* filter) {
    if (g_failed > prevFailed || filter) {
        for (const auto& line : lines) {
            std::printf("    %s\n", line.c_str());
        }
    }
}

static void test_flipxyc_c_writeback_contract() {
    std::printf("  [flipxyc_c_writeback_contract]\n");

    auto loadResult = mb3d::LoadM3PFile(comparisonScenePath("extall_Ads__FlipXYc.m3p"));
    CHECK(loadResult.success, "load extall_Ads__FlipXYc scene for C writeback contract test");
    if (!loadResult.success) return;

    int flipSlot = findFormulaSlotByName(loadResult.addon, "_FlipXYc");
    CHECK(flipSlot >= 0, "_FlipXYc slot found for C writeback contract test");
    if (flipSlot < 0) return;

    loadResult.addon.Formulas[flipSlot].dOptionValue[0] = 1.0; // Flip also C = enabled

    mb3d::RenderOptions opts{};

    opts.precision = mb3d::PrecisionMode::Compatible;
    opts.thread_count = 1;
    opts.deterministic_mode = true;
    opts.diag_only = true;
    opts.diag_points.push_back({81, 33});
    opts.parity_trace_dump_path = tempPath("flipxyc_c_writeback_contract_trace.txt");
    // A formula_pre -> formula_post window closes on slot 0, before the
    // FlipXYc callback in slot 1. Capture the full selected-pixel contract so
    // the assertions below still inspect the actual Julia-constant swap.

    auto traceResult = mb3d::Render(loadResult.header, loadResult.addon, opts);
    CHECK(traceResult.success, "targeted FlipXYc contract render succeeds");
    if (!traceResult.success) return;

    std::string inLine;
    std::string outLine;
    for (const auto& line : traceResult.parity_trace_lines) {
        if (line.find("stage=formula_contract_in") != std::string::npos &&
            line.find("slot=1") != std::string::npos &&
            line.find("formula=FORMULA__FLIPXYC") != std::string::npos) {
            inLine = line;
        }
        if (line.find("stage=formula_contract_out") != std::string::npos &&
            line.find("slot=1") != std::string::npos &&
            line.find("formula=FORMULA__FLIPXYC") != std::string::npos) {
            outLine = line;
        }
    }

    CHECK(!inLine.empty(), "targeted FlipXYc contract has formula_contract_in");
    CHECK(!outLine.empty(), "targeted FlipXYc contract has formula_contract_out");
    if (inLine.empty() || outLine.empty()) return;

    if (g_failed > 0 && g_verbose) {
        std::printf("    %s\n", inLine.c_str());
        std::printf("    %s\n", outLine.c_str());
    }

    auto fieldValue = [](const std::string& line, const char* key) {
        std::string needle = std::string(key) + "=";
        std::size_t start = line.find(needle);
        if (start == std::string::npos) return std::string();
        start += needle.size();
        std::size_t end = line.find(' ', start);
        return line.substr(start, end == std::string::npos ? std::string::npos : end - start);
    };

    const std::string inJ1 = fieldValue(inLine, "J1");
    const std::string inJ2 = fieldValue(inLine, "J2");
    const std::string outJ1 = fieldValue(outLine, "J1");
    const std::string outJ2 = fieldValue(outLine, "J2");

    CHECK(outJ1 == inJ2, "FlipXYc with Flip also C swaps J1 to prior J2");
    CHECK(outJ2 == inJ1, "FlipXYc with Flip also C swaps J2 to prior J1");
}

static void test_flipxyc_flipcxyz_chain_contract() {
    std::printf("  [flipxyc_flipcxyz_chain_contract]\n");

    auto loadResult = mb3d::LoadM3PFile(comparisonScenePath("extall_Ads__FlipXYc.m3p"));
    CHECK(loadResult.success, "load extall_Ads__FlipXYc scene for downstream chain contract test");
    if (!loadResult.success) return;

    int flipSlot = findFormulaSlotByName(loadResult.addon, "_FlipXYc");
    CHECK(flipSlot >= 0, "_FlipXYc slot found for downstream chain contract test");
    if (flipSlot < 0) return;

    loadResult.addon.Formulas[flipSlot].dOptionValue[0] = 1.0;
    loadResult.addon.Formulas[2].iFnr = 20;
    std::memset(loadResult.addon.Formulas[2].CustomFname, 0, sizeof(loadResult.addon.Formulas[2].CustomFname));
    std::memcpy(loadResult.addon.Formulas[2].CustomFname, "_flipC-xyz", 10);
    loadResult.addon.Formulas[2].iItCount = 1;
    loadResult.addon.Formulas[2].iOptionCount = 0;
    std::memset(loadResult.addon.Formulas[2].dOptionValue, 0, sizeof(loadResult.addon.Formulas[2].dOptionValue));
    loadResult.addon.iFCount = 3;

    ExternalFormulaRunState state{};
    state.x = 1.0;
    state.y = 2.0;
    state.z = 3.0;
    state.cx = 10.0;
    state.cy = 20.0;
    state.cz = 30.0;
    state.itResultI = 8;

    state = runExternalFormulaByName("_FlipXYc", {1.0}, state);
    state = runExternalFormulaByName("_flipC-xyz", {}, state);

    CHECK_NEAR(state.x, 20.0, 1e-12, "_FlipXYc -> _flipC-xyz consumes swapped Cx");
    CHECK_NEAR(state.y, 10.0, 1e-12, "_FlipXYc -> _flipC-xyz consumes swapped Cy");
    CHECK_NEAR(state.z, 30.0, 1e-12, "_FlipXYc -> _flipC-xyz preserves Cz for downstream consumer");
    CHECK_NEAR(state.cx, 2.0, 1e-12, "_flipC-xyz writes previous y into Cx after FlipXYc swap");
    CHECK_NEAR(state.cy, 1.0, 1e-12, "_flipC-xyz writes previous x into Cy after FlipXYc swap");
    CHECK_NEAR(state.cz, 3.0, 1e-12, "_flipC-xyz writes previous z into Cz after FlipXYc swap");
}

static void test_conditcountswitch_transform_juliax2_chain_contract() {
    std::printf("  [conditcountswitch_transform_juliax2_chain_contract]\n");

    std::array<int, 7> itcount{};
    itcount[3] = 1; // _Transform active
    itcount[4] = 0; // _juliax2 initially disabled
    int execCounter = 0;

    applyCondItcountSwitch(itcount, execCounter,
                           1, 4,
                           0, 1, 0);

    CHECK(execCounter == 1, "_CondItcountSwitch increments execution counter once");
    CHECK(itcount[4] == 1, "_CondItcountSwitch enables downstream _juliax2 slot on equality branch");

    ExternalFormulaRunState state{};
    state.x = 1.0;
    state.y = 2.0;
    state.z = 3.0;
    state.cx = 10.0;
    state.cy = 20.0;
    state.cz = 30.0;

    if (itcount[3] > 0) {
        state = runExternalFormulaByName("_Transform",
                                         {2.0, 1.0, -2.0, 0.5, 0.0, 0.0, 0.0, 1.0},
                                         state);
    }
    if (itcount[4] > 0) {
        state = runExternalFormulaByName("_juliax2",
                                         {0.0, 100.0, 101.0, 102.0, -100.0, -101.0, -102.0},
                                         state);
    }

    CHECK_NEAR(state.x, 4.0, 1e-12, "branch-state chain enables downstream _juliax2 x result");
    CHECK_NEAR(state.y, 0.0, 1e-12, "branch-state chain enables downstream _juliax2 y result");
    CHECK_NEAR(state.z, 7.0, 1e-12, "branch-state chain enables downstream _juliax2 z result");
    CHECK_NEAR(state.cx, -100.0, 1e-12, "enabled _juliax2 writes Julia x");
    CHECK_NEAR(state.cy, -101.0, 1e-12, "enabled _juliax2 writes Julia y");
    CHECK_NEAR(state.cz, -102.0, 1e-12, "enabled _juliax2 writes Julia z");
}

namespace heightmap_reference {
#include "fixtures/heightmap_regressions.h"
static double u, v;
static int number, calls;
static void sample(TPVec3D result, TPVec3D uv, int32_t map) {
    u=(*uv)[0]; v=(*uv)[1]; number=map; ++calls;
    (*result)[0]=u; (*result)[1]=v; (*result)[2]=map;
}
}

namespace spheremap_reference {
static double x,y,z;
static int number,calls;
static void sample(TPVec3D result, TPVec3D xyz, int32_t map) {
    x=(*xyz)[0];y=(*xyz)[1];z=(*xyz)[2];number=map;++calls;
    (*result)[0]=z;(*result)[1]=x;(*result)[2]=map;
}
}

// New captures retain the independent oracle process boundary explicitly.
template<class Row>
static auto fixture_case_id(const Row& row, int) -> decltype(row.case_id) { return row.case_id; }
template<class Row>
static const char* fixture_case_id(const Row&, long) { return nullptr; }

template<class Row, size_t N>
static void check_external_formula_regressions(const Row (&rows)[N], bool exactFormulaState = false) {
#if defined(_MSC_VER) && defined(_M_X64)
    // Independent oracle processes start with TOP=0. Keep exceptional rows
    // inside their group from changing the preconditions of other groups.
    FixtureX87Environment environment;
    const auto traceFpu = [&](const char* when) {
        if (std::getenv("MB3D_TEST_FPU_TRACE")) {
            alignas(16) unsigned char fx[512]; _fxsave64(fx);
            std::printf("FPU %s %s sw=%02x%02x tag=%02x\n", when, rows[0].name,fx[3],fx[2],fx[4]);
        }
    };
    traceFpu("before");
    const char* previousCase = nullptr;
#endif
    for (const auto& row : rows) {
        if (g_formulaFilter && std::strcmp(g_formulaFilter, row.name) != 0) continue;
        ++g_formulaRows;
        const int passedBefore=g_passed, failedBefore=g_failed;
#if defined(_MSC_VER) && defined(_M_X64)
        const char* currentCase = fixture_case_id(row, 0);
        if (currentCase && (!previousCase || std::strcmp(currentCase, previousCase) != 0)) {
            // One oracle process per scene, with all its inputs in order.
            // Preserve state within that process, but not across other cases.
            environment.restart();
            previousCase = currentCase;
        }
#endif
        ExternalFormulaRunState input{};
        input.x = row.input[0]; input.y = row.input[1];
        input.z = row.input[2]; input.w = row.input[3];
        input.cx = .2; input.cy = -.3; input.cz = .4;
        input.px = input.x + .125; input.py = input.y - .25; input.pz = input.z + .5;
        input.rout = input.x*input.x + input.y*input.y + input.z*input.z;
        input.rOld = .001; input.rStopD = 4; input.varyScale = 1; input.bFirstIt = -1;
        input.deriv1 = -2; input.deriv2 = .375; input.deriv3 = 3;
        if constexpr (sizeof(row.input) / sizeof(double) == 7) {
            input.cx=row.input[4]; input.cy=row.input[5]; input.cz=row.input[6];
        }
        if constexpr (sizeof(row.input) / sizeof(double) == 15) {
            input.cx=row.input[4]; input.cy=row.input[5]; input.cz=row.input[6];
            input.varyScale=row.input[7]; input.bFirstIt=static_cast<int32_t>(row.input[8]);
            input.deriv1=1; input.deriv2=input.deriv3=0;
            std::copy_n(row.input+11, 4, input.sse01);
        }
        if constexpr (sizeof(row.input) / sizeof(double) == 5) input.rout = row.input[4];
        if constexpr (sizeof(row.input) / sizeof(double) == 8) {
            input.bFirstIt = static_cast<int32_t>(row.input[4]);
            input.deriv1 = row.input[5]; input.deriv2 = row.input[6]; input.deriv3 = row.input[7];
        }
        if constexpr (sizeof(row.input) / sizeof(double) == 6) {
            input.rOld=row.input[4]; input.dFree1=row.input[5];
            input.deriv1=1; input.deriv2=input.deriv3=0;
            input.directMap=heightmap_reference::sample;
            heightmap_reference::u=heightmap_reference::v=0;
            heightmap_reference::number=heightmap_reference::calls=0;
        }
        if constexpr (sizeof(row.output)/sizeof(double)==25) {
            input.sphereMap=spheremap_reference::sample;
            spheremap_reference::x=spheremap_reference::y=spheremap_reference::z=0;
            spheremap_reference::number=spheremap_reference::calls=0;
        }
        const auto r = runExternalFormulaByName(row.name,
            std::vector<double>(row.options, row.options + row.option_count), input, true);
        double actual[] = {r.x,r.y,r.z,r.w,r.cx,r.cy,r.cz,r.cw,
            r.px,r.py,r.pz,r.rout,r.varyScale,double(r.bFirstIt),r.dFree1,r.dFree2,
            r.deriv1,r.deriv2,r.deriv3,double(r.itResultI),
            heightmap_reference::u,heightmap_reference::v,
            double(heightmap_reference::number),double(heightmap_reference::calls),0.};
        if constexpr (sizeof(row.output)/sizeof(double)==25) {
            actual[20]=spheremap_reference::x;actual[21]=spheremap_reference::y;
            actual[22]=spheremap_reference::z;actual[23]=spheremap_reference::number;
            actual[24]=spheremap_reference::calls;
        }
        if constexpr (sizeof(row.input)/sizeof(double)==15)
            std::copy_n(r.sse01, 4, actual+20);
        for (size_t lane = 0; lane < sizeof(row.output)/sizeof(double); ++lane) {
            const double expected = row.output[lane];
            if constexpr (sizeof(row.input)/sizeof(double)==15) {
                if (lane>=20) {
                    CHECK(std::isnan(expected) ? std::isnan(actual[lane]) : actual[lane]==expected,
                          "original formula SSE state exact");
                    continue;
                }
            }
#if defined(_MSC_VER) && defined(_M_X64)
            // These rounding regressions can pass the general tolerance and still
            // change an image. Check every state lane written by the strict groups.
            const bool writtenLane = lane < 3 ||
                ((std::strcmp(row.name, "CylinderHeightMap") == 0 ||
                  std::strcmp(row.name, "CylinderHMVoid") == 0 ||
                  std::strcmp(row.name, "TorusHMVoid") == 0) &&
                 (lane == 11 || lane == 14 || lane >= 20)) ||
                ((std::strcmp(row.name, "QuatPow") == 0 ||
                  std::strcmp(row.name, "PG-bulb") == 0) && lane == 3) ||
                (std::strstr(row.name, "IFS") != nullptr &&
                 (lane == 11 || lane == 12 || lane == 14));
            if (exactFormulaState && writtenLane && !std::isnan(expected)) {
                CHECK(std::memcmp(&actual[lane], &expected, sizeof(double)) == 0,
                      "original formula exact written state bits");
            }
#endif
            const bool matches = std::isnan(expected) ? std::isnan(actual[lane]) :
                std::isinf(expected) ? actual[lane] == expected :
                std::isfinite(actual[lane]) &&
                    std::abs(actual[lane]-expected) <= 2e-12*std::max(1., std::abs(expected));
            if (!matches)
                std::printf("  fixture %s input=(%.17g,%.17g,%.17g,%.17g) lane=%zu actual=%.17g expected=%.17g\n",
                    row.name, row.input[0], row.input[1], row.input[2], row.input[3], lane, actual[lane], expected);
            if (std::isnan(expected)) CHECK(std::isnan(actual[lane]), "original formula NaN");
            else if (std::isinf(expected)) CHECK(actual[lane] == expected, "original formula infinity sign");
            else {
                CHECK(std::isfinite(actual[lane]), "original formula finite state");
                CHECK_NEAR(actual[lane], expected, 2e-12*std::max(1., std::abs(expected)), "original formula state");
            }
        }
        auto& stats=g_formulaStats[row.name];
        ++stats.rows;
        stats.checks += (g_passed-passedBefore)+(g_failed-failedBefore);
        stats.failures += g_failed-failedBefore;
    }
#if defined(_MSC_VER) && defined(_M_X64)
    traceFpu("after");
#endif
}

static void test_external_formula_regressions() {
#if defined(MB3D_SOFTWARE_EXT80)
    if (!g_formulaFilter || std::strcmp(g_formulaFilter, "ABoxVSWavy") == 0) {
        // Original ABoxVSWavy CODE 00AC..00CB: an out-of-range FSINCOS
        // does not push. The extra FSTP consumes a coordinate; missing
        // operands in 0157..016A produce x87 indefinite (negative NaN).
        // These instruction-contract checks supplement the captured point
        // rows, whose NAN literals retain classification but not sign.
        for (double scale : {2., -2.}) for (double huge : {0x1p70, -0x1p70})
            for (int axis = 0; axis < 3; ++axis) {
                ExternalFormulaRunState input{};
                input.x=.125; input.y=-.25; input.z=.5; input.w=.375;
                if (axis == 0) input.x=huge;
                else if (axis == 1) input.y=huge;
                else input.z=huge;
                input.cx=.25; input.cy=-.5; input.cz=.75;
                input.varyScale=1; input.bFirstIt=-1;
                const auto r=runExternalFormulaByName("ABoxVSWavy",
                    {scale,.5,1,0,.6,1,-.4,1.3,0}, input);
                const double actual[]={r.x,r.y,r.z};
                const double multiplier=scale*4;
                const double finite[]={0,.125*multiplier-.5,
                    (axis == 1 ? .125 : -.25)*multiplier+.75};
                for (int lane=0; lane<3; ++lane) {
                    if (lane < 3-axis) {
                        uint64_t bits; std::memcpy(&bits, actual+lane, sizeof(bits));
                        CHECK(bits == UINT64_C(0xfff8000000000000),
                              "ABoxVSWavy missing operand preserves indefinite NaN bits");
                    } else CHECK(actual[lane] == finite[lane],
                                 "ABoxVSWavy remaining stack coordinate");
                }
                CHECK(r.w == .375*multiplier, "ABoxVSWavy range failure signed derivative");
                CHECK(r.varyScale == scale, "ABoxVSWavy range failure scale state");
                CHECK(r.bFirstIt == 0, "ABoxVSWavy range failure first-iteration state");
            }
    }
#endif
#if defined(_MSC_VER) && defined(_M_X64)
    {
        FixtureX87Environment environment;
        for (const auto& row : radiusDEReferences) {
            double v[6], result[3]; memcpy(v,row.input,sizeof(v));
            Rout3_compiled_pas_sse2(result,v);
            result[1] = mb3d::NumericalDistanceEstimate(v[0],v[1],v[2],v[3],v[4],v[5],result+2);
            for (int lane=0; lane<3; ++lane) {
                uint64_t bits; memcpy(&bits,result+lane,8);
                CHECK(bits == row.expected[lane], "Pascal radius and numerical DE bits");
            }
        }
        for (const auto& row : smoothIterationReferences) {
            TIteration3Dext state{};
            memcpy(&state.Rout, &row.radius, 8); memcpy(&state.Rold, &row.oldRadius, 8);
            auto* it = reinterpret_cast<TPIteration3D>(reinterpret_cast<char*>(&state)+56);
            memcpy(&it->LNRStop, &row.logStop, 4); memcpy(&it->fHln[0], &row.power, 4);
            it->ItResultI = row.iterations;
            mb3d::CalcSmoothIterations(it, 0);
            uint32_t bits; memcpy(&bits, &it->SmoothItD, 4);
            CHECK(bits == row.result, "Pascal smooth iteration bits");
        }
        for (const auto& row : integerPower8References) {
            double v[14]; memcpy(v, row.input, sizeof(v));
            TIteration3Dext state{};
            auto* it = reinterpret_cast<TPIteration3D>(reinterpret_cast<char*>(&state)+56);
            it->J1=v[3]; it->J2=v[4]; it->J3=v[5];
            alignas(16) double params[32]{};
            double* base=params+4;
            base[-2]=v[6]; base[18]=v[7]; base[19]=v[8]; base[20]=v[9];
            base[24]=v[10]; base[26]=v[11]; base[4]=v[12]; base[3]=v[13];
            it->PVar=base;
            double w=0;
            mb3d::HybridIntP8(v[0],v[1],v[2],w,it);
            for (int lane=0; lane<3; ++lane) {
                uint64_t bits; memcpy(&bits,v+lane,8);
                CHECK(bits == row.expected[lane], "Pascal integer power 8 bits");
            }
        }
    }
#endif
    check_external_formula_regressions(room_triplex_reference::formulaRegressions);
    check_external_formula_regressions(fold_cut_reference::formulaRegressions);
    check_external_formula_regressions(box_tetra_pine_reference::formulaRegressions);
    check_external_formula_regressions(iq_hex_golden_reference::formulaRegressions);
    check_external_formula_regressions(iq_hex_boundary_reference::formulaRegressions);
    check_external_formula_regressions(fuzzy_radius_reference::formulaRegressions);
    check_external_formula_regressions(vary2_counter_reference::formulaRegressions);
    check_external_formula_regressions(slice_coord_golden_reference::formulaRegressions);
    check_external_formula_regressions(slice_coord_boundary_reference::formulaRegressions);
    check_external_formula_regressions(fold_kali_tube_golden_reference::formulaRegressions);
    check_external_formula_regressions(fold_kali_tube_boundary_reference::formulaRegressions);
    check_external_formula_regressions(fold_kali_tube_rout_reference::formulaRegressions);
    check_external_formula_regressions(logsp_full_options_reference::formulaRegressions);
    check_external_formula_regressions(varyscale_pair_parameters_reference::formulaRegressions);
    check_external_formula_regressions(varyscale_pair_boundary_reference::formulaRegressions);
    check_external_formula_regressions(amazingbox_sse2_parameters_reference::formulaRegressions);
    check_external_formula_regressions(amazingbox_sse2_boundary_reference::formulaRegressions);
    check_external_formula_regressions(devil_citrus_parameters_reference::formulaRegressions);
    check_external_formula_regressions(devil_citrus_boundary_reference::formulaRegressions);
    check_external_formula_regressions(magnet_bicomplex_parameters_reference::formulaRegressions);
    check_external_formula_regressions(magnet_bicomplex_boundary_reference::formulaRegressions);
    check_external_formula_regressions(fold_poincare_reference::formulaRegressions, true);
    check_external_formula_regressions(last_image_reference::formulaRegressions, true);
    check_external_formula_regressions(quatpow_exp_boundary_reference::formulaRegressions, true);
    check_external_formula_regressions(remaining_points_parameters_reference::formulaRegressions, true);
    check_external_formula_regressions(remaining_points_exceptional_reference::formulaRegressions, true);
    check_external_formula_regressions(cyl_torus_parameters_reference::formulaRegressions, true);
    check_external_formula_regressions(cyl_torus_exceptional_reference::formulaRegressions, true);
    check_external_formula_regressions(trifox_reference::formulaRegressions, true);
    check_external_formula_regressions(scalar_map_parameters_reference::formulaRegressions, true);
    check_external_formula_regressions(scalar_map_exceptional_reference::formulaRegressions, true);
    check_external_formula_regressions(knots_weird_parameters_reference::formulaRegressions);
    check_external_formula_regressions(knots_weird_boundary_reference::formulaRegressions);
    check_external_formula_regressions(fold_order_parameters_reference::formulaRegressions);
    check_external_formula_regressions(fold_order_boundary_reference::formulaRegressions);
    check_external_formula_regressions(ifs_apply_parameters_reference::formulaRegressions);
    check_external_formula_regressions(ifs_apply_boundary_reference::formulaRegressions);
    check_external_formula_regressions(branch_julia_parameters_reference::formulaRegressions);
    check_external_formula_regressions(branch_julia_boundary_reference::formulaRegressions);
    check_external_formula_regressions(heightmap_family_planar_parameters_reference::formulaRegressions);
    check_external_formula_regressions(heightmap_family_planar_boundary_reference::formulaRegressions);
    check_external_formula_regressions(heightmap_family_sphere_parameters_reference::formulaRegressions);
    check_external_formula_regressions(heightmap_family_sphere_boundary_reference::formulaRegressions);
    check_external_formula_regressions(angular_radius_parameters_reference::formulaRegressions);
    check_external_formula_regressions(angular_radius_boundary_reference::formulaRegressions);
    check_external_formula_regressions(angular_radius_radius_reference::formulaRegressions);
    check_external_formula_regressions(projection_reciprocal_parameters_reference::formulaRegressions);
    check_external_formula_regressions(projection_reciprocal_boundary_reference::formulaRegressions);
    check_external_formula_regressions(mapfold_golden_reference::formulaRegressions);
    check_external_formula_regressions(icen_golden_reference::formulaRegressions);
    check_external_formula_regressions(mapfold_boundary_reference::formulaRegressions);
    check_external_formula_regressions(icen_boundary_reference::formulaRegressions);
    check_external_formula_regressions(icen_counter_reference::formulaRegressions);
    check_external_formula_regressions(axis_wave_box_golden_reference::formulaRegressions);
    check_external_formula_regressions(axis_wave_box_boundary_golden_reference::formulaRegressions);
    check_external_formula_regressions(dodeca_wavy_golden_reference::formulaRegressions);
    check_external_formula_regressions(dodeca_wavy_boundary_golden_reference::formulaRegressions);
    check_external_formula_regressions(wavy_counter_golden_reference::formulaRegressions);
    check_external_formula_regressions(gnarly_family_reference::formulaRegressions);
    check_external_formula_regressions(gnarly_family_boundary_reference::formulaRegressions);
    check_external_formula_regressions(box_tetra_pine_boundary_reference::formulaRegressions);
    check_external_formula_regressions(manowar_sse_reference::formulaRegressions);
    check_external_formula_regressions(aboxvsshapes_reference::formulaRegressions);
    check_external_formula_regressions(aboxvsshapes_counter_reference::formulaRegressions);
    check_external_formula_regressions(sliceex_reference::formulaRegressions);
    check_external_formula_regressions(e3dtess_reference::formulaRegressions);

    check_external_formula_regressions(columns_reference::formulaRegressions);
    check_external_formula_regressions(piriform_reference::formulaRegressions);
    check_external_formula_regressions(helispiral_reference::formulaRegressions);
    check_external_formula_regressions(mengerkochv2_reference::formulaRegressions);
    check_external_formula_regressions(kali_reference::formulaRegressions);
    check_external_formula_regressions(amazingboxtransform_reference::formulaRegressions);
    check_external_formula_regressions(cubetube_reference::formulaRegressions);
    check_external_formula_regressions(maptranslate_reference::formulaRegressions);
    check_external_formula_regressions(heightmap_reference::formulaRegressions);
    check_external_formula_regressions(formulaRegressions);
    check_external_formula_regressions(julia_state_reference::formulaRegressions);
    check_external_formula_regressions(transform_state_reference::formulaRegressions);
    check_external_formula_regressions(sine_approx_reference::formulaRegressions);
    check_external_formula_regressions(state_writeback_reference::formulaRegressions);
    check_external_formula_regressions(setw_cancellation_reference::formulaRegressions);
    check_external_formula_regressions(pinegon_reference::formulaRegressions);
    check_external_formula_regressions(cayley_reference::formulaRegressions);
    check_external_formula_regressions(ifs_reference::formulaRegressions);
    check_external_formula_regressions(gear_reference::formulaRegressions);
    check_external_formula_regressions(gear2_reference::formulaRegressions);
    check_external_formula_regressions(hexgrid_reference::formulaRegressions);
    check_external_formula_regressions(hexgrid_family_reference::formulaRegressions);
    check_external_formula_regressions(discoball_reference::formulaRegressions);
    check_external_formula_regressions(sphcage_reference::formulaRegressions);
    check_external_formula_regressions(riemann2_reference::formulaRegressions);
    check_external_formula_regressions(hycinv_reference::formulaRegressions);
    check_external_formula_regressions(mag_sign_reference::formulaRegressions);
    check_external_formula_regressions(tiling_rucker_reference::formulaRegressions);
    check_external_formula_regressions(boxifs_reference::formulaRegressions);
    check_external_formula_regressions(bulbox_reference::formulaRegressions);
    check_external_formula_regressions(knots_reference::formulaRegressions);
    check_external_formula_regressions(kleinbot_reference::formulaRegressions);
    check_external_formula_regressions(general_reference::formulaRegressions);
    check_external_formula_regressions(ifstess_reference::formulaRegressions);
    check_external_formula_regressions(cantor_reference::formulaRegressions);
    check_external_formula_regressions(amazing_surf2_reference::formulaRegressions);
    check_external_formula_regressions(surf2_smooth_selector_reference::formulaRegressions);
    check_external_formula_regressions(newfold_restore_reference::formulaRegressions);
    check_external_formula_regressions(amazing_surf_reference::formulaRegressions);
    check_external_formula_regressions(aboxsmooth_reference::formulaRegressions);
    check_external_formula_regressions(spherefold_reference::formulaRegressions);
    check_external_formula_regressions(aexion_reference::formulaRegressions);
    check_external_formula_regressions(pg_bulb_power_reference::formulaRegressions, true);
    check_external_formula_regressions(selector_overflow_reference::formulaRegressions, true);
    check_external_formula_regressions(cell_conversion_reference::formulaRegressions);
    check_external_formula_regressions(comma_reference::formulaRegressions);
    check_external_formula_regressions(mengerhyper_reference::formulaRegressions);
    check_external_formula_regressions(juliasets_reference::formulaRegressions);
    check_external_formula_regressions(atetravs_reference::formulaRegressions);
    check_external_formula_regressions(platinum_b_reference::formulaRegressions);
    check_external_formula_regressions(celtic_reference::formulaRegressions);
    check_external_formula_regressions(sphereoffset4d_reference::formulaRegressions);
    check_external_formula_regressions(mandalay_reference::formulaRegressions);
    check_external_formula_regressions(mandalex_reference::formulaRegressions);
    check_external_formula_regressions(menger_original_reference::formulaRegressions);
    check_external_formula_regressions(boxifs_twist_reference::formulaRegressions);
    check_external_formula_regressions(logspiral_reference::formulaRegressions);
    check_external_formula_regressions(iteration_rotation_reference::formulaRegressions);
    check_external_formula_regressions(random_transform_reference::formulaRegressions);
    check_external_formula_regressions(foldcut_cancellation_reference::formulaRegressions);
}

static void test_external_four_d_original_code() {
    for (const auto& row : kFourDOriginalCodeCases) {
        ExternalFormulaRunState input{};
        input.x = row.input[0]; input.y = row.input[1];
        input.z = row.input[2]; input.w = row.input[3];
        input.cx = .2; input.cy = -.3; input.cz = .4;
        input.deriv1 = -2; input.deriv2 = .375; input.deriv3 = 3;
        auto result = runExternalFormulaByName(row.name,
            std::vector<double>(row.options, row.options + row.count), input);
        CHECK_NEAR(result.x, row.output[0], 2e-12, "4D original-code X");
        CHECK_NEAR(result.y, row.output[1], 2e-12, "4D original-code Y");
        CHECK_NEAR(result.z, row.output[2], 2e-12, "4D original-code Z");
        CHECK_NEAR(result.w, row.output[3], 2e-12, "4D original-code W");
        CHECK_NEAR(result.deriv1, row.deriv1, 2e-12, "4D signed derivative");
        CHECK_NEAR(result.deriv2, input.deriv2, 0, "4D preserves Deriv2");
        CHECK_NEAR(result.deriv3, input.deriv3, 0, "4D preserves Deriv3");
        CHECK_NEAR(result.cx, input.cx, 0, "4D preserves Julia X");
        CHECK_NEAR(result.cy, input.cy, 0, "4D preserves Julia Y");
        CHECK_NEAR(result.cz, input.cz, 0, "4D preserves Julia Z");
    }
}

static void test_vsshapes_original_code() {
    for (const auto& row : vsShapesReference) {
        ExternalFormulaRunState input{};
        input.x = .375; input.y = -.25; input.z = .125; input.w = .375;
        input.cx = .2; input.cy = -.3; input.cz = .4;
        input.bFirstIt = -1; input.varyScale = 1;
        const auto result = runExternalFormulaByName("ABoxVSShapes",
            std::vector<double>(row.options, row.options + 7), input);
        const double actual[] = {result.x, result.y, result.z, result.w};
        for (int k = 0; k < 4; ++k) {
            if (std::isnan(row.output[k])) {
                CHECK(std::isnan(actual[k]), "ABoxVSShapes original invalid-mode stack underflow");
            } else {
                CHECK(std::isfinite(actual[k]), "ABoxVSShapes finite original output");
                CHECK_NEAR(actual[k], row.output[k], 2e-12, "ABoxVSShapes original-code output");
            }
        }
        CHECK(result.bFirstIt == 0, "ABoxVSShapes advances the initial flag once");
        CHECK_NEAR(result.varyScale, 2.0, 0.0, "ABoxVSShapes initializes scale");
    }
}

static void test_smooth_fold_original_code() {
    for (const auto& row : smoothFoldReference) {
        ExternalFormulaRunState input{};
        input.x = -2; input.y = -2; input.z = -2; input.w = .375;
        input.cx = .2; input.cy = -.3; input.cz = .4;
        input.bFirstIt = -1; input.varyScale = 1;
        const auto result = runExternalFormulaByName(row.name,
            std::vector<double>(row.options, row.options + row.count), input);
        CHECK(std::isfinite(result.x) && std::isfinite(result.y) &&
              std::isfinite(result.z) && std::isfinite(result.w),
              "smooth-fold zero/one sharpness must remain finite");
        CHECK_NEAR(result.x, row.output[0], 2e-12, "smooth-fold original-code X");
        CHECK_NEAR(result.y, row.output[1], 2e-12, "smooth-fold original-code Y");
        CHECK_NEAR(result.z, row.output[2], 2e-12, "smooth-fold original-code Z");
        CHECK_NEAR(result.w, row.output[3], 2e-12, "smooth-fold original-code W");
        CHECK(result.bFirstIt == 0, "smooth-fold advances the initial flag once");
    }
}

static void test_pinehedron_original_code() {
    for (const auto& row : pinehedronReference) {
        ExternalFormulaRunState input{};
        input.x = row.input[0]; input.y = row.input[1]; input.z = row.input[2];
        input.w = .375; input.cx = .2; input.cy = -.3; input.cz = .4;
        input.itResultI = row.iteration;
        const auto result = runExternalFormulaByName("BT1Pinehedron",
            std::vector<double>(row.options, row.options + 13), input);
        CHECK(std::isfinite(result.x) && std::isfinite(result.y) && std::isfinite(result.z),
              "Pinehedron cancellation tails must remain finite");
        CHECK_NEAR(result.x, row.output[0], 2e-12, "Pinehedron original-code X");
        CHECK_NEAR(result.y, row.output[1], 2e-12, "Pinehedron original-code Y");
        CHECK_NEAR(result.z, row.output[2], 2e-12, "Pinehedron original-code Z");
        CHECK_NEAR(result.w, row.output[3], 0, "Pinehedron preserves W");
    }
}

static void test_external_stored_radius_and_cwarp_contract() {
    ExternalFormulaRunState state{};
    state.x = 2; state.y = -3; state.z = 4; state.w = .5;
    state.cx = .2; state.cy = -.3; state.cz = .4;
    // Deliberately differs from xyz/w length: transforms within a hybrid
    // consume the radius cached by the engine at its iteration boundary.
    state.rout = 9;
    for (const char* name : {"_Rpow3", "_Rpow3_4d", "_SquareR"}) {
        auto result = runExternalFormulaByName(name, {}, state);
        const double scale = std::strcmp(name, "_SquareR") == 0 ? 3 : 9;
        CHECK_NEAR(result.x, 2*scale, 0, "stored radius x");
        CHECK_NEAR(result.y, -3*scale, 0, "stored radius y");
        CHECK_NEAR(result.z, 4*scale, 0, "stored radius z");
        CHECK_NEAR(result.w, std::strcmp(name, "_Rpow3_4d") == 0 ? 4.5 : .5, 0, "stored radius w");
        CHECK_NEAR(result.cx, state.cx, 0, "radius transform preserves Julia C");
    }
    state.cx = -.5490615088717341;
    state.cy = -.769827967913236;
    state.cz = .6100025569703844;
    auto warped = runExternalFormulaByName("_GenCWarp", {0,0,0,8,8,0,18,0,1,0}, state);
    // Independent shipped x86 output; mode 18 follows the executable's
    // AND/JB behavior, not the minimum described in the formula text.
    CHECK_NEAR(warped.cx, .0656700459842144, 2e-12, "C warp mode18 J1");
    CHECK_NEAR(warped.cy, -.2637863235562023, 2e-12, "C warp mode18 J2");
    CHECK_NEAR(warped.cz, 1.091929934007429, 2e-12, "C warp mode18 J3");
    CHECK_NEAR(warped.x, state.x, 0, "C warp preserves x");
    CHECK_NEAR(warped.y, state.y, 0, "C warp preserves y");
    CHECK_NEAR(warped.z, state.z, 0, "C warp preserves z");
}

static void test_juliasets_flipcxyz_chain_contract() {
    std::printf("  [juliasets_flipcxyz_chain_contract]\n");

    ExternalFormulaRunState state{};
    // The original x86 code reads [PIt3D+0/8/16], not current xyz.
    state.px = 5.2;
    state.py = 6.6;
    state.pz = 7.8;
    state.x = 123.0;
    state.y = 456.0;
    state.z = 789.0;
    state.cx = 0.0;
    state.cy = 0.0;
    state.cz = 0.0;

    state = runExternalFormulaByName("_JuliaSets", {4.0, 0.2}, state);
    CHECK_NEAR(state.x, 1.2, 1e-12, "_JuliaSets keeps fractional x tile position");
    CHECK_NEAR(state.y, -1.4, 1e-12, "_JuliaSets keeps fractional y tile position");
    CHECK_NEAR(state.z, -0.2, 1e-12, "_JuliaSets keeps fractional z tile position");
    CHECK_NEAR(state.cx, 0.2, 1e-12, "_JuliaSets remaps rounded x into downstream Cx");
    CHECK_NEAR(state.cy, 0.4, 1e-12, "_JuliaSets remaps rounded y into downstream Cy");
    CHECK_NEAR(state.cz, 0.4, 1e-12, "_JuliaSets remaps rounded z into downstream Cz");

    state = runExternalFormulaByName("_flipC-xyz", {}, state);
    CHECK_NEAR(state.x, 0.2, 1e-12, "_JuliaSets -> _flipC-xyz consumes remapped Cx");
    CHECK_NEAR(state.y, 0.4, 1e-12, "_JuliaSets -> _flipC-xyz consumes remapped Cy");
    CHECK_NEAR(state.z, 0.4, 1e-12, "_JuliaSets -> _flipC-xyz consumes remapped Cz");
    CHECK_NEAR(state.cx, 1.2, 1e-12, "_flipC-xyz writes prior x into downstream Cx");
    CHECK_NEAR(state.cy, -1.4, 1e-12, "_flipC-xyz writes prior y into downstream Cy");
    CHECK_NEAR(state.cz, -0.2, 1e-12, "_flipC-xyz writes prior z into downstream Cz");
}

static void test_partlyjuliaroff_flipcxyz_chain_contract() {
    std::printf("  [partlyjuliaroff_flipcxyz_chain_contract]\n");

    ExternalFormulaRunState state{};
    state.x = 3.0;
    state.y = 4.0;
    state.z = 0.0;
    state.cx = 10.0;
    state.cy = 20.0;
    state.cz = 30.0;
    state.px = 6.0; state.py = 8.0; state.pz = 0.0; // Saved point radius is 10.

    state = runExternalFormulaByName("_PartlyJuliaRoff", {0.0, 0.0, 0.0, 1.0}, state);
    CHECK_NEAR(state.x, 3.0, 1e-12, "_PartlyJuliaRoff preserves position x");
    CHECK_NEAR(state.y, 4.0, 1e-12, "_PartlyJuliaRoff preserves position y");
    CHECK_NEAR(state.z, 0.0, 1e-12, "_PartlyJuliaRoff preserves position z");
    CHECK_NEAR(state.cx, 15.4, 1e-12, "_PartlyJuliaRoff remaps Cx outside the sphere");
    CHECK_NEAR(state.cy, 27.2, 1e-12, "_PartlyJuliaRoff remaps Cy outside the sphere");
    CHECK_NEAR(state.cz, 30.0, 1e-12, "_PartlyJuliaRoff preserves Cz when z offset is zero");

    state = runExternalFormulaByName("_flipC-xyz", {}, state);
    CHECK_NEAR(state.x, 15.4, 1e-12, "_PartlyJuliaRoff -> _flipC-xyz consumes remapped Cx");
    CHECK_NEAR(state.y, 27.2, 1e-12, "_PartlyJuliaRoff -> _flipC-xyz consumes remapped Cy");
    CHECK_NEAR(state.z, 30.0, 1e-12, "_PartlyJuliaRoff -> _flipC-xyz consumes remapped Cz");
    CHECK_NEAR(state.cx, 3.0, 1e-12, "_flipC-xyz writes prior x into downstream Cx");
    CHECK_NEAR(state.cy, 4.0, 1e-12, "_flipC-xyz writes prior y into downstream Cy");
    CHECK_NEAR(state.cz, 0.0, 1e-12, "_flipC-xyz writes prior z into downstream Cz");
}

static void test_partlyjulia_flipcxyz_chain_contract() {
    std::printf("  [partlyjulia_flipcxyz_chain_contract]\n");

    ExternalFormulaRunState state{};
    state.x = 5.2;
    state.y = 6.6;
    state.z = 7.8;
    state.cx = 10.0;
    state.cy = 20.0;
    state.cz = 30.0;
    state.px = 10.0; state.py = 20.0; state.pz = 30.0; // Original point differs from orbit xyz.

    state = runExternalFormulaByName("_PartlyJulia",
                                     {5.0, -999.0, 1.5,
                                      6.0, -999.0, 2.5,
                                      7.0, -999.0, 3.5},
                                     state);
    CHECK_NEAR(state.x, 5.2, 1e-12, "_PartlyJulia preserves position x");
    CHECK_NEAR(state.y, 6.6, 1e-12, "_PartlyJulia preserves position y");
    CHECK_NEAR(state.z, 7.8, 1e-12, "_PartlyJulia preserves position z");
    CHECK_NEAR(state.cx, 11.5, 1e-12, "_PartlyJulia remaps Cx when saved Px exceeds upper threshold");
    CHECK_NEAR(state.cy, 22.5, 1e-12, "_PartlyJulia remaps Cy when saved Py exceeds upper threshold");
    CHECK_NEAR(state.cz, 33.5, 1e-12, "_PartlyJulia remaps Cz when saved Pz exceeds upper threshold");

    state = runExternalFormulaByName("_flipC-xyz", {}, state);
    CHECK_NEAR(state.x, 11.5, 1e-12, "_PartlyJulia -> _flipC-xyz consumes remapped Cx");
    CHECK_NEAR(state.y, 22.5, 1e-12, "_PartlyJulia -> _flipC-xyz consumes remapped Cy");
    CHECK_NEAR(state.z, 33.5, 1e-12, "_PartlyJulia -> _flipC-xyz consumes remapped Cz");
    CHECK_NEAR(state.cx, 5.2, 1e-12, "_flipC-xyz writes prior x into downstream Cx after PartlyJulia");
    CHECK_NEAR(state.cy, 6.6, 1e-12, "_flipC-xyz writes prior y into downstream Cy after PartlyJulia");
    CHECK_NEAR(state.cz, 7.8, 1e-12, "_flipC-xyz writes prior z into downstream Cz after PartlyJulia");
}

static void test_translatec4d_local_contract() {
    std::printf("  [translatec4d_local_contract]\n");

    ExternalFormulaRunState state{};
    state.x = 1.0;
    state.y = 2.0;
    state.z = 3.0;
    state.w = 4.0;
    state.cx = 10.0;
    state.cy = 20.0;
    state.cz = 30.0;
    state.cw = 40.0;

    state = runExternalFormulaByName("_TranslateC4d",
                                     {0.5, -1.5, 2.0, -2.5, 100.0, -200.0, 300.0, -400.0},
                                     state);

    CHECK_NEAR(state.x, 1.5, 1e-12, "_TranslateC4d updates x directly");
    CHECK_NEAR(state.y, 0.5, 1e-12, "_TranslateC4d updates y directly");
    CHECK_NEAR(state.z, 5.0, 1e-12, "_TranslateC4d updates z directly");
    CHECK_NEAR(state.w, 1.5, 1e-12, "_TranslateC4d updates w directly");
    CHECK_NEAR(state.cx, 110.0, 1e-12, "_TranslateC4d updates downstream Cx");
    CHECK_NEAR(state.cy, -180.0, 1e-12, "_TranslateC4d updates downstream Cy");
    CHECK_NEAR(state.cz, 330.0, 1e-12, "_TranslateC4d updates downstream Cz");
    CHECK_NEAR(state.cw, -360.0, 1e-12, "_TranslateC4d updates downstream Cw");
}

static void test_aboxmod4d_local_contract() {
    std::printf("  [aboxmod4d_local_contract]\n");

    ExternalFormulaRunState state{};
    state.x = 0.2;
    state.y = -0.5;
    state.z = 0.75;
    state.w = 0.25;
    state.cx = 10.0;
    state.cy = 20.0;
    state.cz = 30.0;
    state.cw = 40.0;

    state = runExternalFormulaByName("ABoxMod4d", {2.0, 1.0, 0.5}, state);

    CHECK_NEAR(state.x, 10.437158469945356, 1e-12, "ABoxMod4d adds Cx after 4D fold/scale");
    CHECK_NEAR(state.y, 18.90710382513661, 1e-12, "ABoxMod4d adds Cy after 4D fold/scale");
    CHECK_NEAR(state.z, 31.639344262295083, 1e-12, "ABoxMod4d adds Cz after 4D fold/scale");
    CHECK_NEAR(state.w, 41.046448087431696, 1e-12, "ABoxMod4d adds Cw and W_add to w");
}

static void test_aboxvaryscale4d_persistent_state_contract() {
    std::printf("  [aboxvaryscale4d_persistent_state_contract]\n");

    ExternalFormulaRunState state{};
    state.x = 0.2;
    state.y = -0.5;
    state.z = 0.75;
    state.w = 0.25;
    state.cx = 10.0;
    state.cy = 20.0;
    state.cz = 30.0;
    state.cw = 40.0;
    state.varyScale = 0.0;
    state.bFirstIt = 0;

    const std::vector<double> options = {2.0, 0.5, 1.0, 0.5, 1.0, 0.25};

    state = runExternalFormulaByName("ABoxVaryScale4d", options, state);
    CHECK_NEAR(state.x, 10.546448087431694, 1e-12, "ABoxVaryScale4d adds Cx after first iteration");
    CHECK_NEAR(state.y, 18.633879781420764, 1e-12, "ABoxVaryScale4d adds Cy after first iteration");
    CHECK_NEAR(state.z, 32.049180327868854, 1e-12, "ABoxVaryScale4d adds Cz after first iteration");
    CHECK_NEAR(state.w, 40.93306010928962, 1e-12, "ABoxVaryScale4d adds Cw and W_add after first iteration");
    CHECK_NEAR(state.varyScale, 2.5, 1e-12, "ABoxVaryScale4d initializes and writes back VaryScale");
    CHECK(state.bFirstIt == 1, "ABoxVaryScale4d marks first iteration as consumed");

    state = runExternalFormulaByName("ABoxVaryScale4d", options, state);
    CHECK_NEAR(state.varyScale, 3.25, 1e-12, "ABoxVaryScale4d reuses prior persistent VaryScale on next call");
}

static void test_translatec4d_scene_runtime_trace() {
    std::printf("  [translatec4d_scene_runtime_trace]\n");

    auto loadResult = mb3d::LoadM3PFile(comparisonScenePath("extall_Ads__TranslateC4d.m3p"));
    CHECK(loadResult.success, "load extall_Ads__TranslateC4d scene for runtime trace");
    if (!loadResult.success) return;

    CHECK(findFormulaSlotByName(loadResult.addon, "_TranslateC4d") >= 0,
          "extall_Ads__TranslateC4d scene contains _TranslateC4d");

    auto probe = findFirstFormulaContractProbe(loadResult.header,
                                               loadResult.addon,
                                               "translatec4d_scene_runtime_trace.txt",
                                               "formula=FORMULA__TRANSLATEC4D");
    if (!probe.found) {
        std::printf("    bounded probe did not reach live _TranslateC4d contract surface; scene kept as structural file-based sanity only\n");
        return;
    }

    std::printf("    first live pixel=%d,%d\n", probe.x, probe.y);
    std::printf("    %s\n", probe.inLine.c_str());
    std::printf("    %s\n", probe.outLine.c_str());

    CHECK(traceFieldValue(probe.inLine, "x") != traceFieldValue(probe.outLine, "x"),
          "_TranslateC4d scene changes x on the live scene surface");
    CHECK(traceFieldValue(probe.inLine, "w") != traceFieldValue(probe.outLine, "w"),
          "_TranslateC4d scene changes w on the live scene surface");
    CHECK(traceFieldValue(probe.inLine, "J4") != traceFieldValue(probe.outLine, "J4"),
          "_TranslateC4d scene changes downstream Cw on the live scene surface");
}

static void test_aboxmod4d_scene_runtime_trace() {
    std::printf("  [aboxmod4d_scene_runtime_trace]\n");

    auto loadResult = mb3d::LoadM3PFile(comparisonScenePath("extall_4Da_ABoxMod4d.m3p"));
    CHECK(loadResult.success, "load extall_4Da_ABoxMod4d scene for runtime trace");
    if (!loadResult.success) return;

    auto probe = findFirstFormulaContractProbe(loadResult.header,
                                               loadResult.addon,
                                               "aboxmod4d_scene_runtime_trace.txt",
                                               "formula=FORMULA_ABOXMOD4D");
    CHECK(probe.found, "extall_4Da_ABoxMod4d scene reaches ABoxMod4d contract surface");
    if (!probe.found) return;

    std::printf("    first live pixel=%d,%d\n", probe.x, probe.y);
    std::printf("    %s\n", probe.inLine.c_str());
    std::printf("    %s\n", probe.outLine.c_str());

    CHECK(traceFieldValue(probe.inLine, "z") != traceFieldValue(probe.outLine, "z"),
          "ABoxMod4d scene changes coordinates on the live scene surface");
}

static void test_aboxvaryscale4d_scene_runtime_trace() {
    std::printf("  [aboxvaryscale4d_scene_runtime_trace]\n");

    auto loadResult = mb3d::LoadM3PFile(comparisonScenePath("extall_4Da_ABoxVaryScale4d.m3p"));
    CHECK(loadResult.success, "load extall_4Da_ABoxVaryScale4d scene for runtime trace");
    if (!loadResult.success) return;

    auto probe = findFirstFormulaContractProbe(loadResult.header,
                                               loadResult.addon,
                                               "aboxvaryscale4d_scene_runtime_trace.txt",
                                               "formula=FORMULA_ABOXVARYSCALE4D");
    CHECK(probe.found, "extall_4Da_ABoxVaryScale4d scene reaches ABoxVaryScale4d contract surface");
    if (!probe.found) return;

    std::printf("    first live pixel=%d,%d\n", probe.x, probe.y);
    std::printf("    %s\n", probe.inLine.c_str());
    std::printf("    %s\n", probe.outLine.c_str());

    CHECK(traceFieldValue(probe.inLine, "z") != traceFieldValue(probe.outLine, "z"),
          "ABoxVaryScale4d scene changes coordinates on the live scene surface");
}


// ============================================================================
// Test: Multithreaded render matches single-threaded (core pixels)
// ============================================================================
static void test_multithreaded_consistency() {
    std::printf("  [multithreaded_consistency]\n");

    // Render with 1 thread
    auto single = renderRef("formula_0_IntPow2.m3p", 64, true, true, true, true, 1);
    CHECK(single.success, "single-threaded render");
    if (!single.success) return;

    // Render with 4 threads
    auto multi = renderRef("formula_0_IntPow2.m3p", 64, true, true, true, true, 4);
    CHECK(multi.success, "multi-threaded render");
    if (!multi.success) return;

    CHECK(single.width == multi.width, "same width");
    CHECK(single.height == multi.height, "same height");

    // Compare: pixel values should match closely
    // (floating point order differences may cause tiny variations)
    int totalPixels = single.width * single.height;
    int matchCount = 0;
    int closeCount = 0;
    for (int i = 0; i < totalPixels * 3; i++) {
        int diff = std::abs(static_cast<int>(single.image[i]) - static_cast<int>(multi.image[i]));
        if (diff == 0) matchCount++;
        else if (diff <= 2) closeCount++;
    }

    int totalChannels = totalPixels * 3;
    double matchPct = 100.0 * matchCount / totalChannels;
    double closePct = 100.0 * (matchCount + closeCount) / totalChannels;

    // At least 95% of pixels should be within +/- 2
    CHECK(closePct >= 95.0, "multithread >= 95% pixels close to singlethread");
    if (closePct < 95.0) {
        std::printf("    exact match: %.1f%%, close match (+-2): %.1f%%\n", matchPct, closePct);
    }
}

// ============================================================================
// Test: Z-buffer export contains non-zero values
// ============================================================================
static void test_zbuffer_export() {
    std::printf("  [zbuffer_export]\n");

    auto loadResult = mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    CHECK(loadResult.success, "load for zbuffer test");
    if (!loadResult.success) return;

    mb3d::RenderOptions opts;

    opts.precision = mb3d::PrecisionMode::Compatible;
    opts.width_override = 64;
    opts.height_override = 64;
    opts.thread_count = 1;
    opts.no_shadows = true;
    opts.no_ao = true;
    opts.no_dof = true;
    opts.no_reflections = true;
    opts.verbose = false;

    auto result = mb3d::Render(loadResult.header, loadResult.addon, opts);
    CHECK(result.success, "render for zbuffer test");
    if (!result.success) return;

    // Extract z-buffer
    std::vector<uint16_t> zbuf(result.width * result.height);
    for (int i = 0; i < result.width * result.height; i++) {
        zbuf[i] = result.silight[i].Zpos;
    }

    // Count non-zero z-buffer values
    int nonZero = 0;
    for (int i = 0; i < result.width * result.height; i++) {
        if (zbuf[i] != 0) nonZero++;
    }
    CHECK(nonZero > 0, "zbuffer has non-zero values");

    // Write to temp file and verify it succeeds
    std::string tmpPath = tempPath("mb3d_test_zbuf.png");
    bool writeOk = mb3d::WriteZBuffer(tmpPath, zbuf.data(), result.width, result.height);
    CHECK(writeOk, "zbuffer write succeeds");

    // Verify file exists and has content
    if (writeOk) {
        FILE* f = std::fopen(tmpPath.c_str(), "rb");
        CHECK(f != nullptr, "zbuffer file exists");
        if (f) {
            std::fseek(f, 0, SEEK_END);
            long sz = std::ftell(f);
            CHECK(sz > 0, "zbuffer file has content");
            std::fclose(f);
        }
        std::remove(tmpPath.c_str());
    }

    // 8-bit BMP writer compatible with GUI MakeZbufBMP path
    std::string bmpPath = tempPath("mb3d_test_zbuf.bmp");
    bool bmpOk = mb3d::WriteZBufferBMP_8bit(bmpPath, result.silight.data(), result.width, result.height);
    CHECK(bmpOk, "8-bit BMP zbuffer write succeeds");
    if (bmpOk) {
        auto bytes = readFileBytes(bmpPath);
        CHECK(bytes.size() > 54, "8-bit BMP zbuffer has content");
        CHECK(bytes.size() >= 2 && bytes[0] == 'B' && bytes[1] == 'M', "8-bit BMP zbuffer has BMP header");
        std::remove(bmpPath.c_str());
    }

    // 8-bit grayscale PNG
    std::string png8Path = tempPath("mb3d_test_zbuf8.png");
    bool png8Ok = mb3d::WriteZBufferPNG_8bit(png8Path, result.silight.data(), result.width, result.height);
    CHECK(png8Ok, "8-bit PNG zbuffer write succeeds");
    if (png8Ok) {
        auto bytes = readFileBytes(png8Path);
        CHECK(bytes.size() > 33, "8-bit PNG zbuffer has content");
        CHECK(bytes.size() >= 26 &&
              bytes[0] == 0x89 && bytes[1] == 'P' && bytes[2] == 'N' && bytes[3] == 'G',
              "8-bit PNG zbuffer has PNG signature");
        CHECK(bytes[24] == 8 && bytes[25] == 0, "8-bit PNG zbuffer is grayscale 8-bit");
        std::remove(png8Path.c_str());
    }

    // 16-bit grayscale PNG using Pascal-compatible Z decode
    std::string png16Path = tempPath("mb3d_test_zbuf16.png");
    double zcorr = 0.0;
    double zcmul = 0.0;
    double zzst = 0.0;
    mb3d::CalcPPZvals(loadResult.header, zcorr, zcmul, zzst);
    bool png16Ok = mb3d::WriteZBufferPNG_16bit(png16Path, result.silight.data(),
                                               result.width, result.height, zcmul, zcorr);
    CHECK(png16Ok, "16-bit PNG zbuffer write succeeds");
    if (png16Ok) {
        auto bytes = readFileBytes(png16Path);
        CHECK(bytes.size() > 33, "16-bit PNG zbuffer has content");
        CHECK(bytes.size() >= 26 &&
              bytes[0] == 0x89 && bytes[1] == 'P' && bytes[2] == 'N' && bytes[3] == 'G',
              "16-bit PNG zbuffer has PNG signature");
        CHECK(bytes[24] == 16 && bytes[25] == 0, "16-bit PNG zbuffer is grayscale 16-bit");
        std::remove(png16Path.c_str());
    }
}

// ============================================================================
// Test: Normal map export contains valid range values
// ============================================================================
static void test_normalmap_export() {
    std::printf("  [normalmap_export]\n");

    auto result = renderRef("formula_0_IntPow2.m3p", 64);
    CHECK(result.success, "render for normalmap test");
    if (!result.success) return;

    // Extract normals
    std::vector<int16_t> normals(result.width * result.height * 3);
    int validNormals = 0;
    for (int i = 0; i < result.width * result.height; i++) {
        normals[i * 3 + 0] = result.silight[i].NormalX;
        normals[i * 3 + 1] = result.silight[i].NormalY;
        normals[i * 3 + 2] = result.silight[i].NormalZ;

        // Check that normals are in valid int16 range and not all zero
        if (result.silight[i].NormalX != 0 || result.silight[i].NormalY != 0 ||
            result.silight[i].NormalZ != 0) {
            validNormals++;
        }
    }
    CHECK(validNormals > 0, "normalmap has valid normals");

    // Write to temp file
    std::string tmpPath = tempPath("mb3d_test_normals.png");
    bool writeOk = mb3d::WriteNormalMap(tmpPath, normals.data(), result.width, result.height);
    CHECK(writeOk, "normalmap write succeeds");

    if (writeOk) {
        FILE* f = std::fopen(tmpPath.c_str(), "rb");
        CHECK(f != nullptr, "normalmap file exists");
        if (f) {
            std::fseek(f, 0, SEEK_END);
            long sz = std::ftell(f);
            CHECK(sz > 0, "normalmap file has content");
            std::fclose(f);
        }
        std::remove(tmpPath.c_str());
    }
}

// ============================================================================
// Test: BMP writer produces valid BMP file
// ============================================================================
static void test_bmp_writer() {
    std::printf("  [bmp_writer]\n");

    auto result = renderRef("formula_0_IntPow2.m3p", 64);
    CHECK(result.success, "render for BMP test");
    if (!result.success) return;

    std::string tmpPath = tempPath("mb3d_test_output.bmp");
    bool writeOk = mb3d::WriteBMP(tmpPath, result.image.data(), result.width, result.height);
    CHECK(writeOk, "BMP write succeeds");

    if (writeOk) {
        FILE* f = std::fopen(tmpPath.c_str(), "rb");
        CHECK(f != nullptr, "BMP file exists");
        if (f) {
            // Check BMP magic bytes
            uint8_t magic[2];
            std::fread(magic, 1, 2, f);
            CHECK(magic[0] == 'B' && magic[1] == 'M', "BMP has correct magic bytes");

            // Check file size
            std::fseek(f, 0, SEEK_END);
            long sz = std::ftell(f);
            int rowStride = (result.width * 3 + 3) & ~3;
            long expectedSize = 14 + 40 + rowStride * result.height;
            CHECK(sz == expectedSize, "BMP has correct file size");

            // Check DIB header size
            std::fseek(f, 14, SEEK_SET);
            uint32_t dibSize = 0;
            std::fread(&dibSize, 4, 1, f);
            CHECK(dibSize == 40, "BMP DIB header size == 40");

            // Check dimensions
            int32_t bmpWidth = 0, bmpHeight = 0;
            std::fread(&bmpWidth, 4, 1, f);
            std::fread(&bmpHeight, 4, 1, f);
            CHECK(bmpWidth == result.width, "BMP width matches");
            CHECK(bmpHeight == result.height, "BMP height matches");

            std::fclose(f);
        }
        std::remove(tmpPath.c_str());
    }
}

// ============================================================================
// Test: PNG writer produces valid PNG file
// ============================================================================
static void test_png_writer() {
    std::printf("  [png_writer]\n");

    auto result = renderRef("formula_0_IntPow2.m3p", 64);
    CHECK(result.success, "render for PNG test");
    if (!result.success) return;

    std::string tmpPath = tempPath("mb3d_test_output.png");
    bool writeOk = mb3d::WritePNG(tmpPath, result.image.data(), result.width, result.height, 3);
    CHECK(writeOk, "PNG write succeeds");

    if (writeOk) {
        FILE* f = std::fopen(tmpPath.c_str(), "rb");
        CHECK(f != nullptr, "PNG file exists");
        if (f) {
            // Check PNG magic bytes
            uint8_t magic[8];
            std::fread(magic, 1, 8, f);
            CHECK(magic[0] == 0x89 && magic[1] == 'P' && magic[2] == 'N' && magic[3] == 'G',
                  "PNG has correct magic bytes");

            std::fseek(f, 0, SEEK_END);
            long sz = std::ftell(f);
            CHECK(sz > 100, "PNG file has reasonable size");

            std::fclose(f);
        }
        std::remove(tmpPath.c_str());
    }
}

// ============================================================================
// Test: Error handling (nonexistent file)
// ============================================================================
static void test_error_nonexistent_file() {
    std::printf("  [error_nonexistent_file]\n");

    auto result = mb3d::LoadM3PFile(tempPath("nonexistent_file_12345.m3p"));
    CHECK(!result.success, "nonexistent file load fails");
    CHECK(!result.error.empty(), "nonexistent file has error message");
}

// ============================================================================
// Test: Render with invalid dimensions fails gracefully
// ============================================================================
static void test_error_invalid_dimensions() {
    std::printf("  [error_invalid_dimensions]\n");

    auto loadResult = mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    if (!loadResult.success) {
        std::printf("    SKIP: load failed\n");
        return;
    }

    // Force header to have zero dimensions, then render with no override
    // This should trigger the "Invalid image dimensions" error
    loadResult.header.Width = 0;
    loadResult.header.Height = 0;

    mb3d::RenderOptions opts;

    opts.precision = mb3d::PrecisionMode::Compatible;
    opts.width_override = 0;  // 0 means "use from header" which is also 0
    opts.height_override = 0;
    opts.thread_count = 1;
    opts.no_shadows = true;
    opts.no_ao = true;
    opts.no_dof = true;
    opts.no_reflections = true;
    opts.verbose = false;

    auto result = mb3d::Render(loadResult.header, loadResult.addon, opts);
    CHECK(!result.success, "zero dimensions fail");
}

// ============================================================================
// Test: Public C API (mb3d_render_file)
// ============================================================================
static void test_c_api() {
    std::printf("  [c_api]\n");

    // Test NULL path
    MB3DRenderResult r1 = mb3d_render_file(nullptr, nullptr);
    CHECK(r1.success == 0, "C API: NULL path fails");
    CHECK(std::strlen(r1.error) > 0, "C API: NULL path has error message");

    // Test nonexistent file
    MB3DRenderResult r2 = mb3d_render_file(tempPath("nonexistent_c_api_test.m3p").c_str(), nullptr);
    CHECK(r2.success == 0, "C API: nonexistent file fails");
    CHECK(r2.pixels == nullptr, "C API: nonexistent file has no pixels");

    // Test actual render with small size
    MB3DRenderOptions opts;
    std::memset(&opts, 0, sizeof(opts));
    opts.width = 32;
    opts.height = 32;
    opts.thread_count = 1;
    opts.no_shadows = 1;
    opts.no_ao = 1;
    opts.no_dof = 1;
    opts.no_reflections = 1;

    MB3DRenderResult r3 = mb3d_render_file(refPath("formula_0_IntPow2.m3p").c_str(), &opts);
    CHECK(r3.success != 0, "C API: render succeeds");

    if (r3.success) {
        CHECK(r3.width == 32, "C API: width == 32");
        CHECK(r3.height == 32, "C API: height == 32");
        CHECK(r3.pixels != nullptr, "C API: pixels allocated");

        // Check that pixels are non-trivial
        if (r3.pixels) {
            int nonBlack = countNonBlackPixels(r3.pixels, r3.width, r3.height);
            CHECK(nonBlack > 0, "C API: non-black pixels");
        }

        // Check z-buffer
        CHECK(r3.zbuffer != nullptr, "C API: zbuffer allocated");

        // Free
        mb3d_free(r3.pixels);
        mb3d_free(r3.zbuffer);
    }
}

// ============================================================================
// Test: Render result dimensions match options
// ============================================================================
static void test_dimension_override() {
    std::printf("  [dimension_override]\n");

    auto result = renderRef("formula_0_IntPow2.m3p", 48);
    CHECK(result.success, "48x48 render succeeds");
    if (result.success) {
        CHECK(result.width == 48, "width == 48");
        CHECK(result.height == 48, "height == 48");

        int imageBytes = static_cast<int>(result.image.size());
        CHECK(imageBytes == 48 * 48 * 3, "image buffer correct size");
    }

    auto result2 = renderRef("formula_0_IntPow2.m3p", 128);
    CHECK(result2.success, "128x128 render succeeds");
    if (result2.success) {
        CHECK(result2.width == 128, "width == 128");
        CHECK(result2.height == 128, "height == 128");
    }
}

// ============================================================================
// Test: silight buffer populated for surface pixels
// ============================================================================
static void test_silight_populated() {
    std::printf("  [silight_populated]\n");

    auto result = renderRef("formula_0_IntPow2.m3p", 64);
    CHECK(result.success, "render for silight test");
    if (!result.success) return;

    // Count pixels with Zpos < 32768 (surface hits)
    int surfacePixels = 0;
    int backgroundPixels = 0;
    for (int i = 0; i < result.width * result.height; i++) {
        if (result.silight[i].Zpos < 32768) {
            surfacePixels++;
        } else {
            backgroundPixels++;
        }
    }

    CHECK(surfacePixels > 0, "silight has surface pixels");
    // Note: with inside rendering enabled (bOptions2 & 6), all rays hit surfaces
    // so backgroundPixels may be 0. Just verify total is correct.
    CHECK(surfacePixels + backgroundPixels == result.width * result.height,
          "all pixels accounted for");

    // Surface pixels should have non-trivial SIgradient values
    int withGradient = 0;
    for (int i = 0; i < result.width * result.height; i++) {
        if (result.silight[i].Zpos < 32768 && result.silight[i].SIgradient > 0) {
            withGradient++;
        }
    }
    CHECK(withGradient > 0, "surface pixels have SIgradient");
}

// ============================================================================
// Test: Diagnostic samples capture deterministic per-pixel state
// ============================================================================
static void test_diagnostics_capture() {
    std::printf("  [diagnostics_capture]\n");

    auto loadResult = mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    CHECK(loadResult.success, "load for diagnostics test");
    if (!loadResult.success) return;

    mb3d::RenderOptions opts;

    opts.precision = mb3d::PrecisionMode::Compatible;
    opts.width_override = 32;
    opts.height_override = 32;
    opts.thread_count = 1;
    opts.no_shadows = true;
    opts.no_ao = true;
    opts.no_dof = true;
    opts.no_reflections = true;
    opts.diag_points = {
        {0, 0},
        {16, 16},
        {31, 31},
        {-1, 8}
    };

    auto result = mb3d::Render(loadResult.header, loadResult.addon, opts);
    CHECK(result.success, "render for diagnostics test");
    if (!result.success) return;

    CHECK(result.diagnostics.size() == opts.diag_points.size(), "diagnostic record count matches");
    if (result.diagnostics.size() != opts.diag_points.size()) return;

    const auto& topLeft = result.diagnostics[0];
    CHECK(topLeft.in_bounds, "top-left diagnostic in bounds");
    CHECK(topLeft.x == 0 && topLeft.y == 0, "top-left diagnostic coordinates preserved");

    const auto& center = result.diagnostics[1];
    CHECK(center.in_bounds, "center diagnostic in bounds");
    CHECK(center.x == 16 && center.y == 16, "center diagnostic coordinates preserved");
    CHECK(std::isfinite(center.cafx) && std::isfinite(center.cafy), "center diagnostic captures CAF values");
    CHECK(std::isfinite(center.view_vec[0]) && std::isfinite(center.view_vec[1]) &&
          std::isfinite(center.view_vec[2]), "center diagnostic captures view vector");
    CHECK(center.view_vec[0] != 0.0 || center.view_vec[1] != 0.0 || center.view_vec[2] != 0.0,
          "center diagnostic view vector is non-zero");
    CHECK(center.de_stop > 0.0, "center diagnostic captures DE stop");
    CHECK(center.step_count >= 0.0f, "center diagnostic captures step count");

    const auto& bottomRight = result.diagnostics[2];
    CHECK(bottomRight.in_bounds, "bottom-right diagnostic in bounds");
    CHECK(bottomRight.x == 31 && bottomRight.y == 31, "bottom-right diagnostic coordinates preserved");

    const auto& outOfBounds = result.diagnostics[3];
    CHECK(!outOfBounds.in_bounds, "out-of-bounds diagnostic flagged");
    CHECK(outOfBounds.x == -1 && outOfBounds.y == 8, "out-of-bounds coordinates preserved");
}

// ============================================================================
// Test: Ray-setup dump captures full-frame pre-first-CalcDE state
// ============================================================================
static void test_ray_setup_dump_capture() {
    std::printf("  [ray_setup_dump_capture]\n");

    auto loadResult = mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    CHECK(loadResult.success, "load for ray-setup dump test");
    if (!loadResult.success) return;

    mb3d::RenderOptions opts;

    opts.precision = mb3d::PrecisionMode::Compatible;
    opts.width_override = 32;
    opts.height_override = 32;
    opts.thread_count = 1;
    opts.deterministic_mode = true;
    opts.no_shadows = true;
    opts.no_ao = true;
    opts.no_dof = true;
    opts.no_reflections = true;
    opts.ray_setup_dump_path = tempPath("renderer_stage1_contract.bin");

    auto result = mb3d::Render(loadResult.header, loadResult.addon, opts);
    CHECK(result.success, "render for ray-setup dump test");
    if (!result.success) return;

    const size_t expectedCount = static_cast<size_t>(result.width) * static_cast<size_t>(result.height);
    CHECK(result.ray_setup_records.size() == expectedCount, "ray-setup dump covers full frame");
    if (result.ray_setup_records.size() != expectedCount) return;

    size_t rayReadyCount = 0;
    size_t calcdeCount = 0;
    size_t noCalcdeCount = 0;
    for (size_t i = 0; i < result.ray_setup_records.size(); ++i) {
        const auto& rec = result.ray_setup_records[i];
        const int expectedX = static_cast<int>(i % static_cast<size_t>(result.width));
        const int expectedY = static_cast<int>(i / static_cast<size_t>(result.width));
        CHECK(rec.x == expectedX && rec.y == expectedY, "ray-setup dump preserves pixel coordinates");
        if (rec.ray_ready_captured != 0) ++rayReadyCount;
        if (rec.calcde_enter_captured != 0) {
            ++calcdeCount;
        } else {
            ++noCalcdeCount;
        }
    }

    CHECK(rayReadyCount == expectedCount, "ray-setup dump captures ray_ready for every pixel");
    CHECK(calcdeCount + noCalcdeCount == expectedCount,
          "every pixel records either first CalcDE entry or an explicit no-CalcDE outcome");

    const auto& center = result.ray_setup_records[(result.height / 2) * result.width + (result.width / 2)];
    CHECK(center.seed_before_first_step != 0, "ray-setup dump records deterministic seed");
    CHECK(std::isfinite(center.cafx) && std::isfinite(center.cafy), "ray-setup dump records CAF values");
    CHECK(std::isfinite(center.mvgrads_fov[0]) && std::isfinite(center.mvgrads_fov[1]) &&
          std::isfinite(center.mvgrads_fov[2]), "ray-setup dump records view direction");
    CHECK(center.mvgrads_fov[0] != 0.0 || center.mvgrads_fov[1] != 0.0 || center.mvgrads_fov[2] != 0.0,
          "ray-setup dump view direction is non-zero");
    CHECK(std::isfinite(center.ray_ready_start[0]) && std::isfinite(center.ray_ready_start[1]) &&
          std::isfinite(center.ray_ready_start[2]), "ray-setup dump records ray start position");
    CHECK(center.calcde_enter_captured != 0,
          "center pixel reaches first CalcDE in reference scene");
    CHECK(std::isfinite(center.calcde_enter_pos[0]) && std::isfinite(center.calcde_enter_pos[1]) &&
          std::isfinite(center.calcde_enter_pos[2]), "ray-setup dump records first CalcDE position");
}

// ============================================================================
// Test: Hybrid option normalization matches Pascal helper logic
// ============================================================================
static void test_hybrid_option_normalization() {
    std::printf("  [hybrid_option_normalization]\n");

    TMandHeader10 header{};
    header.Width = 480;
    header.Height = 360;
    header.Iterations = 20;
    header.MinimumIterations = 1;
    header.iMaxItsF2 = 20;
    header.RStop = 16.0;
    header.dFOVy = 45.0;
    header.bCalc3D = 1;
    header.sDEstop = 0.001f;
    header.mZstepDiv = 1.0f;
    header.sRaystepLimiter = 1.0f;

    THeaderCustomAddon addon{};
    addon.Formulas[0].iFnr = 0;
    addon.Formulas[0].iItCount = 3;
    addon.Formulas[3].iFnr = 0;
    addon.Formulas[3].iItCount = 4;
    addon.bOptions1 = 0;
    addon.bHybOpt1 = static_cast<uint8_t>(5 << 4);
    addon.bHybOpt2 = static_cast<uint16_t>(4 | (5 << 4) | (6 << 8));

    auto alternating = mb3d::GetMCTparasFromHeader(header, &addon, 0);
    CHECK(alternating.bMCTisValid != 0, "alternating hybrid is valid");
    CHECK(addon.bHybOpt1 == 0x33, "alternating bHybOpt1 normalized like Pascal");
    CHECK(addon.bHybOpt2 == 0x333, "alternating bHybOpt2 normalized like Pascal");
    CHECK(alternating.wEndTo == 3, "alternating end1 matches normalized slot");
    CHECK(alternating.StartFrom2 == 3, "alternating start2 matches normalized slot");
    CHECK(alternating.iEnd2 == 3, "alternating end2 matches normalized slot");
    CHECK(alternating.RepeatFrom1 == 3, "alternating repeat1 matches normalized slot");
    CHECK(alternating.RepeatFrom2 == 3, "alternating repeat2 matches normalized slot");

    THeaderCustomAddon interpAddon{};
    interpAddon.Formulas[0].iFnr = 0;
    interpAddon.Formulas[0].iItCount = 0x3f000000;
    interpAddon.Formulas[1].iFnr = 0;
    interpAddon.Formulas[1].iItCount = 0x3f000000;
    interpAddon.bOptions1 = 1;
    interpAddon.bHybOpt1 = 0xff;
    interpAddon.bHybOpt2 = 0xffff;

    auto interpolating = mb3d::GetMCTparasFromHeader(header, &interpAddon, 0);
    CHECK(interpolating.bMCTisValid != 0, "interpolating hybrid is valid");
    CHECK(interpAddon.bHybOpt1 == 0x00, "interpolating bHybOpt1 forced to Pascal layout");
    CHECK(interpAddon.bHybOpt2 == 0x151, "interpolating bHybOpt2 forced to Pascal layout");
    CHECK(interpolating.wEndTo == 1, "interpolating end1 promoted to formula 1");
    CHECK(interpolating.StartFrom2 == 1, "interpolating start2 matches Pascal layout");
    CHECK(interpolating.iEnd2 == 5, "interpolating end2 matches Pascal layout");
    CHECK(interpolating.RepeatFrom2 == 1, "interpolating repeat2 matches Pascal layout");

    THeaderCustomAddon decombSingle{};
    decombSingle.Formulas[0].iFnr = 0;
    decombSingle.Formulas[0].iItCount = 3;
    decombSingle.bOptions1 = 2;
    decombSingle.bHybOpt1 = 0;
    decombSingle.bHybOpt2 = 1;

    auto decombInvalid = mb3d::GetMCTparasFromHeader(header, &decombSingle, 0);
    CHECK(decombInvalid.bMCTisValid == 0, "DE-comb with single formula stays invalid like Pascal");
}

static void test_parity_trace_capture() {
    std::printf("  [parity_trace_capture]\n");

    auto loadResult = mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    CHECK(loadResult.success, "load for parity trace test");
    if (!loadResult.success) return;

    mb3d::RenderOptions opts;

    opts.precision = mb3d::PrecisionMode::Compatible;
    opts.width_override = 32;
    opts.height_override = 32;
    opts.thread_count = 1;
    opts.no_shadows = true;
    opts.no_ao = true;
    opts.no_dof = true;
    opts.no_reflections = true;
    opts.diag_points = {{16, 16}};
    opts.parity_trace_dump_path = "enabled";

    auto result = mb3d::Render(loadResult.header, loadResult.addon, opts);
    CHECK(result.success, "render for parity trace test");
    if (!result.success) return;

    CHECK(!result.parity_trace_lines.empty(), "parity trace lines captured");
    if (result.parity_trace_lines.empty()) return;

    auto hasStage = [&](const char* needle) {
        return std::any_of(result.parity_trace_lines.begin(), result.parity_trace_lines.end(),
                           [&](const std::string& line) { return line.find(needle) != std::string::npos; });
    };

    CHECK(hasStage("stage=mct_ready"), "parity trace has mct_ready");
    CHECK(hasStage("stage=it3d_init"), "parity trace has it3d_init");
    CHECK(hasStage("stage=pixel_enter"), "parity trace has pixel_enter");
    CHECK(hasStage("stage=ray_ready"), "parity trace has ray_ready");
    CHECK(hasStage("stage=calcde_enter"), "parity trace has calcde_enter");
    CHECK(hasStage("stage=formula_pre"), "parity trace has formula_pre");
    CHECK(hasStage("stage=formula_post"), "parity trace has formula_post");
}

static void test_parity_trace_stage_session_handoff() {
    std::printf("  [parity_trace_stage_session_handoff]\n");

    mb3d::ResetParityTraceStageRange();
    mb3d::ConfigureParityTraceStageRange("pixel_enter", "ray_ready");
    auto session = mb3d::CaptureParityTraceStageRangeState();

    CHECK(session != nullptr, "parity trace session captured");
    CHECK(!mb3d::ShouldLogParityStage("mct_ready"), "pre-start stage filtered in source thread");

    bool workerPixelEnter = false;
    bool workerRayReady = false;
    bool workerCalcdeEnter = true;
    std::thread worker([&]() {
        mb3d::RestoreParityTraceStageRangeState(session);
        workerPixelEnter = mb3d::ShouldLogParityStage("pixel_enter");
        workerRayReady = mb3d::ShouldLogParityStage("ray_ready");
        workerCalcdeEnter = mb3d::ShouldLogParityStage("calcde_enter");
    });
    worker.join();

    CHECK(workerPixelEnter, "restored worker session reaches start stage");
    CHECK(workerRayReady, "restored worker session logs configured end stage");
    CHECK(!workerCalcdeEnter, "restored worker session stops after end stage");
    CHECK(!mb3d::ShouldLogParityStage("calcde_enter"),
          "source thread sees worker-updated shared session state");

    mb3d::ResetParityTraceStageRange();
    mb3d::ConfigureParityTraceStageRange("formula_pre", "");
    CHECK(!mb3d::ShouldLogParityStage("pixel_enter"), "fresh session is not contaminated by prior run");
    CHECK(mb3d::ShouldLogParityStage("formula_pre"), "fresh session reaches its own start stage");

    mb3d::ResetParityTraceStageRange();
    std::vector<std::string> lines;
    mb3d::SetParityTraceContext(4, 5, &lines);
    mb3d::AppendParityTrace("formula_pre", "value=1");
    mb3d::ClearParityTraceContext();
    mb3d::AppendParityTrace("formula_post", "value=2");

    CHECK(lines.size() == 1, "cleared parity trace context stops appending to prior sink");
    if (lines.size() == 1) {
        CHECK(lines[0].find("stage=formula_pre x=4 y=5 value=1") != std::string::npos,
              "parity trace line keeps pixel context after session refactor");
    }
}

static void test_parity_trace_render_stage_window() {
    std::printf("  [parity_trace_render_stage_window]\n");

    auto loadResult = mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    CHECK(loadResult.success, "load for parity trace render window test");
    if (!loadResult.success) return;

    mb3d::RenderOptions opts;

    opts.precision = mb3d::PrecisionMode::Compatible;
    opts.width_override = 32;
    opts.height_override = 32;
    opts.thread_count = 1;
    opts.no_shadows = true;
    opts.no_ao = true;
    opts.no_dof = true;
    opts.no_reflections = true;
    opts.diag_points = {{16, 16}};
    opts.parity_trace_dump_path = "enabled";
    opts.parity_trace_start_stage = "ray_ready";
    opts.parity_trace_end_stage = "paint_pre";

    mb3d::ResetParityTraceStageRange();
    mb3d::ConfigureParityTraceStageRange(opts.parity_trace_start_stage,
                                         opts.parity_trace_end_stage);
    opts.parity_trace_stage_state = mb3d::CaptureParityTraceStageRangeState();

    auto result = mb3d::Render(loadResult.header, loadResult.addon, opts);
    CHECK(result.success, "render for parity trace render window test");
    if (!result.success) return;

    auto hasStage = [&](const char* needle) {
        return std::any_of(result.parity_trace_lines.begin(), result.parity_trace_lines.end(),
                           [&](const std::string& line) { return line.find(needle) != std::string::npos; });
    };

    CHECK(!hasStage("stage=mct_ready"), "stage window excludes pre-start mct_ready");
    CHECK(!hasStage("stage=it3d_init"), "stage window excludes pre-start it3d_init");
    CHECK(!hasStage("stage=pixel_enter"), "stage window excludes pre-start pixel_enter");
    CHECK(hasStage("stage=ray_ready"), "stage window starts at ray_ready inside render worker");
    CHECK(hasStage("stage=calcde_enter"), "stage window continues after render worker handoff");
    CHECK(hasStage("stage=paint_pre"), "stage window reaches color pass end stage");
    CHECK(!hasStage("stage=paint_result"), "stage window end stage suppresses later color-pass lines");
}

static void test_paint_replay_restores_completed_hard_shadow_state() {
    std::printf("  [paint_replay_restores_completed_hard_shadow_state]\n");

    std::string scene = comparisonScenePath("test_11_shadows.m3p");
    auto loadResult = mb3d::LoadM3PFile(scene);
    CHECK(loadResult.success, "load test_11_shadows for hard-shadow replay test");
    if (!loadResult.success) return;

    auto renderHeader = loadResult.header;
    auto renderAddon = loadResult.addon;

    mb3d::RenderOptions renderOpts;

    renderOpts.precision = mb3d::PrecisionMode::Compatible;
    renderOpts.thread_count = 1;
    renderOpts.no_shadows = false;
    renderOpts.no_ao = false;
    renderOpts.no_dof = true;
    renderOpts.no_reflections = true;

    auto render = mb3d::Render(renderHeader, renderAddon, renderOpts);
    CHECK(render.success, "render test_11_shadows for hard-shadow replay test");
    if (!render.success) return;

    auto replayHeader = loadResult.header;
    mb3d::RenderOptions replayOpts;
    replayOpts.precision = mb3d::PrecisionMode::Compatible;
    replayOpts.thread_count = 1;
    replayOpts.no_shadows = false;
    replayOpts.no_ao = false;
    replayOpts.no_dof = true;
    replayOpts.no_reflections = true;
    replayOpts.diag_points = {{78, 225}};
    replayOpts.parity_trace_dump_path = "enabled";
    replayOpts.parity_trace_start_stage = "paint_pre";
    replayOpts.parity_trace_end_stage = "paint_result";

    mb3d::ResetParityTraceStageRange();
    mb3d::ConfigureParityTraceStageRange(replayOpts.parity_trace_start_stage,
                                         replayOpts.parity_trace_end_stage);
    replayOpts.parity_trace_stage_state = mb3d::CaptureParityTraceStageRangeState();

    auto replay = mb3d::PaintFromSiLight(replayHeader, loadResult.addon, render.silight, replayOpts);
    CHECK(replay.success, "paint replay succeeds for hard-shadow state test");
    if (!replay.success) return;

#ifdef MB3D_ENABLE_PARITY_TRACE
    auto it = std::find_if(replay.parity_trace_lines.begin(), replay.parity_trace_lines.end(),
                           [](const std::string& line) {
                               return line.find("stage=paint_light0") != std::string::npos;
                           });
    CHECK(it != replay.parity_trace_lines.end(), "paint replay captured paint_light0 trace");

#endif

    const size_t pixelIndex = (static_cast<size_t>(225) * render.width + 78) * 3;
    CHECK(pixelIndex + 2 < render.image.size(), "hard-shadow replay hotspot is in bounds for render image");
    CHECK(pixelIndex + 2 < replay.image.size(), "hard-shadow replay hotspot is in bounds for replay image");
    if (pixelIndex + 2 < render.image.size() && pixelIndex + 2 < replay.image.size()) {
        CHECK(render.image[pixelIndex + 0] == replay.image[pixelIndex + 0] &&
              render.image[pixelIndex + 1] == replay.image[pixelIndex + 1] &&
              render.image[pixelIndex + 2] == replay.image[pixelIndex + 2],
              "hard-shadow replay hotspot matches native render");
    }
}

static void test_paint_replay_does_not_force_hard_shadow_state_without_shadow_pass() {
    std::printf("  [paint_replay_does_not_force_hard_shadow_state_without_shadow_pass]\n");

    std::string scene = comparisonScenePath("test_13_ao_threshold.m3p");
    auto loadResult = mb3d::LoadM3PFile(scene);
    CHECK(loadResult.success, "load test_13_ao_threshold for AO replay gating test");
    if (!loadResult.success) return;

    auto renderHeader = loadResult.header;
    auto renderAddon = loadResult.addon;

    mb3d::RenderOptions renderOpts;

    renderOpts.precision = mb3d::PrecisionMode::Compatible;
    renderOpts.thread_count = 1;
    renderOpts.no_shadows = false;
    renderOpts.no_ao = false;
    renderOpts.no_dof = true;
    renderOpts.no_reflections = true;

    auto render = mb3d::Render(renderHeader, renderAddon, renderOpts);
    CHECK(render.success, "render test_13_ao_threshold for AO replay gating test");
    if (!render.success) return;

    auto replayHeader = loadResult.header;
    mb3d::RenderOptions replayOpts;
    replayOpts.precision = mb3d::PrecisionMode::Compatible;
    replayOpts.thread_count = 1;
    replayOpts.no_shadows = false;
    replayOpts.no_ao = false;
    replayOpts.no_dof = true;
    replayOpts.no_reflections = true;
    replayOpts.diag_points = {{313, 51}};
    replayOpts.parity_trace_dump_path = "enabled";
    replayOpts.parity_trace_start_stage = "paint_pre";
    replayOpts.parity_trace_end_stage = "paint_result";

    mb3d::ResetParityTraceStageRange();
    mb3d::ConfigureParityTraceStageRange(replayOpts.parity_trace_start_stage,
                                         replayOpts.parity_trace_end_stage);
    replayOpts.parity_trace_stage_state = mb3d::CaptureParityTraceStageRangeState();

    auto replay = mb3d::PaintFromSiLight(replayHeader, loadResult.addon, render.silight, replayOpts);

    CHECK(replay.success, "paint replay succeeds for AO replay gating test");
    if (!replay.success) return;

#ifdef MB3D_ENABLE_PARITY_TRACE
    auto preIt = std::find_if(replay.parity_trace_lines.begin(), replay.parity_trace_lines.end(),
                              [](const std::string& line) {
                                  return line.find("stage=paint_pre") != std::string::npos;
                              });
    CHECK(preIt != replay.parity_trace_lines.end(), "AO replay gating trace captured paint_pre");
    if (preIt != replay.parity_trace_lines.end()) {
        CHECK(preIt->find("LightFunc0=3") != std::string::npos,
              "AO replay keeps original light function when hard-shadow pass is skipped");
    }

    auto lightIt = std::find_if(replay.parity_trace_lines.begin(), replay.parity_trace_lines.end(),
                                [](const std::string& line) {
                                    return line.find("stage=paint_light0") != std::string::npos;
                                });
    CHECK(lightIt != replay.parity_trace_lines.end(), "AO replay gating trace captured paint_light0");
    if (lightIt != replay.parity_trace_lines.end()) {
        CHECK(lightIt->find("bSubAmbSh=1") != std::string::npos,
              "AO replay does not force completed hard-shadow state without shadow pass");
    }

#endif

    const size_t pixelIndex = (static_cast<size_t>(51) * render.width + 313) * 3;
    CHECK(pixelIndex + 2 < render.image.size(), "AO replay gating hotspot is in bounds for render image");
    CHECK(pixelIndex + 2 < replay.image.size(), "AO replay gating hotspot is in bounds for replay image");
    if (pixelIndex + 2 < render.image.size() && pixelIndex + 2 < replay.image.size()) {
        CHECK(render.image[pixelIndex + 0] == replay.image[pixelIndex + 0] &&
              render.image[pixelIndex + 1] == replay.image[pixelIndex + 1] &&
              render.image[pixelIndex + 2] == replay.image[pixelIndex + 2],
              "AO replay hotspot matches native render when hard-shadow pass is skipped");
    }
}

static void test_paint_replay_reproduces_reflection_late_pass() {
    std::printf("  [paint_replay_reproduces_reflection_late_pass]\n");

    std::string scene = comparisonScenePath("test_15_reflections.m3p");
    auto loadResult = mb3d::LoadM3PFile(scene);
    CHECK(loadResult.success, "load test_15_reflections for reflection replay test");
    if (!loadResult.success) return;

    auto renderHeader = loadResult.header;
    auto renderAddon = loadResult.addon;

    mb3d::RenderOptions renderOpts;

    renderOpts.precision = mb3d::PrecisionMode::Compatible;
    renderOpts.thread_count = 1;
    renderOpts.no_shadows = false;
    renderOpts.no_ao = false;
    renderOpts.no_dof = true;
    renderOpts.no_reflections = false;

    auto render = mb3d::Render(renderHeader, renderAddon, renderOpts);
    CHECK(render.success, "render test_15_reflections for reflection replay test");
    if (!render.success) return;

    auto replayHeader = loadResult.header;
    auto replayAddon = loadResult.addon;
    mb3d::RenderOptions replayOpts;
    replayOpts.precision = mb3d::PrecisionMode::Compatible;
    replayOpts.thread_count = 1;
    replayOpts.no_shadows = false;
    replayOpts.no_ao = false;
    replayOpts.no_dof = true;
    replayOpts.no_reflections = false;
    replayOpts.diag_points = {{313, 51}};

    auto replay = mb3d::PaintFromSiLight(replayHeader, replayAddon, render.silight, replayOpts);
    CHECK(replay.success, "paint replay succeeds for reflection replay test");
    if (!replay.success) return;

    CHECK(render.image.size() == replay.image.size(),
          "reflection replay image size matches native render");
    if (render.image.size() == replay.image.size()) {
        CHECK(std::equal(render.image.begin(), render.image.end(), replay.image.begin()),
              "reflection replay reproduces full native RGB image");
    }
}

static void test_paint_replay_diag_only_traces_reflection_row_window() {
    std::printf("  [paint_replay_diag_only_traces_reflection_row_window]\n");

    std::string scene = comparisonScenePath("test_15_reflections.m3p");
    auto loadResult = mb3d::LoadM3PFile(scene);
    CHECK(loadResult.success, "load test_15_reflections for diag-only reflection replay trace test");
    if (!loadResult.success) return;

    auto renderHeader = loadResult.header;
    auto renderAddon = loadResult.addon;

    mb3d::RenderOptions renderOpts;

    renderOpts.precision = mb3d::PrecisionMode::Compatible;
    renderOpts.thread_count = 1;
    renderOpts.no_shadows = false;
    renderOpts.no_ao = false;
    renderOpts.no_dof = true;
    renderOpts.no_reflections = false;

    auto render = mb3d::Render(renderHeader, renderAddon, renderOpts);
    CHECK(render.success, "render test_15_reflections for diag-only reflection replay trace test");
    if (!render.success) return;

    auto replayHeader = loadResult.header;
    auto replayAddon = loadResult.addon;
    mb3d::RenderOptions replayOpts;
    replayOpts.precision = mb3d::PrecisionMode::Compatible;
    replayOpts.thread_count = 1;
    replayOpts.no_shadows = false;
    replayOpts.no_ao = false;
    replayOpts.no_dof = true;
    replayOpts.no_reflections = false;
    replayOpts.diag_only = true;
    replayOpts.diag_points = {{153, 128}};
    replayOpts.parity_trace_dump_path = "enabled";
    replayOpts.parity_trace_start_stage = "sr_primary_inputs";
    replayOpts.parity_trace_end_stage = "sr_primary";
    mb3d::ConfigureParityTraceStageRange(replayOpts.parity_trace_start_stage,
                                         replayOpts.parity_trace_end_stage);
    replayOpts.parity_trace_stage_state = mb3d::CaptureParityTraceStageRangeState();

    auto replay = mb3d::PaintFromSiLight(replayHeader, replayAddon, render.silight, replayOpts);
    CHECK(replay.success, "diag-only reflection replay succeeds");
    if (!replay.success) return;

    auto hasStage = [&](const char* stage) {
        return std::any_of(replay.parity_trace_lines.begin(), replay.parity_trace_lines.end(),
                           [&](const std::string& line) {
                               return line.find(std::string("stage=") + stage) != std::string::npos;
                           });
    };
    CHECK(hasStage("sr_primary_inputs"), "diag-only reflection replay captured sr_primary_inputs");
    CHECK(hasStage("sr_primary"), "diag-only reflection replay captured sr_primary");
}

static void test_paint_replay_diag_only_traces_reflection_sr_ray_enter_window() {
    std::printf("  [paint_replay_diag_only_traces_reflection_sr_ray_enter_window]\n");

    std::string scene = comparisonScenePath("test_15_reflections.m3p");
    auto loadResult = mb3d::LoadM3PFile(scene);
    CHECK(loadResult.success, "load test_15_reflections for sr_ray_enter replay trace test");
    if (!loadResult.success) return;

    mb3d::RenderOptions renderOpts;

    renderOpts.precision = mb3d::PrecisionMode::Compatible;
    renderOpts.thread_count = 1;
    renderOpts.no_shadows = false;
    renderOpts.no_ao = false;
    renderOpts.no_dof = true;
    renderOpts.no_reflections = false;

    auto render = mb3d::Render(loadResult.header, loadResult.addon, renderOpts);
    CHECK(render.success, "render test_15_reflections for sr_ray_enter replay trace test");
    if (!render.success) return;

    auto replayHeader = loadResult.header;
    auto replayAddon = loadResult.addon;
    mb3d::RenderOptions replayOpts;
    replayOpts.precision = mb3d::PrecisionMode::Compatible;
    replayOpts.thread_count = 1;
    replayOpts.no_shadows = false;
    replayOpts.no_ao = false;
    replayOpts.no_dof = true;
    replayOpts.no_reflections = false;
    replayOpts.diag_only = true;
    replayOpts.diag_points = {{153, 128}};
    replayOpts.parity_trace_dump_path = "enabled";
    replayOpts.parity_trace_start_stage = "sr_primary";
    replayOpts.parity_trace_end_stage = "sr_ray_enter";
    mb3d::ConfigureParityTraceStageRange(replayOpts.parity_trace_start_stage,
                                         replayOpts.parity_trace_end_stage);
    replayOpts.parity_trace_stage_state = mb3d::CaptureParityTraceStageRangeState();

    auto replay = mb3d::PaintFromSiLight(replayHeader, replayAddon, render.silight, replayOpts);
    CHECK(replay.success, "diag-only reflection replay succeeds for sr_ray_enter window");
    if (!replay.success) return;

    auto hasStage = [&](const char* stage) {
        return std::any_of(replay.parity_trace_lines.begin(), replay.parity_trace_lines.end(),
                           [&](const std::string& line) {
                               return line.find(std::string("stage=") + stage) != std::string::npos;
                           });
    };
    CHECK(hasStage("sr_primary"), "diag-only reflection replay captured sr_primary");
    CHECK(hasStage("sr_ray_enter"), "diag-only reflection replay captured sr_ray_enter");
}

static void test_reflection_primary_trace_matches_pascal_hotspot() {
    std::printf("  [reflection_primary_trace_matches_pascal_hotspot]\n");

    std::string scene = comparisonScenePath("test_15_reflections.m3p");
    auto loadResult = mb3d::LoadM3PFile(scene);
    CHECK(loadResult.success, "load test_15_reflections for reflection hotspot trace test");
    if (!loadResult.success) return;

    mb3d::RenderOptions opts;

    opts.precision = mb3d::PrecisionMode::Compatible;
    opts.thread_count = 1;
    opts.no_shadows = false;
    opts.no_ao = false;
    opts.no_dof = true;
    opts.no_reflections = false;
    opts.deterministic_mode = true;
    opts.diag_only = true;
    opts.diag_points = {{295, 2}};
    opts.parity_trace_dump_path = "enabled";
    opts.parity_trace_start_stage = "sr_primary_inputs";
    opts.parity_trace_end_stage = "sr_ray_enter";
    mb3d::ConfigureParityTraceStageRange(opts.parity_trace_start_stage,
                                         opts.parity_trace_end_stage);
    opts.parity_trace_stage_state = mb3d::CaptureParityTraceStageRangeState();

    auto render = mb3d::Render(loadResult.header, loadResult.addon, opts);
    CHECK(render.success, "diag-only reflection hotspot render succeeds");
    if (!render.success) return;

    auto findStageLine = [&](const char* stage, const char* extraNeedle = nullptr) {
        const std::string stageNeedle = std::string("stage=") + stage;
        for (const auto& line : render.parity_trace_lines) {
            std::size_t stagePos = line.find(stageNeedle);
            if (stagePos == std::string::npos) continue;
            std::size_t stageEnd = stagePos + stageNeedle.size();
            if (stageEnd < line.size() && line[stageEnd] != ' ') continue;
            if (extraNeedle && line.find(extraNeedle) == std::string::npos) continue;
            return line;
        }
        return std::string();
    };
    auto fieldValue = [](const std::string& line, const char* key) {
        std::string needle = std::string(key) + "=";
        std::size_t start = line.find(needle);
        if (start == std::string::npos) return std::string();
        start += needle.size();
        std::size_t end = line.find(' ', start);
        return line.substr(start, end == std::string::npos ? std::string::npos : end - start);
    };
    auto fieldDouble = [&](const std::string& line, const char* key) {
        const std::string value = fieldValue(line, key);
        return value.empty() ? 0.0 : std::stod(value);
    };

    const std::string primaryInputs = findStageLine("sr_primary_inputs");
    const std::string primary = findStageLine("sr_primary");
    const std::string rayEnter1 = findStageLine("sr_ray_enter", "Rit=1");

    CHECK(!primaryInputs.empty(), "reflection hotspot trace captured sr_primary_inputs");
    CHECK(!primary.empty(), "reflection hotspot trace captured sr_primary");
    CHECK(!rayEnter1.empty(), "reflection hotspot trace captured first sr_ray_enter");
    if (primaryInputs.empty() || primary.empty() || rayEnter1.empty()) return;

    CHECK_NEAR(fieldDouble(primaryInputs, "ViewVecX"), 0.081341035664, 1e-12,
               "reflection hotspot ViewVecX matches Pascal trace");
    CHECK_NEAR(fieldDouble(primaryInputs, "ViewVecY"), -0.255952715874, 1e-12,
               "reflection hotspot ViewVecY matches Pascal trace");
    CHECK_NEAR(fieldDouble(primaryInputs, "PreDepthColZ"), 139.644439697266, 1e-9,
               "reflection hotspot PreDepthColZ matches Pascal trace");

    CHECK_NEAR(fieldDouble(primary, "tAmbY"), 0.599268257618, 1e-12,
               "reflection hotspot sr_primary tAmbY matches Pascal trace");
    CHECK_NEAR(fieldDouble(primary, "SD0X"), 0.714034557343, 1e-12,
               "reflection hotspot sr_primary SD0X matches Pascal trace");
    CHECK_NEAR(fieldDouble(primary, "SD0Y"), 0.736714243889, 1e-12,
               "reflection hotspot sr_primary SD0Y matches Pascal trace");
    CHECK_NEAR(fieldDouble(primary, "SD0Z"), 0.627060234547, 1e-12,
               "reflection hotspot sr_primary SD0Z matches Pascal trace");

    CHECK_NEAR(fieldDouble(rayEnter1, "ZZ"), 281.372925552544, 1e-12,
               "reflection hotspot first sr_ray_enter ZZ matches Pascal trace");
    CHECK_NEAR(fieldDouble(rayEnter1, "AbsorbX"), 0.500000000000, 1e-12,
               "reflection hotspot first sr_ray_enter AbsorbX matches Pascal trace");
}

static void test_ao_threshold_replay_clears_background_quartet_lanes() {
    std::printf("  [ao_threshold_replay_clears_background_quartet_lanes]\n");

    auto loadResult = mb3d::LoadM3PFile(comparisonScenePath("test_13_ao_threshold.m3p"));
    CHECK(loadResult.success, "load AO threshold scene for quartet replay test");
    if (!loadResult.success) return;

    mb3d::RenderOptions opts;

    opts.precision = mb3d::PrecisionMode::Compatible;
    opts.thread_count = 1;
    opts.no_ao = true;
    opts.no_dof = true;
    opts.no_reflections = true;

    auto render = mb3d::Render(loadResult.header, loadResult.addon, opts);
    CHECK(render.success, "render AO threshold no-AO baseline for quartet replay test");
    if (!render.success) return;

    const int x = 292;
    const int y = 0;
    const size_t idx = static_cast<size_t>(y) * render.width + x;
    CHECK(idx < render.silight.size(), "AO threshold quartet hotspot is in bounds");
    if (idx >= render.silight.size()) return;

    CHECK(render.silight[idx].Zpos >= 32768, "AO threshold quartet hotspot stays background pre-replay");
    CHECK(render.silight[idx].AmbShadow == 5000, "AO threshold quartet hotspot keeps pre-AO AmbShadow before replay");

    auto replay = render.silight;
    auto mct = mb3d::GetMCTparasFromHeader(loadResult.header, &loadResult.addon, 0);
    CHECK(mct.bMCTisValid != 0, "AO threshold quartet replay MCT setup valid");

    TLightVals baseLightVals{};
    TLValigned baseAlignedStorage{};
    baseLightVals.PLValigned = &baseAlignedStorage;
    mb3d::MakeLightValsFromHeaderLight(&loadResult.header, &baseLightVals, 1.0, 0);
    mb3d::CompleteLightValsSetup(&loadResult.header, &baseLightVals);

    const int aoType = (loadResult.header.bCalcAmbShadowAutomatic >> 2) & 3;
    const int aoQuality = (loadResult.header.bCalcAmbShadowAutomatic >> 4) & 3;
    CHECK(((loadResult.header.bCalcAmbShadowAutomatic >> 1) & 7) == 0,
          "AO threshold quartet scene uses Type0 threshold AO");

    mb3d::ReplayAmbientOcclusion(replay.data(), render.width, render.height,
                                 render.width * static_cast<int>(sizeof(TsiLight5)),
                                 &mct, &baseLightVals, aoType, aoQuality, &loadResult.header, 1);

    CHECK(replay[idx].Zpos >= 32768, "AO threshold quartet hotspot stays background after replay");
    CHECK(replay[idx].AmbShadow == 0,
          "AO threshold quartet replay clears background AmbShadow inside active quartet");
}

static void captureInitialHybridRadius(double&, double&, double&, double&, void* raw) {
    auto* ext = reinterpret_cast<TIteration3Dext*>(reinterpret_cast<char*>(raw)
                                                - offsetof(TIteration3Dext, C1));
    ext->Dfree1 = ext->Rout;
}

static void test_hybrid_radius_double_rounding() {
    // Independently captured Pascal radii at the _IFS_tess p1-alternate
    // diagnostic pixel. An x87 sum differs by one ULP in each of these rows.
    const uint64_t samples[][4] = {
        {0xC00657E2A90D1FCDULL, 0xBFB32C549B091440ULL, 0xBFD54BD47FFA0C19ULL, 0x401FAAB2541AE0F0ULL},
        {0xC0719A1526123FBBULL, 0x408D3FF6AF1F8E7BULL, 0xC08DD379C2A13C8BULL, 0x413C7A6C28BB4958ULL},
        {0xC0008B143CAFBD1FULL, 0xC017057FEA268190ULL, 0x400BD4620ED2D6ACULL, 0x4048C03EC30FF340ULL},
    };
    for (const auto& sample : samples) {
        for (bool analyticDE : {false, true}) {
            TIteration3Dext ext{};
            std::memcpy(&ext.C1, &sample[0], 8);
            std::memcpy(&ext.C2, &sample[1], 8);
            std::memcpy(&ext.C3, &sample[2], 8);
            ext.maxIt = 1;
            ext.RStop = 256.0;
            ext.nHybrid[0] = 1;
            ext.fHybrid[0] = captureInitialHybridRadius;
            auto* pit = reinterpret_cast<TPIteration3D>(&ext.C1);
            if (analyticDE) mb3d::doHybridPasDE(pit);
            else mb3d::doHybridPas(pit);
            uint64_t initial, final;
            std::memcpy(&initial, &ext.Dfree1, 8);
            std::memcpy(&final, &ext.Rout, 8);
            CHECK(initial == sample[3], "initial hybrid radius matches Pascal Double arithmetic");
            CHECK(final == sample[3], "iterated hybrid radius matches Pascal Double arithmetic");
        }
    }
}

static void unorderedOrbitSlot(double& x, double& y, double& z, double& w, void*) {
    x = std::numeric_limits<double>::quiet_NaN();
    y = z = 0.0;
    w = 1.0;
}

static void test_unordered_orbit_reaches_maxit() {
    // Pascal repeat-until does not escape on NaN. The original renderer also
    // reaches 60 iterations for _setw with all coordinate weights zero.
    for (int driver = 0; driver < 8; ++driver) {
        TIteration3Dext ext{};
        ext.C1 = ext.C2 = ext.C3 = 1.0;
        ext.maxIt = 60;
        ext.RStop = 256.0;
        ext.nHybrid[0] = ext.nHybrid[1] = 1;
        ext.fHybrid[0] = ext.fHybrid[1] = unorderedOrbitSlot;
        for (int i = 0; i < 4; ++i) ext.SMatrix4[i][i] = 1.0f;
        if (driver >= 4) {
            float weight = 0.5f;
            std::memcpy(&ext.nHybrid[0], &weight, sizeof(weight));
            std::memcpy(&ext.nHybrid[1], &weight, sizeof(weight));
        }
        auto* pit = reinterpret_cast<TPIteration3D>(&ext.C1);
        switch (driver) {
        case 0: mb3d::doHybridPas(pit); break;
        case 1: mb3d::doHybridPasDE(pit); break;
        case 2: mb3d::doHybrid4DPas(pit); break;
        case 3: mb3d::doHybrid4DDEPas(pit); break;
        case 4: mb3d::doInterpolHybridPas(pit); break;
        case 5: mb3d::doInterpolHybridPasDE(pit); break;
        case 6: mb3d::doInterpolHybridPas4D(pit); break;
        case 7: mb3d::doInterpolHybridPas4DDE(pit); break;
        }
        CHECK(ext.ItResultI == 60, "unordered orbit reaches maximum iterations");
        CHECK(std::isnan(ext.Rout), "unordered orbit preserves NaN radius");
    }
}

static void unorderedIFSSubset(double&, double&, double&, double&, void* raw) {
    auto* pit = static_cast<TPIteration3D>(raw);
    auto* ext = reinterpret_cast<TIteration3Dext*>(reinterpret_cast<char*>(pit)
                                                - offsetof(TIteration3Dext, C1));
    ext->Rout = 0.0;
    ext->VaryScale = 0.0;
    ext->Dfree1 = ext->ItResultI + 0.25;
}

static void test_unordered_ifs_minimum_and_inside_flags() {
    // formulas.pas: UCOMISD/JNC accepts unordered minima in both drivers.
    // The initialized driver uses JS for inside, the noVecIni driver JNE.
    for (int driver = 0; driver < 2; ++driver) {
        for (int inside : {INT32_MIN, -1, 0, 1, INT32_MAX}) {
            TIteration3Dext ext{};
            ext.maxIt = 4;
            ext.RStopD = 0.001;
            ext.bIsInsideRender = inside;
            ext.nHybrid[0] = 1;
            ext.fHybrid[0] = unorderedIFSSubset;
            auto* pit = reinterpret_cast<TPIteration3D>(&ext.C1);
            const double result = driver ? mb3d::doHybridIFS3DnoVecIni(pit) : mb3d::doHybridIFS3D(pit);
            const int iterations = (driver ? inside != 0 : inside < 0) ? 4 : 1;
            CHECK(std::isnan(result), "IFS preserves unordered minimum");
            CHECK(ext.ItResultI == iterations, "IFS original signed inside branch");
            CHECK(ext.SmoothItD == iterations, "IFS minimum iteration for color");
            CHECK(ext.OTrap == iterations - 0.75, "IFS color belongs to final minimum");
        }
    }
}

// A controlled dIFS callback exposes the driver's carry state independently
// of any external formula. Ju1/Ju2 hold the requested DE/color.
static void controlledIFSMinimum(double&, double&, double&, double&, void* raw) {
    auto* pit = static_cast<TPIteration3D>(raw);
    auto* ext = reinterpret_cast<TIteration3Dext*>(reinterpret_cast<char*>(pit)
                                                - offsetof(TIteration3Dext, C1));
    ext->Rout = ext->Ju1;
    ext->Dfree1 = ext->Ju2;
}

#include "fixtures/iteration_step_regressions.h"
#include "fixtures/iteration_refine_regressions.h"

static const IterationRefineFixture* activeIterationFixture;
static int iterationFixtureCalls;
static double iterationFixtureDE(TPIteration3Dext it, void*) {
    const auto& fixture = *activeIterationFixture;
    uint32_t bits = fixture.samples[std::min(iterationFixtureCalls++, fixture.count - 1)];
    std::memcpy(&it->SmoothItD, &bits, sizeof(bits));
    return 0;
}

static void test_iteration_boundary_original_pascal() {
    for (const auto& fixture : kIterationStepFixtures) {
        float target, previous, current; double multiplier;
        std::memcpy(&target, &fixture.target, 4);
        std::memcpy(&previous, &fixture.previous, 4);
        std::memcpy(&current, &fixture.current, 4);
        std::memcpy(&multiplier, &fixture.multiplier, 8);
        double step = mb3d::CalcIterationBoundaryStep(target, previous, current, multiplier);
        uint64_t bits; std::memcpy(&bits, &step, 8);
        if (bits != fixture.expected)
            std::printf("  iteration target=%08x previous=%08x current=%08x multiplier=%016llx actual=%016llx expected=%016llx\n",
                fixture.target, fixture.previous, fixture.current,
                (unsigned long long)fixture.multiplier, (unsigned long long)bits,
                (unsigned long long)fixture.expected);
        CHECK(bits == fixture.expected, "iteration step matches original Pascal bits including NaN signs");
    }
    for (const auto& fixture : kIterationRefineFixtures) {
        activeIterationFixture = &fixture; iterationFixtureCalls = 0;
        TMCTparameter params{}; TIteration3Dext it{};
        params.pIt3Dext = &it; params.CalcDE = iterationFixtureDE;
        params.iMaxIt = params.iMaxitF2 = it.maxIt = fixture.maxIt;
        params.iDEAddSteps = fixture.steps;
        std::memcpy(&params.mZZ, &fixture.depth, 8);
        mb3d::RMdoBinSearchIt(&params, &params.mZZ);
        uint64_t depth; uint32_t smooth;
        std::memcpy(&depth, &params.mZZ, 8); std::memcpy(&smooth, &it.SmoothItD, 4);
        CHECK(depth == fixture.expectedDepth, "iteration refinement preserves original depth including signed NaNs");
        CHECK(smooth == fixture.expectedSmooth, "iteration refinement preserves original smoothing");
        CHECK(iterationFixtureCalls == fixture.calls, "iteration refinement follows original branch sequence");
        CHECK(it.maxIt == fixture.maxIt && params.iMaxIt == fixture.maxIt && params.iMaxitF2 == fixture.maxIt,
              "iteration refinement restores iteration limits");
    }
}

static void test_ifs_minimum_carry_between_calls() {
    // Original formulas.pas initializes bTmp only in noVecIni. Neither driver
    // resets Dfree2. A DE >= 65535 never replaces these carried values.
    for (int driver = 0; driver < 2; ++driver) {
        TIteration3Dext ext{};
        ext.maxIt = 3;
        ext.RStopD = 0.001;
        ext.nHybrid[0] = 1;
        ext.fHybrid[0] = controlledIFSMinimum;
        auto* pit = reinterpret_cast<TPIteration3D>(&ext.C1);
        auto call = [&]() { return driver ? mb3d::doHybridIFS3DnoVecIni(pit)
                                          : mb3d::doHybridIFS3D(pit); };
        ext.Ju1 = 0.25;
        ext.Ju2 = 0.75;
        CHECK(call() == 0.25, "IFS establishes a minimum");
        CHECK(ext.bTmp == 1 && ext.OTrap == 0.75, "IFS minimum carries iteration and color");
        for (double distance : {65535.0, 65536.0, std::numeric_limits<double>::infinity()}) {
            ext.Ju1 = distance;
            ext.Ju2 = 9.0;
            CHECK(call() == 65535.0, "IFS leaves initial distance when no minimum found");
            CHECK(ext.ItResultI == (driver ? 0 : 1), "IFS preserves original driver-specific iteration carry");
            CHECK(ext.SmoothItD == (driver ? 0.0f : 1.0f), "IFS color iteration survives missing minimum");
            CHECK(ext.OTrap == 0.75 && ext.Dfree2 == 0.75, "IFS retains previous minimum color");
        }
        ext.Ju1 = -0.5;
        ext.Ju2 = 2.5;
        CHECK(call() == -0.5, "IFS resumes minimum updates after missing minimum");
        CHECK(ext.ItResultI == 1 && ext.OTrap == 2.5, "IFS replaces retained state on new minimum");
    }
}

static void test_iteration_refinement_keeps_depth_and_color_mode() {
    auto loaded = mb3d::LoadM3PFile(comparisonScenePath(
        "../scenes_validation/state_writeback/challenge__updateC2_p3_zero.m3p"));
    CHECK(loaded.success, "load original iteration-boundary regression");
    if (!loaded.success) return;
    mb3d::RenderOptions opts;
    opts.precision = mb3d::PrecisionMode::Compatible;
    opts.thread_count = 1;
    opts.deterministic_mode = true;
    auto result = mb3d::Render(loaded.header, loaded.addon, opts);
    CHECK(result.success, "render iteration-boundary regression");
    if (!result.success) return;
    // Frozen original Pascal RGB. The first pixel detects lost refined depth;
    // the second detects recomputation of DElimited after normal probes.
    const int pixels[][5] = {{31, 32, 3, 9, 10}, {62, 14, 90, 230, 69}};
    for (const auto& pixel : pixels) {
        const size_t offset = (pixel[1] * result.width + pixel[0]) * 3;
        for (int channel = 0; channel < 3; ++channel)
            CHECK_NEAR(result.image[offset + channel], pixel[channel + 2], 2,
                       "iteration-boundary RGB matches original Pascal");
    }
}

static void test_otrap_calcde_uses_live_maxit() {
    TIteration3Dext ext{};
    ext.C1 = 1.0;
    ext.C2 = 1.0;
    ext.C3 = 0.0;
    ext.maxIt = 1;
    ext.RStop = 1000.0f;
    ext.iStartFrom = 0;
    ext.iRepeatFrom = 0;
    ext.EndTo = 0;
    ext.nHybrid[0] = 1;
    ext.fHybrid[0] = otrapHybridSlotTight;

    TMCTparameter mct{};
    mct.dDEscale = 1.0f;
    mct.iMaxIt = 9;
    mct.bCalcInside = 0;

    double result = mb3d::CalcOTrapDE(&ext, &mct);
    CHECK(result > 0.0, "CalcOTrapDE produces positive result");
    CHECK(mct.MaxItsResult == 2, "CalcOTrapDE uses live ext.maxIt for MaxItsResult");
}

static void test_otrap_calcde_noade_restores_smoothitd() {
    g_otrapGradientProbeCalls = 0;

    TIteration3Dext ext{};
    ext.C1 = 1.0;
    ext.C2 = 2.0;
    ext.C3 = 3.0;
    ext.ItResultI = 4;
    ext.maxIt = 9;
    ext.RStop = 2.0f;
    ext.Rout = 4.0;
    ext.SmoothItD = 77.0f;
    ext.CalcSIT = 1;
    ext.DEoption = 6;

    TMCTparameter mct{};
    mct.mMandFunction = otrapGradientProbe;
    mct.dDEscale = 1.0f;
    mct.dRStop = 2.0f;
    mct.msDEstop = 0.1f;
    mct.mctDEoffset = 0.5f;
    mct.mctDEoffset006 = 0.003f;
    mct.Vgrads[0][0] = 1.0; mct.Vgrads[0][1] = 0.0; mct.Vgrads[0][2] = 0.0;
    mct.Vgrads[1][0] = 0.0; mct.Vgrads[1][1] = 1.0; mct.Vgrads[1][2] = 0.0;
    mct.Vgrads[2][0] = 0.0; mct.Vgrads[2][1] = 0.0; mct.Vgrads[2][2] = 1.0;

    double result = mb3d::CalcOTrapDEnoADE(&ext, &mct);
    CHECK(result > 0.0, "CalcOTrapDEnoADE produces positive result");
    CHECK_NEAR(ext.SmoothItD, 101.0, 1e-12,
               "CalcOTrapDEnoADE restores SmoothItD from base pass after probes");
    CHECK(mct.MaxItsResult == 9, "CalcOTrapDEnoADE restores MaxItsResult from ext.maxIt");
}

static void test_otrap_calcde_full_runs_second_decomb_group() {
    TIteration3Dext ext{};
    ext.C1 = 1.0;
    ext.C2 = 1.0;
    ext.C3 = 0.0;
    ext.maxIt = 1;
    ext.RStop = 1000.0f;
    ext.iStartFrom = 0;
    ext.iRepeatFrom = 0;
    ext.EndTo = 0;
    ext.nHybrid[0] = 1;
    ext.nHybrid[1] = 1;
    ext.fHybrid[0] = otrapHybridSlotTight;
    ext.fHybrid[1] = otrapHybridSlotWide;

    TMCTparameter mct{};
    mct.pIt3Dext = &ext;
    mct.FormulaType = 1;
    mct.dDEscale = 1.0f;
    mct.dDEscale2 = 1.0f;
    mct.iMaxIt = 1;
    mct.iMaxitF2 = 1;
    mct.wEndTo = 0;
    mct.RepeatFrom1 = 0;
    mct.StartFrom1 = 0;
    mct.iEnd2 = 1;
    mct.RepeatFrom2 = 1;
    mct.StartFrom2 = 1;
    mct.bInsideRendering = 0;
    mct.bCalcInside = 0;

    double result = mb3d::CalcOTrapDEfull(&ext, &mct);
    CHECK_NEAR(result, 0.1, 1e-12,
               "CalcOTrapDEfull executes second DEcomb group for min combine");
    CHECK(mct.MaxItsResult == 2, "CalcOTrapDEfull keeps first-group MaxItsResult after min combine");
}

static void test_external_formula_map_slot_masks() {
    mb3d::InitExternalFormulaRegistry();

    const auto* cylinderHeightMap = mb3d::FindExternalFormula("CylinderHeightMap");
    CHECK(cylinderHeightMap != nullptr, "CylinderHeightMap registry entry exists");
    if (cylinderHeightMap) {
        CHECK(cylinderHeightMap->mapValueMask == 0x0003,
              "CylinderHeightMap exposes map number and map channel slots");
    }

    const auto* heightMapIFS = mb3d::FindExternalFormula("HeightMapIFS");
    CHECK(heightMapIFS != nullptr, "HeightMapIFS registry entry exists");
    if (heightMapIFS) {
        CHECK(heightMapIFS->mapValueMask == 0x0401,
              "HeightMapIFS includes map number and channel after three angle value slots");
    }

    const auto* mapTranslate = mb3d::FindExternalFormula("_MapTranslate");
    CHECK(mapTranslate != nullptr, "_MapTranslate registry entry exists");
    if (mapTranslate) {
        CHECK(mapTranslate->mapValueMask == 0x0001,
              "_MapTranslate marks its first integer slot as a map reference");
    }
}

// ============================================================================
// main
// ============================================================================
static void test_corrected_parity_image_hotspots() {
    struct Case { const char* scene; int x, y; int rgb[3]; };
    const Case cases[] = {
        {"../scenes_phase3_decomb/phase3_decomb_intpow-bulbox_v0_medium.m3p", 24, 151, {179,174,152}},
        {"../scenes_phase3_julia/phase3_julia_realpower_medium.m3p", 92, 152, {150,196,147}},
        {"ext_09_aboxmod1.m3p", 127, 71, {10,19,55}},
        {"ext_06_rotate.m3p", 100, 75, {163,138,140}},
        {"ext_05_flipxyc.m3p", 100, 75, {255,232,144}},
    };
    // RGB samples frozen from the independently rendered Pascal baseline.
    for (const auto& sample : cases) {
        auto loaded = mb3d::LoadM3PFile(comparisonScenePath(sample.scene));
        CHECK(loaded.success, "load corrected parity hotspot");
        if (!loaded.success) continue;
        mb3d::RenderOptions options;
        options.precision = mb3d::PrecisionMode::Compatible;
        options.thread_count = 16;
        options.deterministic_mode = true;
        auto result = mb3d::Render(loaded.header, loaded.addon, options);
        CHECK(result.success, "render corrected parity hotspot");
        if (!result.success) continue;
        const size_t index = (sample.y * result.width + sample.x) * 3;
        for (int channel = 0; channel < 3; ++channel)
            CHECK_NEAR(result.image[index + channel], sample.rgb[channel], 2,
                       "corrected surface/DOF/formula pixel matches Pascal");
    }
}

static void test_directional_volumetric_map_matches_pascal() {
    const auto scene = comparisonScenePath("../scenes_phase3_vollight/phase3_vl_bulbox_medium.m3p");
    auto loaded = mb3d::LoadM3PFile(scene);
    CHECK(loaded.success, "load directional volumetric Bulbox regression");
    if (!loaded.success) return;
    // Public renders release their private volume map when they return. Keep
    // an explicit low-level owner while sampling the same preparation kernel;
    // the isolation suite exercises full public volume renders separately.
    auto context = std::make_shared<mb3d::RenderContext>();
    mb3d::ScopedRenderContext binding(context);
    loaded.header.iOptions &= ~1;
    loaded.header.bCalcAmbShadowAutomatic &= static_cast<uint8_t>(~128u);
    auto mct = mb3d::GetMCTparasFromHeader(loaded.header, &loaded.addon, 0);
    CHECK(mct.bMCTisValid != 0, "directional volumetric MCT setup succeeds");
    if (!mct.bMCTisValid) return;
    TLightVals light{};
    TLValigned aligned{};
    light.PLValigned = &aligned;
    mb3d::MakeLightValsFromHeaderLight(&loaded.header, &light, 1.0, 0);
    mb3d::CompleteLightValsSetup(&loaded.header, &light);
    mb3d::CalcHSVecsFromLights(&light, &mct);
    TIteration3Dext iteration{};
    mct.pIt3Dext = &iteration;
    mb3d::IniIt3D(&mct, &iteration);
    mct.PLVals = &light;
    mb3d::CalcVolLightMap(&loaded.header, &light, &mct, 16);
    CHECK(context->volume_light_map.Initialized, "directional volume map is initialized");
    CHECK(context->volume_light_map.CubeSize == 815, "directional volume map keeps the Pascal dimensions");
    CHECK(context->volume_light_map.SideCount == 1, "directional volume map has one directional side");
    const TVec3D direction = {0, 0, 1};
    // Frozen Pascal map: 815x815, all cells have this exact Single value.
    // A double matrix reinterpreted as float produced NaNs and a blue frame.
    CHECK_NEAR(mb3d::GetVolLightMapValue(direction), 975.2835693359375, 0.0,
               "directional volume distance matches Pascal cell");
}

#include "core/ray_marcher.h"
#include "fixtures/depth_codec_regressions.h"

static void test_depth_codec_original_assembly() {
    for (const auto& row : kDepthCodecCases) {
        TMCTparameter parameters{};
        TsiLight5 light;
        std::memset(&light, 0xA5, sizeof(light));
        double depth;
        std::memcpy(&depth, &row.z, 8);
        std::memcpy(&parameters.Zcorr, &row.c, 8);
        std::memcpy(&parameters.ZcMul, &row.m, 8);
        std::memcpy(&parameters.sRoughness, &row.rough, 4);
        parameters.iSmNormals = row.smooth;
        mb3d::CalcZposAndRough(&light, &parameters, depth);
        uint32_t packed;
        std::memcpy(&packed, &light.RoughZposFine, sizeof(packed));
        CHECK(packed == row.expected, "depth/roughness matches original Pascal ASM");
        const auto* bytes = reinterpret_cast<const unsigned char*>(&light);
        bool intact = true;
        for (size_t i = 0; i < sizeof(light); ++i)
            if ((i < 6 || i >= 10) && bytes[i] != 0xA5) intact = false;
        CHECK(intact, "depth packing preserves adjacent fields");
    }
}

void test_volume_sampling_original() {
    size_t rowIndex=0, mismatches=0;
    for(const auto& row : kVolumeSamplingFixtures) {
        mb3d::TVolumeLightMap map{};
        map.CubeSize=row.size;map.HalfSize=row.size/2;map.SizeFactor=float(map.HalfSize);
        map.CsizeS=float(row.size-1);map.HSizeS=float(map.HalfSize);map.SideCount=6;map.Initialized=true;
        std::memcpy(&map.StretchSide1,&row.stretch,4);
        map.LightPos[0]=0.125;map.LightPos[1]=-0.25;map.LightPos[2]=0.5;
        if(row.rotation==0) {
            map.RotMatrix[0][0]=1;map.RotMatrix[1][1]=1;map.RotMatrix[2][2]=1;
        } else {
            map.RotMatrix[0][0]=0.8f;map.RotMatrix[0][2]=-0.6f;
            map.RotMatrix[1][1]=1;map.RotMatrix[2][0]=0.6f;map.RotMatrix[2][2]=0.8f;
        }
        for(int side=0;side<6;++side) {
            map.CubeSides[side].resize(row.size*row.size);
            for(int y=0;y<row.size;++y)for(int x=0;x<row.size;++x)
                map.CubeSides[side][y*row.size+x]=row.mode==0 ? side*10000+y*100+x+0.25f
                    : ((x*11+y*7)%17)*0.25f-2.0f;
        }
        TVec3D input;
        std::memcpy(&input[0],&row.x,8);std::memcpy(&input[1],&row.y,8);std::memcpy(&input[2],&row.z,8);
        float value=row.mode==0 ? mb3d::SamplePositionalVolumeLightMap(map,input)
            : (mb3d::DirectionalVolumeLightMapContains(map,input)?1.0f:0.0f);
        uint32_t actual;std::memcpy(&actual,&value,4);
        if(actual!=row.expected && mismatches++<20)
            std::printf("Volume sample mismatch row=%zu mode=%d size=%d rotation=%d expected=%08x actual=%08x\n",
                rowIndex,row.mode,row.size,row.rotation,row.expected,actual);
        CHECK(actual==row.expected,"ordinary volume lookup matches immutable Pascal SSE face/seam/rounding oracle");
        const double squaredLength=mb3d::VolumeLightDirectionSquaredLength(input);
        uint64_t lengthBits;std::memcpy(&lengthBits,&squaredLength,8);
        CHECK(lengthBits==row.squaredLength,"volume distance uses original Single-vector and x87 return-to-Double contract");
        ++rowIndex;
    }
    mb3d::TVolumeLightMap map{};map.Initialized=true;map.SideCount=6;map.CubeSize=7;map.HalfSize=3;map.SizeFactor=3;
    const TVec3D invalidDirections[2]={{0,0,0},{1e-300,0,0}};
    for(const TVec3D& direction : invalidDirections) {
        bool rejected=false;
        try { mb3d::SamplePositionalVolumeLightMap(map,direction); }
        catch(const std::invalid_argument&) { rejected=true; }
        CHECK(rejected,"undefined zero-Single-direction original cube lookup is explicitly rejected");
    }
}

void test_map_preprocessing_original() {
    for (const auto& row : kMapPreprocessingFixtures) {
        TLightMap map{};
        const int w=row.size==0?8:32, h=row.size==0?8:16;
        map.LMWidth=w; map.LMHeight=h; map.iMapType=row.type;
        map.LMa=new uint32_t[(row.bytes+3)/4]{};
        for(int y=0;y<=h;++y) for(int x=0;x<=w;++x) {
            if(row.type==0) map.LMa[y*(w+1)+x]=((x%w*17+y%h*31)&255) |
                (((x%w*53+y%h*7)&255)<<8) | (((x%w*11+y%h*67)&255)<<16);
            else {
                auto* p=reinterpret_cast<uint16_t*>(map.LMa)+(y*(w+1)+x)*3;
                for(int c=0;c<3;++c) p[c]=uint16_t((x%w*1709+y%h*2741+c*13337)&65535);
            }
        }
        mb3d::PrepareLightMap(&map,(row.flags&1)!=0,(row.flags&2)!=0,(row.flags&4)!=0);
        uint64_t hash=14695981039346656037ull;
        const auto* bytes=reinterpret_cast<const uint8_t*>(map.LMa);
        for(int i=0;i<row.bytes;++i) hash=(hash^bytes[i])*1099511628211ull;
        if(hash!=row.hash) std::printf("Map preparation mismatch size=%d type=%d flags=%d expected=%016llx actual=%016llx\n",
            row.size,row.type,row.flags,static_cast<unsigned long long>(row.hash),static_cast<unsigned long long>(hash));
        CHECK(hash==row.hash,"full prepared map matches original Pascal buffer");
        mb3d::FreeLightMap(&map);
    }
}

void test_map_sampling_original() {
    size_t index=0;
    for (const auto& row : kMapSamplingFixtures) {
        TLightMap map{};
        map.LMWidth=8; map.LMHeight=6; map.iMapType=row.type;
        map.sLMXfactor=8; map.sLMYfactor=6; map.sIntensity=0.7f;
        map.LMa=new uint32_t[(9*7*(4+2*row.type)+8+3)/4]{};
        for(int y=0;y<=6;++y) for(int x=0;x<=8;++x) {
            if(row.type==0) map.LMa[y*9+x]=((x*17+y*31)&255) |
                (((x*53+y*7)&255)<<8) | (((x*11+y*67)&255)<<16);
            else {
                auto* p=reinterpret_cast<uint16_t*>(map.LMa)+(y*9+x)*3;
                for(int c=0;c<3;++c) p[c]=uint16_t((x*1709+y*2741+c*13337)&65535);
            }
        }
        double x,y; std::memcpy(&x,&row.x,8);std::memcpy(&y,&row.y,8);
        TVec3D actual{};
        if(row.mode==0) {
            TSVec value{};
            mb3d::GetLightMapPixel(value,float(x),float(y),&map,row.squared,row.wrap);
            for(int c=0;c<3;++c)actual[c]=value[c];
        } else mb3d::SplineIpolMap(actual,x,y,&map);
        const uint64_t expected[3]={row.r,row.g,row.b};
        for(int c=0;c<3;++c) {
            uint64_t bits;std::memcpy(&bits,&actual[c],8);
            if(bits!=expected[c]) std::printf("Map sample mismatch row=%zu mode=%d type=%d wrap=%d squared=%d channel=%d expected=%016llx actual=%016llx\n",
                index,row.mode,row.type,row.wrap,row.squared,c,
                static_cast<unsigned long long>(expected[c]),static_cast<unsigned long long>(bits));
            CHECK(bits==expected[c],"ordinary spline sample matches original Pascal x87/SSE2 fixture exactly");
        }
        mb3d::FreeLightMap(&map);++index;
    }
}

void test_map_preprocessing_small_widths() {
    for(int width=4;width<=6;++width) for(int type=0;type<=1;++type) {
        TLightMap map{};map.LMWidth=width;map.LMHeight=6;map.iMapType=type;
        map.LMa=new uint32_t[((width+1)*7*(4+2*type)+8+3)/4]{};
        bool rejected=false;
        try { mb3d::PrepareLightMap(&map,true,true,false); }
        catch(const std::exception&) { rejected=true; }
        CHECK(!rejected,"loaded width4/5 maps remain supported without undefined border access");
        try { mb3d::PrepareLightMap(&map,false,false,true); }
        catch(const std::invalid_argument& error) {
            rejected=std::strstr(error.what(),"width >=6")!=nullptr;
        }
        CHECK(rejected==(width<6),"only border preprocessing widths4/5 reject original out-of-bounds access");
        mb3d::FreeLightMap(&map);
    }
}

void test_transmission_fpc_predicate() {
    for (const auto& row : kTransmissionPredicates) {
        const bool outside=mb3d::ReflectionFpcNotInsideXor(row.initial!=0,row.current!=0);
        CHECK(outside==(row.outside!=0),"transmission predicate matches compiled original FPC LongBool expression");
        CHECK((!row.transmitting || outside)==(row.multiply_specular!=0),
              "transmission specular attenuation follows original composite predicate");
        CHECK(((row.initial!=0)!=(row.current!=0))==(row.attenuate!=0),
              "plain original LongBool XOR remains distinct from NOT/XOR expression");
    }
}

void test_transmission_power_boundaries() {
    auto single = [](unsigned bits) { float value; std::memcpy(&value, &bits, 4); return value; };
    for (const auto& row : kTransmissionPowers) {
        const float expected=single(row.expected);
        const float actual=mb3d::ReflectionAbsorptionPower(single(row.base),single(row.exponent));
        unsigned bits;std::memcpy(&bits,&actual,4);
        CHECK(std::isnan(expected) ? std::isnan(actual) : bits==row.expected,
              "absorption Power preserves zero exponent, signed and tiny bases");
    }
    for (const auto& row : kTransmissionExponents) {
        double distance;std::memcpy(&distance,&row.distance,8);
        const float actual=mb3d::ReflectionAbsorptionExponent(distance,single(row.absorption));
        unsigned bits;std::memcpy(&bits,&actual,4);
        CHECK(bits==row.expected,"absorption exponent narrows once after Double multiplication");
    }
}

void test_visible_light_original_math() {
    auto fromBits = [](uint32_t bits) { float value; std::memcpy(&value, &bits, 4); return value; };
    for (const auto& row : kVisibleLightMathFixtures) {
        const float input = fromBits(row.flux);
        float actualPow = mb3d::FastPow(input, 50);
        uint32_t actualPowBits;
        std::memcpy(&actualPowBits, &actualPow, 4);
        CHECK(actualPowBits == row.fast_pow50, "FastPow matches original Pascal rational approximation");
        TLightVals light{};
        light.iLightPos[0] = row.option;
        light.bCalcPixColSqr = row.squared;
        float flux = input, transparency = fromBits(row.radius);
        mb3d::CalcPosLightShape(flux, transparency, 0, &light);
        const float expectedFlux = fromBits(row.expected_flux);
        const float expectedTransparency = fromBits(row.expected_transparency);
        CHECK(std::abs(flux - expectedFlux) <= 2e-6f * std::max(1e-6f, std::abs(expectedFlux)),
              "visible light flux matches original Pascal finite scalar fixture");
        CHECK(std::abs(transparency - expectedTransparency) <= 2e-6f * std::max(1e-6f, std::abs(expectedTransparency)),
              "visible light transparency matches original Pascal finite scalar fixture");
    }
}

static void test_external_exp_range() {
    for (double value : {1e12, 1e30, std::numeric_limits<double>::max()}) {
        CHECK(mb3d::x87_exp(value) == std::numeric_limits<double>::infinity(),
              "finite huge positive exponential overflows without an invalid int cast");
        const double underflow = mb3d::x87_exp(-value);
        CHECK(underflow == 0.0 && !std::signbit(underflow),
              "finite huge negative exponential underflows to positive zero");
    }
    CHECK(mb3d::x87_exp(0.0) == 1.0 && mb3d::x87_exp(-0.0) == 1.0,
          "zero exponential remains exact");
    CHECK(std::isnan(mb3d::x87_exp(std::numeric_limits<double>::quiet_NaN())),
          "NaN exponential remains NaN");
}

int main(int argc, char* argv[]) {
    install_test_console_crash_handlers();
    unsigned matchedTests = 0;
    const char* filter = (argc > 1) ? argv[1] : nullptr;
    // Optional third argument narrows the independent fixture corpus by name;
    // a typo is a failure, never an empty successful qualification.
    g_formulaFilter = (argc > 2) ? argv[2] : nullptr;
    if (!filter) std::printf("Running Renderer regression tests...\n\n");

#define RUN_TEST_CASE(fn) do { \
    if (!filter || std::strcmp(filter, #fn) == 0) { \
        ++matchedTests; \
        int _prevFailed = g_failed; \
        fn(); \
        if (!filter) { \
            if (g_failed == _prevFailed) std::printf("[ PASS ] %s\n", #fn); \
            else std::printf("[ FAIL ] %s\n", #fn); \
        } \
    } \
} while (0)

#ifdef MB3D_ENABLE_PARITY_TRACE
#define RUN_TRACE_TEST_CASE(fn) RUN_TEST_CASE(fn)
#else
// Instrumentation-specific cases are unavailable in this configuration. Keep
// ordinary rendering, image comparisons and all numerical fixtures enabled.
#define RUN_TRACE_TEST_CASE(fn) do { \
    if (!filter || std::strcmp(filter, #fn) == 0) { \
        std::printf("[ SKIP ] %s: parity trace is disabled\n", #fn); \
        if (filter) return 77; \
    } \
} while (0)
#endif

    RUN_TEST_CASE(test_formulas_render_nonblack);
    RUN_TEST_CASE(test_getmct_destop_factor_regression);
    RUN_TEST_CASE(test_shadows_affect_output);
    RUN_TEST_CASE(test_multithreaded_consistency);
    RUN_TEST_CASE(test_final_rgb_surface);
    RUN_TEST_CASE(test_scene_native_imagescale_header);
    RUN_TEST_CASE(test_m3i_load_path);
    RUN_TEST_CASE(test_zbuffer_export);
    RUN_TEST_CASE(test_normalmap_export);
    RUN_TEST_CASE(test_bmp_writer);
    RUN_TEST_CASE(test_png_writer);
    RUN_TEST_CASE(test_error_nonexistent_file);
    RUN_TEST_CASE(test_error_invalid_dimensions);
    RUN_TEST_CASE(test_c_api);
    RUN_TEST_CASE(test_dimension_override);
    RUN_TEST_CASE(test_silight_populated);
    RUN_TEST_CASE(test_diagnostics_capture);
    RUN_TEST_CASE(test_ray_setup_dump_capture);
    RUN_TEST_CASE(test_hybrid_option_normalization);
    RUN_TEST_CASE(test_external_formula_registry);
    RUN_TEST_CASE(test_external_legacy_option_defaults);
    RUN_TEST_CASE(test_formula_const_buffer_layouts);
#if (defined(_MSC_VER) && defined(_M_X64)) || defined(MB3D_SOFTWARE_EXT80)
    RUN_TEST_CASE(test_original_formula_angle_loader);
#endif
    RUN_TEST_CASE(test_external_formula_chain_semantics);
    RUN_TEST_CASE(test_flipxyc_scene_schema);
    RUN_TRACE_TEST_CASE(test_flipxyc_runtime_trace_slot_presence);
    RUN_TRACE_TEST_CASE(test_flipxyc_c_writeback_contract);
    RUN_TEST_CASE(test_flipxyc_flipcxyz_chain_contract);
    RUN_TEST_CASE(test_conditcountswitch_transform_juliax2_chain_contract);
    RUN_TEST_CASE(test_juliasets_flipcxyz_chain_contract);
    RUN_TEST_CASE(test_external_stored_radius_and_cwarp_contract);
    RUN_TEST_CASE(test_external_four_d_original_code);
    RUN_TEST_CASE(test_external_exp_range);
    RUN_TEST_CASE(test_external_formula_regressions);
    RUN_TEST_CASE(test_unordered_orbit_reaches_maxit);
    RUN_TEST_CASE(test_hybrid_radius_double_rounding);
    RUN_TEST_CASE(test_depth_codec_original_assembly);
    RUN_TEST_CASE(test_unordered_ifs_minimum_and_inside_flags);
    RUN_TEST_CASE(test_ifs_minimum_carry_between_calls);
    RUN_TEST_CASE(test_iteration_boundary_original_pascal);
    RUN_TEST_CASE(test_iteration_refinement_keeps_depth_and_color_mode);
    RUN_TEST_CASE(test_pinehedron_original_code);
    RUN_TEST_CASE(test_smooth_fold_original_code);
    RUN_TEST_CASE(test_vsshapes_original_code);
    RUN_TEST_CASE(test_partlyjuliaroff_flipcxyz_chain_contract);
    RUN_TEST_CASE(test_partlyjulia_flipcxyz_chain_contract);
    RUN_TEST_CASE(test_translatec4d_local_contract);
    RUN_TEST_CASE(test_aboxmod4d_local_contract);
    RUN_TEST_CASE(test_aboxvaryscale4d_persistent_state_contract);
    RUN_TRACE_TEST_CASE(test_translatec4d_scene_runtime_trace);
    RUN_TRACE_TEST_CASE(test_aboxmod4d_scene_runtime_trace);
    RUN_TRACE_TEST_CASE(test_aboxvaryscale4d_scene_runtime_trace);
    RUN_TRACE_TEST_CASE(test_parity_trace_stage_session_handoff);
    RUN_TRACE_TEST_CASE(test_parity_trace_render_stage_window);
    RUN_TRACE_TEST_CASE(test_parity_trace_capture);
    RUN_TEST_CASE(test_paint_replay_restores_completed_hard_shadow_state);
    RUN_TEST_CASE(test_paint_replay_does_not_force_hard_shadow_state_without_shadow_pass);
    RUN_TEST_CASE(test_paint_replay_reproduces_reflection_late_pass);
    RUN_TRACE_TEST_CASE(test_paint_replay_diag_only_traces_reflection_row_window);
    RUN_TRACE_TEST_CASE(test_paint_replay_diag_only_traces_reflection_sr_ray_enter_window);
    RUN_TRACE_TEST_CASE(test_reflection_primary_trace_matches_pascal_hotspot);
    RUN_TEST_CASE(test_ao_threshold_replay_clears_background_quartet_lanes);
    RUN_TEST_CASE(test_otrap_calcde_uses_live_maxit);
    RUN_TEST_CASE(test_otrap_calcde_noade_restores_smoothitd);
    RUN_TEST_CASE(test_otrap_calcde_full_runs_second_decomb_group);
    RUN_TEST_CASE(test_external_formula_map_slot_masks);
    RUN_TEST_CASE(test_corrected_parity_image_hotspots);
    RUN_TEST_CASE(test_directional_volumetric_map_matches_pascal);
    RUN_TEST_CASE(test_visible_light_original_math);
    RUN_TEST_CASE(test_transmission_fpc_predicate);
    RUN_TEST_CASE(test_transmission_power_boundaries);
    RUN_TEST_CASE(test_volume_sampling_original);
    RUN_TEST_CASE(test_map_preprocessing_original);
    RUN_TEST_CASE(test_map_preprocessing_small_widths);
    RUN_TEST_CASE(test_map_sampling_original);

#undef RUN_TRACE_TEST_CASE
#undef RUN_TEST_CASE
    CHECK(matchedTests > 0, "test selector must match at least one test");

    for (const auto& [name,stats] : g_formulaStats)
        std::printf("FORMULA_RESULT\t%s\t%zu\t%zu\t%zu\n",name.c_str(),stats.rows,stats.checks,stats.failures);

    if (g_formulaFilter) {
        CHECK(g_formulaRows > 0, "selected external formula has independent fixture rows");
        std::printf("Selected formula %s: %zu fixture rows\n", g_formulaFilter, g_formulaRows);
    }

    std::printf("\n========================================\n");
    std::printf("Results: %d passed, %d failed\n", g_passed, g_failed);
    std::printf("========================================\n");

    return g_failed > 0 ? 1 : 0;
}
