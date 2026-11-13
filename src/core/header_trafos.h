#ifndef MB3D_CORE_HEADER_TRAFOS_H
#define MB3D_CORE_HEADER_TRAFOS_H

#include "core/types.h"

namespace mb3d {

// ============================================================================
// Parameter setup functions (ported from HeaderTrafos.pas)
// ============================================================================

// The MAIN parameter setup function -- converts TMandHeader10 to TMCTparameter
TMCTparameter GetMCTparasFromHeader(TMandHeader10& Header,
                                     THeaderCustomAddon* pHCA,
                                     LongBool bIniCalcMaps);

// Lighting parameter setup
void MakeLightValsFromHeaderLight(TMandHeader10* Header, TPLightVals Lvals,
                                  double ImScale, int StereoMode);

// Helper functions
double GetDEstopFactor(TMandHeader10* Header);
void   NormVGrads(TMandHeader10* Header);
void   CalcVGradsFromHeader8rots(TMandHeader10* Header);
void   CalcHSVecsFromLights(TPLightVals Lvals, PMCTparameter MCTpars);
void   CalcPosLightsRelPos(TMandHeader10* Header, TPLightVals Lvals);
void   CalcSCstartAndSCmul(TMandHeader10* Header, float& sCStart, float& sCmul,
                            LongBool bIsInside);
void   CalcStepWidth(TMandHeader10* Header);
void   CalcPPZvals(TMandHeader10& Header, double& Zcorr, double& ZcMul, double& ZZstmitDif);
float  CalcXoff(const TMandHeader10* Header);
float CalcRayStepLimiter(int width, int height, float stepDiv, float limiter);

// Legacy formula migration — sets byOptionType[], iOptionCount, dOptionValue[]
// defaults and CustomFname for internal formula f (0..9) in addon slot t.
// From HeaderTrafos.pas lines 238-281
void GetHAddOnFromInternFormula(THeaderCustomAddon* pAddon, int f, int t);

// Tiling: compute tile paint dimensions and pixel offsets from TilingOptions.
// When TilingOptions == 0, returns full image dims with zero offsets.
void GetPaintTileSizes(const TMandHeader10* Header,
                       int& wid, int& hei, int& Xplus, int& Yplus);

// Tiling: extract crop border size from TilingOptions (0 if no tiling).
int GetTilingCrop(const TMandHeader10* Header);

// Utility
double NotZero(double d);

// ============================================================================
// Formula constant buffer -- mirrors Pascal's pConstPointer16 layout
// Each formula slot gets a 1024-byte buffer. PVar points to the center.
// Positive offsets contain PAligned16 global constants (3.0, 4.0, etc.)
// Negative offsets contain processed option values (scale, fold, minR, etc.)
// ============================================================================

struct alignas(16) FormulaConstBuffers {
    // 6 formula slots, each with 1024 bytes
    // PVar points to buf[slot][512] so there's room for negative offsets
    uint8_t buf[6][1024];
    void* pConstPointer[6]; // pointer to center of each buffer
};

// Constants belong to the bound render context and outlive all of its workers.
FormulaConstBuffers& GetFormulaConstBuffers();

// Build the PVar constant buffer for a formula slot
void BuildFormulaConstBuffer(int slot, const THeaderCustomAddon* pHCA,
                             int formulaIndex);

} // namespace mb3d

#endif // MB3D_CORE_HEADER_TRAFOS_H
