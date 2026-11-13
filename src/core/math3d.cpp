#include "core/math3d.h"
#include "portable_math.h"
#include "core/extended80.h"
#include "core/numeric_precision.h"
#include "core/extended_transcendentals.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdlib>
#include <bit>

#if defined(_MSC_VER) && defined(_M_X64)
#include <emmintrin.h>
#endif

namespace mb3d {
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void RefineRayStep_x87(double*, float*);
#endif

double CalcCameraRowAngle(int y, int height, double fovy) {
    return (static_cast<double>(y) / static_cast<double>(height) - 0.5) * fovy;
}

void BuildViewVectorFOVDouble(double CY, double CX, TSVec& result) {
    double sinY = std::sin(CY);
    double cosY = std::cos(CY);
    double sinX = std::sin(CX);
    double cosX = std::cos(CX);
    double rx = -sinX;
    double ry = sinY;
    double rz = cosX * cosY;
    double lenSq = rx * rx + ry * ry + rz * rz;
    if (lenSq > 1e-30) {
        double invLen = 1.0 / std::sqrt(lenSq);
        rx *= invLen;
        ry *= invLen;
        rz *= invLen;
    }
    result[0] = static_cast<float>(rx);
    result[1] = static_cast<float>(ry);
    result[2] = static_cast<float>(rz);
    result[3] = 0;
}



static inline bool DebugPascalVecAddSse2Enabled() {
#if defined(_MSC_VER) && defined(_M_X64)
    static const bool value = [] {
        const char* spec = std::getenv("MB3D_DEBUG_PASCAL_VECADD_SSE2");
        return spec && *spec;
    }();
    return value;
#else
    return false;
#endif
}

// ============================================================================
// Utility: Min / Max / Clamp
// ============================================================================

double MinCD(double s1, double s2) {
    return (s1 < s2) ? s1 : s2;
}

double MaxCD(double s1, double s2) {
    return (s1 > s2) ? s1 : s2;
}

float MinCS(float s1, float s2) {
    return (s1 < s2) ? s1 : s2;
}

float MaxCS(float s1, float s2) {
    return (s1 > s2) ? s1 : s2;
}

float MinMaxCS(float smin, float s, float smax) {
    if (s < smin) return smin;
    if (s > smax) return smax;
    return s;
}

float Min0MaxCS(float s, float smax) {
    if (s < 0.0f) return 0.0f;
    if (s > smax) return smax;
    return s;
}

double Min0MaxCD(double d, double dmax) {
    if (d < 0.0) return 0.0;
    if (d > dmax) return dmax;
    return d;
}

double MinMaxCD(double dmin, double d, double dmax) {
    if (d < dmin) return dmin;
    if (d > dmax) return dmax;
    return d;
}

double Clamp0D(double d) {
    return (d <= 0.0) ? 0.0 : d;
}

float Clamp01S(float sv) {
    if (sv <= 0.0f) return 0.0f;
    if (sv > 1.0f) return 1.0f;
    return sv;
}

double Clamp01D(double dv) {
    if (dv <= 0.0) return 0.0;
    if (dv > 1.0) return 1.0;
    return dv;
}

double MaxAbsCD(double s1, double s2) {
    double a1 = std::fabs(s1);
    double a2 = std::fabs(s2);
    return (a1 > a2) ? s1 : s2;
}

double MinAbsCD(double s1, double s2) {
    double a1 = std::fabs(s1);
    double a2 = std::fabs(s2);
    return (a1 < a2) ? s1 : s2;
}

// ============================================================================
// Custom type conversions
// ============================================================================

// ShortFloat: [0] = mantissa (signed), [1] = exponent (signed)
// Result = sf[0] * 10^(clamp(sf[1], -25, 25) - 1)
float ShortFloatToSingle(const ShortFloat& sf) {
    int8_t mant = sf[0];
    int8_t exp  = sf[1];
    int clamped = exp;
    if (clamped < -25) clamped = -25;
    if (clamped >  25) clamped =  25;
    return static_cast<float>(mant) * std::pow(10.0f, static_cast<float>(clamped - 1));
}

void SingleToShortFloat(float s, ShortFloat& result) {
    int e = 0;
    if (std::fabs(s) < 1e-45f) {
        result[0] = 0;
        result[1] = 0;
    } else if (std::fabs(s) > 1e38f) {
        result[0] = 99;
        result[1] = 38;
    } else {
        while (std::fabs(s) >= 9.95f) {
            s *= 0.1f;
            ++e;
        }
        while (std::fabs(s) <= 0.995f) {
            s *= 10.0f;
            --e;
        }
        result[0] = static_cast<int8_t>(std::lround(s * 10.0f));
        result[1] = static_cast<int8_t>(e);
    }
}

// Double7B: copy 7 upper bytes of a double (bytes 1..7)
// Pascal: memcpy bytes [1..7] of the double to the 7-byte array
double D7BtoDouble(const Double7B& d7b) {
    uint8_t buf[8];
    buf[0] = 0;
    std::memcpy(&buf[1], d7b, 7);
    double result;
    std::memcpy(&result, buf, 8);
    return result;
}

void DoubleToD7B(double d, Double7B& result) {
    uint8_t buf[8];
    std::memcpy(buf, &d, 8);
    std::memcpy(result, &buf[1], 7);
}

// ============================================================================
// Vector construction
// ============================================================================

void Vec3D(TVec3D& result, double x, double y, double z) {
    result[0] = x;
    result[1] = y;
    result[2] = z;
}

// ============================================================================
// Double-precision vector ops (TVec3D)
// ============================================================================

void AddVec3D(TVec3D& result, const TVec3D& v1, const TVec3D& v2) {
    result[0] = v1[0] + v2[0];
    result[1] = v1[1] + v2[1];
    result[2] = v1[2] + v2[2];
}

void AddVec3D_inplace(TVec3D& v1, const TVec3D& v2) {
    v1[0] += v2[0];
    v1[1] += v2[1];
    v1[2] += v2[2];
}

void SubtractVec3D(TVec3D& result, const TVec3D& v1, const TVec3D& v2) {
    result[0] = v1[0] - v2[0];
    result[1] = v1[1] - v2[1];
    result[2] = v1[2] - v2[2];
}

void ScaleVec3D(TVec3D& result, const TVec3D& v, double s) {
    result[0] = v[0] * s;
    result[1] = v[1] * s;
    result[2] = v[2] * s;
}

void ScaleVec3D_inplace(TVec3D& v, double s) {
    v[0] *= s;
    v[1] *= s;
    v[2] *= s;
}

double DotVec3D(const TVec3D& v1, const TVec3D& v2) {
    return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void CrossVec3D(TVec3D& result, const TVec3D& v1, const TVec3D& v2) {
    double r0 = v1[1] * v2[2] - v1[2] * v2[1];
    double r1 = v1[2] * v2[0] - v1[0] * v2[2];
    double r2 = v1[0] * v2[1] - v1[1] * v2[0];
    result[0] = r0;
    result[1] = r1;
    result[2] = r2;
}

void NormalizeVec3D(TVec3D& result, const TVec3D& v) {
    double d = 1.0 / std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + 1e-100);
    result[0] = v[0] * d;
    result[1] = v[1] * d;
    result[2] = v[2] * d;
}

void NormalizeVec3D_inplace(TVec3D& v) {
    double d = 1.0 / std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + 1e-100);
    v[0] *= d;
    v[1] *= d;
    v[2] *= d;
}

double LengthOfVec(const TVec3D& v) {
    return std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

double SqrLengthOfVec(const TVec3D& v) {
    return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

double DistanceOf2Vecs(const TVec3D& v1, const TVec3D& v2) {
    double dx = v1[0] - v2[0];
    double dy = v1[1] - v2[1];
    double dz = v1[2] - v2[2];
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

// V1 += V2 * W
void mAddVecWeight(TVec3D& v1, const TVec3D& v2, double w) {
#if defined(_MSC_VER) && defined(_M_X64)
    if (DebugPascalVecAddSse2Enabled()) {
        const __m128d weight2 = _mm_load1_pd(&w);
        const __m128d v2xy = _mm_loadu_pd(&v2[0]);
        __m128d v1xy = _mm_loadu_pd(&v1[0]);
        v1xy = _mm_add_pd(v1xy, _mm_mul_pd(v2xy, weight2));
        _mm_storeu_pd(&v1[0], v1xy);

        double z = v1[2];
        z += v2[2] * w;
        v1[2] = z;
        return;
    }
#endif
    v1[0] += v2[0] * w;
    v1[1] += v2[1] * w;
    v1[2] += v2[2] * w;
}

float CalcMarchStepLimit(float stop, float limiter) {
    return MaxCS(stop, 0.4f) * limiter;
}

// Keep refinement separate from ordinary SSE2 marching (Calc.pas).
float RefineRayStep(TVec3D& pos, double& zz, const TVec3D& direction,
                    double step, float factor, float stop) {
#if defined(_MSC_VER) && defined(_M_X64)
    double values[]={pos[0],pos[1],pos[2],zz,step,
                     direction[0],direction[1],direction[2],factor,stop};
    float result;
    RefineRayStep_x87(values,&result);
    for (int i=0;i<3;++i) pos[i]=values[i];
    zz=values[3];
    return result;
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    zz = static_cast<double>(E(zz) + E(step));
    for (int i = 0; i < 3; ++i)
        pos[i] = static_cast<double>(E(direction[i]) * E(step) + E(pos[i]));
    return static_cast<float>((E(zz) * E(factor) + E(1)) * E(stop));
#else
    zz+=step;
    for (int i=0;i<3;++i)
        pos[i]=static_cast<double>(static_cast<long double>(direction[i])*step+pos[i]);
    return static_cast<float>((static_cast<long double>(zz)*factor+1)*stop);
#endif
}

// dest = src + add * w
void mCopyAddVecWeight(TVec3D& dest, const TVec3D& src, const TVec3D& add, double w) {
#if defined(_MSC_VER) && defined(_M_X64)
    if (DebugPascalVecAddSse2Enabled()) {
        const __m128d weight2 = _mm_load1_pd(&w);
        const __m128d addxy = _mm_loadu_pd(&add[0]);
        const __m128d srcxy = _mm_loadu_pd(&src[0]);
        const __m128d destxy = _mm_add_pd(srcxy, _mm_mul_pd(addxy, weight2));
        _mm_storeu_pd(&dest[0], destxy);

        double z = src[2];
        z += add[2] * w;
        dest[2] = z;
        return;
    }
#endif
    dest[0] = src[0] + add[0] * w;
    dest[1] = src[1] + add[1] * w;
    dest[2] = src[2] + add[2] * w;
}

// ============================================================================
// Single-precision vector ops (TSVec)
// ============================================================================

void AddSVec(TSVec& result, const TSVec& v1, const TSVec& v2) {
    result[0] = v1[0] + v2[0];
    result[1] = v1[1] + v2[1];
    result[2] = v1[2] + v2[2];
    result[3] = 0.0f;
}

void SubSVec(TSVec& result, const TSVec& v1, const TSVec& v2) {
    result[0] = v1[0] - v2[0];
    result[1] = v1[1] - v2[1];
    result[2] = v1[2] - v2[2];
    result[3] = 0.0f;
}

void ScaleSVec(TSVec& result, const TSVec& v, float s) {
    result[0] = v[0] * s;
    result[1] = v[1] * s;
    result[2] = v[2] * s;
    result[3] = 0.0f;
}

void ScaleSVec_inplace(TSVec& v, float s) {
    v[0] *= s;
    v[1] *= s;
    v[2] *= s;
}

float DotSVec(const TSVec& v1, const TSVec& v2) {
    return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void CrossSVec(TSVec& result, const TSVec& v1, const TSVec& v2) {
    float r0 = v1[1] * v2[2] - v1[2] * v2[1];
    float r1 = v1[2] * v2[0] - v1[0] * v2[2];
    float r2 = v1[0] * v2[1] - v1[1] * v2[0];
    result[0] = r0;
    result[1] = r1;
    result[2] = r2;
    result[3] = 0.0f;
}

float LengthOfSVec(const TSVec& v) {
    return std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

float SqrLengthOfSVec(const TSVec& v) {
    return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

void NormalizeSVec(TSVec& result, const TSVec& v) {
    double d = 1.0 / std::sqrt(
        static_cast<double>(v[0])*v[0] +
        static_cast<double>(v[1])*v[1] +
        static_cast<double>(v[2])*v[2] + 1e-100);
    result[0] = static_cast<float>(v[0] * d);
    result[1] = static_cast<float>(v[1] * d);
    result[2] = static_cast<float>(v[2] * d);
    result[3] = 0.0f;
}

void NormalizeSVec_inplace(TSVec& v) {
    double d = 1.0 / std::sqrt(
        static_cast<double>(v[0])*v[0] +
        static_cast<double>(v[1])*v[1] +
        static_cast<double>(v[2])*v[2] + 1e-100);
    v[0] = static_cast<float>(v[0] * d);
    v[1] = static_cast<float>(v[1] * d);
    v[2] = static_cast<float>(v[2] * d);
}

// YofSVec: luminance = 0.3*R + 0.59*G + 0.11*B, clamped to >= 0
float YofSVec(const TSVec& sv) {
    float y = sv[0] * 0.3f + sv[1] * 0.59f + sv[2] * 0.11f;
    return (y < 0.0f) ? 0.0f : y;
}

void ClearSVec(TSVec& sv) {
    sv[0] = 0.0f;
    sv[1] = 0.0f;
    sv[2] = 0.0f;
    sv[3] = 0.0f;
}

void ClearDVec(TVec3D& dv) {
    dv[0] = 0.0;
    dv[1] = 0.0;
    dv[2] = 0.0;
}

// ============================================================================
// Color/conversion helpers
// ============================================================================

void RGBtoSVecScale(TSVec& result, const TRGB& rgb, float scale) {
    result[0] = static_cast<float>(rgb[0]) * scale;
    result[1] = static_cast<float>(rgb[1]) * scale;
    result[2] = static_cast<float>(rgb[2]) * scale;
    result[3] = 0.0f;
}

void ColToSVec(TSVec& result, uint32_t col) {
    result[0] = static_cast<float>(col & 0xFF);
    result[1] = static_cast<float>((col >> 8) & 0xFF);
    result[2] = static_cast<float>((col >> 16) & 0xFF);
    result[3] = 0.0f;
}

void SVecToDVec(TVec3D& result, const TSVec& v) {
    result[0] = static_cast<double>(v[0]);
    result[1] = static_cast<double>(v[1]);
    result[2] = static_cast<double>(v[2]);
}

void DVecToSVec(TSVec& result, const TVec3D& v) {
    result[0] = static_cast<float>(v[0]);
    result[1] = static_cast<float>(v[1]);
    result[2] = static_cast<float>(v[2]);
    result[3] = 0.0f;
}

// ============================================================================
// Matrix ops (TMatrix3)
// ============================================================================

// M1 = M1 * M2 (in-place on M1)
void MultiplyMatrix3(TMatrix3& m1, const TMatrix3& m2) {
    TMatrix3 mt;
    std::memcpy(mt, m1, sizeof(TMatrix3));
    for (int i = 0; i < 3; ++i) {
        m1[i][0] = mt[i][0] * m2[0][0] + mt[i][1] * m2[1][0] + mt[i][2] * m2[2][0];
        m1[i][1] = mt[i][0] * m2[0][1] + mt[i][1] * m2[1][1] + mt[i][2] * m2[2][1];
        m1[i][2] = mt[i][0] * m2[0][2] + mt[i][1] * m2[1][2] + mt[i][2] * m2[2][2];
    }
}

// Normalize each row of M to length n
void NormaliseMatrixTo(TMatrix3& result, double n, const TMatrix3& m) {
    for (int i = 0; i < 3; ++i) {
        double d = n / std::sqrt(m[i][0]*m[i][0] + m[i][1]*m[i][1] + m[i][2]*m[i][2] + 1e-100);
        result[i][0] = m[i][0] * d;
        result[i][1] = m[i][1] * d;
        result[i][2] = m[i][2] * d;
    }
}

void TransposeMatrix3(TMatrix3& result, const TMatrix3& m) {
    TMatrix3 tmp;
    std::memcpy(tmp, m, sizeof(TMatrix3));
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            result[i][j] = tmp[j][i];
}

void BuildRotMatrix(double xa, double ya, double za, TMatrix3& m) {
    double sinX, cosX, sinY, cosY, sinZ, cosZ;
    SinCosD(xa, sinX, cosX);
    SinCosD(ya, sinY, cosY);
    SinCosD(za, sinZ, cosZ);
    m[0][0] = cosY * cosZ;
    m[0][1] = -cosY * sinZ;
    m[0][2] = sinY;
    m[1][0] = sinX * sinY * cosZ + cosX * sinZ;
    m[1][1] = cosX * cosZ - sinX * sinY * sinZ;
    m[1][2] = -sinX * cosY;
    m[2][0] = sinX * sinZ - cosX * sinY * cosZ;
    m[2][1] = cosX * sinY * sinZ + sinX * cosZ;
    m[2][2] = cosX * cosY;
}

// V = V rotated by M: V[i] = V[0]*M[0][i] + V[1]*M[1][i] + V[2]*M[2][i]
// This matches the Pascal RotateVector which does:
// V[0] = VT[0]*M[0,0] + VT[1]*M[0,1] + VT[2]*M[0,2]
// V[1] = VT[0]*M[1,0] + VT[1]*M[1,1] + VT[2]*M[1,2]
// V[2] = VT[0]*M[2,0] + VT[1]*M[2,1] + VT[2]*M[2,2]
void RotateVector(TVec3D& v, const TMatrix3& m) {
    TVec3D vt;
    std::memcpy(vt, v, sizeof(TVec3D));
    v[0] = vt[0] * m[0][0] + vt[1] * m[0][1] + vt[2] * m[0][2];
    v[1] = vt[0] * m[1][0] + vt[1] * m[1][1] + vt[2] * m[1][2];
    v[2] = vt[0] * m[2][0] + vt[1] * m[2][1] + vt[2] * m[2][2];
}

// Reverse rotation: V[i] = V[0]*M[i][0] + V[1]*M[i][1] + V[2]*M[i][2]
void RotateVectorReverse(TVec3D& v, const TMatrix3& m) {
    TVec3D vt;
    std::memcpy(vt, v, sizeof(TVec3D));
    v[0] = vt[0] * m[0][0] + vt[1] * m[1][0] + vt[2] * m[2][0];
    v[1] = vt[0] * m[0][1] + vt[1] * m[1][1] + vt[2] * m[2][1];
    v[2] = vt[0] * m[0][2] + vt[1] * m[1][2] + vt[2] * m[2][2];
}

void ScaleMatrix(double s, TMatrix3& m) {
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            m[i][j] *= s;
}

// ============================================================================
// Quaternion ops
// ============================================================================

// Hamilton product: result = q1 * q2
// Q = [x, y, z, w] where w is the scalar part
void QuatMultiply(TQuaternion& result, const TQuaternion& q1, const TQuaternion& q2) {
    result[3] = q1[3]*q2[3] - q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2];
    result[0] = q1[3]*q2[0] + q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1];
    result[1] = q1[3]*q2[1] - q1[0]*q2[2] + q1[1]*q2[3] + q1[2]*q2[0];
    result[2] = q1[3]*q2[2] + q1[0]*q2[1] - q1[1]*q2[0] + q1[2]*q2[3];
}

// Create rotation matrix from quaternion
// Matches Pascal CreateMatrixFromQuat
void QuatToMatrix(TMatrix3& m, const TQuaternion& q) {
    m[0][0] = 1.0 - 2.0 * (q[1]*q[1] + q[2]*q[2]);
    m[0][1] = 2.0 * (q[0]*q[1] + q[2]*q[3]);
    m[0][2] = 2.0 * (q[0]*q[2] - q[1]*q[3]);
    m[1][0] = 2.0 * (q[0]*q[1] - q[2]*q[3]);
    m[1][1] = 1.0 - 2.0 * (q[0]*q[0] + q[2]*q[2]);
    m[1][2] = 2.0 * (q[2]*q[1] + q[0]*q[3]);
    m[2][0] = 2.0 * (q[0]*q[2] + q[1]*q[3]);
    m[2][1] = 2.0 * (q[1]*q[2] - q[0]*q[3]);
    m[2][2] = 1.0 - 2.0 * (q[0]*q[0] + q[1]*q[1]);
}

// Extract quaternion from rotation matrix
// Matches Pascal MatrixToQuat
void MatrixToQuat(TQuaternion& q, const TMatrix3& m_in) {
    TMatrix3 m;
    NormaliseMatrixTo(m, 1.0, m_in);
    double T = 1.0 + m[0][0] + m[1][1] + m[2][2];
    if (T > 1e-15) {
        double S = 2.0 * std::sqrt(T);
        q[3] = 0.25 * S;
        S = 1.0 / S;
        q[0] = (m[2][1] - m[1][2]) * S;
        q[1] = (m[0][2] - m[2][0]) * S;
        q[2] = (m[1][0] - m[0][1]) * S;
    } else {
        if ((m[0][0] > m[1][1]) && (m[0][0] > m[2][2])) {
            double S = std::sqrt(1.0 + m[0][0] - m[1][1] - m[2][2]) * 2.0;
            q[0] = 0.25 * S;
            S = 1.0 / S;
            q[1] = (m[1][0] + m[0][1]) * S;
            q[2] = (m[0][2] + m[2][0]) * S;
            q[3] = (m[2][1] - m[1][2]) * S;
        } else if (m[1][1] > m[2][2]) {
            double S = std::sqrt(1.0 + m[1][1] - m[0][0] - m[2][2]) * 2.0;
            q[1] = 0.25 * S;
            S = 1.0 / S;
            q[0] = (m[1][0] + m[0][1]) * S;
            q[2] = (m[2][1] + m[1][2]) * S;
            q[3] = (m[0][2] - m[2][0]) * S;
        } else {
            double S = std::sqrt(1.0 + m[2][2] - m[0][0] - m[1][1]) * 2.0;
            q[2] = 0.25 * S;
            S = 1.0 / S;
            q[0] = (m[0][2] + m[2][0]) * S;
            q[1] = (m[2][1] + m[1][2]) * S;
            q[3] = (m[1][0] - m[0][1]) * S;
        }
    }
}

// Spherical linear interpolation
void Slerp(TQuaternion& result, const TQuaternion& q1, const TQuaternion& q2_in, double t) {
    TQuaternion q2;
    std::memcpy(q2, q2_in, sizeof(TQuaternion));

    double dot = DotOfQuats(q1, q2);
    if (dot < 0.0) {
        dot = -dot;
        q2[0] = -q2[0];
        q2[1] = -q2[1];
        q2[2] = -q2[2];
        q2[3] = -q2[3];
    }
    if (dot > 0.9999) dot = 0.9999;

    double angle = std::acos(dot);
    double a1 = std::sin(angle * (1.0 - t));
    double a2 = std::sin(angle * t);

    result[0] = a1 * q1[0] + a2 * q2[0];
    result[1] = a1 * q1[1] + a2 * q2[1];
    result[2] = a1 * q1[2] + a2 * q2[2];
    result[3] = a1 * q1[3] + a2 * q2[3];
    NormaliseQuat(result);
}

void NormaliseQuat(TQuaternion& q) {
    double norm = std::sqrt(q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
    if (norm < 1e-50) {
        q[0] = 0.0;
        q[1] = 0.0;
        q[2] = 0.0;
        q[3] = 1.0;
        return;
    }
    norm = 1.0 / norm;
    q[0] *= norm;
    q[1] *= norm;
    q[2] *= norm;
    q[3] *= norm;
}

double DotOfQuats(const TQuaternion& q1, const TQuaternion& q2) {
    return q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2] + q1[3]*q2[3];
}

void InvertQuat(TQuaternion& result, const TQuaternion& q) {
    double lenSq = q[0]*q[0] + q[1]*q[1] + q[2]*q[2] + q[3]*q[3];
    if (lenSq < 1e-100) {
        result[0] = 0.0; result[1] = 0.0; result[2] = 0.0; result[3] = 1.0;
        return;
    }
    double inv = 1.0 / lenSq;
    result[0] = -q[0] * inv;
    result[1] = -q[1] * inv;
    result[2] = -q[2] * inv;
    result[3] =  q[3] * inv;
}

void GetQuatFromEuler(TQuaternion& result, const TVec3D& angles) {
    double sx, cx, sy, cy, sz, cz;
    SinCosD(0.5 * angles[2], sz, cz);
    SinCosD(0.5 * angles[1], sy, cy);
    SinCosD(0.5 * angles[0], sx, cx);
    result[3] = cz * cy * cx + sz * sy * sx;
    result[0] = cz * cy * sx - sz * sy * cx;
    result[1] = cz * sy * cx + sz * cy * sx;
    result[2] = sz * cy * cx - cz * sy * sx;
}

void GetEulerFromQuat(TVec3D& result, const TQuaternion& q) {
    double sqx = q[0] * q[0];
    double sqy = q[1] * q[1];
    double sqz = q[2] * q[2];
    result[0] = std::atan2(2.0 * (q[2]*q[1] + q[0]*q[3]), 1.0 - 2.0 * (sqx + sqy));
    result[1] = std::asin(-2.0 * (q[0]*q[2] - q[1]*q[3]));
    result[2] = std::atan2(2.0 * (q[0]*q[1] + q[2]*q[3]), 1.0 - 2.0 * (sqy + sqz));
}

// ============================================================================
// Trigonometry helpers
// ============================================================================

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void SinCosD_x87_pascw(double* sinOut, double* cosOut, const double* a);
#endif

void SinCosD(double a, double& sinVal, double& cosVal) {
    if (NativePrecisionEnabled()) {
        sinVal=std::sin(a);
        cosVal=std::cos(a);
        return;
    }
#if defined(_MSC_VER) && defined(_M_X64)
    // Pascal code issues fsincos (x87 hardware sin/cos under FPC CW $137F).
    // SSE2 pm_sin/pm_cos differ from fsincos by up to 1 ULP on most inputs,
    // enough to flip orbit bailouts on boundary pixels. Use the x87 helper
    // for parity.
    SinCosD_x87_pascw(&sinVal, &cosVal, &a);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    const auto values=numeric::extendedSinCos(numeric::Extended80(a));
    sinVal=static_cast<double>(values.sin);
    cosVal=static_cast<double>(values.cos);
#else
    sinVal = pm_sin(a);
    cosVal = pm_cos(a);
#endif
}

void SinCosS(double a, float& sinVal, float& cosVal) {
    sinVal = static_cast<float>(pm_sin(a));
    cosVal = static_cast<float>(pm_cos(a));
}

double ArcCosSafe(double d) {
    if (d > 0.9999999) return 0.0;
    if (d < -0.9999999) return Pi;
    return std::acos(d);
}

double ArcSinSafe(double d) {
    if (d > 0.9999999) return Pi * 0.5;
    if (d < -0.9999999) return Pi * -0.5;
    return std::asin(d);
}

// ============================================================================
// Shared helper functions (deduplicated from rendering modules)
// ============================================================================

void MakeDVecFromNormals(const TsiLight5* psi, TVec3D& result) {
    if (psi->NormalX == 0 && psi->NormalY == 0 && psi->NormalZ == 0) {
        result[0] = 0.0;
        result[1] = 0.0;
        result[2] = -1.0;
        return;
    }
    result[0] = static_cast<double>(psi->NormalX);
    result[1] = static_cast<double>(psi->NormalY);
    result[2] = static_cast<double>(psi->NormalZ);
    NormalizeVec3D_inplace(result);
}

void AddSVec2Vec3(const TSVec& sv, const TVec3D& dv, TVec3D& result) {
    result[0] = static_cast<double>(sv[0]) + dv[0];
    result[1] = static_cast<double>(sv[1]) + dv[1];
    result[2] = static_cast<double>(sv[2]) + dv[2];
}

void NormaliseVectorToInPlace(double n, TVec3D& v) {
    double d = n / std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2] + 1e-100);
    v[0] *= d;
    v[1] *= d;
    v[2] *= d;
}

double DotOfVectorsNormalize(const TVec3D& v1, const TVec3D& v2) {
    double sqrLen1 = v1[0]*v1[0] + v1[1]*v1[1] + v1[2]*v1[2];
    double sqrLen2 = v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2];
    double dot = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
    return dot / std::sqrt(sqrLen1 * sqrLen2);
}

double GetZPosFromSiLight(const TsiLight5* psi, const TMCTparameter* mcp) {
    uint32_t packedZ;
    std::memcpy(&packedZ, &psi->RoughZposFine, 4);
    packedZ >>= 8;
    double inner = (8388352.0 - static_cast<double>(packedZ)) / mcp->ZcMul + 1.0;
    return (inner * inner - 1.0) / mcp->Zcorr;
}

void MinMaxClip15bit(float s, uint16_t& out) {
#if defined(__aarch64__)
    // ARM FMINNM propagates an sNaN once; Intel MINSS selects its second
    // operand for every NaN. Preserve the latter before compiler min folding.
    if ((std::bit_cast<uint32_t>(s) & 0x7fffffffu) > 0x7f800000u) {
        out = 32767;
        return;
    }
#endif
    s = s < 32767.0f ? s : 32767.0f;
    s = s > 0.0f ? s : 0.0f;
    out = static_cast<uint16_t>(std::nearbyint(s));
}

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void PackNormals_x87(int16_t* dst, const double* src);
#endif
void MakeWNormalsFromDVec(int16_t* dst, const TVec3D& src) {
#if defined(_MSC_VER) && defined(_M_X64)
    PackNormals_x87(dst, src);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    const E x(src[0]), y(src[1]), z(src[2]);
    const E scale = E(32767) / sqrt(x*x + y*y + z*z + E(1e-100));
    for (int i = 0; i < 3; ++i) {
        const int32_t rounded = (E(src[i]) * scale).integer();
        dst[i] = (rounded >= -32768 && rounded <= 32767) ? static_cast<int16_t>(rounded) : int16_t(-32768);
    }
#else
    const long double x=src[0], y=src[1], z=src[2];
    const long double scale=32767.0L/std::sqrt(x*x+y*y+z*z+static_cast<long double>(1e-100));
    for (int i=0;i<3;++i) {
        const long double rounded=std::nearbyint(static_cast<long double>(src[i])*scale);
        // FISTP m16 returns the integer-indefinite word for invalid conversion.
        dst[i]=(rounded>=-32768 && rounded<=32767) ? static_cast<int16_t>(rounded) : int16_t(-32768);
    }
#endif
}

} // namespace mb3d
