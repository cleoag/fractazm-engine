#include "core/types.h"
#include "core/math3d.h"
#include "core/header_trafos.h"
#include "core/external_formulas.h"
#include "io/m3p_parser.h"
#include "windows_console_crash_guard.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <filesystem>

static int g_passed = 0;
static int g_failed = 0;

#define CHECK(cond, msg) do { \
    if (cond) { g_passed++; } \
    else { g_failed++; std::printf("FAIL: %s (line %d)\n", msg, __LINE__); } \
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

static void writeCustomFname(uint8_t fname[32], const char* text) {
    std::memset(fname, 0, 32);
    size_t len = std::min<size_t>(31, std::strlen(text));
    std::memcpy(fname, text, len);
}

static int externalValueWidth(uint8_t t) {
    switch (t) {
        case 5:
        case 6:
            return 3;
        case 12:
            return 6;
        case 18:
            return 4;
        case 19:
            return 2;
        default:
            return 1;
    }
}

static int expandedExternalOptionCount(const mb3d::ExternalFormulaInfo& info) {
    int valueIdx = 0;
    for (int entry = 0; entry < info.optionTypeCount && valueIdx < 16; ++entry) {
        valueIdx += externalValueWidth(info.optionTypes[entry]);
    }
    return valueIdx;
}

static void buildExpandedExternalOptionTypes(const mb3d::ExternalFormulaInfo& info,
                                             uint8_t out[16]) {
    std::memset(out, 0, 16);
    int valueIdx = 0;
    for (int entry = 0; entry < info.optionTypeCount && valueIdx < 16; ++entry) {
        uint8_t t = info.optionTypes[entry];
        int width = externalValueWidth(t);
        for (int k = 0; k < width && valueIdx + k < 16; ++k) {
            out[valueIdx + k] = (t == 18 && k > 0) ? 6 : t;
        }
        valueIdx += width;
    }
}

// ============================================================================
// Test: Load each reference file successfully
// ============================================================================
static void test_load_IntPow2() {
    auto r = mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    CHECK(r.success, "IntPow2 load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
    CHECK(r.header.Width > 0, "IntPow2 Width > 0");
    CHECK(r.header.Height > 0, "IntPow2 Height > 0");
    CHECK(r.header.Iterations > 0, "IntPow2 Iterations > 0");
}

static void test_load_IntPow3() {
    auto r = mb3d::LoadM3PFile(refPath("formula_0_IntPow3.m3p"));
    CHECK(r.success, "IntPow3 load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
}

static void test_load_IntPow4() {
    auto r = mb3d::LoadM3PFile(refPath("formula_0_IntPow4.m3p"));
    CHECK(r.success, "IntPow4 load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
}

static void test_load_IntPow5() {
    auto r = mb3d::LoadM3PFile(refPath("formula_0_IntPow5.m3p"));
    CHECK(r.success, "IntPow5 load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
}

static void test_load_IntPow6() {
    auto r = mb3d::LoadM3PFile(refPath("formula_0_IntPow6.m3p"));
    CHECK(r.success, "IntPow6 load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
}

static void test_load_IntPow7() {
    auto r = mb3d::LoadM3PFile(refPath("formula_0_IntPow7.m3p"));
    CHECK(r.success, "IntPow7 load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
}

static void test_load_IntPow8() {
    auto r = mb3d::LoadM3PFile(refPath("formula_0_IntPow8.m3p"));
    CHECK(r.success, "IntPow8 load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
}

static void test_load_RealPower() {
    auto r = mb3d::LoadM3PFile(refPath("formula_1_RealPower.m3p"));
    CHECK(r.success, "RealPower load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
}

static void test_load_Quaternion() {
    auto r = mb3d::LoadM3PFile(refPath("formula_2_Quaternion.m3p"));
    CHECK(r.success, "Quaternion load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
}

static void test_load_Tricorn() {
    auto r = mb3d::LoadM3PFile(refPath("formula_3_Tricorn.m3p"));
    CHECK(r.success, "Tricorn load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
}

static void test_load_AmazingBox() {
    auto r = mb3d::LoadM3PFile(refPath("formula_4_AmazingBox.m3p"));
    CHECK(r.success, "AmazingBox load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
}

static void test_load_Bulbox() {
    auto r = mb3d::LoadM3PFile(refPath("formula_5_Bulbox.m3p"));
    CHECK(r.success, "Bulbox load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
}

static void test_load_FoldIntPow() {
    auto r = mb3d::LoadM3PFile(refPath("formula_6_FoldIntPow.m3p"));
    CHECK(r.success, "FoldIntPow load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
}

static void test_load_AexionC() {
    auto r = mb3d::LoadM3PFile(refPath("formula_9_AexionC.m3p"));
    CHECK(r.success, "AexionC load success");
    if (!r.success) std::printf("  error: %s\n", r.error.c_str());
}

// ============================================================================
// Test: Header field validation
// ============================================================================
static void test_header_fields() {
    auto r = mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    if (!r.success) {
        std::printf("SKIP: header_fields test (load failed)\n");
        return;
    }

    // The file has MandId=39 (0x27), which is version 39
    // After migration from 39 to 44, various fields are updated
    CHECK(r.header.Width == 2000, "Width == 2000");
    CHECK(r.header.Height == 2000, "Height == 2000");
    CHECK(r.header.Iterations > 0, "Iterations > 0");
    CHECK(r.header.dZoom > 0, "dZoom > 0");
    CHECK(r.header.RStop > 0, "RStop > 0");

    // Check MandId: original was 39, but UpdateFormulaOptionAbove20 doesn't change it
    // The file's MandId stays as read
    int32_t originalMId;
    {
        FILE* f = std::fopen(refPath("formula_0_IntPow2.m3p").c_str(), "rb");
        if (f) {
            std::fread(&originalMId, 4, 1, f);
            std::fclose(f);
        } else {
            originalMId = 0;
        }
    }
    CHECK(originalMId >= 1 && originalMId <= 44, "Original MandId in range 1..44");

    // Verify pointer fields are zeroed
    for (int i = 0; i < 6; i++) {
        CHECK(r.header.PHCustomF[i] == 0, "PHCustomF[i] zeroed");
    }
    CHECK(r.header.PCFAddon == 0, "PCFAddon zeroed");
}

// ============================================================================
// Test: All reference files have reasonable header fields
// ============================================================================
static void test_all_headers_reasonable() {
    const char* files[] = {
        "formula_0_IntPow2.m3p", "formula_0_IntPow3.m3p",
        "formula_0_IntPow4.m3p", "formula_0_IntPow5.m3p",
        "formula_0_IntPow6.m3p", "formula_0_IntPow7.m3p",
        "formula_0_IntPow8.m3p", "formula_1_RealPower.m3p",
        "formula_2_Quaternion.m3p", "formula_3_Tricorn.m3p",
        "formula_4_AmazingBox.m3p", "formula_5_Bulbox.m3p",
        "formula_6_FoldIntPow.m3p", "formula_9_AexionC.m3p"
    };

    for (const char* fname : files) {
        auto r = mb3d::LoadM3PFile(refPath(fname));
        std::string prefix = std::string(fname) + " ";

        CHECK(r.success, (prefix + "loads").c_str());
        if (!r.success) {
            std::printf("  error: %s\n", r.error.c_str());
            continue;
        }
        CHECK(r.header.Width > 0, (prefix + "Width > 0").c_str());
        CHECK(r.header.Height > 0, (prefix + "Height > 0").c_str());
        CHECK(r.header.Iterations > 0, (prefix + "Iterations > 0").c_str());
    }
}

// ============================================================================
// Test: Round-trip (load -> save -> load -> compare)
// ============================================================================
static void test_roundtrip() {
    auto r1 = mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    if (!r1.success) {
        std::printf("SKIP: roundtrip test (initial load failed)\n");
        return;
    }

    // Save to temp file
    std::string tmpFile = tempPath("mb3d_test_roundtrip.m3p");
    bool saved = mb3d::SaveM3PFile(tmpFile, r1.header, r1.addon);
    CHECK(saved, "roundtrip save");
    if (!saved) return;

    // Load back
    auto r2 = mb3d::LoadM3PFile(tmpFile);
    CHECK(r2.success, "roundtrip reload");
    if (!r2.success) {
        std::printf("  error: %s\n", r2.error.c_str());
        return;
    }

    // Zero pointer fields before compare (they get zeroed on load)
    TMandHeader10 h1 = r1.header;
    TMandHeader10 h2 = r2.header;
    std::memset(h1.PHCustomF, 0, sizeof(h1.PHCustomF));
    h1.PCFAddon = 0;
    std::memset(h2.PHCustomF, 0, sizeof(h2.PHCustomF));
    h2.PCFAddon = 0;

    // After a second load, UpdateFormulaOptionAbove20 and UpdateLightParasAbove3
    // may modify the header again (idempotency concerns).
    // The key fields should match.
    CHECK(h1.Width == h2.Width, "roundtrip Width match");
    CHECK(h1.Height == h2.Height, "roundtrip Height match");
    CHECK(h1.Iterations == h2.Iterations, "roundtrip Iterations match");
    CHECK_NEAR(h1.dZoom, h2.dZoom, 1e-10, "roundtrip dZoom match");
    CHECK_NEAR(h1.dXmid, h2.dXmid, 1e-10, "roundtrip dXmid match");
    CHECK_NEAR(h1.dYmid, h2.dYmid, 1e-10, "roundtrip dYmid match");
    CHECK_NEAR(h1.dZmid, h2.dZmid, 1e-10, "roundtrip dZmid match");
    CHECK_NEAR(h1.RStop, h2.RStop, 1e-10, "roundtrip RStop match");
    CHECK_NEAR(h1.dZstart, h2.dZstart, 1e-10, "roundtrip dZstart match");
    CHECK_NEAR(h1.dZend, h2.dZend, 1e-10, "roundtrip dZend match");

    // Compare addon
    THeaderCustomAddon a1 = r1.addon;
    THeaderCustomAddon a2 = r2.addon;
    // The bHCAversion gets set to 16 on save
    // Compare formula fields
    for (int i = 0; i < 6; i++) {
        CHECK(a1.Formulas[i].iFnr == a2.Formulas[i].iFnr,
              ("roundtrip formula " + std::to_string(i) + " iFnr match").c_str());
        CHECK(a1.Formulas[i].iItCount == a2.Formulas[i].iItCount,
              ("roundtrip formula " + std::to_string(i) + " iItCount match").c_str());
    }

    // Clean up
    std::remove(tmpFile.c_str());
}

// ============================================================================
// Test: Byte-for-byte roundtrip (save -> load gives same raw bytes)
// ============================================================================
static void test_roundtrip_bytes() {
    auto r1 = mb3d::LoadM3PFile(refPath("formula_4_AmazingBox.m3p"));
    if (!r1.success) {
        std::printf("SKIP: roundtrip_bytes test (initial load failed)\n");
        return;
    }

    // Save
    std::string tmpFile = tempPath("mb3d_test_roundtrip_bytes.m3p");
    bool saved = mb3d::SaveM3PFile(tmpFile, r1.header, r1.addon);
    CHECK(saved, "roundtrip_bytes save");
    if (!saved) return;

    // Read raw bytes of saved file
    FILE* f1 = std::fopen(tmpFile.c_str(), "rb");
    std::fseek(f1, 0, SEEK_END);
    long sz = std::ftell(f1);
    CHECK(sz == sizeof(TMandHeader10) + sizeof(THeaderCustomAddon), "roundtrip_bytes file size");
    std::fclose(f1);

    // Load the saved file
    auto r2 = mb3d::LoadM3PFile(tmpFile);
    CHECK(r2.success, "roundtrip_bytes reload");

    // Save again
    std::string tmpFile2 = tempPath("mb3d_test_roundtrip_bytes2.m3p");
    mb3d::SaveM3PFile(tmpFile2, r2.header, r2.addon);

    // Compare the two saved files byte-by-byte
    FILE* fa = std::fopen(tmpFile.c_str(), "rb");
    FILE* fb = std::fopen(tmpFile2.c_str(), "rb");
    if (fa && fb) {
        uint8_t bufA[2048], bufB[2048];
        size_t szA = std::fread(bufA, 1, 2048, fa);
        size_t szB = std::fread(bufB, 1, 2048, fb);
        CHECK(szA == szB, "roundtrip_bytes same size");
        if (szA == szB) {
            bool match = (std::memcmp(bufA, bufB, szA) == 0);
            CHECK(match, "roundtrip_bytes byte-identical");
            if (!match) {
                // Find first difference
                for (size_t i = 0; i < szA; i++) {
                    if (bufA[i] != bufB[i]) {
                        std::printf("  First diff at byte %zu: 0x%02X vs 0x%02X\n", i, bufA[i], bufB[i]);
                        break;
                    }
                }
            }
        }
    }
    if (fa) std::fclose(fa);
    if (fb) std::fclose(fb);

    std::remove(tmpFile.c_str());
    std::remove(tmpFile2.c_str());
}

// ============================================================================
// Test: Error handling -- nonexistent file
// ============================================================================
static void test_error_nonexistent() {
    auto r = mb3d::LoadM3PFile(tempPath("this_file_does_not_exist_12345.m3p"));
    CHECK(!r.success, "nonexistent file fails");
    CHECK(r.error.find("Cannot open") != std::string::npos, "nonexistent file error message");
}

// ============================================================================
// Test: Error handling -- corrupt data
// ============================================================================
static void test_error_corrupt() {
    // Write random garbage to a file
    std::string tmpFile = tempPath("mb3d_test_corrupt.m3p");
    FILE* f = std::fopen(tmpFile.c_str(), "wb");
    if (f) {
        uint8_t garbage[200];
        for (int i = 0; i < 200; i++) garbage[i] = static_cast<uint8_t>((i * 37 + 13) & 0xFF);
        // Set MandId to something invalid
        int32_t badId = 999;
        std::memcpy(garbage, &badId, 4);
        std::fwrite(garbage, 1, 200, f);
        std::fclose(f);

        auto r = mb3d::LoadM3PFile(tmpFile);
        CHECK(!r.success, "corrupt data fails");
        std::remove(tmpFile.c_str());
    }
}

// ============================================================================
// Test: Error handling -- truncated file
// ============================================================================
static void test_error_truncated() {
    std::string tmpFile = tempPath("mb3d_test_truncated.m3p");
    FILE* f = std::fopen(tmpFile.c_str(), "wb");
    if (f) {
        // Write just 10 bytes
        uint8_t smallBuf[10] = {39, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        std::fwrite(smallBuf, 1, 10, f);
        std::fclose(f);

        auto r = mb3d::LoadM3PFile(tmpFile);
        CHECK(!r.success, "truncated file fails");
        std::remove(tmpFile.c_str());
    }
}

// ============================================================================
// Test: Save and load empty-ish header
// ============================================================================
static void test_save_load_minimal() {
    TMandHeader10 h;
    std::memset(&h, 0, sizeof(h));
    h.MandId = 44;
    h.Width = 640;
    h.Height = 480;
    h.Iterations = 25;
    h.dZoom = 1.0;
    h.RStop = 4.0;
    h.dZstart = -2.0;
    h.dZend = 2.0;

    THeaderCustomAddon a;
    std::memset(&a, 0, sizeof(a));
    a.bHCAversion = 16;
    a.Formulas[0].iItCount = 1;
    a.Formulas[0].iFnr = 0; // IntPow
    a.Formulas[0].dOptionValue[0] = 8; // power

    std::string tmpFile = tempPath("mb3d_test_minimal.m3p");
    bool saved = mb3d::SaveM3PFile(tmpFile, h, a);
    CHECK(saved, "minimal save");

    auto r = mb3d::LoadM3PFile(tmpFile);
    CHECK(r.success, "minimal load");
    if (r.success) {
        CHECK(r.header.Width == 640, "minimal Width");
        CHECK(r.header.Height == 480, "minimal Height");
        CHECK(r.header.Iterations == 25, "minimal Iterations");
        CHECK_NEAR(r.header.dZoom, 1.0, 1e-10, "minimal dZoom");
    }
    std::remove(tmpFile.c_str());
}

// ============================================================================
// Test: Struct sizes (critical for binary I/O)
// ============================================================================
static void test_struct_sizes() {
    CHECK(sizeof(TMandHeader10) == 840, "TMandHeader10 == 840");
    CHECK(sizeof(THeaderCustomAddon) == 1136, "THeaderCustomAddon == 1136");
    CHECK(sizeof(TLightingParas9) == 408, "TLightingParas9 == 408");
    CHECK(sizeof(TLight8) == 32, "TLight8 == 32");
    CHECK(sizeof(THAformula) == 188, "THAformula == 188");
    CHECK(sizeof(TsiLight5) == 18, "TsiLight5 == 18");
}

// ============================================================================
// Test: Formula identification after load
// ============================================================================
static void test_formula_identification() {
    // IntPow2 should have formula 0 (IntPow) with power 2
    auto r = mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    if (!r.success) {
        std::printf("SKIP: formula_identification (load failed)\n");
        return;
    }

    // Check that at least the first formula slot is populated
    CHECK(r.addon.Formulas[0].iItCount > 0, "formula[0] has iterations");
    CHECK(r.addon.Formulas[0].iFnr >= 0, "formula[0] has valid iFnr");
}

// ============================================================================
// Test: Old internal formula schema normalization keeps on-disk values
// Pascal ParseCFfromOld updates option types/count, but does not invent missing
// dOptionValue entries for internal formulas loaded from the header addon.
// ============================================================================
static void test_comparison_scene_internal_formula_schema() {
    {
        auto r = mb3d::LoadM3PFile(comparisonScenePath("test_09_foldintpow.m3p"));
        CHECK(r.success, "comparison foldintpow load success");
        if (r.success) {
            CHECK(r.addon.Formulas[0].iFnr == 6, "comparison foldintpow fnr");
            CHECK(r.addon.Formulas[0].iOptionCount == 3, "comparison foldintpow option count");
            CHECK(r.addon.Formulas[0].byOptionType[0] == 10, "comparison foldintpow opt0 type");
            CHECK(r.addon.Formulas[0].byOptionType[1] == 0, "comparison foldintpow opt1 type");
            CHECK(r.addon.Formulas[0].byOptionType[2] == 8, "comparison foldintpow opt2 type");
            CHECK_NEAR(r.addon.Formulas[0].dOptionValue[0], 2.0, 1e-12, "comparison foldintpow power");
            CHECK_NEAR(r.addon.Formulas[0].dOptionValue[1], -1.0, 1e-12, "comparison foldintpow zmul");
            CHECK_NEAR(r.addon.Formulas[0].dOptionValue[2], 0.0, 1e-12, "comparison foldintpow fold preserved");
        }
    }

    {
        auto r = mb3d::LoadM3PFile(comparisonScenePath("test_10_aexionc.m3p"));
        CHECK(r.success, "comparison aexionc load success");
        if (r.success) {
            CHECK(r.addon.Formulas[0].iFnr == 9, "comparison aexionc fnr");
            CHECK(r.addon.Formulas[0].iOptionCount == 8, "comparison aexionc option count");
            CHECK(r.addon.Formulas[0].byOptionType[2] == 2, "comparison aexionc opt2 type");
            CHECK(r.addon.Formulas[0].byOptionType[3] == 2, "comparison aexionc opt3 type");
            CHECK(r.addon.Formulas[0].byOptionType[6] == 2, "comparison aexionc opt6 type");
            CHECK(r.addon.Formulas[0].byOptionType[7] == 2, "comparison aexionc opt7 type");
            CHECK_NEAR(r.addon.Formulas[0].dOptionValue[2], 0.0, 1e-12, "comparison aexionc rotate-c preserved");
            CHECK_NEAR(r.addon.Formulas[0].dOptionValue[3], 0.0, 1e-12, "comparison aexionc cond-phi preserved");
            CHECK_NEAR(r.addon.Formulas[0].dOptionValue[4], 0.0, 1e-12, "comparison aexionc power-c preserved");
            CHECK_NEAR(r.addon.Formulas[0].dOptionValue[5], 0.0, 1e-12, "comparison aexionc czmul preserved");
            CHECK_NEAR(r.addon.Formulas[0].dOptionValue[6], 0.0, 1e-12, "comparison aexionc powc-dist preserved");
            CHECK_NEAR(r.addon.Formulas[0].dOptionValue[7], 0.0, 1e-12, "comparison aexionc mode preserved");
        }
    }
}

static void test_hybrid_scene_mct_setup() {
    auto r = mb3d::LoadM3PFile(comparisonScenePath("test_21_hybrid.m3p"));
    CHECK(r.success, "comparison hybrid load success");
    if (!r.success) {
        return;
    }

    auto mct = mb3d::GetMCTparasFromHeader(r.header, &r.addon, 0);
    CHECK(mct.bMCTisValid != 0, "comparison hybrid MCT valid");
    CHECK(mct.FormulaType == 0, "comparison hybrid FormulaType");
    CHECK(mct.DEoption == 0, "comparison hybrid DEoption");
    CHECK(mct.wEndTo == 1, "comparison hybrid end1");
    CHECK(mct.RepeatFrom1 == 0, "comparison hybrid repeat1");
    CHECK(mct.StartFrom2 == 1, "comparison hybrid start2");
    CHECK(mct.iEnd2 == 1, "comparison hybrid end2");
    CHECK(mct.RepeatFrom2 == 1, "comparison hybrid repeat2");
    CHECK(mct.nHybrid[0] == 3, "comparison hybrid nHybrid0");
    CHECK(mct.nHybrid[1] == 2, "comparison hybrid nHybrid1");
    CHECK(mct.nHybrid[2] == 0, "comparison hybrid nHybrid2");
    CHECK(mct.nHybrid[3] == 0, "comparison hybrid nHybrid3");
    CHECK(mct.nHybrid[4] == 0, "comparison hybrid nHybrid4");
    CHECK(mct.nHybrid[5] == 0, "comparison hybrid nHybrid5");
}

static void test_external_formula_alias_and_schema_migration() {
    mb3d::InitExternalFormulaRegistry();

    TMandHeader10 header{};
    header.MandId = 44;
    header.mZstepDiv = 1.0f;

    {
        const auto* info = mb3d::FindExternalFormula("ABoxModKali");
        CHECK(info != nullptr, "ABoxModKali registry entry exists");
        if (info) {
            THeaderCustomAddon addon{};
            THAformula& formula = addon.Formulas[0];
            formula.iFnr = 25;
            formula.iOptionCount = 4;
            writeCustomFname(formula.CustomFname, "ABOXMODKALIFIXED");
            formula.dOptionValue[1] = 1.25;
            formula.dOptionValue[2] = 2.5;
            formula.dOptionValue[3] = 3.75;
            formula.dOptionValue[4] = 5.0;
            formula.dOptionValue[5] = 6.25;

            mb3d::UpdateFormulaOptionAbove20(header, addon);

            CHECK(formula.iFnr == 20, "ABoxModKaliFixed normalizes to generic external iFnr");
            CHECK(customFnameString(formula.CustomFname) == "ABoxModKali",
                  "ABoxModKaliFixed alias normalized");
            CHECK(formula.iOptionCount == expandedExternalOptionCount(*info),
                  "ABoxModKali option count expanded from registry");
            CHECK_NEAR(formula.dOptionValue[2], 3.75, 1e-12,
                       "ABoxModKaliFixed migrated option 3 -> slot 2");
            CHECK_NEAR(formula.dOptionValue[3], 5.0, 1e-12,
                       "ABoxModKaliFixed migrated option 4 -> slot 3");
            CHECK_NEAR(formula.dOptionValue[4], 6.25, 1e-12,
                       "ABoxModKaliFixed migrated option 5 -> slot 4");

            uint8_t expectedTypes[16];
            buildExpandedExternalOptionTypes(*info, expectedTypes);
            for (int i = 0; i < formula.iOptionCount; ++i) {
                CHECK(formula.byOptionType[i] == expectedTypes[i],
                      "ABoxModKali normalized option type matches registry");
            }
        }
    }

    {
        const auto* info = mb3d::FindExternalFormula("SphereHeightMap");
        CHECK(info != nullptr, "SphereHeightMap registry entry exists");
        if (info) {
            THeaderCustomAddon addon{};
            THAformula& formula = addon.Formulas[0];
            formula.iFnr = 27;
            writeCustomFname(formula.CustomFname, "SphereHeightMap");
            formula.iOptionCount = 10;
            formula.dOptionValue[2] = 10.0;
            formula.dOptionValue[3] = 21.0;
            formula.dOptionValue[5] = 2.0;
            formula.dOptionValue[6] = 3.0;

            mb3d::UpdateFormulaOptionAbove20(header, addon);

            CHECK(formula.iFnr == 20, "SphereHeightMap normalizes to generic external iFnr");
            CHECK(customFnameString(formula.CustomFname) == "SphereHeightMap",
                  "SphereHeightMap name preserved during normalization");
            CHECK(formula.iOptionCount == expandedExternalOptionCount(*info),
                  "SphereHeightMap option count expanded from registry");
            CHECK_NEAR(formula.dOptionValue[8], 1.0, 1e-12,
                       "SphereHeightMap legacy scale slot inserted");
            CHECK_NEAR(formula.dOptionValue[2], 5.0, 1e-12,
                       "SphereHeightMap X offset renormalized by scale axis");
            CHECK_NEAR(formula.dOptionValue[3], 7.0, 1e-12,
                       "SphereHeightMap Y offset renormalized by scale axis");

            uint8_t expectedTypes[16];
            buildExpandedExternalOptionTypes(*info, expectedTypes);
            for (int i = 0; i < formula.iOptionCount; ++i) {
                CHECK(formula.byOptionType[i] == expectedTypes[i],
                      "SphereHeightMap normalized option type matches registry");
            }
        }
    }

    {
        THeaderCustomAddon addon{};
        THAformula& formula = addon.Formulas[0];
        formula.iFnr = 25;
        formula.iItCount = 7;
        formula.iOptionCount = 3;
        writeCustomFname(formula.CustomFname, "DefinitelyMissingFormula");
        formula.dOptionValue[0] = 42.0;

        mb3d::UpdateFormulaOptionAbove20(header, addon);

        CHECK(formula.iFnr == 25, "missing external formula preserves slot identity");
        CHECK(formula.iItCount == 7, "missing external formula preserves iteration count for validation");
        CHECK(formula.iOptionCount == 3, "missing external formula preserves option count");
        CHECK(customFnameString(formula.CustomFname) == "DefinitelyMissingFormula",
              "missing external formula preserves diagnostic name");
        CHECK_NEAR(formula.dOptionValue[0], 42.0, 0.0,
                   "missing external formula preserves editable parameters");
    }
}

// ============================================================================
// Test: Multiple roundtrips are stable
// ============================================================================
static void test_triple_roundtrip() {
    auto r1 = mb3d::LoadM3PFile(refPath("formula_3_Tricorn.m3p"));
    if (!r1.success) {
        std::printf("SKIP: triple_roundtrip (load failed)\n");
        return;
    }

    std::string tmpA = tempPath("mb3d_test_trip_a.m3p");
    std::string tmpB = tempPath("mb3d_test_trip_b.m3p");
    std::string tmpC = tempPath("mb3d_test_trip_c.m3p");

    mb3d::SaveM3PFile(tmpA, r1.header, r1.addon);
    auto r2 = mb3d::LoadM3PFile(tmpA);
    CHECK(r2.success, "trip2 load");

    mb3d::SaveM3PFile(tmpB, r2.header, r2.addon);
    auto r3 = mb3d::LoadM3PFile(tmpB);
    CHECK(r3.success, "trip3 load");

    mb3d::SaveM3PFile(tmpC, r3.header, r3.addon);

    // Compare B and C files byte-for-byte (after 2nd round, should stabilize)
    FILE* fb = std::fopen(tmpB.c_str(), "rb");
    FILE* fc = std::fopen(tmpC.c_str(), "rb");
    if (fb && fc) {
        uint8_t bufB[2048], bufC[2048];
        size_t szB = std::fread(bufB, 1, 2048, fb);
        size_t szC = std::fread(bufC, 1, 2048, fc);
        CHECK(szB == szC, "trip stable size");
        if (szB == szC) {
            CHECK(std::memcmp(bufB, bufC, szB) == 0, "trip stable bytes");
        }
    }
    if (fb) std::fclose(fb);
    if (fc) std::fclose(fc);

    std::remove(tmpA.c_str());
    std::remove(tmpB.c_str());
    std::remove(tmpC.c_str());
}

// ============================================================================
// main
// ============================================================================
static void test_mix_power_control_normalization() {
    auto source = mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    CHECK(source.success, "mix normalization source loads");
    if (!source.success) return;
    source.addon.bOptions1 = 2;
    source.addon.bOptions3 = 5;
    struct PowerCase { float saved; float effective; };
    // Pascal Mand.MakeHeader + Math3D.sNotZero: signed near-zero floor and
    // [-100,100] clamp. Negative zero follows the nonnegative branch.
    const PowerCase cases[] = {{0.0f,.01f},{-0.0f,.01f},{.005f,.01f},
        {-.005f,-.01f},{.01f,.01f},{-.01f,-.01f},{2,2},{-2,-2},
        {100,100},{-100,-100},{101,100},{-101,-100}};
    const auto path = tempPath("mix_power_controls.m3p");
    for (const auto& row : cases) {
        source.header.sFmixPow = row.saved;
        source.header.DEmixColorOption = 255;
        CHECK(mb3d::SaveM3PFile(path, source.header, source.addon), "save mix controls");
        const auto loaded = mb3d::LoadM3PFile(path);
        CHECK(loaded.success, "load mix controls");
        CHECK(loaded.header.sFmixPow == row.effective, "effective mix power matches Pascal controls");
        CHECK(loaded.header.DEmixColorOption == 2, "mix color control clamps to2");
    }
    source.addon.bOptions3 = 0;
    source.header.sFmixPow = 0;
    CHECK(mb3d::SaveM3PFile(path, source.header, source.addon), "save inactive mix controls");
    CHECK(mb3d::LoadM3PFile(path).header.sFmixPow == 0, "inactive mix power remains unchanged");
    for (uint8_t bits : {uint8_t{0},uint8_t{2},uint8_t{4},uint8_t{6},uint8_t{7},uint8_t{0xF7}}) {
        source.addon.bOptions2 = bits;
        CHECK(mb3d::SaveM3PFile(path, source.header, source.addon), "save inside combo controls");
        const uint8_t expected = (bits & 6u) == 6u ? static_cast<uint8_t>(bits & ~6u) : bits;
        CHECK(mb3d::LoadM3PFile(path).addon.bOptions2 == expected, "file inside combo matches Pascal readback");
    }
    std::remove(path.c_str());
}

static void test_reflection_control_normalization() {
    auto source=mb3d::LoadM3PFile(refPath("formula_0_IntPow2.m3p"));
    CHECK(source.success,"load absorption control base");
    if (!source.success) return;
    const auto path=tempPath("reflection_absorption_controls.m3p");
    source.header.bCalcSRautomatic|=1;
    source.header.MCoptions&=static_cast<uint8_t>(~1u);
    const struct {float saved,effective;} cases[]={{-1,1e-30f},{0,1e-30f},{1e-30f,1e-30f},
        {0.2f,0.2f},{1e10f,1e10f},{1e20f,1e10f}};
    for (const auto& row:cases) {
        source.header.sTransmissionAbsorption=row.saved;
        CHECK(mb3d::SaveM3PFile(path,source.header,source.addon),"save absorption control");
        const auto loaded=mb3d::LoadM3PFile(path);
        CHECK(loaded.success,"load absorption control");
        CHECK(loaded.header.sTransmissionAbsorption==row.effective,"active absorption matches original control bounds");
        CHECK(mb3d::SaveM3PFile(path,loaded.header,loaded.addon),"save normalized absorption");
        CHECK(mb3d::LoadM3PFile(path).header.sTransmissionAbsorption==row.effective,"absorption normalization is idempotent");
    }
    struct Controls {
        float amount, index, scattering;
        uint8_t count;
    };
    struct ControlCase { Controls saved, effective; };
    const ControlCase controls[] = {
        {{-1, -1, -1, 0}, {0, .1f, 0, 1}},
        {{0, .1f, 0, 1}, {0, .1f, 0, 1}},
        {{.5f, 1.5f, 1, 3}, {.5f, 1.5f, 1, 3}},
        {{100, 10, 1e30f, 8}, {100, 10, 1e30f, 8}},
        {{101, 11, 1e31f, 255}, {100, 10, 1e30f, 8}},
    };
    for (const auto& row : controls) {
        source.header.SRamount = row.saved.amount;
        source.header.SRreflectioncount = row.saved.count;
        source.header.sTRIndex = row.saved.index;
        source.header.sTRscattering = row.saved.scattering;
        CHECK(mb3d::SaveM3PFile(path, source.header, source.addon), "save reflection controls");
        const auto loaded = mb3d::LoadM3PFile(path);
        CHECK(loaded.success, "load reflection controls");
        CHECK(loaded.header.SRamount == row.effective.amount, "reflection amount matches original bounds");
        CHECK(loaded.header.SRreflectioncount == row.effective.count, "reflection count matches original bounds");
        CHECK(loaded.header.sTRIndex == row.effective.index, "refraction index matches original bounds");
        CHECK(loaded.header.sTRscattering == row.effective.scattering, "scattering matches original bounds");
        CHECK(mb3d::SaveM3PFile(path, loaded.header, loaded.addon), "save normalized reflection controls");
        const auto reloaded = mb3d::LoadM3PFile(path);
        CHECK(reloaded.header.SRamount == loaded.header.SRamount &&
              reloaded.header.SRreflectioncount == loaded.header.SRreflectioncount &&
              reloaded.header.sTRIndex == loaded.header.sTRIndex &&
              reloaded.header.sTRscattering == loaded.header.sTRscattering,
              "reflection control normalization is idempotent");
    }
    source.header.bCalcSRautomatic&=static_cast<uint8_t>(~1u);
    source.header.sTransmissionAbsorption=0;
    CHECK(mb3d::SaveM3PFile(path,source.header,source.addon),"save dormant absorption");
    CHECK(mb3d::LoadM3PFile(path).header.sTransmissionAbsorption==0,"dormant absorption remains unmodified");
    const auto dormant = mb3d::LoadM3PFile(path);
    CHECK(dormant.header.SRamount == source.header.SRamount &&
          dormant.header.SRreflectioncount == source.header.SRreflectioncount &&
          dormant.header.sTRIndex == source.header.sTRIndex &&
          dormant.header.sTRscattering == source.header.sTRscattering,
          "dormant reflection controls remain unmodified");
    std::remove(path.c_str());
}

int main() {
    install_test_console_crash_handlers();
    std::printf("Running M3P Parser tests...\n\n");

    // Struct size tests
    test_struct_sizes();

    // Load tests for each reference file
    test_load_IntPow2();
    test_load_IntPow3();
    test_load_IntPow4();
    test_load_IntPow5();
    test_load_IntPow6();
    test_load_IntPow7();
    test_load_IntPow8();
    test_load_RealPower();
    test_load_Quaternion();
    test_load_Tricorn();
    test_load_AmazingBox();
    test_load_Bulbox();
    test_load_FoldIntPow();
    test_load_AexionC();

    // Header field validation
    test_header_fields();
    test_all_headers_reasonable();
    test_formula_identification();
    test_comparison_scene_internal_formula_schema();
    test_hybrid_scene_mct_setup();
    test_mix_power_control_normalization();
    test_reflection_control_normalization();
    test_external_formula_alias_and_schema_migration();

    // Round-trip tests
    test_roundtrip();
    test_roundtrip_bytes();
    test_triple_roundtrip();

    // Save/load minimal
    test_save_load_minimal();

    // Error handling
    test_error_nonexistent();
    test_error_corrupt();
    test_error_truncated();

    std::printf("\n========================================\n");
    std::printf("Results: %d passed, %d failed\n", g_passed, g_failed);
    std::printf("========================================\n");

    return g_failed > 0 ? 1 : 0;
}
