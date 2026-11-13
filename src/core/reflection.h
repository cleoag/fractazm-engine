#ifndef MB3D_CORE_REFLECTION_H
#define MB3D_CORE_REFLECTION_H

#include "core/types.h"
#include "core/colorizer.h"
#include <vector>

namespace mb3d {

// FPC i386 reference semantics for `not initialInside xor currentInside`.
// LongBool assignments store True as -1, but NOT produces a 0/1 intermediate
// before XOR. This is deliberately not logical negation of XOR. Inputs here
// are logical states; the implementation reconstructs the reference storage.
// Delphi and other Pascal compiler profiles have not been qualified.
bool ReflectionFpcNotInsideXor(bool initialInside, bool currentInside);
// Original absorption uses signed Math.Power inputs and one final Single store.
float ReflectionAbsorptionPower(float base, float exponent);
float ReflectionAbsorptionExponent(double distance, float absorption);

// ============================================================================
// Reflection / Transparency Calculation
// Ported from mb3d/CalcSR.pas
//
// After the main coloring pass, this pass traces reflection and
// refraction rays for every surface pixel. Supports:
//   - Recursive reflections up to SRreflectioncount depth
//   - Fresnel factor (angle-dependent reflectivity)
//   - Snell's law refraction (transparency / transmission)
//   - Absorption inside transparent media
//   - Light scattering inside transparent media
//   - Cutting-plane support for reflected/refracted rays
//   - Per-pixel ambient-occlusion on reflected surfaces
//   - Dynamic fog along reflected rays
//   - Volume-light accumulation along reflected rays
//   - Diffuse shadowing on reflections
//   - Hard shadow on reflected surfaces
//   - iExModes reflection-simulation variants
// ============================================================================

void CalcReflections(
    TsiLight5* siLight,       // per-pixel surface data buffer
    uint8_t* imageBuffer,     // RGB image to modify (3 bytes per pixel)
    int width, int height,
    int slOffset,             // row stride in TsiLight5 units (typically == width)
    TMCTparameter* mcp,
    TLightVals* lvals,
    const TMandHeader10* header,  // header for SR parameters
    const THeaderCustomAddon* addon
);

// Row-range variant for multithreaded rendering.
// Processes rows [y_start, y_end).
// Caller must provide a per-thread copy of mcp and lvals (they will be mutated).
void CalcReflectionsRows(
    TsiLight5* siLight,
    uint8_t* imageBuffer,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    const TMandHeader10* header,
    const THeaderCustomAddon* addon,
    int y_start, int y_end,
    int x_start = 0, int x_end = -1
);

// ============================================================================
// Volume Light Map Precomputation
// Ported from mb3d/maps/MB3DMaps.pas: CalcVolLightMap + MakeVolumicLightMapThreads
//
// Precomputes a cube map of distances from a light source through the fractal,
// used by SR_DoDynFog during reflections for volumetric light effects.
//
// Called from CalcReflections before the main loop when volume light is enabled.
// ============================================================================

// TVolumeLightMap -- cube map for volumetric light effects
struct TVolumeLightMap {
    std::vector<float> CubeSides[6]; // distance data per cube face
    TVec3D   LightPos;               // light position in world space
    TSMatrix3 RotMatrix;              // rotation matrix for directional lights
    float    StretchSide1;            // side stretch factor
    float    MinDistance;             // minimum ray start distance
    float    SizeFactor;             // half cube size - 0.5
    int      CubeSize;               // number of pixels per cube face side
    int      HalfSize;               // CubeSize / 2
    int      SideCount;              // 1 for directional, 6 for positional
    int      HeightS2to5;            // = CubeSize
    float    CsizeS;                 // = CubeSize - 1
    float    HSizeS;                 // = HalfSize
    bool     IsPosLight;             // true for positional, false for directional
    bool     Rotate;                 // apply rotation matrix
    bool     Initialized;            // set to true after computation
};

// PrepareVolLightMap -- CalcVolLightMap's map geometry and march setup,
// without cells or rays. False when no map is computed (no volume light, or
// its light is disabled); primary substeps then add nothing. Native Metal
// marches the map rays itself from these exact values.
struct TVolumeLightMapSetup {
    TMCTparameter params;   // private copy; Ystart centered unless bPlanarOptic==2
    float maxDist = 0.0f;   // march limit in steps
    float sgl = 0.0f;       // 1/StretchSide1
    TVec3D dirVec = {};     // directional march direction (invMs row 2)
    TSMatrix3 invMs = {};   // inverse RotMatrix (directional)
};
bool PrepareVolLightMap(
    const TMandHeader10* header,
    TLightVals* lvals,
    const TMCTparameter* mcp,
    TVolumeLightMap& vlm,
    TVolumeLightMapSetup& setup
);

// CalcVolLightMap -- precompute volume light map if needed.
// MUST be called on the main thread before any parallel reflection workers
// are spawned. Mirrors Pascal CalcSR.pas:107 topology.
//
// numThreads controls the internal worker pool used to compute rays across
// the map cube faces. Pascal MB3DMaps.pas:345 uses
// Min(UpDown3.Position, CubeSize) worker threads; the C++ port mirrors that
// by capping the requested count to the CubeSize at compute time. Pass 0 to
// auto-detect (std::thread::hardware_concurrency()).
void CalcVolLightMap(
    const TMandHeader10* header,
    TLightVals* lvals,
    TMCTparameter* mcp,
    int numThreads = 0
);

// ResetVolLightMap -- clear the bound job's volume light map.
// Only call between stages, with no active workers for this context.
// Other jobs own independent maps and are unaffected.
void ResetVolLightMap();

// GetVolLightMapValue -- look up a value from the volume light map.
// Returns the distance through the scene at the given direction.
float GetVolLightMapValue(const TVec3D& direction);

// Pure ordinary lookup kernels. Explicit map ownership supports independent
// fixture checks and future per-job/backend resource boundaries.
float SamplePositionalVolumeLightMap(const TVolumeLightMap& map, const TVec3D& direction);
double VolumeLightDirectionSquaredLength(const TVec3D& direction);
bool DirectionalVolumeLightMapContains(const TVolumeLightMap& map, const TVec3D& worldPos);


// VolLightDoDynFogSubstep -- volume-light fog sub-step accumulation.
// Mirrors the else-branch (DFogOnIt == 65535) of Pascal DoDynFog in
// CalcThread.pas/CalcSR.pas: walks the previous ray step in small
// sub-steps and adds VLmul * s1 (or VLmul * s1 / (d1 + stepWidth^2)
// for positional lights) to stepCount for every sub-step whose
// position is "inside" the lit volume per the precomputed cube map.
// The map must already be initialized via CalcVolLightMap().
//
// currentPos  = It3Dext.C1..C3 at the END of the advance (after
//               mAddVecWeight(..., dTmp)). The function walks
//               backwards by LastStepWidth and marches forward again.
// marchVec    = mVgradsFOV (ray direction vector used for the step).
// LastStepWidth = the step width that was just traversed.
// mZZ         = current z-distance accumulator along the ray.
// VLstepmul / VLmul / mctDEstopFactor / stepWidth = per-scene constants.
void VolLightDoDynFogSubstep(
    const TVec3D& currentPos,
    const TVec3D& marchVec,
    float LastStepWidth,
    double mZZ,
    float mctDEstopFactor,
    float VLstepmul,
    float VLmul,
    double stepWidth,
    float& stepCount);

} // namespace mb3d

#endif // MB3D_CORE_REFLECTION_H
