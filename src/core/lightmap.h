#ifndef MB3D_CORE_LIGHTMAP_H
#define MB3D_CORE_LIGHTMAP_H

#include "core/types.h"
#include <atomic>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace mb3d {

// ============================================================================
// Lightmap / Map System
// Ported from mb3d/Calc.pas, mb3d/PaintThread.pas, mb3d/ImageProcess.pas
//
// Provides:
//   - Map buffer management (CalcMaps[0..2])
//   - Map sequence support for animation frames (MapSequences.pas)
//   - Bicubic spline interpolation for map sampling
//   - Sphere-projected map lookups (linear, bicubic, spline)
//   - Direct XY map lookups (linear, spline)
//   - Lightmap pixel sampling for lighting
//   - 3D procedural texture wrapping
// ============================================================================

// ============================================================================
// Map sequence support (from MapSequences.pas)
// Maps a map channel to a numbered image sequence for animation.
// For single-frame CLI renders (no animation), sequences are empty and
// the frame number stays at 1 -- effectively a no-op pass-through.
// ============================================================================

struct MapSequence {
    int destChannel;       // map number this sequence replaces
    std::string imageFilename; // template filename with frame number digits
    int firstImage;
    int lastImage;
    int increment;
    bool loop;

    MapSequence()
        : destChannel(0), firstImage(0), lastImage(0), increment(1), loop(false) {}

    // Format a filename by replacing trailing digits before extension with frame number.
    // E.g. "texture001.bmp" + frame=5 -> "texture005.bmp"
    static std::string FormatFrameFilename(const std::string& filename, int frame);

    // Get the resolved filename for a given animation frame.
    std::string GetFilename(int frame) const;
};

class MapSequenceList {
    std::vector<MapSequence> sequences_;
public:
    void Clear() { sequences_.clear(); }
    int Count() const { return static_cast<int>(sequences_.size()); }

    // Find sequence for a given map channel number. Returns nullptr if none.
    const MapSequence* GetSequence(int channel) const;

    void AddSequence(const MapSequence& seq) { sequences_.push_back(seq); }
};

// Map resources belong to one render job, shared by its preparation and workers.
// Resource paths are relative to the absolute directory captured by RenderContext.
// The three-slot cache and on-demand load synchronization preserve Pascal behavior.
struct MapResources {
    TLightMap calc_maps[3] = {};
    std::atomic<LongBool> missing{0};
    MapSequenceList sequences;
    int frame_number = 1;
    std::mutex load_mutex;
    std::unordered_set<int> banned_numbers;
    std::string resource_root;

    MapResources() = default;
    ~MapResources();
    MapResources(const MapResources&) = delete;
    MapResources& operator=(const MapResources&) = delete;
    MapResources(MapResources&&) = delete;
    MapResources& operator=(MapResources&&) = delete;
};

// Access the current job's sequence list. Ordinary single-frame CLI jobs start empty.
MapSequenceList& GetMapSequenceList();

// Current job's animation frame number. Defaults to 1 (single-frame render).
int  GetCurrMapFrameNumber();
void SetCurrMapFrameNumber(int frame);

// ============================================================================
// Current job's map buffers (max 3 maps, matching Pascal CalcMaps[0..2])
// ============================================================================

TLightMap (&GetCalcMaps())[3];
std::atomic<LongBool>& GetMapMissingState();

// ============================================================================
// Map loading
// ============================================================================

// Load a lightmap from a BMP/PNG file by map number.
// Returns true if successfully loaded.
bool LoadLightMapNr(int mapNr, TPLightMap lm);

// Load a lightmap from a specific file path.
bool LoadLightMapFile(const char* filename, TPLightMap lm);

// Background filenames saved in scenes may refer to the map directories.
// Keep explicit paths exact; resolve bare names at the resource root first,
// then in the same map directories used by numbered resources.
bool LoadBackgroundMapFile(const char* filename, TPLightMap lm);

// Load a lightmap from an in-memory buffer (e.g. embedded texture blob).
// Uses stbi_load_from_memory internally. Supports BMP/PNG/etc.
bool LoadLightMapFromMemory(const uint8_t* data, size_t len, TPLightMap lm);

// Original MB3DMaps background preparation, in the original operation order.
// The smoothing filter applies to RGB8 only; projection/border support RGB16.
void PrepareLightMap(TPLightMap lm, bool smooth, bool spherical, bool fitBorder);

// Free lightmap pixel data.
void FreeLightMap(TPLightMap lm);

// ============================================================================
// Map initialization
// ============================================================================

// IniCalcMaps -- scan header formulas for MAP references and load needed maps.
// Ported from Calc.pas: procedure IniCalcMaps
void IniCalcMaps(TMandHeader10* header, THeaderCustomAddon* addon);

// ============================================================================
// Spline coefficient helper (from Math3D.pas)
// ============================================================================

// MakeSplineCoeff -- compute cubic B-spline coefficients for fractional part.
// Result[0..3] are the four weights.
void MakeSplineCoeff(float xs, TSVec& result);

// ============================================================================
// FracA -- fractional part helper, always returns [0, 1)
// ============================================================================

double FracA(double d);

// ============================================================================
// Map sampling functions (from Calc.pas)
// ============================================================================

// SplineIpolMap -- bicubic spline interpolation at direct map coordinates.
// x, y are in pixel coords [0..LMWidth), [0..LMHeight).
// Returns RGB in [0..1] range per channel.
void SplineIpolMap(TVec3D& result, double x, double y, TPLightMap pCalcMap);

// GetMapPixelSphereSpline -- sphere-projected spline map lookup.
// PVec3D is a direction vector; MapNr is the map number to look up.
// Result in [0..1] range.
void GetMapPixelSphereSpline(TVec3D& result, TPVec3D pVec3D, int32_t mapNr);

// GetMapPixelDirectXYspline -- direct XY map lookup with tiling, spline interp.
// PVec3D[0] = X coord, PVec3D[1] = Y coord, in [0..1] range (tiles).
void GetMapPixelDirectXYspline(TVec3D& result, TPVec3D pVec3D, int32_t mapNr);

// GetMapPixelSphere -- sphere-projected linear interpolation (bicubic variant).
// Commented out in Pascal source but kept for reference/completeness.
void GetMapPixelSphere(TVec3D& result, TPVec3D pVec3D, int32_t mapNr);

// GetMapPixelSphereBicubic -- sphere-projected bicubic interpolation.
void GetMapPixelSphereBicubic(TVec3D& result, TPVec3D pVec3D, int32_t mapNr);

// GetMapPixelDirectXY -- direct XY map lookup with cos-approx interpolation.
void GetMapPixelDirectXY(TVec3D& result, TPVec3D pVec3D, int32_t mapNr);

// ============================================================================
// Lightmap pixel sampling (from ImageProcess.pas / PaintThread.pas)
// ============================================================================

// GetLightMapPixel -- spline-interpolated lightmap pixel at (x,y) in [0,1].
// WrapAround: 0=no wrap, 1=horiz+vert wrap, 2=horiz only wrap.
// bSqr: if true, square the result (gamma correction).
void GetLightMapPixel(TSVec& result, float x, float y, TPLightMap lm,
                      LongBool bSqr, int wrapAround);

// GetLightMapPixelSphere -- sphere-projected lightmap lookup.
// svec is a direction vector; SM is an optional rotation matrix (can be nullptr).
void GetLightMapPixelSphere(TSVec& result, const TSVec& svec,
                            const TSMatrix3* SM, TLightMap& lm, LongBool bSqr);

// Nearest-neighbor sampling used by the Pascal Monte Carlo integrator.
void GetLightMapPixelNN(TSVec& result, float x, float y, TPLightMap lm, LongBool bSqr);
void GetLightMapPixelSphereNN(TSVec& result, const TSVec& direction,
                             const TSMatrix3* rotation, TLightMap& lm, LongBool bSqr);

// GetDiffMapWrap3D -- 3D procedural texture wrapping for diffuse color maps.
// Uses object position and normal to project a 3D texture.
void GetDiffMapWrap3D(TSVec& result, TLightVals* plVals, TSVec* lns,
                      TPPaintLightVals plv);

// ============================================================================
// TLMSfunction wrappers for formula system
// ============================================================================

// These match the TLMSfunction signature for use in TIteration3Dext.PMapFunc/PMapFunc2
void MapFuncSphereSpline(TPVec3D resultVec, TPVec3D pVec3D, int32_t mapNr);
void MapFuncDirectXYspline(TPVec3D resultVec, TPVec3D pVec3D, int32_t mapNr);

} // namespace mb3d

#endif // MB3D_CORE_LIGHTMAP_H
