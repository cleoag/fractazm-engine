#include "core/types.h"
#include "core/math3d.h"
#include "core/header_trafos.h"
#include "fixtures/camera_setup_regressions.h"
#include "fixtures/ray_refinement_regressions.h"
#include "windows_console_crash_guard.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "fixtures/clip15_regressions.h"
#include "fixtures/packed_normal_regressions.h"

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
// Vector ops
// ============================================================================

static void test_vec3d_add() {
    TVec3D a = {1.0, 2.0, 3.0};
    TVec3D b = {4.0, 5.0, 6.0};
    TVec3D r;
    mb3d::AddVec3D(r, a, b);
    CHECK_NEAR(r[0], 5.0, 1e-15, "AddVec3D [0]");
    CHECK_NEAR(r[1], 7.0, 1e-15, "AddVec3D [1]");
    CHECK_NEAR(r[2], 9.0, 1e-15, "AddVec3D [2]");
}

static void test_vec3d_subtract() {
    TVec3D a = {10.0, 20.0, 30.0};
    TVec3D b = {1.0, 2.0, 3.0};
    TVec3D r;
    mb3d::SubtractVec3D(r, a, b);
    CHECK_NEAR(r[0], 9.0, 1e-15, "SubtractVec3D [0]");
    CHECK_NEAR(r[1], 18.0, 1e-15, "SubtractVec3D [1]");
    CHECK_NEAR(r[2], 27.0, 1e-15, "SubtractVec3D [2]");
}

static void test_vec3d_scale() {
    TVec3D a = {1.0, 2.0, 3.0};
    TVec3D r;
    mb3d::ScaleVec3D(r, a, 2.5);
    CHECK_NEAR(r[0], 2.5, 1e-15, "ScaleVec3D [0]");
    CHECK_NEAR(r[1], 5.0, 1e-15, "ScaleVec3D [1]");
    CHECK_NEAR(r[2], 7.5, 1e-15, "ScaleVec3D [2]");
}

static void test_vec3d_dot() {
    TVec3D a = {1.0, 0.0, 0.0};
    TVec3D b = {0.0, 1.0, 0.0};
    CHECK_NEAR(mb3d::DotVec3D(a, b), 0.0, 1e-15, "DotVec3D orthogonal");
    TVec3D c = {1.0, 2.0, 3.0};
    TVec3D d = {4.0, 5.0, 6.0};
    CHECK_NEAR(mb3d::DotVec3D(c, d), 32.0, 1e-15, "DotVec3D");
}

static void test_vec3d_cross() {
    TVec3D x = {1.0, 0.0, 0.0};
    TVec3D y = {0.0, 1.0, 0.0};
    TVec3D r;
    mb3d::CrossVec3D(r, x, y);
    CHECK_NEAR(r[0], 0.0, 1e-15, "CrossVec3D x cross y [0]");
    CHECK_NEAR(r[1], 0.0, 1e-15, "CrossVec3D x cross y [1]");
    CHECK_NEAR(r[2], 1.0, 1e-15, "CrossVec3D x cross y [2]");
}

static void test_vec3d_normalize() {
    TVec3D a = {3.0, 4.0, 0.0};
    TVec3D r;
    mb3d::NormalizeVec3D(r, a);
    CHECK_NEAR(r[0], 0.6, 1e-10, "NormalizeVec3D [0]");
    CHECK_NEAR(r[1], 0.8, 1e-10, "NormalizeVec3D [1]");
    CHECK_NEAR(r[2], 0.0, 1e-10, "NormalizeVec3D [2]");
    CHECK_NEAR(mb3d::LengthOfVec(r), 1.0, 1e-10, "NormalizeVec3D length");
}

static void test_vec3d_length() {
    TVec3D a = {3.0, 4.0, 0.0};
    CHECK_NEAR(mb3d::LengthOfVec(a), 5.0, 1e-15, "LengthOfVec");
    CHECK_NEAR(mb3d::SqrLengthOfVec(a), 25.0, 1e-15, "SqrLengthOfVec");
}

static void test_vec3d_add_weight() {
    TVec3D a = {1.0, 2.0, 3.0};
    TVec3D b = {10.0, 20.0, 30.0};
    mb3d::mAddVecWeight(a, b, 0.5);
    CHECK_NEAR(a[0], 6.0, 1e-15, "mAddVecWeight [0]");
    CHECK_NEAR(a[1], 12.0, 1e-15, "mAddVecWeight [1]");
    CHECK_NEAR(a[2], 18.0, 1e-15, "mAddVecWeight [2]");
}

// ============================================================================
// Single-precision vector ops
// ============================================================================

static void test_svec_ops() {
    TSVec a = {1.0f, 2.0f, 3.0f, 0.0f};
    TSVec b = {4.0f, 5.0f, 6.0f, 0.0f};
    TSVec r;

    mb3d::AddSVec(r, a, b);
    CHECK_NEAR(r[0], 5.0f, 1e-6, "AddSVec [0]");
    CHECK_NEAR(r[1], 7.0f, 1e-6, "AddSVec [1]");
    CHECK_NEAR(r[2], 9.0f, 1e-6, "AddSVec [2]");
    CHECK_NEAR(r[3], 0.0f, 1e-6, "AddSVec [3]");

    mb3d::SubSVec(r, a, b);
    CHECK_NEAR(r[0], -3.0f, 1e-6, "SubSVec [0]");

    mb3d::ScaleSVec(r, a, 3.0f);
    CHECK_NEAR(r[0], 3.0f, 1e-6, "ScaleSVec [0]");
    CHECK_NEAR(r[2], 9.0f, 1e-6, "ScaleSVec [2]");

    CHECK_NEAR(mb3d::DotSVec(a, b), 32.0f, 1e-5, "DotSVec");
}

static void test_svec_yof() {
    TSVec sv = {100.0f, 100.0f, 100.0f, 0.0f};
    float y = mb3d::YofSVec(sv);
    // 100*0.3 + 100*0.59 + 100*0.11 = 100
    CHECK_NEAR(y, 100.0f, 0.01f, "YofSVec uniform");

    TSVec sv2 = {255.0f, 0.0f, 0.0f, 0.0f};
    float y2 = mb3d::YofSVec(sv2);
    CHECK_NEAR(y2, 255.0f * 0.3f, 0.01f, "YofSVec red only");
}

// ============================================================================
// Matrix multiply
// ============================================================================

static void test_matrix_multiply() {
    // Identity * anything = anything
    TMatrix3 identity = {{1,0,0},{0,1,0},{0,0,1}};
    TMatrix3 m = {{1,2,3},{4,5,6},{7,8,9}};
    TMatrix3 original;
    std::memcpy(original, m, sizeof(TMatrix3));

    mb3d::MultiplyMatrix3(m, identity);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            CHECK_NEAR(m[i][j], original[i][j], 1e-10, "MultiplyMatrix3 identity");
}

static void test_matrix_build_rot() {
    // Build a rotation matrix for zero angles -> identity
    TMatrix3 m;
    mb3d::BuildRotMatrix(0.0, 0.0, 0.0, m);
    CHECK_NEAR(m[0][0], 1.0, 1e-15, "BuildRotMatrix zero [0][0]");
    CHECK_NEAR(m[1][1], 1.0, 1e-15, "BuildRotMatrix zero [1][1]");
    CHECK_NEAR(m[2][2], 1.0, 1e-15, "BuildRotMatrix zero [2][2]");
    CHECK_NEAR(m[0][1], 0.0, 1e-15, "BuildRotMatrix zero [0][1]");

    // 90-degree rotation around Z axis
    TMatrix3 m2;
    mb3d::BuildRotMatrix(0.0, 0.0, mb3d::Pi * 0.5, m2);
    // cosZ=0, sinZ=1 -> m[0][0]=0, m[0][1]=-1, m[1][0]=1, m[1][1]=0
    CHECK_NEAR(m2[0][0], 0.0, 1e-10, "BuildRotMatrix 90Z [0][0]");
    CHECK_NEAR(m2[0][1], -1.0, 1e-10, "BuildRotMatrix 90Z [0][1]");
    CHECK_NEAR(m2[1][0], 1.0, 1e-10, "BuildRotMatrix 90Z [1][0]");
}

static void test_normalise_matrix() {
    TMatrix3 m = {{2,0,0},{0,3,0},{0,0,4}};
    TMatrix3 r;
    mb3d::NormaliseMatrixTo(r, 1.0, m);
    CHECK_NEAR(r[0][0], 1.0, 1e-10, "NormaliseMatrixTo row0");
    CHECK_NEAR(r[1][1], 1.0, 1e-10, "NormaliseMatrixTo row1");
    CHECK_NEAR(r[2][2], 1.0, 1e-10, "NormaliseMatrixTo row2");
}

static void test_rotate_vector() {
    // Rotate (1,0,0) by 90 degrees around Z
    // BuildRotMatrix with za=pi/2: cosZ=0, sinZ=1
    // M[0][0]=0, M[0][1]=-1, M[1][0]=1, M[1][1]=0
    // RotateVector: v'[0] = 1*0 = 0, v'[1] = 1*1 = 1
    TMatrix3 m;
    mb3d::BuildRotMatrix(0.0, 0.0, mb3d::Pi * 0.5, m);
    TVec3D v = {1.0, 0.0, 0.0};
    mb3d::RotateVector(v, m);
    CHECK_NEAR(v[0], 0.0, 1e-10, "RotateVector 90Z [0]");
    CHECK_NEAR(v[1], 1.0, 1e-10, "RotateVector 90Z [1]");
    CHECK_NEAR(v[2], 0.0, 1e-10, "RotateVector 90Z [2]");
}

// ============================================================================
// Quaternion ops
// ============================================================================

static void test_quat_identity() {
    TQuaternion q = {0, 0, 0, 1};
    TMatrix3 m;
    mb3d::QuatToMatrix(m, q);
    CHECK_NEAR(m[0][0], 1.0, 1e-10, "QuatToMatrix identity [0][0]");
    CHECK_NEAR(m[1][1], 1.0, 1e-10, "QuatToMatrix identity [1][1]");
    CHECK_NEAR(m[2][2], 1.0, 1e-10, "QuatToMatrix identity [2][2]");
}

static void test_quat_roundtrip() {
    // In Pascal, MatrixToQuat and QuatToMatrix (CreateMatrixFromQuat)
    // are self-consistent: MatrixToQuat extracts the quaternion conjugate,
    // and QuatToMatrix uses the same transposed convention.
    // The roundtrip MatrixToQuat -> QuatToMatrix produces the transpose.
    // This is by design in the Pascal code.

    // Test: QuatToMatrix -> MatrixToQuat -> QuatToMatrix roundtrip
    // The double roundtrip (transpose of transpose) should give back the original.
    double halfAngle = 0.3 * 0.5;
    double s = std::sin(halfAngle) / std::sqrt(3.0);
    TQuaternion q1 = {s, s, s, std::cos(halfAngle)};
    mb3d::NormaliseQuat(q1);

    TMatrix3 m1;
    mb3d::QuatToMatrix(m1, q1);

    TQuaternion q2;
    mb3d::MatrixToQuat(q2, m1);

    TMatrix3 m1t; // transposed
    mb3d::QuatToMatrix(m1t, q2);

    TQuaternion q3;
    mb3d::MatrixToQuat(q3, m1t);

    TMatrix3 m1_back;
    mb3d::QuatToMatrix(m1_back, q3);

    // Double roundtrip should restore original
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            CHECK_NEAR(m1_back[i][j], m1[i][j], 1e-6, "Quat double roundtrip");

    // Also verify that QuatToMatrix produces a valid rotation matrix
    // (orthogonal, det = 1)
    double det = m1[0][0]*(m1[1][1]*m1[2][2] - m1[1][2]*m1[2][1])
               - m1[0][1]*(m1[1][0]*m1[2][2] - m1[1][2]*m1[2][0])
               + m1[0][2]*(m1[1][0]*m1[2][1] - m1[1][1]*m1[2][0]);
    CHECK_NEAR(det, 1.0, 1e-10, "QuatToMatrix determinant");

    // Verify MatrixToQuat gives unit quaternion
    double qlen = std::sqrt(q2[0]*q2[0] + q2[1]*q2[1] + q2[2]*q2[2] + q2[3]*q2[3]);
    CHECK_NEAR(qlen, 1.0, 1e-10, "MatrixToQuat unit length");
}

static void test_slerp() {
    TQuaternion q1 = {0, 0, 0, 1};
    TQuaternion q2;
    // 90 degrees around Z
    double halfAngle = mb3d::Pi * 0.25;
    q2[0] = 0;
    q2[1] = 0;
    q2[2] = std::sin(halfAngle);
    q2[3] = std::cos(halfAngle);

    TQuaternion result;
    mb3d::Slerp(result, q1, q2, 0.5);
    mb3d::NormaliseQuat(result);

    // Should be approximately 45/2 = 22.5 degrees around Z
    double expectedAngle = mb3d::Pi * 0.125;
    CHECK_NEAR(result[2], std::sin(expectedAngle), 0.01, "Slerp z component");
    CHECK_NEAR(result[3], std::cos(expectedAngle), 0.01, "Slerp w component");
}

// ============================================================================
// ShortFloat round-trip
// ============================================================================

static void test_shortfloat_roundtrip() {
    float testValues[] = {0.0f, 1.0f, -1.0f, 0.5f, 42.0f, -0.001f, 1e10f, 1e-10f, 3.14f};
    for (float val : testValues) {
        ShortFloat sf;
        mb3d::SingleToShortFloat(val, sf);
        float recovered = mb3d::ShortFloatToSingle(sf);
        if (val == 0.0f) {
            CHECK_NEAR(recovered, 0.0, 1e-30, "ShortFloat zero");
        } else {
            // ShortFloat has limited precision (~2 significant digits)
            double relErr = std::fabs((recovered - val) / val);
            CHECK(relErr < 0.1, "ShortFloat roundtrip relative error < 10%");
        }
    }
}

// ============================================================================
// Double7B round-trip
// ============================================================================

static void test_double7b_roundtrip() {
    double testValues[] = {0.0, 1.0, -1.0, 3.14159265358979, 1e100, -1e-50, 42.5};
    for (double val : testValues) {
        Double7B d7b;
        mb3d::DoubleToD7B(val, d7b);
        double recovered = mb3d::D7BtoDouble(d7b);
        // The lowest byte is truncated, so we lose some precision
        // but it should be very close for most values
        if (val == 0.0) {
            CHECK_NEAR(recovered, 0.0, 1e-300, "Double7B zero");
        } else {
            double relErr = std::fabs((recovered - val) / val);
            CHECK(relErr < 1e-6, "Double7B roundtrip relative error < 1e-6");
        }
    }
}

// ============================================================================
// Min/Max/Clamp
// ============================================================================

static void test_min_max_clamp() {
    CHECK_NEAR(mb3d::MinCD(3.0, 5.0), 3.0, 1e-15, "MinCD");
    CHECK_NEAR(mb3d::MaxCD(3.0, 5.0), 5.0, 1e-15, "MaxCD");
    CHECK_NEAR(mb3d::MinCS(3.0f, 5.0f), 3.0f, 1e-6, "MinCS");
    CHECK_NEAR(mb3d::MaxCS(3.0f, 5.0f), 5.0f, 1e-6, "MaxCS");
    CHECK_NEAR(mb3d::MinMaxCS(0.0f, -1.0f, 1.0f), 0.0f, 1e-6, "MinMaxCS below");
    CHECK_NEAR(mb3d::MinMaxCS(0.0f, 0.5f, 1.0f), 0.5f, 1e-6, "MinMaxCS within");
    CHECK_NEAR(mb3d::MinMaxCS(0.0f, 2.0f, 1.0f), 1.0f, 1e-6, "MinMaxCS above");
    CHECK_NEAR(mb3d::Clamp0D(-5.0), 0.0, 1e-15, "Clamp0D negative");
    CHECK_NEAR(mb3d::Clamp0D(5.0), 5.0, 1e-15, "Clamp0D positive");
    CHECK_NEAR(mb3d::Clamp01S(-0.5f), 0.0f, 1e-6, "Clamp01S below");
    CHECK_NEAR(mb3d::Clamp01S(0.5f), 0.5f, 1e-6, "Clamp01S within");
    CHECK_NEAR(mb3d::Clamp01S(1.5f), 1.0f, 1e-6, "Clamp01S above");
}

// ============================================================================
// Conversions
// ============================================================================

static void test_svec_dvec_conversion() {
    TSVec sv = {1.5f, 2.5f, 3.5f, 0.0f};
    TVec3D dv;
    mb3d::SVecToDVec(dv, sv);
    CHECK_NEAR(dv[0], 1.5, 1e-6, "SVecToDVec [0]");
    CHECK_NEAR(dv[1], 2.5, 1e-6, "SVecToDVec [1]");
    CHECK_NEAR(dv[2], 3.5, 1e-6, "SVecToDVec [2]");

    TSVec sv2;
    mb3d::DVecToSVec(sv2, dv);
    CHECK_NEAR(sv2[0], 1.5f, 1e-6, "DVecToSVec [0]");
    CHECK_NEAR(sv2[3], 0.0f, 1e-6, "DVecToSVec [3]");
}

static void test_col_to_svec() {
    // 0xRRGGBB in little-endian: col = 0x00BBGGRR
    uint32_t col = 0x00FF8040;  // R=0x40, G=0x80, B=0xFF
    TSVec sv;
    mb3d::ColToSVec(sv, col);
    CHECK_NEAR(sv[0], 64.0f, 1e-6, "ColToSVec R");
    CHECK_NEAR(sv[1], 128.0f, 1e-6, "ColToSVec G");
    CHECK_NEAR(sv[2], 255.0f, 1e-6, "ColToSVec B");
}

static void test_rgb_to_svec_scale() {
    TRGB rgb = {100, 200, 50};
    TSVec sv;
    mb3d::RGBtoSVecScale(sv, rgb, 1.0f / 255.0f);
    CHECK_NEAR(sv[0], 100.0f / 255.0f, 1e-5, "RGBtoSVecScale R");
    CHECK_NEAR(sv[1], 200.0f / 255.0f, 1e-5, "RGBtoSVecScale G");
    CHECK_NEAR(sv[2],  50.0f / 255.0f, 1e-5, "RGBtoSVecScale B");
}

// ============================================================================
// Struct size checks (just to double-check static_asserts)
// ============================================================================

static void test_struct_sizes() {
    CHECK(sizeof(TsiLight5) == 18, "TsiLight5 size");
    CHECK(sizeof(TLight8) == 32, "TLight8 size");
    CHECK(sizeof(TLightingParas9) == 408, "TLightingParas9 size");
    CHECK(sizeof(THAformula) == 188, "THAformula size");
    CHECK(sizeof(THeaderCustomAddon) == 1136, "THeaderCustomAddon size");
    CHECK(sizeof(TMandHeader10) == 840, "TMandHeader10 size");
    CHECK(sizeof(TLCol8) == 10, "TLCol8 size");
    CHECK(sizeof(TICol8) == 6, "TICol8 size");
}

// ============================================================================
// Distance
// ============================================================================

static void test_distance() {
    TVec3D a = {0.0, 0.0, 0.0};
    TVec3D b = {3.0, 4.0, 0.0};
    CHECK_NEAR(mb3d::DistanceOf2Vecs(a, b), 5.0, 1e-15, "DistanceOf2Vecs");
}

// ============================================================================
// Trig helpers
// ============================================================================

static void test_sincos() {
    double s, c;
    mb3d::SinCosD(0.0, s, c);
    CHECK_NEAR(s, 0.0, 1e-15, "SinCosD sin(0)");
    CHECK_NEAR(c, 1.0, 1e-15, "SinCosD cos(0)");

    mb3d::SinCosD(mb3d::Pi * 0.5, s, c);
    CHECK_NEAR(s, 1.0, 1e-10, "SinCosD sin(pi/2)");
    CHECK_NEAR(c, 0.0, 1e-10, "SinCosD cos(pi/2)");
}

static void test_arccos_safe() {
    CHECK_NEAR(mb3d::ArcCosSafe(1.5), 0.0, 1e-10, "ArcCosSafe > 1");
    CHECK_NEAR(mb3d::ArcCosSafe(-1.5), mb3d::Pi, 1e-10, "ArcCosSafe < -1");
    CHECK_NEAR(mb3d::ArcCosSafe(0.0), mb3d::Pi * 0.5, 1e-10, "ArcCosSafe 0");
}

// ============================================================================
// Main
// ============================================================================

static void test_pascal_packing_regressions() {
    for (const auto& row : clip15Regressions) {
        float input; std::memcpy(&input, &row.bits, 4);
        uint16_t output; mb3d::MinMaxClip15bit(input, output);
        if (output != row.expected)
            std::printf("  clip input=%08x actual=%u expected=%u\n", row.bits, output, row.expected);
        CHECK(output == row.expected, "15-bit color matches original SSE");
    }
    for (const auto& row : packedNormalRegressions) {
        TVec3D input; std::memcpy(input, row.bits, 24);
        int16_t output[3]; mb3d::MakeWNormalsFromDVec(output, input);
        for (int lane=0;lane<3;++lane)
            CHECK(output[lane] == row.expected[lane], "packed normal matches original x87");
    }
}

static void test_pascal_camera_setup_regressions() {
    for (const auto& row : cameraRowRegressions) {
        const double output = mb3d::CalcCameraRowAngle(row.y,row.height,row.fovy);
        uint64_t bits; std::memcpy(&bits,&output,sizeof bits);
        CHECK(bits == row.expected, "integer row camera angle matches Pascal SSE2");
    }
    for (const auto& row : rayLimiterRegressions) {
        const float output = mb3d::CalcRayStepLimiter(row.width,row.height,row.step,row.limiter);
        uint32_t bits; std::memcpy(&bits,&output,sizeof bits);
        CHECK(bits == row.expected, "typed ray step limiter matches Pascal SSE2");
    }
}

static void test_pascal_ray_refinement_regressions() {
    for (const auto& row : rayRefinementRegressions) {
        TVec3D pos = {row.input[0], row.input[1], row.input[2]};
        TVec3D direction = {row.input[5], row.input[6], row.input[7]};
        double zz = row.input[3];
        const float stop = mb3d::RefineRayStep(pos, zz, direction, row.input[4],
            static_cast<float>(row.input[8]), static_cast<float>(row.input[9]));
        const double actual[] = {pos[0], pos[1], pos[2], zz};
        for (int lane = 0; lane < 4; ++lane) {
            uint64_t bits; std::memcpy(&bits, &actual[lane], sizeof bits);
            CHECK(bits == row.expected[lane], "ray refinement matches original x87 bits");
        }
        uint32_t bits; std::memcpy(&bits, &stop, sizeof bits);
        CHECK(bits == row.expected[4], "refined DE stop matches original x87 bits");
    }
    for (const auto& row : marchLimitRegressions) {
        const double actual = mb3d::CalcMarchStepLimit(row.stop, row.limiter);
        uint64_t bits; std::memcpy(&bits, &actual, sizeof bits);
        CHECK(bits == row.expected, "march limit rounds Single before widening to Double");
    }
}

int main() {
    install_test_console_crash_handlers();
    std::printf("Running Math3D tests...\n\n");

    test_vec3d_add();
    test_vec3d_subtract();
    test_vec3d_scale();
    test_vec3d_dot();
    test_vec3d_cross();
    test_vec3d_normalize();
    test_vec3d_length();
    test_vec3d_add_weight();
    test_svec_ops();
    test_svec_yof();
    test_matrix_multiply();
    test_matrix_build_rot();
    test_normalise_matrix();
    test_rotate_vector();
    test_quat_identity();
    test_quat_roundtrip();
    test_slerp();
    test_shortfloat_roundtrip();
    test_double7b_roundtrip();
    test_min_max_clamp();
    test_svec_dvec_conversion();
    test_col_to_svec();
    test_rgb_to_svec_scale();
    test_struct_sizes();
    test_distance();
    test_sincos();
    test_arccos_safe();
    test_pascal_packing_regressions();
    test_pascal_camera_setup_regressions();
    test_pascal_ray_refinement_regressions();

    std::printf("\n========================================\n");
    std::printf("Results: %d passed, %d failed\n", g_passed, g_failed);
    std::printf("========================================\n");

    return g_failed > 0 ? 1 : 0;
}
