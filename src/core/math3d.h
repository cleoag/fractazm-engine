#ifndef MB3D_CORE_MATH3D_H
#define MB3D_CORE_MATH3D_H

#include "core/types.h"

namespace mb3d {

// CalcThread.CAFY: the qualified FPC SSE2 build rounds each Double operation.
double CalcCameraRowAngle(int y, int height, double fovy);

void BuildViewVectorFOVDouble(double CY, double CX, TSVec& result);


// ============================================================================
// Constants
// ============================================================================

constexpr double Pi       = 3.14159265358979323846;

// ============================================================================
// Utility: Min / Max / Clamp
// ============================================================================

double MinCD(double s1, double s2);
double MaxCD(double s1, double s2);
float  MinCS(float s1, float s2);
float  MaxCS(float s1, float s2);
float  MinMaxCS(float smin, float s, float smax);
float  Min0MaxCS(float s, float smax);
double Min0MaxCD(double d, double dmax);
double MinMaxCD(double dmin, double d, double dmax);
double Clamp0D(double d);
float  Clamp01S(float sv);
double Clamp01D(double dv);
double MaxAbsCD(double s1, double s2);
double MinAbsCD(double s1, double s2);

// DivUtils.MinMaxClip15bit SSE path: clamp before nearest-even conversion.
// MINSS selects the second operand for NaN, so NaN saturates to 32767.
void MinMaxClip15bit(float s, uint16_t& out);
void MakeWNormalsFromDVec(int16_t* dst, const TVec3D& src);

// ============================================================================
// Custom type conversions
// ============================================================================

float  ShortFloatToSingle(const ShortFloat& sf);
void   SingleToShortFloat(float s, ShortFloat& result);
double D7BtoDouble(const Double7B& d7b);
void   DoubleToD7B(double d, Double7B& result);

// ============================================================================
// Vector construction
// ============================================================================

void Vec3D(TVec3D& result, double x, double y, double z);

// ============================================================================
// Double-precision vector ops (TVec3D)
// ============================================================================

void   AddVec3D(TVec3D& result, const TVec3D& v1, const TVec3D& v2);
void   AddVec3D_inplace(TVec3D& v1, const TVec3D& v2);
void   SubtractVec3D(TVec3D& result, const TVec3D& v1, const TVec3D& v2);
void   ScaleVec3D(TVec3D& result, const TVec3D& v, double s);
void   ScaleVec3D_inplace(TVec3D& v, double s);
double DotVec3D(const TVec3D& v1, const TVec3D& v2);
void   CrossVec3D(TVec3D& result, const TVec3D& v1, const TVec3D& v2);
void   NormalizeVec3D(TVec3D& result, const TVec3D& v);
void   NormalizeVec3D_inplace(TVec3D& v);
double LengthOfVec(const TVec3D& v);
double SqrLengthOfVec(const TVec3D& v);
double DistanceOf2Vecs(const TVec3D& v1, const TVec3D& v2);
void   mAddVecWeight(TVec3D& v1, const TVec3D& v2, double w);
void   mCopyAddVecWeight(TVec3D& dest, const TVec3D& src, const TVec3D& add, double w);

// ============================================================================
// Single-precision vector ops (TSVec)
// ============================================================================

void   AddSVec(TSVec& result, const TSVec& v1, const TSVec& v2);
void   SubSVec(TSVec& result, const TSVec& v1, const TSVec& v2);
void   ScaleSVec(TSVec& result, const TSVec& v, float s);
void   ScaleSVec_inplace(TSVec& v, float s);
float  DotSVec(const TSVec& v1, const TSVec& v2);
void   CrossSVec(TSVec& result, const TSVec& v1, const TSVec& v2);
float  LengthOfSVec(const TSVec& v);
float  SqrLengthOfSVec(const TSVec& v);
void   NormalizeSVec(TSVec& result, const TSVec& v);
void   NormalizeSVec_inplace(TSVec& v);
float  YofSVec(const TSVec& sv);
void   ClearSVec(TSVec& sv);
void   ClearDVec(TVec3D& dv);

// Color/conversion helpers
void   RGBtoSVecScale(TSVec& result, const TRGB& rgb, float scale);
void   ColToSVec(TSVec& result, uint32_t col);
void   SVecToDVec(TVec3D& result, const TSVec& v);
void   DVecToSVec(TSVec& result, const TVec3D& v);

// ============================================================================
// Matrix ops (TMatrix3)
// ============================================================================

void   MultiplyMatrix3(TMatrix3& m1, const TMatrix3& m2);
void   NormaliseMatrixTo(TMatrix3& result, double n, const TMatrix3& m);
void   TransposeMatrix3(TMatrix3& result, const TMatrix3& m);
void   BuildRotMatrix(double xa, double ya, double za, TMatrix3& m);
void   RotateVector(TVec3D& v, const TMatrix3& m);
void   RotateVectorReverse(TVec3D& v, const TMatrix3& m);
void   ScaleMatrix(double s, TMatrix3& m);

// ============================================================================
// Quaternion ops
// ============================================================================

void   QuatMultiply(TQuaternion& result, const TQuaternion& q1, const TQuaternion& q2);
void   QuatToMatrix(TMatrix3& m, const TQuaternion& q);
void   MatrixToQuat(TQuaternion& q, const TMatrix3& m);
void   Slerp(TQuaternion& result, const TQuaternion& q1, const TQuaternion& q2, double t);
void   NormaliseQuat(TQuaternion& q);
double DotOfQuats(const TQuaternion& q1, const TQuaternion& q2);
void   InvertQuat(TQuaternion& result, const TQuaternion& q);
void   GetQuatFromEuler(TQuaternion& result, const TVec3D& angles);
void   GetEulerFromQuat(TVec3D& result, const TQuaternion& q);

// ============================================================================
// Trigonometry helpers
// ============================================================================

void   SinCosD(double a, double& sinVal, double& cosVal);
void   SinCosS(double a, float& sinVal, float& cosVal);
double ArcCosSafe(double d);
double ArcSinSafe(double d);

// ============================================================================
// Shared rendering constants (used across shadow, ray_marcher, reflection,
// colorizer, ambient_occlusion, dof)
// ============================================================================

constexpr float s011f  = 0.11f;
constexpr float s05f   = 0.5f;
constexpr float s255f  = 255.0f;
constexpr float s001f  = 0.01f;

// ============================================================================
// Shared helper functions (deduplicated from rendering modules)
// ============================================================================

// Max(0, s) -- clamped to non-negative
inline float Max0S(float s) { return (s > 0.0f) ? s : 0.0f; }

// Extract double-precision normal vector from TsiLight5, normalized
void MakeDVecFromNormals(const TsiLight5* psi, TVec3D& result);

// Add single-precision SVec to double-precision Vec3D
void AddSVec2Vec3(const TSVec& sv, const TVec3D& dv, TVec3D& result);

// Normalize a vector in-place to a given length
void NormaliseVectorToInPlace(double n, TVec3D& v);

// Cosine of angle between two vectors (dot product normalized by lengths)
double DotOfVectorsNormalize(const TVec3D& v1, const TVec3D& v2);

// Reconstruct Z position from TsiLight5 packed Z data
double GetZPosFromSiLight(const TsiLight5* psi, const TMCTparameter* mcp);

float CalcMarchStepLimit(float stop, float limiter);

// Binary-search refinement uses the Pascal inline x87 expression.
float RefineRayStep(TVec3D& pos, double& zz, const TVec3D& direction,
                    double step, float factor, float stop);

} // namespace mb3d

#endif // MB3D_CORE_MATH3D_H

