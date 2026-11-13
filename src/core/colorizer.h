#ifndef MB3D_CORE_COLORIZER_H
#define MB3D_CORE_COLORIZER_H

#include "core/types.h"

namespace mb3d {

// ============================================================================
// Additional types needed for colorizer (from PaintThread.pas)
// ============================================================================

// TLightLSDAIs = array[0..5] of TSVec
// [0] = normals/diffColScaled, [1] = total shadowed light, [2] = diffuse color,
// [3] = unshadowed light, [4] = ambient/output, [5] = specular/light direction
using TLightLSDAIs = TSVec[6];
using TPLightLSDAIs = TLightLSDAIs*;

// TLightSD = array[0..1] of TSVec
using TLightSD = TSVec[2];

// ============================================================================
// SVec helper functions used by colorizer
// ============================================================================

// Build SVec from TsiLight5 normals (int16 -> float, normalized)
void MakeSVecFromNormalsD(TSVec& result, const TsiLight5* psi);

// DotOf2VecNormalize: specular reflection dot product
float DotOf2VecNormalize(const TSVec& norm, const TSVec& light, const TSVec& view);

// Diffuse function lookup
float GetCosTabVal(int funcType, float dot, float rough);
float GetCosTabValSqr(int funcType, float dot, float rough);

// Fast integer power
float FastIntPow(float base, int exp);
float FastPow(float base, int exp);

// SVec arithmetic helpers
void AddSVecWeights(TSVec* dest, const TSVec* src, float w);
void AddSVectors_inplace(TSVec* dest, const TSVec& src);
void MultiplySVectors(TSVec& result, const TSVec& a, const TSVec& b);
void MultiplySVectorsV(TSVec* dest, const TSVec* src);
void ScaleSVectorV(TSVec* sv, float s);
void LinInterpolate2SVecs(TSVec& result, const TSVec& a, const TSVec& b, float t);
void Add2SVecsWeight(TSVec& result, const TSVec& a, const TSVec& b, float wa, float wb);
void Add2SVecsWeight2(TSVec& result, const TSVec& a, const TSVec& b, float wb);
void Add3SVectors(TSVec& result, const TSVec& a, const TSVec& b, const TSVec& c);
void SubtractSVectors(TSVec& result, const TSVec& a, const TSVec& b);
void mMinMaxSVec(TSVec& result, float mn, float mx, const TSVec& sv);
void mSqrtSVec(TSVec& result, const TSVec& sv);
void SqrSV255(TSVec& result, const TSVec& sv);
// Max0S is inline in math3d.h
float DotOfSVectors(const TSVec& a, const TSVec& b);
// SqrLengthOfSVec is in math3d.h
void NormaliseSVectorVar(TSVec& sv);
float SqrDistSV(const TSVec& diff, const TSVec& dir);
void RotateSVectorS(TSVec* sv, const TSMatrix3* m);
void RotateSVectorReverseS(TSVec* sv, const TSMatrix3* m);
float YofSVec_col(const TSVec& sv);
void Clamp01Svar(float& s);
void AddSVecS(TSVec& result, const TSVec& sv, float s);
void SVecPow(TSVec& result, const TSVec& sv, float p);
void AbsSVecVar(TSVec& sv);

// NormaliseMatrixToS: single-precision matrix normalization
void NormaliseMatrixToS(TSMatrix3& result, float n, const TMatrix3* m);

// ScaleSVectorD: scale SVec by double
void ScaleSVectorD(TSVec& result, const TSMatrix3& row, float scale);

// BuildViewVectorFOV / SphereFOV (for paint thread)
void BuildViewVectorFOV(float CY, float CX, TSVec& result);
void BuildViewVectorSphereFOV(float CY, float CX, TSVec& result);

// ============================================================================
// Ambient shadow calculation
// ============================================================================

void calcAmbshadow(float& dAmbS, float sAmplitude, const TsiLight5* psi);

// ============================================================================
// CalcPosLightShape: visible light shape
// ============================================================================

void CalcPosLightShape(float& flux, float& transp, int LightNr, TLightVals* PLVals);

// ============================================================================
// Color palette interpolation
// ============================================================================

void CalcColors(TSVec* iDif, TSVec* iSpe, const TsiLight5* psi, TLightVals* PLVals);
void CalcColorsInside(TSVec* iDif, TSVec* iSpe, const TsiLight5* psi, TLightVals* PLVals);
void PushPaintColorsInterpTraceSuppression();
void PopPaintColorsInterpTraceSuppression();
void PushPaintSvecTraceScope();
void PopPaintSvecTraceScope();
bool IsPaintSvecTraceScopeEnabled();

// ============================================================================
// Total light combination functions
// ============================================================================

void CalcTotalLight1(TLightLSDAIs* LiLSDAI, TLightVals* PLVals, float* pAmbsh);
void CalcTotalLight2(TLightLSDAIs* LiLSDAI, TLightVals* PLVals, float* pAmbsh);

// ============================================================================
// PreCalcDepthCol
// ============================================================================

void PreCalcDepthCol(int Dfunc, TPaintLightVals* PLV, TLValigned* PLValigned);

// ============================================================================
// ConvertVLight
// ============================================================================

int ConvertVLight(int Win);

// ============================================================================
// Main pixel coloring functions (ported from PaintThread.pas)
// ============================================================================

// CalcPixelColor2: standard per-pixel coloring, writes BGRA to *SL
void CalcPixelColor2(uint32_t* SL, const TsiLight5* PsiLight,
                     TLightVals* PLVals, TPaintLightVals* PLV);

// CalcPixelColorSqr: squared color variant
void CalcPixelColorSqr(uint32_t* SL, const TsiLight5* PsiLight,
                       TLightVals* PLVals, TPaintLightVals* PLV);

// CalcPixelColorSvec: SVec-based coloring (for reflections)
void CalcPixelColorSvec(TSVec* SVcol, TLightSD& InSD, const TsiLight5* PsiLight,
                        TLightVals* PLVals, TPaintLightVals* PLV, TSVec& ResultTransp);

// CalcPixelColorSvecTrans: transparent/transmission coloring
void CalcPixelColorSvecTrans(TSVec* SVcol, TLightSD& InSD, const TsiLight5* PsiLight,
                             TLightVals* PLVals, TPaintLightVals* PLV, TSVec& ResultTransp);

// ============================================================================
// View/position helpers for paint loop
// ============================================================================

void CalcViewVec(TPaintLightVals* PLV, float sAspect);
void GetStartSPosAndAddVecs(TPaintLightVals& PLV, TPaintParameter& PP,
                            TSVec& StartPos, TSVec& SPosYadd, TSVec& SPosXadd);
void CalcObjPos(TPaintLightVals& PLV, TPaintParameter& PP,
                const TsiLight5* PsiLight, const TSVec& SPosX);

// ============================================================================
// MakeLightValsFromHeaderLight completion
// ============================================================================

void CompleteLightValsSetup(TMandHeader10* Header, TPLightVals Lvals);

} // namespace mb3d

#endif // MB3D_CORE_COLORIZER_H
