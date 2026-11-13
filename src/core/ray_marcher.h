#ifndef MB3D_CORE_RAY_MARCHER_H
#define MB3D_CORE_RAY_MARCHER_H

#include "core/types.h"
#include "core/ray_setup_dump.h"
#include <vector>
#include <string>

namespace mb3d {

// ============================================================================
// Ray march record -- passed to RayMarch()
// ============================================================================

struct TRaymarchRec {
    PMCTparameter  PMCTparas;
    TPIteration3Dext PIt3Dex;
    TVec3D   ActPos;
    TVec3D   MarchVec;
    TVec3D   VievVec;
    double   ActZpos;
    double   StartDEstop;
    double   ZZposMul;
    double   DEmulVary;
    double   DEmulConst;
    double   MaxRayLength;
    int32_t  BinSteps;
    int32_t  seed;
    int32_t  RMresult;      // 0: no object  1: object on DE  2: object on ItCount
    int32_t  StartInside;   // start-of-ray hit before stepping
    int32_t  CutPlaneHit;   // cut-plane index from RMmaxLengthToCutPlane
    float    StepCount;
    float    StepForward;
    double   Zstepped;
    double   finalDE;
    int32_t  TracePixelX;
    int32_t  TracePixelY;
    std::vector<std::string>* TraceLines;
    RaySetupDumpRecord* RaySetupRecordOut = nullptr;
    bool     RaySetupOnly = false;
};
using TPRaymarchRec = TRaymarchRec*;

// ============================================================================
// DE calculation functions
// ============================================================================

double CalcDEanalytic(TPIteration3Dext It3Dex, void* mctp);
double CalcDEfull(TPIteration3Dext It3Dex, void* mctp);
double CalcDEnoADE(TPIteration3Dext It3Dex, void* mctp);

// ============================================================================
// Ray march entry point
// ============================================================================

void RayMarch(TPRaymarchRec RMrec);

// ============================================================================
// Search refinement
// ============================================================================

void RMdoBinSearch(PMCTparameter pMCTparas, double& DE, double& RLastStepWidth);
double CalcIterationBoundaryStep(float target, float previous, float current, double multiplier);

// depthAddress can point into a byte-packed parameter record.
void RMdoBinSearchIt(PMCTparameter pMCTparas, void* depthAddress);
void RMdoSecantSearch(PMCTparameter pMCTparas, double& DE, double& RLastStepWidth, double& RLastDE);

// ============================================================================
// Normal calculation
// ============================================================================

using TCalculateNormalsFunc = void (*)(PMCTparameter pMCTparas, float& NN);
void RMCalculateNormals(PMCTparameter pMCTparas, float& NN);
void RMCalculateNormalsOnSmoothIt(PMCTparameter pMCTparas, float& NN);

// ============================================================================
// Camera ray generation
// ============================================================================

void BuildViewVectorDFOV(double CAFY, double CAFX, TVec3D& result);
void BuildViewVectorDSphereFOV(double CAFY, double CAFX, TVec3D& result);
void RMCalculateVgradsFOV(PMCTparameter pMCTparas, int ix);
void RMCalculateStartPos(PMCTparameter pMCTparas, int ix, int iy);

// ============================================================================
// Cutting planes
// ============================================================================

void RMmaxLengthToCutPlane(PMCTparameter pMCTparas, double& dLength, int& itmp, TPPos3D vPos);

// ============================================================================
// Color calculation
// ============================================================================

void RMdoColor(PMCTparameter pMCTparas);
void RMdoColorOnIt(PMCTparameter pMCTparas);
#ifdef MB3D_ENABLE_PARITY_TRACE
double RMTraceLastColorScalar();
double RMTraceLastColorOTrap();
#else
inline double RMTraceLastColorScalar() { return 0.0; }
inline double RMTraceLastColorOTrap() { return 0.0; }
#endif

// ============================================================================
// Roughness
// ============================================================================

void RMCalcRoughness(TPVec3D N, float& sRough, double* dt2, double* dsG);

// ============================================================================
// Z position and roughness output
// ============================================================================

void CalcZposAndRough(TPsiLight5 siLight, PMCTparameter mct, double ZZ);

// ============================================================================
// Iteration3D initialization
// ============================================================================

void IniIt3D(PMCTparameter PMCT, TPIteration3Dext It3Dex);

// ============================================================================
// Normal cutting plane angles
// ============================================================================

void RMcalcNanglesForCut(PMCTparameter pMCTparas, int CutPlane);

// ============================================================================
// Volume light step count
// ============================================================================

int RMcalcVLight(float StepCount);

} // namespace mb3d

#endif // MB3D_CORE_RAY_MARCHER_H
