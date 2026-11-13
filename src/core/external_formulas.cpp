#include "core/external_formulas.h"
#include "core/generated/external_formula_defaults.h"

#include "core/header_trafos.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#include <cctype>
#include <mutex>
#include <stdexcept>

namespace mb3d {

// ============================================================================
// ReadFormulaParams — reads PVar at per-formula byte offsets
// ============================================================================

int ReadFormulaParams(const void* pVar, const FormulaParamMap& map, double* outParams) {
    const uint8_t* base = static_cast<const uint8_t*>(pVar);
    for (int i = 0; i < map.count; ++i) {
        const uint8_t* addr = base + map.entries[i].byteOffset;
        switch (map.entries[i].valueType) {
            case 0: {
                double v;
                std::memcpy(&v, addr, 8);
                outParams[i] = v;
                break;
            }
            case 1: {
                float f;
                std::memcpy(&f, addr, 4);
                outParams[i] = static_cast<double>(f);
                break;
            }
            case 2: {
                int32_t iv;
                std::memcpy(&iv, addr, 4);
                outParams[i] = static_cast<double>(iv);
                break;
            }
            case 3: {
                int64_t iv;
                std::memcpy(&iv, addr, 8);
                outParams[i] = static_cast<double>(iv);
                break;
            }
            default: {
                double v;
                std::memcpy(&v, addr, 8);
                outParams[i] = v;
                break;
            }
        }
    }
    return map.count;
}

// ============================================================================
// It3DextFromPIt3D — get TIteration3Dext* from TPIteration3D
//
// In Pascal, TPIteration3D points to the C1 field inside TIteration3Dext.
// TIteration3Dext has 7 doubles (J4, Rold, RStopD, x, y, z, w) before C1,
// so C1 is at byte offset 56 from the start of TIteration3Dext.
// This matches the existing ExtFromIt3D helper in formulas.cpp.
// ============================================================================

TIteration3Dext* It3DextFromPIt3D(TPIteration3D pit) {
    return reinterpret_cast<TIteration3Dext*>(
        reinterpret_cast<char*>(pit) - 7 * sizeof(double));
}

// ============================================================================
// AppendCValues — append Julia/C constants from TIteration3D
//
// Reads J1, J2, J3 from the TIteration3D struct (accessible via pit),
// and J4 from the parent TIteration3Dext.
// ============================================================================

void AppendCValues(TPIteration3D pit, double* params, int& paramCount) {
    params[paramCount++] = pit->J1;
    params[paramCount++] = pit->J2;
    params[paramCount++] = pit->J3;
    TIteration3Dext* ext = It3DextFromPIt3D(pit);
    params[paramCount++] = ext->J4;
}

void AppendPersistentCValues(TPIteration3D pit, double* params, int& paramCount) {
    params[paramCount++] = pit->C1;
    params[paramCount++] = pit->C2;
    params[paramCount++] = pit->C3;
    TIteration3Dext* ext = It3DextFromPIt3D(pit);
    params[paramCount++] = ext->J4;
}

// ============================================================================
// AppendExtState — append VaryScale and bFirstIt from TIteration3Dext
// Used by formulas that need per-iteration VaryScale state (ABoxVaryScale, ABoxMod1)
// ============================================================================

void AppendExtState(TIteration3Dext* ext, double* params, int& paramCount) {
    params[paramCount++] = ext->VaryScale;
    params[paramCount++] = static_cast<double>(ext->bFirstIt);
}

// ============================================================================
// WriteBackVaryScale — write updated VaryScale back to ext
// Called after execute_formula for formulas that compute VaryScale
// ============================================================================

void WriteBackVaryScale(TIteration3Dext* ext, const double* params, int varyScaleIdx) {
    if (ext && params && varyScaleIdx >= 0 && varyScaleIdx < 64) {
        ext->VaryScale = params[varyScaleIdx];
    }
}

// ============================================================================
// buildRotMatrix3 — build 3x3 rotation matrix from 3 Euler angles (degrees)
//
// Exact port of Pascal BuildRotMatrix from Math3D.pas.
// Convention: M = Rx(ax) * Ry(ay) * Rz(az) (intrinsic X-Y-Z / extrinsic Z-Y-X).
// Input angles in degrees, converted to radians internally.
//
// Pascal reference (Math3D.pas line 2481):
//   M[0,0] := cosY * cosZ
//   M[0,1] := -cosY * sinZ
//   M[0,2] := sinY
//   M[1,0] := sinX * sinY * cosZ + cosX * sinZ
//   M[1,1] := cosX * cosZ - sinX * sinY * sinZ
//   M[1,2] := -sinX * cosY
//   M[2,0] := sinX * sinZ - cosX * sinY * cosZ
//   M[2,1] := cosX * sinY * sinZ + sinX * cosZ
//   M[2,2] := cosX * cosY
// ============================================================================

static constexpr double kDeg2Rad = 3.14159265358979323846 / 180.0;

RotMatrix3 buildRotMatrix3(double ax_deg, double ay_deg, double az_deg) {
    double ax = ax_deg * kDeg2Rad;
    double ay = ay_deg * kDeg2Rad;
    double az = az_deg * kDeg2Rad;

    double sinX = std::sin(ax);
    double cosX = std::cos(ax);
    double sinY = std::sin(ay);
    double cosY = std::cos(ay);
    double sinZ = std::sin(az);
    double cosZ = std::cos(az);

    RotMatrix3 r{};
    r.m[0][0] = cosY * cosZ;
    r.m[0][1] = -cosY * sinZ;
    r.m[0][2] = sinY;
    r.m[1][0] = sinX * sinY * cosZ + cosX * sinZ;
    r.m[1][1] = cosX * cosZ - sinX * sinY * sinZ;
    r.m[1][2] = -sinX * cosY;
    r.m[2][0] = sinX * sinZ - cosX * sinY * cosZ;
    r.m[2][1] = cosX * sinY * sinZ + sinX * cosZ;
    r.m[2][2] = cosX * cosY;

    return r;
}

// ============================================================================
// buildRotMatrix4 — build 4x4 rotation matrix from 6 Euler angles (degrees)
//
// Exact port of Pascal BuildRotMatrix4d from Math3D.pas (line 2551).
// The 6 angles correspond to the 6 rotation planes in 4D:
//   angles[0] = YZ, angles[1] = XZ, angles[2] = XY,
//   angles[3] = XW, angles[4] = YW, angles[5] = ZW
//
// For each angle, a rotation matrix in the corresponding plane is constructed
// and multiplied into the accumulator. The plane indices are:
//   i1 = {1, 0, 0, 0, 1, 2}
//   i2 = {2, 2, 1, 3, 3, 3}
//
// Pascal uses single-precision TSMatrix4 (float[4][4]) intermediates.
// Here we use double precision in the RotMatrix4 struct, matching the
// header declaration. The algorithm is identical.
//
// Pascal Multiply2SMatrix4(@SM4, @ms4) computes SM4 = SM4 * ms4,
// then ms4 := SM4 copies the result back.
// ============================================================================

// Note: matches Pascal TSMatrix4 = array[0..3,0..3] of Single (float).
// See also header_trafos.cpp:870 BuildSMatrix4 which operates on TSMatrix4.
RotMatrix4 buildRotMatrix4(double a1_deg, double a2_deg, double a3_deg,
                           double a4_deg, double a5_deg, double a6_deg) {
    static const int i1[6] = {1, 0, 0, 0, 1, 2};
    static const int i2[6] = {2, 2, 1, 3, 3, 3};

    double angles[6] = {
        a1_deg * kDeg2Rad, a2_deg * kDeg2Rad, a3_deg * kDeg2Rad,
        a4_deg * kDeg2Rad, a5_deg * kDeg2Rad, a6_deg * kDeg2Rad
    };

    // Initialize ms4 to identity (float, matching Pascal TSMatrix4)
    RotMatrix4 ms4{};
    for (int i = 0; i < 4; ++i) ms4.m[i][i] = 1.0f;

    for (int i = 0; i < 6; ++i) {
        float s1 = static_cast<float>(std::sin(angles[i]));
        float c1 = static_cast<float>(std::cos(angles[i]));

        // Initialize SM4 to identity (float)
        float SM4[4][4] = {};
        for (int j = 0; j < 4; ++j) SM4[j][j] = 1.0f;

        // Set rotation plane entries
        SM4[i1[i]][i1[i]] = c1;
        SM4[i2[i]][i2[i]] = c1;
        SM4[i1[i]][i2[i]] = -s1;
        SM4[i2[i]][i1[i]] = s1;

        // Multiply2SMatrix4(@SM4, @ms4): SM4 = SM4 * ms4
        // Pascal: M1[r,c] = sum_k MT[r,k] * M2[k,c]
        float MT[4][4];
        std::memcpy(MT, SM4, sizeof(MT));

        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                SM4[r][c] = MT[r][0] * ms4.m[0][c]
                          + MT[r][1] * ms4.m[1][c]
                          + MT[r][2] * ms4.m[2][c]
                          + MT[r][3] * ms4.m[3][c];
            }
        }

        // ms4 := SM4
        std::memcpy(ms4.m, SM4, sizeof(ms4.m));
    }

    return ms4;
}

// ============================================================================
// Registry — static storage and lookup maps
// ============================================================================

static std::vector<ExternalFormulaInfo> s_registry;
static std::unordered_map<std::string, size_t> s_nameIndex;   // uppercase name -> index
static std::unordered_map<unsigned int, size_t> s_idIndex;     // ID -> index
static std::once_flag s_registryInit;
static thread_local bool s_registering = false;

// ToUpperCase helper for case-insensitive name matching
static std::string ToUpperCase(const char* s) {
    std::string result;
    if (!s) return result;
    while (*s) {
        result += static_cast<char>(std::toupper(static_cast<unsigned char>(*s)));
        ++s;
    }
    return result;
}

// CleanCustomFname — extract valid string from 32-byte CustomFname field.
// CustomFname is NOT null-terminated; it may contain trailing nulls, spaces,
// or garbage bytes after the actual name.
static std::string CleanCustomFname(const char* name) {
    if (!name) return {};
    // Find the end: stop at first null byte or after 32 chars (max CustomFname size)
    int len = 0;
    while (len < 32 && name[len] != '\0') ++len;
    // Trim trailing spaces
    while (len > 0 && (name[len - 1] == ' ' || name[len - 1] == '\0')) --len;
    return std::string(name, len);
}

void RegisterExternalFormula(const ExternalFormulaInfo& info) {
    if (!s_registering) {
        throw std::logic_error("External formulas can only be registered during registry initialization");
    }
    size_t idx = s_registry.size();
    s_registry.push_back(info);
    std::string upperName = ToUpperCase(info.name);
    s_nameIndex[upperName] = idx;
    s_idIndex[info.id] = idx;
}

void InitExternalFormulaRegistry() {
    // Registration must not perform lookups: recursively entering call_once
    // from its own initializer would deadlock.
    if (s_registering) {
        throw std::logic_error("External formula registration cannot recursively initialize or query the registry");
    }
    std::call_once(s_registryInit, [] {
        s_registering = true;
        try {
            RegisterExternalFormulaTable();
        } catch (...) {
            // call_once retries after an exception. Discard every partial entry
            // first, so that retry neither duplicates entries nor publishes a
            // stale index. Readers remain behind call_once until success.
            s_nameIndex.clear();
            s_idIndex.clear();
            s_registry.clear();
            s_registering = false;
            throw;
        }
        s_registering = false;
    });
}

const ExternalFormulaInfo* FindExternalFormula(const char* name) {
    InitExternalFormulaRegistry();
    std::string cleaned = CleanCustomFname(name);
    std::string upper = ToUpperCase(cleaned.c_str());
    auto it = s_nameIndex.find(upper);
    if (it != s_nameIndex.end()) {
        return &s_registry[it->second];
    }
    return nullptr;
}

size_t ExternalFormulaCount() {
    InitExternalFormulaRegistry();
    return s_registry.size();
}

const ExternalFormulaInfo* ExternalFormulaAt(size_t index) {
    InitExternalFormulaRegistry();
    return index < s_registry.size() ? &s_registry[index] : nullptr;
}

const ExternalFormulaInfo* FindExternalFormulaById(unsigned int id) {
    InitExternalFormulaRegistry();
    auto it = s_idIndex.find(id);
    if (it != s_idIndex.end()) {
        return &s_registry[it->second];
    }
    return nullptr;
}

ThybridIteration GetExternalFormula(const char* customFname, int* outDEoption) {
    const ExternalFormulaInfo* info = FindExternalFormula(customFname);
    if (!info || !info->wrapperFunc) return nullptr;
    if (outDEoption) *outDEoption = info->DEoption;
    return info->wrapperFunc;
}

void ApplyExternalFormulaOptionTypes(THAformula& formula) {
    if (formula.iFnr < 20) return;  // only for external formulas
    const char* name = reinterpret_cast<const char*>(formula.CustomFname);
    const ExternalFormulaInfo* info = FindExternalFormula(name);
    if (!info || info->optionTypeCount <= 0) return;
    // Expand entry-based option types to Pascal VALUE-index semantics.
    // Pascal duplicates the type marker across every consumed value slot for
    // multi-value entries like .3SingleAngles and .6SingleAngles.
    std::memset(formula.byOptionType, 0, 16);
    int valueIdx = 0;
    for (int entry = 0; entry < info->optionTypeCount && valueIdx < 16; ++entry) {
        uint8_t t = info->optionTypes[entry];
        int valueCount = 1;
        switch (t) {
            case 5:  // 3DoubleAngles: 3 values
            case 6:  // 3SingleAngles: 3 values
                valueCount = 3;
                break;
            case 12: // 6SingleAngles: 6 values
                valueCount = 6;
                break;
            case 18: // 3ScalesAngles: 4 values (scale + 3 angles)
                valueCount = 4;
                break;
            case 19: // ScalesRot: 2 values
                valueCount = 2;
                break;
            default:
                break;
        }
        for (int k = 0; k < valueCount && valueIdx + k < 16; ++k) {
            // Mirror Pascal VALUE-index semantics for compound option types.
            // .3ScalesAngles occupies 4 value slots but only the first slot keeps
            // type 18; the three angle payloads are exposed as type 6 entries.
            formula.byOptionType[valueIdx + k] = (t == 18 && k > 0) ? 6 : t;
        }
        valueIdx += valueCount;
    }
    // HeaderTrafos.Ini1CFfromHAddon ignores bytes beyond the saved option count.
    // Old scenes must acquire current defaults, even when unused payload is nonzero.
    int loadedCount = std::clamp<int>(formula.iOptionCount, 0, 16);
    const std::string upperName = ToUpperCase(info->name);
    if (loadedCount < valueIdx) {
        if (upperName == "CYLINDERIFS" && loadedCount < 9) {
            formula.dOptionValue[8] = formula.dOptionValue[7];
            formula.dOptionValue[7] = 0;
            if (loadedCount < 8) formula.dOptionValue[6] = 0;
            loadedCount = 9;
        } else if (upperName == "SPHEREHEIGHTMAP") {
            for (int j = valueIdx - 1; j >= 9; --j)
                formula.dOptionValue[j] = formula.dOptionValue[j - 1];
            formula.dOptionValue[8] = 1;
            formula.dOptionValue[2] /= NotZero(formula.dOptionValue[5]);
            formula.dOptionValue[3] /= NotZero(formula.dOptionValue[6]);
            loadedCount = valueIdx;
        }
        for (const auto& defaults : kExternalOptionDefaults) {
            if (upperName != defaults.name) continue;
            for (int j = loadedCount; j < std::min(valueIdx, defaults.count); ++j)
                formula.dOptionValue[j] = defaults.values[j];
            break;
        }
    }
    formula.iOptionCount = valueIdx;
}

} // namespace mb3d
