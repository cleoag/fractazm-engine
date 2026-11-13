#include "io/m3p_parser.h"
#include "core/external_formulas.h"
#include "core/math3d.h"
#include "core/header_trafos.h"
#include <cstring>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <array>
#include <zlib.h>

namespace mb3d {

// Legacy records use byte-packed fields, including odd offsets.
template<class T> static void StorePacked(void* address, T value) {
    std::memcpy(address, &value, sizeof(value));
}

// ============================================================================
// Constants from Pascal source
// ============================================================================

static constexpr float  k_s03      = 0.3f;
static constexpr float  k_s05      = 0.5f;
static constexpr float  k_s001     = 0.01f;
static constexpr float  k_s0001    = 0.001f;
static constexpr float  k_s1d255   = 1.0f / 255.0f;
static constexpr double k_Pid180   = 3.14159265358979323846 / 180.0;
static constexpr double k_Pid16384 = 3.14159265358979323846 / 16384.0;

static constexpr int actMandId    = 44;
static constexpr int actLightId   = 7;    // 0..7
static constexpr int actLightIdEx = 8;    // 0..255
static constexpr float M3dVersion = 1.991f;  // from Mand.pas

static constexpr size_t kTsiLight3Size = 12;
static constexpr size_t kTsiLight4Size = 16;
static constexpr size_t kTsiLight5Size = 18;
static constexpr size_t kHeaderCustomAddonOldSize = 1016;

// Default TLight7
static TLight7 MakeDefaultLight7() {
    TLight7 l;
    std::memset(&l, 0, sizeof(l));
    l.Loption   = 1;
    l.LFunction = 3;
    l.Lcolor    = 0x00FFFFFF;
    l.LXangle   = 0;
    l.LYangle   = 0;
    return l;
}

// Default TLight8
static TLight8 MakeDefaultLight8() {
    TLight8 l;
    std::memset(&l, 0, sizeof(l));
    l.Loption   = 1;
    l.LFunction = 4;
    l.Lamp      = 10;
    l.Lcolor[0] = 0xFF;
    l.Lcolor[1] = 0xFF;
    l.Lcolor[2] = 0xFF;
    l.LightMapNr = 0;
    std::memset(l.LXpos, 0, 7);
    l.AdditionalByteEx = 0;
    std::memset(l.LYpos, 0, 7);
    l.FreeByte = 0;
    std::memset(l.LZpos, 0, 7);
    return l;
}

static const TLight7 defaultLight7 = MakeDefaultLight7();
static const TLight8 defaultLight8 = MakeDefaultLight8();

// ============================================================================
// Helper: CardinalToRGB (from DivUtils.pas)
// ============================================================================
static void CardinalToRGB(uint32_t c, TRGB& rgb) {
    rgb[0] = c & 0xFF;
    rgb[1] = (c >> 8) & 0xFF;
    rgb[2] = (c >> 16) & 0xFF;
}

// ============================================================================
// Helper: MaxOfColor (from DivUtils.pas)
// ============================================================================
static uint32_t MaxOfColor(uint32_t c) {
    uint32_t r = c & 255;
    uint32_t g = (c >> 8) & 255;
    uint32_t b = (c >> 16) & 255;
    return std::max({r, g, b});
}

// ============================================================================
// Helper: InterpolateColor (from Interpolation.pas)
// ============================================================================
static uint32_t InterpolateColor(uint32_t c1, uint32_t c2, double w1, double w2) {
    uint32_t r = static_cast<uint32_t>(std::nearbyint((c1 & 0xFF) * w1 + (c2 & 0xFF) * w2));
    uint32_t g = static_cast<uint32_t>(std::nearbyint((c1 & 0xFF00) * w1 + (c2 & 0xFF00) * w2)) & 0xFF00;
    uint32_t b = static_cast<uint32_t>(std::nearbyint((c1 & 0xFF0000) * w1 + (c2 & 0xFF0000) * w2)) & 0xFF0000;
    return r | g | b;
}

// ============================================================================
// Helper: InterpolateColorB (3-way blend from Interpolation.pas)
// ============================================================================
static uint32_t InterpolateColorB(uint32_t c1, uint32_t c2, uint32_t c3,
                                  double w1, double w2, double w3) {
    uint32_t r = static_cast<uint32_t>(std::nearbyint((c1 & 0xFF) * w1 + (c2 & 0xFF) * w2 + (c3 & 0xFF) * w3));
    uint32_t g = static_cast<uint32_t>(std::nearbyint((c1 & 0xFF00) * w1 + (c2 & 0xFF00) * w2 + (c3 & 0xFF00) * w3)) & 0xFF00;
    uint32_t b = static_cast<uint32_t>(std::nearbyint((c1 & 0xFF0000) * w1 + (c2 & 0xFF0000) * w2 + (c3 & 0xFF0000) * w3)) & 0xFF0000;
    return r | g | b;
}

// ============================================================================
// Helper: ColToSVec_withSqr for light version upgrade (from DivUtils.pas)
// Lightweight version: no sqrt, just normalizes to 0..1
// ============================================================================
static void ColToSVec_NoScale(float result[4], uint32_t c) {
    result[0] = static_cast<float>(c & 0xFF) * k_s1d255;
    result[1] = static_cast<float>((c >> 8) & 0xFF) * k_s1d255;
    result[2] = static_cast<float>((c >> 16) & 0xFF) * k_s1d255;
    result[3] = 0.0f;
}

// ============================================================================
// Helper: YofSVec_local (luminance from TSVec)
// ============================================================================
static float YofSVec_local(const float sv[4]) {
    float y = sv[0] * 0.3f + sv[1] * 0.59f + sv[2] * 0.11f;
    return (y < 0.0f) ? 0.0f : y;
}

// ============================================================================
// ConvertFromOldLightParas
// From FileHandling.pas line 1354
// ============================================================================
void ConvertFromOldLightParas(TLightingParas8& light8, int fSize) {
    if (fSize < static_cast<int>(sizeof(TLightingParas7))) {
        // Smaller than TLightingParas7: only TLightingParas was present
        for (int i = 0; i < 4; i++) {
            light8.Lights[i] = defaultLight7;
        }
        light8.Lights[0].Loption = 0;
        light8.Lights[0].LFunction = static_cast<uint8_t>(light8.TBoptions & 0xFF);
        light8.Lights[0].LXangle = static_cast<int32_t>(std::nearbyint(light8.TBpos[0] * -32768.0 / 360.0));
        light8.Lights[0].LYangle = static_cast<int32_t>(std::nearbyint(light8.TBpos[1] * 32768.0 / 360.0));
    }
    UpdateLightVersion8(light8);
}

// ============================================================================
// UpdateLightVersion8
// From FileHandling.pas line 1404
// ============================================================================
void UpdateLightVersion8(TLightingParas8& light8) {
    light8.TBpos[2] = 0x800080;  // Pascal TBpos[3]
    light8.TBpos[5] = 53;        // Pascal TBpos[6]

    int v = (light8.TBoptions >> 20) & 0x7; // LVersion
    if (v < 2) {
        if (v < 1) {
            double w1 = static_cast<double>(light8.TBpos[4]);  // Pascal TBpos[5] = diff
            double w2 = static_cast<double>(light8.TBpos[4]) + static_cast<double>(light8.TBpos[7]); // Pascal TBpos[5]+TBpos[8]
            if (w2 < 1.0) w2 = 1.0;
            w1 = w1 / w2;
            w2 = 1.0 - w1;
            for (int i = 0; i < 3; i++) {
                // Pascal Cols[i*3+1] and Cols[i*3+3], but Cols is 1-indexed
                // Pascal Cols[1..9], C++ Cols[0..8]
                // So Pascal Cols[i*3+1] = C++ Cols[i*3+0]
                // And Pascal Cols[i*3+3] = C++ Cols[i*3+2]
                light8.Cols[i * 3] = InterpolateColor(light8.Cols[i * 3],
                                                      light8.Cols[i * 3 + 2], w1, w2);
            }
        }
        for (int i = 0; i < 3; i++) {
            light8.LCols[i].Position = static_cast<uint16_t>(i * 10922);
            light8.LCols[i].ColorDif = light8.Cols[i * 3];      // Pascal Cols[i*3+1] = C++ Cols[i*3]
            light8.LCols[i].ColorSpe = light8.Cols[i * 3 + 1];  // Pascal Cols[i*3+2] = C++ Cols[i*3+1]
            light8.ICols[i].Position = static_cast<uint16_t>(i * 10922);
            light8.ICols[i].Color    = light8.Cols[i * 3];
        }
        int n = ((light8.TBoptions & 0x4000) > 0) ? 0 : 2;
        for (int i = 3; i <= 9; i++) {
            light8.LCols[i] = light8.LCols[n];
            light8.LCols[i].Position = static_cast<uint16_t>(32200 + (i - 3) * 84);
        }
        light8.ICols[3] = light8.ICols[n];
        light8.ICols[3].Position = 32760;

        if (v < 1) {
            // Average the 3 ambient colors
            light8.Cols[2] = InterpolateColorB(light8.Cols[2], light8.Cols[5], light8.Cols[8],
                                               0.33333, 0.33333, 0.33333);
            light8.Cols[5] = light8.Cols[2];
            light8.Cols[8] = light8.Cols[2];
            light8.TBoptions = (light8.TBoptions & 0x3F09FFFF) | (2 << 20);
        } else {
            light8.TBoptions = (light8.TBoptions & 0x3F0FFFFF) | (2 << 20);
        }
    }
}

// ============================================================================
// ConvertLight8to9
// From FileHandling.pas line 1476
// ============================================================================
void ConvertLight8to9(const TLightingParas8& light8, TLightingParas9& light9) {
    // Copy first 48 bytes (TBpos[0..10] = 44 bytes + TBoptions = 4 bytes)
    // Pascal: FastMove(Light8, Light9, 48);
    // light8 starts at TBpos[0], light9 starts at VarColZpos.
    // Wait -- Pascal TLightingParas9 starts with VarColZpos(2), RoughnessFactor(1), bColorMap(1),
    //   DynFogCol2(3), AdditionalOptions(1), TBpos[3..11](36), TBoptions(4)... = 48 bytes up to here.
    // Pascal TLightingParas8 starts with TBpos[1..11](44), TBoptions(4)... = 48 bytes.
    // The FastMove copies 48 bytes directly. But the layouts don't align field-by-field for the
    // first 48 bytes -- Pascal FastMove just copies raw memory.
    //
    // In light9: bytes 0-1 = VarColZpos, byte 2 = RoughnessFactor, byte 3 = bColorMap,
    //   bytes 4-6 = DynFogCol2, byte 7 = AdditionalOptions, bytes 8-43 = TBpos[0..8] (9*4=36),
    //   bytes 44-47 = TBoptions
    // In light8: bytes 0-43 = TBpos[0..10] (11*4=44), bytes 44-47 = TBoptions
    //
    // So FastMove(Light8, Light9, 48) copies light8's TBpos[0..10]+TBoptions into
    // light9's {VarColZpos, RoughnessFactor, bColorMap, DynFogCol2, AdditionalOptions, TBpos[0..8], TBoptions}
    std::memcpy(&light9, &light8, 48);

    // Now override specific fields
    light9.TBoptions = light8.TBoptions | (3 << 20);
    light9.FineColAdj1 = static_cast<uint8_t>(light8.TBfineColAdjust & 0xFF);
    // Note: Pascal has a bug here - it assigns FineColAdj1 twice instead of FineColAdj2
    // Line 1483: FineColAdj1 := Light8.TBfineColAdjust and $FF;
    // Line 1484: FineColAdj1 := (Light8.TBfineColAdjust shr 8) and $FF;  <-- should be FineColAdj2
    // We faithfully reproduce the Pascal behavior:
    light9.FineColAdj1 = static_cast<uint8_t>((light8.TBfineColAdjust >> 8) & 0xFF);
    light9.PicOffsetX = 128;
    light9.PicOffsetY = 128;
    light9.PicOffsetZ = 128;

    // Pascal Cols[3] = C++ Cols[2], Cols[6] = C++ Cols[5]
    CardinalToRGB(light8.Cols[2], light9.AmbCol);
    CardinalToRGB(light8.Cols[5], light9.AmbCol2);
    CardinalToRGB(light8.DepthCol >> 8, light9.DepthCol);
    CardinalToRGB(((light8.DepthCol & 0xFF) << 16) | (light8.TBfineColAdjust >> 16), light9.DepthCol2);

    light9.DynFogR = 255;
    light9.DynFogG = 255;
    light9.DynFogB = 255;
    CardinalToRGB(0xFFFFFF, light9.DynFogCol2);

    for (int i = 0; i < 6; i++) {
        light9.Lights[i] = defaultLight8;
    }
    for (int i = 0; i < 4; i++) {
        light9.Lights[i].Loption   = light8.Lights[i].Loption;
        light9.Lights[i].LFunction = light8.Lights[i].LFunction;
        CardinalToRGB(light8.Lights[i].Lcolor, light9.Lights[i].Lcolor);
        light9.Lights[i].LightMapNr = 0;
        DoubleToD7B(light8.Lights[i].LXangle * k_Pid16384, light9.Lights[i].LXpos);
        DoubleToD7B(light8.Lights[i].LYangle * k_Pid16384, light9.Lights[i].LYpos);
    }

    // Copy LCols and ICols (124 bytes = 10*10 + 4*6 = 100 + 24)
    std::memcpy(light9.LCols, light8.LCols, 124);
    light9.BGbmp[0] = 0;

    // TB10 adjustment: Pascal TBpos[10] = C++ TBpos[8] (in light9, 9 elements 0..8, this is index [8])
    // Actually light9 has TBpos[9] = 9 elements indexed 0..8
    // Pascal TBpos[10] in 1-based = light9 TBpos[7] in our 0-based (since light9 TBpos corresponds to Pascal TBpos[3..11])
    // Wait: Pascal TLightingParas9.TBpos: array[3..11] = 9 elements, C++ TBpos[0..8]
    // Pascal TBpos[9] = C++ TBpos[6], Pascal TBpos[10] = C++ TBpos[7]
    int i_val = light9.TBpos[7] + 30;
    i_val = std::max(0, i_val * i_val);
    int j_val = light9.TBpos[6] + 30;
    j_val = std::max(0, j_val * j_val);

    if ((light9.TBoptions & 0x10000) > 0) {
        // Fine down mode
        light9.TBpos[7] = static_cast<int32_t>(std::lround(std::sqrt(std::max(0, i_val + i_val - j_val))) - 30);
        int fi = light9.FineColAdj1;
        int fj = light9.FineColAdj2;
        light9.FineColAdj1 = static_cast<uint8_t>((fi / 2) + 15);
        light9.FineColAdj2 = static_cast<uint8_t>(std::max(0, std::min(255, fj - (fj - fi) / 4)));
    } else {
        light9.TBpos[7] = static_cast<int32_t>(std::lround(std::sqrt(std::max(0, i_val + ((i_val - j_val) * 7) / 4))) - 30);
    }

    // TB12 / TB13 adjustments
    i_val = (light9.TBoptions & 127);
    i_val = i_val * i_val;
    i_val = i_val * i_val; // Sqr(Sqr(...))
    j_val = (light9.TBoptions >> 7) & 127;
    j_val = j_val * j_val;
    j_val = j_val * j_val;
    j_val = static_cast<int32_t>(std::lround(std::sqrt(std::sqrt(std::max(0, j_val + ((j_val - i_val) * 7) / 4)))));
    if (j_val > 120) j_val = 120;
    if (j_val < 0) j_val = 0;

    light9.TBoptions = (light9.TBoptions & 0x3FC07F) | (j_val << 7);
    light9.VarColZpos = static_cast<int16_t>(light9.VarColZpos * 2);
    // Pascal TBpos[4] in 1-based = C++ TBpos[1] in light9 (TBpos corresponds to Pascal 3..11, so Pascal[4]=C++[1])
    light9.TBpos[1] = light9.TBpos[1] * 2;
}

// ============================================================================
// Convert4to7paras
// From FileHandling.pas line 1368
// ============================================================================
void Convert4to7paras(const TMandHeader4& para4, TMandHeader8& para8) {
    // Copy first 108 bytes (up to Light): MandId through dPow
    std::memcpy(&para8.MandId, &para4.MandId, 108);
    // Copy light parameters (TLightingParas size = 92 bytes)
    // Pascal: TBpos[1..11] is 44 bytes, the rest... TLightingParas = 11*4 + 4 + 4 + 4 + 9*4 = 92
    std::memcpy(&para8.Light.TBpos[0], &para4.Light.TBpos[0], sizeof(TLightingParas));
    // Copy from dFOVy onward (72 bytes)
    std::memcpy(&para8.dFOVy, &para4.dFOVy, 72);
    ConvertFromOldLightParas(para8.Light, 92);
    para8.sDEstop = static_cast<float>(((para4.iOptions >> 10) & 15) + 1) * 0.2f;
}

// ============================================================================
// ConvertHeaderFromOldParas
// From FileHandling.pas line 1553
// External formula files (.m3f) are not loaded; addon structure is set up from header data only.
// ============================================================================
static void SetupAddonForOldFormat(const TMandHeader8& para8, THeaderCustomAddon& addon);

bool ConvertHeaderFromOldParas(TMandHeader8& para8, THeaderCustomAddon* addon) {
    static const int cF[16] = {2,2,0,0,1,1,3,3,0,4,5,6,7,8,14,15};
    static const int aF[9]  = {0,0,2,2,3,3,4,4,20};
    static const int nF[12] = {0,1,2,3,-1,4,-1,-1,5,-1,6,20};

    int MId = para8.MandId;
    bool result = (para8.Width > 0) && (para8.Width < 32768) &&
                  (para8.Height > 0) && (para8.Height < 32768);
    if (!result) return false;

    // Save and restore pointer fields
    uint32_t savedPHCustomF[6];
    uint32_t savedPCFAddon;
    std::memcpy(savedPHCustomF, para8.PHCustomF, sizeof(savedPHCustomF));
    savedPCFAddon = para8.PCFAddon;

    if (MId < 4) para8.bCalcAmbShadowAutomatic = 1;
    if (MId < 7) {
        para8.bIsJulia = 0;
        para8.dJx = 0;
        para8.dJy = 0;
        para8.dJz = 0;
    }
    if (MId < 8) {
        ConvertFromOldLightParas(para8.Light, 148);
        para8.sBBmulBulb = 1;
        para8.sBBRbulb = 2;
        // ImageScale is a global, we default to 1
        para8.bImageScale = 1;
        if (para8.bHScalculated == 0) {
            // Pascal TBpos[6] = C++ TBpos[5]
            para8.Light.TBpos[5] = (para8.Light.TBpos[5] - 53) / 3 + 53;
        }
        int i = para8.iOptions & 15;
        para8.iOptions = (para8.iOptions & 0x7FFFFFE0) | cF[i];
        if (i == 3 || i == 8) para8.dZmul = -1;
        else para8.dZmul = 1;
        if (i == 0 || i == 1 || i == 6 || i == 7) {
            para8.iOptions = (para8.iOptions & 0x3FFFF) | ((i & 1) << 18);
        }
        if (i == 2 || i == 3 || i == 8) {
            para8.iOptions = (para8.iOptions & 0x3FFFF) | ((static_cast<int>(i > 3) * 6) << 18);
        }
    }

    int Fnr = para8.iOptions & 31;
    if (MId < 9) {
        para8.dMinR = 0.5;
        if ((Fnr == 5 && ((para8.iOptions >> 18) & 4) > 0) ||
            (Fnr == 8 && ((para8.iOptions >> 18) & 3) == 1)) {
            para8.dMinR = 0.25;
        }
        // Pascal ResetHybrid takes its record by value. Its writes do not
        // modify the saved v8 hybrid, which must survive this conversion.
    }
    if (MId < 10) para8.CustomFname[0] = 0;
    if (MId < 11) {
        para8.bVaryDEstopOnFOV = 1; // Byte(True)
    }
    if (MId < 12) {
        para8.dCutX = para8.sCutX;
        para8.dCutY = para8.sCutY;
        para8.dCutZ = para8.sCutZ;
        if (Fnr == 9) para8.RStop = 1024;
        if (para8.sAmbShadowThreshold < k_s0001) para8.sAmbShadowThreshold = 3;
    }
    if (MId < 13) {
        para8.dCustomOption1 = para8.dPow;
        // Pascal: Light.TBpos[2] := CalcBackgroundShadow
        // TBpos[2] in Pascal 1-based = TBpos[1] in C++ 0-based
        para8.Light.TBpos[1] = para8.CalcBackgroundShadow;
    }
    if (MId < 15) {
        para8.Light.TBoptions = (para8.Light.TBoptions >> 4) | (para8.Light.TBfineColAdjust & 0x10000);
        para8.Light.TBfineColAdjust = (para8.Light.TBfineColAdjust & 0xFFFF) |
                                      ((para8.Light.DepthCol & 0xFFFF) << 16);
        para8.Light.DepthCol = (para8.Light.DepthCol << 8) | ((para8.Light.DepthCol >> 16) & 0xFF);
    }

    // Restore pointer fields
    std::memcpy(para8.PHCustomF, savedPHCustomF, sizeof(savedPHCustomF));
    para8.PCFAddon = savedPCFAddon;

    if (MId < 16) {
        for (int i = 0; i < 4; i++) {
            para8.Light.Lights[i].LFunction =
                (para8.Light.Lights[i].LFunction & 3) |
                ((para8.Light.Lights[i].LFunction << 2) & 0xF0);
        }
    }

    // The old formula parameters overlap the later matrix/DOF fields. Consume
    // them at the same point as Pascal, before those fields are overwritten.
    if (MId < 16 && addon) SetupAddonForOldFormat(para8, *addon);

    if (MId < 17) {
        // Camera rotations have a different sign/order from formula matrices.
        TMandHeader10 camera{};
        std::memcpy(&camera, &para8, sizeof(para8));
        CalcVGradsFromHeader8rots(&camera);
        std::memcpy(&reinterpret_cast<TMandHeader9*>(&para8)->hVGrads,
                    &camera.hVGrads, sizeof(TMatrix3));
    }
    if (MId < 18) {
        // Pascal TBpos[1] in 1-based = C++ TBpos[0]
        para8.Light.TBpos[0] = 0;
        // byRepeatFrom = 0 -- at offset 343 in the struct
        uint8_t* base = reinterpret_cast<uint8_t*>(&para8);
        base[343] = 0;
    }
    if (MId < 20) {
        UpdateLightVersion8(para8.Light);

        // Zero out DOF fields using byte offsets
        uint8_t* base = reinterpret_cast<uint8_t*>(&para8);
        base[181] = 0;     // bCalcDOF := 0
        StorePacked<int32_t>(base + 164, 0);     // sDOFZsharp := 0
        StorePacked<float>(base + 168, 5.0f);    // sDOFradius := 5
        StorePacked<float>(base + 172, 0.5f);    // sDOFosize := 0.5
        StorePacked<int32_t>(base + 327, 0);     // iCalcHStime := 0

        if (para8.bCalculateHardShadow > 0) para8.bCalculateHardShadow = 5;
        para8.bHScalculated = para8.bHScalculated | para8.bCalculateHardShadow;
        base[132] = 0;     // bNormalsOnDE := 0

        int i = std::max(2, ((para8.iOptions >> 14) & 15) + 1); // DEaccuracy 2..16
        StorePacked<float>(base + 182, static_cast<float>(2.2 / (i + 0.25 * i * i))); // mZstepDiv
        para8.sDEcolVar = 0; // = sNaviMinDist
    }

    return result;
}

// ============================================================================
// UpdateFormulaOptionTo20
// From FileHandling.pas line 1899
// ============================================================================
static void NormalizeInternalFormulaSchema(THAformula& formula) {
    const int fnr = formula.iFnr;
    if (fnr < 0 || fnr > 9) return;
    static const char* internalNames[10] = {
        "Integer Power", "Real Power", "Quaternion", "Tricorn", "Amazing Box",
        "Bulbox", "Folding Int Pow", "test", "testIFS", "Aexion C"
    };

    for (int i = 0; i < 16; ++i) {
        formula.byOptionType[i] = 0;
    }

    int requiredCount = 2;

    if (fnr == 0 || fnr == 6) {
        formula.byOptionType[0] = 10; // NOVARIABLE: power selects dispatch, not PVar storage
    }
    if (fnr == 4 || fnr == 5) {
        formula.byOptionType[1] = 7; // BOXSCALE
    }

    switch (fnr) {
        case 4:
            requiredCount = 3;
            formula.byOptionType[2] = 11; // FOLDING16
            break;
        case 5:
            requiredCount = 6;
            formula.byOptionType[2] = 11; // FOLDING16
            formula.byOptionType[3] = 0;
            formula.byOptionType[4] = 9;  // DSQUARE
            formula.byOptionType[5] = 9;  // DSQUARE
            break;
        case 6:
            requiredCount = 3;
            formula.byOptionType[2] = 8; // FOLDING + subformula pointer
            break;
        case 7:
            requiredCount = 8;
            formula.byOptionType[1] = 7;
            formula.byOptionType[2] = 0;
            formula.byOptionType[3] = 0;
            formula.byOptionType[4] = 2;
            formula.byOptionType[5] = 0;
            formula.byOptionType[6] = 2;
            formula.byOptionType[7] = 0;
            break;
        case 8:
            requiredCount = 10;
            {
                const uint8_t types[10] = {0,0,0,0,0,2,2,1,1,2};
                for (int i = 0; i < 10; ++i) formula.byOptionType[i] = types[i];
            }
            break;
        case 9:
            requiredCount = 8;
            formula.byOptionType[2] = 2;
            formula.byOptionType[3] = 2;
            formula.byOptionType[6] = 2;
            formula.byOptionType[7] = 2;
            break;
        default:
            break;
    }

    formula.iOptionCount = requiredCount;
    std::memset(formula.CustomFname, 0, sizeof(formula.CustomFname));
    std::memcpy(formula.CustomFname, internalNames[fnr],
                std::min<size_t>(std::strlen(internalNames[fnr]), sizeof(formula.CustomFname) - 1));
}

static void NormalizeInternalFormulaSchemas(THeaderCustomAddon& addon) {
    for (int i = 0; i < 6; ++i) {
        NormalizeInternalFormulaSchema(addon.Formulas[i]);
    }
}

static void NormalizeResolvedExternalFormulaIds(THeaderCustomAddon& addon) {
    InitExternalFormulaRegistry();

    for (int i = 0; i < 6; ++i) {
        THAformula& formula = addon.Formulas[i];
        if (formula.iFnr <= 9 || formula.CustomFname[0] == 0) {
            continue;
        }

        const char* name = reinterpret_cast<const char*>(formula.CustomFname);
        const ExternalFormulaInfo* extInfo = FindExternalFormula(name);
        if (extInfo != nullptr) {
            // Preserve the on-disk count until Pascal defaults/migrations are applied.
            formula.iFnr = 20;
            ApplyExternalFormulaOptionTypes(formula);
        }
        // Retain unresolved identities and their parameters. Render validation
        // must reject an active missing formula, rather than silently render a
        // different hybrid. Dormant slots remain editable and do not block it.
    }
}

void UpdateFormulaOptionTo20(THeaderCustomAddon& addon) {
    for (int i = 0; i < 6; i++) {
        int j = addon.Formulas[i].iFnr;
        if (j == 2) {
            addon.Formulas[i].dOptionValue[1] = 0; // quat w add
            addon.Formulas[i].byOptionType[1] = 0;
        } else if (j == 4 || j == 5) {
            addon.Formulas[i].iOptionCount = 3;
            if (j == 5) {
                addon.Formulas[i].dOptionValue[4] = addon.Formulas[i].dOptionValue[3];
                addon.Formulas[i].dOptionValue[3] = addon.Formulas[i].dOptionValue[2];
                addon.Formulas[i].byOptionType[3] = 0;
                addon.Formulas[i].byOptionType[4] = 9;
                addon.Formulas[i].iOptionCount = 5;
            }
            addon.Formulas[i].dOptionValue[2] = 1; // box fold
            addon.Formulas[i].byOptionType[2] = 11;
        }
    }

    NormalizeResolvedExternalFormulaIds(addon);
}

// ============================================================================
// ParseExternFormulas
// From FileHandling.pas line 1855
// Renames/adjusts known external formula names.
// ============================================================================
void ParseExternFormulas(THeaderCustomAddon& addon) {
    // Helper to read string from CustomFname (null-terminated, up to 32 bytes)
    auto readCustomFname = [](const uint8_t fname[32]) -> std::string {
        std::string s;
        for (int k = 0; k < 32 && fname[k] != 0; k++) {
            s += static_cast<char>(fname[k]);
        }
        return s;
    };

    // Helper to write string to CustomFname
    auto writeCustomFname = [](uint8_t fname[32], const char* str) {
        std::memset(fname, 0, 32);
        int len = static_cast<int>(std::strlen(str));
        if (len > 31) len = 31;
        std::memcpy(fname, str, len);
    };

    // Helper to uppercase
    auto toUpper = [](std::string s) -> std::string {
        for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return s;
    };

    for (int i = 0; i < 6; i++) {
        if (addon.Formulas[i].iFnr > 19) {
            std::string s = toUpper(readCustomFname(addon.Formulas[i].CustomFname));
            if (s == "_AMAZINGBOXFSSE2") {
                writeCustomFname(addon.Formulas[i].CustomFname, "_AmazingBoxSSE2");
            }
            else if (s == "AMAZINGBOXF") {
                addon.Formulas[i].iFnr = 4;  // Convert to internal formula 4
                writeCustomFname(addon.Formulas[i].CustomFname, "Amazing Box");
            }
            else if (s == "TRAFASSELQUAT") {
                writeCustomFname(addon.Formulas[i].CustomFname, "CommQuat");
            }
            else if (s == "_FLIPXY") {
                writeCustomFname(addon.Formulas[i].CustomFname, "_FlipXYc");
                addon.Formulas[i].dOptionValue[0] = 0;
            }
            else if (s == "_FLIPXZ") {
                writeCustomFname(addon.Formulas[i].CustomFname, "_FlipXZc");
                addon.Formulas[i].dOptionValue[0] = 0;
            }
            else if (s == "_FLIPYZ") {
                writeCustomFname(addon.Formulas[i].CustomFname, "_FlipYZc");
                addon.Formulas[i].dOptionValue[0] = 0;
            }
            else if (s == "ABOXMODKALIFIXED" || s == "ABOXMODKALI") {
                bool isFixed = (s == "ABOXMODKALIFIXED");
                writeCustomFname(addon.Formulas[i].CustomFname, "ABoxModKali");
                if (isFixed) {
                    // Pascal migrates the legacy fixed variant by shifting the tail
                    // left: [3..5] -> [2..4], then expanding the slot count to 5.
                    for (int j = 2; j <= 4; ++j) {
                        addon.Formulas[i].dOptionValue[j] = addon.Formulas[i].dOptionValue[j + 1];
                    }
                    addon.Formulas[i].iOptionCount = 5;
                }
            }
        }
    }
}

// ============================================================================
// UpdateFormulaOptionAbove20
// From FileHandling.pas line 138
// ============================================================================
void UpdateFormulaOptionAbove20(TMandHeader10& para, THeaderCustomAddon& addon) {
    // Bulbox second r parameter
    for (int i = 0; i < 6; i++) {
        int j = addon.Formulas[i].iFnr;
        if (j == 5 && addon.Formulas[i].iOptionCount < 6) {
            addon.Formulas[i].dOptionValue[5] = addon.Formulas[i].dOptionValue[4];
            addon.Formulas[i].byOptionType[5] = 9;
            addon.Formulas[i].iOptionCount = 6;
        }
    }

    ParseExternFormulas(addon);
    NormalizeResolvedExternalFormulaIds(addon);

    int MId = para.MandId;

    if (MId < 21) {
        if (para.mZstepDiv < k_s001) para.mZstepDiv = k_s03;
        para.bPlanarOptic = 0;
    }
    if (MId < 22) {
        para.byColor2Option = 0;
        para.dXWrot = 0;
        para.dYWrot = 0;
        para.dZWrot = 0;
        para.sColorMul = 1;
        para.dJw = 0;
        para.Light.TBoptions = (para.Light.TBoptions & 0xE07FFFFF) |
                               (((para.Light.TBoptions & 0x1F000000) >> 1) | 0x10000000);
    }
    if (MId < 23) para.sRaystepLimiter = 1;
    if (MId < 24) {
        para.StereoScreenWidth = 1;
        para.StereoScreenDistance = 2;
        para.StereoMinDistance = k_s05;
    }
    if (MId < 25) para.HSmaxLengthMultiplier = 1;
    if (MId < 26) para.AODEdithering = 0;
    if (MId < 27) {
        para.SRamount = k_s05;
        para.SRreflectioncount = 1;
        para.bCalcSRautomatic = 0;
        para.byCalcNsOnZBufAuto = 0;
    }
    if (MId < 28) para.SSAORcount = 1;
    if (MId < 29) {
        para.sDOFclipR = 25;
        para.sDOFaperture = 0.02f;
    }
    if (MId < 30) para.sDEAOmaxL = 1;
    if (MId < 31) para.sDEcombS = k_s05;
    if (MId < 32) para.sDOFZsharp2 = para.sDOFZsharp;
    if (MId < 33) para.iMaxItsF2 = para.Iterations;
    if (MId < 34) para.DEmixColorOption = 0;
    if (MId < 35) {
        para.TilingOptions = 0;
        para.bDFogIt = 0;
    }
    if (MId < 36) {
        para.iAmbCalcTime = 0;
        para.bSSAO24BorderMirrorSize = 0;
        para.iOptions = static_cast<uint16_t>(para.iOptions & 0xFFFB);
    }
    para.bStereoMode = 0;

    // Check for specific beta version
    float m3dBeta1796 = 1.796f;
    if (para.sM3dVersion == m3dBeta1796) {
        para.Light.Lights[2].FreeByte = 1;
        para.Light.Lights[3].AdditionalByteEx = 77;
        para.Light.Lights[0].AdditionalByteEx = 3;
    }

    if (MId < 37) {
        para.sTransmissionAbsorption = 0.2f;
        para.sTRIndex = 1.5f;
    }
    if (MId < 38) para.sTRscattering = 1;
    if (MId < 39) {
        para.sFmixPow = 2;
        para.iReflectsCalcTime = 0;
    }
    if (MId < 40) {
        SingleToShortFloat(1.0f, para.MCSoftShadowRadius);
        para.MCDepth = 3;
        para.MCcontrast = 128;
        para.bCalc1HSsoft = 0;
        para.MCoptions = 2;  // secant mode
        para.MCdiffReflects = 0;
        para.iOptions = static_cast<uint16_t>(para.iOptions & 0xFFFD);
    }
    if (MId < 42) {
        para.bNewOptions = 0;
        para.MCoptions = para.MCoptions | 16;  // bokeh2
        para.bVolLightNr = static_cast<uint8_t>(std::min(5, static_cast<int>(para.bVolLightNr)));

        // Find last non-zero formula
        int j = 5;
        while (j > 0 && addon.Formulas[j].iItCount == 0) j--;

        if (addon.bOptions1 != 0)
            addon.bHybOpt1 = 0;
        else
            addon.bHybOpt1 = static_cast<uint8_t>(j | (para.bVolLightNr << 4));

        addon.bHybOpt2 = 0x151;

        if (addon.bOptions1 == 2) { // DEcomb
            addon.bHybOpt2 = static_cast<uint16_t>(1 | (j << 4) |
                (std::max(1, static_cast<int>(para.bVolLightNr)) << 8));
            addon.Formulas[0].iItCount = 1;
            if (addon.bOptions3 != 2 && addon.bOptions3 != 6) {
                // FlipInt
                int32_t tmp = para.iMaxItsF2;
                para.iMaxItsF2 = para.Iterations;
                para.Iterations = tmp;
            }
        } else {
            addon.bOptions3 = 0;
        }

        if (addon.bOptions3 == 2 || addon.bOptions3 == 6) {
            // Rotate formulas: move [0] to [j], shift others down
            THAformula tmpFormula = addon.Formulas[0];
            for (int i = 0; i < j; i++) {
                addon.Formulas[i] = addon.Formulas[i + 1];
            }
            addon.Formulas[j] = tmpFormula;
            if (addon.bOptions3 == 6) addon.bOptions3 = 5;
            addon.bHybOpt1 = static_cast<uint8_t>(std::max(0, j - 1) |
                (std::max(0, static_cast<int>(para.bVolLightNr) - 1) << 4));
            addon.bHybOpt2 = static_cast<uint16_t>(j | (j << 4) | (j << 8));
        }
    }
    if (MId < 43) {
        para.bMCSaturation = 32;
        para.bColorOnIt = 0;
        if (para.byColor2Option == 5) para.bColorOnIt = 1;
        else if (para.byColor2Option == 6) para.byColor2Option = 5;
    }
    if (MId > 43 && (para.bNewOptions & 1) != 0) {
        // Convert Quaternion back to RotationMatrix
        TQuaternion q;
        std::memcpy(q, para.hVGrads, sizeof(TQuaternion));
        QuatToMatrix(para.hVGrads, q);
        double d = 2.1345 / (para.dZoom * para.Width);
        TMatrix3 tmpM;
        std::memcpy(tmpM, para.hVGrads, sizeof(TMatrix3));
        NormaliseMatrixTo(para.hVGrads, d, tmpM);
    }
    if (MId < 44) {
        para.bVolLightNr = 2 << 4;
    }

    // Update M3D version (matches Pascal: sM3dVersion := M3dVersion)
    para.sM3dVersion = M3dVersion;
}

// ============================================================================
// UpdateLightParasAbove3
// From FileHandling.pas line 297
// ============================================================================
void UpdateLightParasAbove3(TLightingParas9& light) {
    int verex = 0;
    int ver = (light.TBoptions >> 20) & 7;

    if (ver < 4) {
        light.DynFogCol2[0] = light.DynFogR;
        light.DynFogCol2[1] = light.DynFogG;
        light.DynFogCol2[2] = light.DynFogB;
    }
    if (ver < 5) {
        // Pascal TBpos[6] in 1-indexed (3..11) = C++ TBpos[3] in 0-indexed
        // Wait: Pascal TLightingParas9 TBpos[3..11] = 9 elements
        // Pascal TBpos[6] = C++ TBpos[3] (offset by 3)
        // NI10: Lower bound is max(0,...) not max(-159,...)
        light.TBpos[3] = std::max(0, std::min(159, (light.TBpos[3] - 53) * 2 + 53));
        light.VarColZpos = static_cast<int16_t>(std::max(-120, std::min(360, static_cast<int>(light.VarColZpos) * 2)));
    }
    if (ver < 6) {
        light.bColorMap = 0;
        light.RoughnessFactor = 255;
        light.PicOffsetZ = static_cast<uint8_t>((static_cast<int>(light.PicOffsetZ) + 128) & 255);
        for (int i = 0; i < 6; i++) {
            light.Lights[i].Loption = light.Lights[i].Loption & 0xFD; // clear bit2 (lightmap)
        }
    }
    if (ver < 7) {
        for (int i = 0; i < 6; i++) {
            light.Lights[i].LightMapNr = light.Lights[i].LightMapNr & 0xFF;
            light.Lights[i].AdditionalByteEx = 0;
            light.Lights[i].FreeByte = 0;
        }
    } else {
        verex = light.Lights[0].AdditionalByteEx;
    }

    light.TBoptions = (light.TBoptions & 0xFF8FFFFF) | (actLightId << 20);

    if (verex < 1) {
        light.Lights[0].FreeByte = 0;
    }
    if (verex < 2) {
        light.Lights[2].AdditionalByteEx = 30;  // diffmap scaling
        light.Lights[1].FreeByte = (light.AdditionalOptions >> 1) & 1;
        light.AdditionalOptions = light.AdditionalOptions & 0xFB;
    }
    if (verex < 3) {
        light.Lights[3].AdditionalByteEx = 128;  // amount of diffuse shadowing
        light.Lights[2].FreeByte = 0;  // iExModes
    }
    if (verex < 5) {
        for (int i = 0; i < 4; i++) {
            light.ICols[i].Color = light.ICols[i].Color & 0xFFFFFF;
        }
        for (int i = 0; i < 10; i++) {
            light.LCols[i].ColorSpe = (light.LCols[i].ColorSpe & 0xFFFFFF) |
                                      (MaxOfColor(light.LCols[i].ColorSpe) << 24);
        }
    }
    if (verex < 6) {
        int i = (light.AdditionalOptions >> 4) & 7;
        if (i != 0) i = 16;
        light.AdditionalOptions = (light.AdditionalOptions & 0x8F) | static_cast<uint8_t>(i);

        for (int idx = 0; idx < 6; idx++) {
            if ((light.Lights[idx].Loption & 6) != 4) {
                ShortFloat sf;
                SingleToShortFloat(1.0f, sf);
                uint16_t val;
                std::memcpy(&val, sf, 2);
                light.Lights[idx].Lamp = val;
            }
        }
        light.Lights[4].AdditionalByteEx = 40;

        for (int idx = 0; idx < 4; idx++) {
            float sv[4];
            ColToSVec_NoScale(sv, light.ICols[idx].Color);
            float y = YofSVec_local(sv);
            light.ICols[idx].Color = (light.ICols[idx].Color & 0xFFFFFF) |
                                     (static_cast<uint32_t>(std::lround(y * 255.0f)) << 24);
        }
    }
    if (verex < 7) {
        for (int i = 0; i < 6; i++) {
            light.Lights[i].LFunction = light.Lights[i].LFunction & 0x3F;
        }
    }
    if (verex < 8) {
        light.Lights[0].FreeByte = light.Lights[0].FreeByte & 1;
        light.Lights[3].FreeByte = 0;  // No col-ipol
    }
    light.Lights[0].AdditionalByteEx = actLightIdEx;
}

// ============================================================================
// SetupAddonForOldFormat
// Initialize addon from old header format (MId < 16)
// ============================================================================
static void SetupAddonForOldFormat(const TMandHeader8& para8, THeaderCustomAddon& addon) {
    static const int nF[12] = {0,1,2,3,-1,4,-1,-1,5,-1,6,20};

    std::memset(&addon, 0, sizeof(THeaderCustomAddon));
    addon.bHCAversion = 1;
    addon.bOptions1 = 0;
    addon.bOptions2 = 0;
    addon.iFCount = 1;

    for (int i = 1; i < 6; i++) {
        addon.Formulas[i].iItCount = 0;
        addon.Formulas[i].iFnr = -1;
    }
    for (int j = 0; j < 5; j++) {
        addon.Formulas[0].byOptionType[j] = 0;
    }

    int Fnr = para8.iOptions & 31;
    int safeFnr = std::min(11, Fnr);
    addon.Formulas[0].iFnr = nF[safeFnr];
    addon.Formulas[0].iItCount = 1;

    if (Fnr == 9) {
        // Old hybrid -> parse from header
        static const int aF[9] = {0,0,2,2,3,3,4,4,20};
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 5; j++) addon.Formulas[i].byOptionType[j] = 0;
            addon.Formulas[i].iItCount = para8.iHcount[i];
            addon.Formulas[i].iFnr = aF[std::max(0, std::min(8, static_cast<int>(para8.iHformula[i])))];
            if (para8.iHcount[i] > 0) {
                addon.Formulas[i].dOptionValue[0] = para8.dHPow[i];
                addon.Formulas[i].dOptionValue[1] = para8.dHZmul[i];
                switch (addon.Formulas[i].iFnr) {
                    case 2: // Quat
                        if (para8.iHformula[i] == 2)
                            addon.Formulas[i].dOptionValue[0] = 1;
                        else
                            addon.Formulas[i].dOptionValue[0] = -1;
                        break;
                    case 3: // Tricorn
                        if (para8.iHformula[i] == 4) {
                            addon.Formulas[i].dOptionValue[0] = -2;
                            addon.Formulas[i].dOptionValue[1] = 1;
                        } else {
                            addon.Formulas[i].dOptionValue[0] = 2;
                            addon.Formulas[i].dOptionValue[1] = 0;
                        }
                        break;
                    case 4: // Box
                        addon.Formulas[i].dOptionValue[0] = para8.dHPow[i];
                        addon.Formulas[i].dOptionValue[1] = para8.dHMinR[i];
                        addon.Formulas[i].byOptionType[1] = 7; // MinR = type .BOXSCALE
                        break;
                }
                if (para8.iHformula[i] > 7) {
                    // External formula -- mark as external
                    std::memcpy(addon.Formulas[i].CustomFname, para8.CustomFname, 16);
                    // Fresh Pascal state retains the initial Integer Power
                    // slot's two saved values; later slots have count zero.
                    // Ini1CFfromHAddon subsequently fills unsaved defaults.
                    addon.Formulas[i].iOptionCount = i == 0 ? 2 : 0;
                    addon.Formulas[i].dOptionValue[0] = para8.dCustomOption1;
                    addon.Formulas[i].dOptionValue[1] = para8.dZmul;
                    addon.Formulas[i].dOptionValue[2] = para8.dMinR;
                    addon.Formulas[i].iFnr = 20;
                }
            }
        }
    } else if (Fnr == 11) {
        // External formula
        std::memcpy(addon.Formulas[0].CustomFname, para8.CustomFname, 16);
        addon.Formulas[0].iOptionCount = 2;
        addon.Formulas[0].dOptionValue[0] = para8.dCustomOption1;
        addon.Formulas[0].dOptionValue[1] = para8.dZmul;
        addon.Formulas[0].dOptionValue[2] = para8.dMinR;
        addon.Formulas[0].iFnr = 20;
    } else {
        // Internal formula -- set option types, counts, and defaults
        // From FileHandling.pas line 1711:
        //   i := PTHeaderCustomAddon(PCFAddon).Formulas[0].iFnr;
        //   if i >= 0 then GetHAddOnFromInternFormula(@para8, i, 0);
        if (addon.Formulas[0].iFnr >= 0) {
            GetHAddOnFromInternFormula(&addon, addon.Formulas[0].iFnr, 0);
        }
        // Override option values from header (after GetHAddOnFromInternFormula sets defaults)
        addon.Formulas[0].dOptionValue[0] = para8.dPow;
        addon.Formulas[0].dOptionValue[1] = para8.dZmul;
        int iopts = (para8.iOptions >> 18) & 7;
        switch (Fnr) {
            case 0:
            case 10:
                addon.Formulas[0].dOptionValue[0] = iopts + 2;
                if (Fnr == 10) addon.Formulas[0].dOptionValue[2] = para8.sBBRbulb;
                break;
            case 2: // Quaternion
                addon.Formulas[0].dOptionValue[0] = 1 - (iopts & 1) * 2;
                break;
            case 3: // Tricorn
                addon.Formulas[0].dOptionValue[0] = (iopts & 1) * 4 - 2;
                addon.Formulas[0].dOptionValue[1] = 1 - (iopts & 1);
                break;
            case 5:
            case 8: // Box / Bulbox
                addon.Formulas[0].dOptionValue[1] = para8.dMinR;
                if (Fnr == 8) {
                    addon.Formulas[0].dOptionValue[2] = std::max(0.0, static_cast<double>(para8.sBBmulBulb));
                    addon.Formulas[0].dOptionValue[3] = std::max(0.0, static_cast<double>(para8.sBBRbulb));
                }
                break;
        }
        if (Fnr == 4 || Fnr == 6 || Fnr == 7) {
            auto& f = addon.Formulas[0];
            const char* name = Fnr == 4 ? "_SinePow2" : Fnr == 6 ? "Riemann" : "Fuzzy";
            std::memset(f.CustomFname, 0, sizeof(f.CustomFname));
            std::memcpy(f.CustomFname, name, std::strlen(name));
            f.iFnr = 20;
            f.iOptionCount = Fnr == 6 ? 1 : 2;
            f.dOptionValue[0] = para8.dPow;
            f.dOptionValue[1] = 1 - 2 * (iopts & 1);
            if (Fnr == 4) {
                f.dOptionValue[0] = 1;
                f.dOptionValue[1] = 1;
                addon.Formulas[1] = f;
                addon.Formulas[1].dOptionValue[0] = 1 - ((iopts >> 1) & 1) * 2;
                GetHAddOnFromInternFormula(&addon, 0, 2);
                addon.Formulas[2].iItCount = 1;
                addon.Formulas[2].dOptionValue[0] = 2;
                addon.Formulas[2].dOptionValue[1] = 1 - (iopts & 1) * 2;
            }
        }
    }
}

// ============================================================================
// LoadHAddon -- Load header addon from buffer
// From FileHandling.pas line 1810
// ============================================================================
static void LoadHAddonFromBuffer(const uint8_t* data, size_t dataSize, size_t offset,
                                 THeaderCustomAddon& addon) {
    if (offset >= dataSize) return;

    size_t remaining = dataSize - offset;
    // Check first byte to determine format
    if (remaining >= 4) {
        int32_t firstInt;
        std::memcpy(&firstInt, data + offset, 4);
        int firstByte = firstInt & 0xFF;
        if (firstByte >= 16 && firstByte <= 99) {
            // New format -- read THeaderCustomAddon directly
            size_t toCopy = std::min(remaining, sizeof(THeaderCustomAddon));
            std::memcpy(&addon, data + offset, toCopy);
            return;
        }
    }

    // Old format -- THeaderCustomAddonOld
    // THeaderCustomAddonOld layout:
    //   iHCAversion: Integer (4)
    //   iCustomCount: Integer (4)
    //   iItCounts: array[0..5] of Integer (24)
    //   iFormula: array[0..5] of Integer (24)
    //   CustomFname: array[0..5] of array[0..31] of Byte (192)
    //   dOptionValues: array[0..5] of array[0..15] of Double (768)
    // Total: 4+4+24+24+192+768 = 1016
    struct THeaderCustomAddonOld {
        int32_t  iHCAversion;
        int32_t  iCustomCount;
        int32_t  iItCounts[6];
        int32_t  iFormula[6];
        uint8_t  CustomFname[6][32];
        double   dOptionValues[6][16];
    };
    static_assert(sizeof(THeaderCustomAddonOld) == kHeaderCustomAddonOldSize,
                  "THeaderCustomAddonOld must be 1016 bytes");

    THeaderCustomAddonOld haOld;
    std::memset(&haOld, 0, sizeof(haOld));
    size_t toCopy = std::min(remaining, sizeof(THeaderCustomAddonOld));
    std::memcpy(&haOld, data + offset, toCopy);

    for (int i = 0; i < 6; i++) {
        addon.Formulas[i].iItCount = haOld.iItCounts[i];
        addon.Formulas[i].iFnr = haOld.iFormula[i];
        std::memcpy(addon.Formulas[i].CustomFname, haOld.CustomFname[i], 32);
        for (int t = 0; t < 16; t++) {
            addon.Formulas[i].dOptionValue[t] = haOld.dOptionValues[i][t];
        }
    }
}

// ============================================================================
// TryDecompress -- attempt zlib decompression
// ============================================================================
static bool TryDecompress(const std::vector<uint8_t>& compressed,
                          std::vector<uint8_t>& decompressed) {
    // Try progressively larger output buffers
    for (uLongf outSize = 8192; outSize <= 1024 * 1024; outSize *= 2) {
        decompressed.resize(outSize);
        uLongf actualSize = outSize;
        int ret = uncompress(decompressed.data(), &actualSize,
                             compressed.data(), static_cast<uLong>(compressed.size()));
        if (ret == Z_OK) {
            decompressed.resize(actualSize);
            return true;
        }
        if (ret != Z_BUF_ERROR) {
            break; // Not a buffer size issue, data is not zlib
        }
    }
    return false;
}

static size_t GetRawHeaderSizeForMandId(int32_t mandId) {
    if (mandId < 4) return 200;
    if (mandId < 7) return sizeof(TMandHeader4);
    if (mandId < 8) return sizeof(TMandHeader9) - 124;
    if (mandId < 20) return sizeof(TMandHeader9);
    return sizeof(TMandHeader10);
}

// FileHandling.LoadParameter/LoadFullM3I use DivUtils.GetTileSize, which
// clamps BOTH untiled and tiled dimensions. This is a serialized-cache rule;
// it does not change direct runtime header dimensions or render allocation.
static bool GetLegacyCacheTileDimensions(int32_t mandId, const TMandHeader10& header,
                                        size_t& width, size_t& height) {
    const uint32_t tiling = mandId < 35 ? 0 : header.TilingOptions;
    int64_t w = header.Width, h = header.Height;
    if (tiling != 0) {
        const int countX = tiling & 0x7F;
        const int countY = (tiling >> 7) & 0x7F;
        if (countX == 0 || countY == 0) return false; // original divides by zero
        const int crop = std::max(1u, (tiling >> 28) & 3u);
        w = w / countX + 2 * crop;
        h = h / countY + 2 * crop;
    }
    width = static_cast<size_t>(std::clamp<int64_t>(w, 1, 30000));
    height = static_cast<size_t>(std::clamp<int64_t>(h, 1, 30000));
    return true;
}

static bool GetStoredPixelDataSize(int32_t mandId, const TMandHeader10& header,
                                   size_t& bytes) {
    bytes = 0;
    if (mandId <= 15 || header.Width <= 0 || header.Height <= 0) return true;
    // The old LoadParameter seek deliberately predates GetTileSize.
    if (mandId <= 18) {
        const uint64_t pixels = static_cast<uint64_t>(header.Width) * header.Height;
        const size_t recordSize = mandId <= 17 ? kTsiLight3Size : kTsiLight4Size;
        if (pixels > SIZE_MAX / recordSize) return false;
        bytes = static_cast<size_t>(pixels) * recordSize;
        return true;
    }
    size_t width, height;
    if (!GetLegacyCacheTileDimensions(mandId, header, width, height)) return false;
    bytes = width * height * kTsiLight5Size;
    return true;
}

// ============================================================================
// LoadM3PFile
// ============================================================================
M3PLoadResult LoadM3PFile(const std::string& filepath) {
    M3PLoadResult result;
    std::memset(&result.header, 0, sizeof(TMandHeader10));
    std::memset(&result.addon, 0, sizeof(THeaderCustomAddon));
    // Fresh Pascal application defaults retained by the old-addon reader.
    result.addon.bHCAversion = 1;
    result.addon.bHybOpt2 = 0x151;
    result.success = false;

    // Read file
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        result.error = "Cannot open file: " + filepath;
        return result;
    }
    auto fileSize = file.tellg();
    if (fileSize < 20) {
        result.error = "File too small: " + filepath;
        return result;
    }
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> fileData(static_cast<size_t>(fileSize));
    file.read(reinterpret_cast<char*>(fileData.data()), fileSize);
    file.close();
    result.raw_file_size = static_cast<uint64_t>(fileData.size());

    // Determine if data is compressed or raw
    // Raw: first 4 bytes = MandId (typically 1..250)
    // Compressed: first 2 bytes are zlib header (0x78 0x01/0x5E/0x9C/0xDA)
    const uint8_t* data = fileData.data();
    size_t dataSize = fileData.size();

    std::vector<uint8_t> decompressedBuf;
    bool isCompressed = false;

    // Check for zlib header
    if (dataSize >= 2 && data[0] == 0x78) {
        if (TryDecompress(fileData, decompressedBuf)) {
            data = decompressedBuf.data();
            dataSize = decompressedBuf.size();
            isCompressed = true;
        }
    }
    result.is_compressed = isCompressed;
    result.loaded_data_size = static_cast<uint64_t>(dataSize);

    // If not detected as zlib, check if raw MandId makes sense
    if (!isCompressed) {
        int32_t testMId;
        std::memcpy(&testMId, data, 4);
        if (testMId < 0 || testMId > 250 || dataSize < 100) {
            // Try decompression as last resort
            if (TryDecompress(fileData, decompressedBuf)) {
                data = decompressedBuf.data();
                dataSize = decompressedBuf.size();
                isCompressed = true;
            } else {
                result.error = "Invalid file format or corrupt data";
                return result;
            }
        }
    }

    // Validate before reading the word: a valid zlib stream may contain less
    // than four bytes of uncompressed data.
    if (dataSize < 100) {
        result.error = "Decompressed parameter record is too small";
        return result;
    }
    // Validate MandId
    int32_t MId;
    std::memcpy(&MId, data, 4);
    if (MId < 0 || MId > 250 || dataSize < 100) {
        result.error = "Invalid MandId or file too small";
        return result;
    }
    result.source_mand_id = MId;
    const size_t requiredHeaderSize = GetRawHeaderSizeForMandId(MId);
    if (dataSize < requiredHeaderSize) {
        result.error = "Truncated parameter header";
        return result;
    }

    // ==============================
    // VERSION-DEPENDENT LOADING
    // ==============================
    TMandHeader4 mandHeader4;
    TMandHeader9 para9;
    std::memset(&mandHeader4, 0, sizeof(mandHeader4));
    std::memset(&para9, 0, sizeof(para9));

    if (MId < 4) {
        // TMandHeaderOld: 5 ints + 6 doubles + TLightingParas = 20 + 48 + 92 = 160
        // Actually: 5*4 + 3*8 + 3*8 + 3*8 + 3*8 + TLightingParas(92) = 20+24+24+24+24+92 = 208?
        // Pascal TMandHeaderOld = 5*4 + 6*8 + TLightingParas = 20 + 48 + 92 = 160
        // TMandHeaderOld: MandId(4)+Width(4)+Height(4)+Iterations(4)+iOptions(4) = 20
        //   + dZstart(8)+dZend(8) = 16
        //   + dXmid(8)+dYmid(8)+dZmid(8) = 24
        //   + dXrot(8)+dYrot(8)+dZrot(8) = 24
        //   + dZoom(8)+RStop(8)+dPow(8) = 24
        //   + Light(TLightingParas = 11*4+4+4+4+9*4 = 92) = 92
        //   Total = 20+16+24+24+24+92 = 200
        // Let's just compute: sizeof(TMandHeaderOld) is not in our types.h,
        // but it's the header up to and including dPow (108 bytes) plus TLightingParas (92)
        // That's exactly sizeof(TMandHeader4) minus the fields after Light.
        // TMandHeader4 in our types.h has fields after Light, so TMandHeaderOld
        // is everything up to Light + Light = just 108 + 92 = 200
        size_t oldSize = 200; // TMandHeaderOld size = 108 + sizeof(TLightingParas)
        size_t toCopy = std::min(dataSize, oldSize);
        std::memcpy(&mandHeader4, data, toCopy);
        // Zero out fields after Light (dFOVy onward -- 72 bytes)
        std::memset(&mandHeader4.dFOVy, 0, 72);
    } else if (MId < 7) {
        size_t toCopy = std::min(dataSize, sizeof(TMandHeader4));
        std::memcpy(&mandHeader4, data, toCopy);
    } else if (MId < 8) {
        // TMandHeader9 minus last 124 bytes (LightingParas8 = 272 + some buffers)
        size_t toCopy = std::min(dataSize, sizeof(TMandHeader9) - 124);
        std::memcpy(&para9, data, toCopy);
    } else if (MId < 20) {
        size_t toCopy = std::min(dataSize, sizeof(TMandHeader9));
        std::memcpy(&para9, data, toCopy);
    } else {
        size_t toCopy = std::min(dataSize, sizeof(TMandHeader10));
        std::memcpy(&result.header, data, toCopy);
    }

    // Pre-conversion adjustments for very old formats
    if (MId < 5 && mandHeader4.dZstart > mandHeader4.dZend) {
        mandHeader4.dZstart = -mandHeader4.dZstart;
        mandHeader4.dZend   = -mandHeader4.dZend;
        double d = mandHeader4.dXrot;
        mandHeader4.dXrot = mandHeader4.dYrot * k_Pid180;
        mandHeader4.dYrot = d * k_Pid180;
        // Pascal Cols[7] = C++ Cols[6], etc. (1-based to 0-based)
        mandHeader4.Light.Cols[6] = mandHeader4.Light.Cols[3];
        mandHeader4.Light.Cols[7] = mandHeader4.Light.Cols[4];
        mandHeader4.Light.Cols[8] = mandHeader4.Light.Cols[5];
    }
    if (MId < 5) {
        mandHeader4.dZmid = (mandHeader4.dZstart + mandHeader4.dZend) * 0.5;
    } else if (MId == 5 && (mandHeader4.iOptions & 15) == 10) {
        mandHeader4.dPow = (mandHeader4.iOptions >> 18) & 3;
        int i = (mandHeader4.iOptions & 0x38000) >> 1;
        mandHeader4.iOptions = (mandHeader4.iOptions & 0x7FFC3FFF) | i;
    }

    if (MId < 7) {
        if (MId < 6) mandHeader4.bHScalculated = 1;
        mandHeader4.Light.TBoptions = mandHeader4.Light.TBoptions | (43 << 11);
        // Convert4to7paras -- converts TMandHeader4 -> TMandHeader8 (which overlaps TMandHeader9)
        Convert4to7paras(mandHeader4, *reinterpret_cast<TMandHeader8*>(&para9));
    } else if (MId < 8) {
        ConvertFromOldLightParas(reinterpret_cast<TMandHeader8*>(&para9)->Light, 148);
    }

    result.header.MandId = MId;

    // Zero out pointer fields on the header
    std::memset(result.header.PHCustomF, 0, sizeof(result.header.PHCustomF));
    result.header.PCFAddon = 0;

    size_t rawHeaderSize = GetRawHeaderSizeForMandId(MId);
    result.raw_header_size = static_cast<uint64_t>(rawHeaderSize);

    if (MId < 20) {
        // Old format path
        std::memset(para9.PHCustomF, 0, sizeof(para9.PHCustomF));
        para9.PCFAddon = 0;

        bool convOk = ConvertHeaderFromOldParas(*reinterpret_cast<TMandHeader8*>(&para9), &result.addon);
        if (!convOk) {
            result.error = "Header conversion failed (invalid dimensions)";
            return result;
        }

        // Copy para9 -> header10 (up to sizeof TMandHeader9)
        std::memcpy(&result.header, &para9, sizeof(TMandHeader9));
        // Convert lighting
        ConvertLight8to9(reinterpret_cast<TMandHeader9*>(&para9)->Light, result.header.Light);

    }

    if (MId > 15) {
        size_t addonOffset = rawHeaderSize;
        size_t pixelDataSize;
        if (!GetStoredPixelDataSize(MId, result.header, pixelDataSize)) {
            result.error = "Invalid cache extent: size overflow or zero tiling divisor";
            return result;
        }
        result.pixel_data_size = static_cast<uint64_t>(pixelDataSize);
        const size_t available = dataSize - addonOffset;
        if (pixelDataSize > 0 && available >= kHeaderCustomAddonOldSize &&
            pixelDataSize <= available - kHeaderCustomAddonOldSize) {
            addonOffset += pixelDataSize;
        }
        result.addon_offset = static_cast<uint64_t>(addonOffset);
        if (addonOffset > dataSize || dataSize - addonOffset < 4) {
            result.error = "Missing formula addon";
            return result;
        }
        const uint8_t addonVersion = data[addonOffset];
        const size_t requiredAddon = addonVersion >= 16 && addonVersion <= 99
            ? sizeof(THeaderCustomAddon) : kHeaderCustomAddonOldSize;
        if (dataSize - addonOffset < requiredAddon) {
            result.error = "Truncated formula addon";
            return result;
        }
        LoadHAddonFromBuffer(data, dataSize, addonOffset, result.addon);
    }

    if (MId < 20) {
        UpdateFormulaOptionTo20(result.addon);
    }

    // Zero out pointer fields
    std::memset(result.header.PHCustomF, 0, sizeof(result.header.PHCustomF));
    result.header.PCFAddon = 0;

    // Apply version upgrades
    UpdateFormulaOptionAbove20(result.header, result.addon);
    UpdateLightParasAbove3(result.header.Light);
    NormalizeInternalFormulaSchemas(result.addon);

    // SetEditsFromHeader writes this complete byte to a three-entry RadioGroup.
    // Original LoadParameter rejects out-of-range values; the core's direct
    // camera selector is a separate domain and must not be masked here.
    if (result.header.bPlanarOptic > 2) {
        result.error = "Invalid file camera optic: original GUI selector accepts 0..2";
        return result;
    }

    // MakeHeader reads a maximum light number of six (Mand.pas:1500).
    if ((result.header.bVolLightNr & 7u) == 7u)
        result.header.bVolLightNr = static_cast<uint8_t>(result.header.bVolLightNr - 1u);

    // Original file-load reflection controls are read back before the active
    // ordinary SR pass (Mand.pas:1408, PostProcessForm.pas:471). In particular,
    // saved zero absorption becomes the positive lower bound. Preserve direct
    // Render(header) semantics and dormant file fields outside this loader path.
    if ((result.header.bCalcSRautomatic & 1u) != 0 &&
        (result.header.MCoptions & 1u) == 0) {
        result.header.SRamount = Min0MaxCS(result.header.SRamount, 100.0f);
        result.header.SRreflectioncount =
            std::max<uint8_t>(1, std::min<uint8_t>(8, result.header.SRreflectioncount));
        result.header.sTransmissionAbsorption =
            MinMaxCS(1e-30f, result.header.sTransmissionAbsorption, 1e10f);
        result.header.sTRIndex = MinMaxCS(0.1f, result.header.sTRIndex, 10.0f);
        result.header.sTRscattering = Min0MaxCS(result.header.sTRscattering, 1e30f);
    }

    // FormulaGUI maps the reserved low-bit mode 3 to its Alternating selection
    // when MakeHeader reads the loaded controls back (3->0, 7->4, 255->252).
    // Core/direct-header mode 3 retains the original CheckHybridOptions branch.
    if ((result.addon.bOptions1 & 3u) == 3u)
        result.addon.bOptions1 &= static_cast<uint8_t>(~3u);

    // FormulaGUI's Inside combo has three entries. On a fresh file load its
    // invalid fourth index keeps the Outside selection before MakeHeader.
    // Preserve the analytic-DE disable bit and unrelated options.
    if ((result.addon.bOptions2 & 6u) == 6u)
        result.addon.bOptions2 &= static_cast<uint8_t>(~6u);

    // Headless Pascal loads the formula controls and reads them back before
    // rendering (Mand.pas, MakeHeader: DE mix settings; Math3D.sNotZero).
    // In particular a zero saved mix power becomes +0.01, not pow(0, -n).
    if (result.addon.bOptions3 >= 5) {
        result.header.DEmixColorOption = std::min<uint8_t>(2, result.header.DEmixColorOption);
        float power = std::max(-100.0f, std::min(100.0f, result.header.sFmixPow));
        if (std::fabs(power) <= 0.01f) power = power < 0.0f ? -0.01f : 0.01f;
        result.header.sFmixPow = power;
    }

    // Mark as fully migrated so re-loading won't re-apply migrations
    result.header.MandId = actMandId;

    // Validate formula references
    for (int i = 0; i < 6; i++) {
        int fnr = result.addon.Formulas[i].iFnr;
        if (fnr >= 20) {
            // External formula -- warn but don't fail
            // In the full application this would load formula files
        }
    }

    // Basic validation
    if (result.header.Width <= 0 || result.header.Height <= 0) {
        result.error = "Invalid header: Width or Height <= 0";
        return result;
    }

    result.success = true;
    return result;
}

// ============================================================================
// ConvertSiLight4To5
// From FileHandling.pas line 1386
// Converts siLight4 angle-based normals to siLight5 sin-based normals.
// ============================================================================
static void ConvertSiLight4To5(const uint8_t* sl4Data, TsiLight5* sl5, int n) {
    // Build sin lookup table: SinTab[i] = sin(i * pi/16384), i = 0..32767
    static const auto SinTab = [] {
        std::array<float, 32768> table{};
        for (int i = 0; i < 32768; ++i)
            table[i] = static_cast<float>(std::sin(i * k_Pid16384));
        return table;
    }();

    // TsiLight4 layout (16 bytes):
    //   [0..1] LightAngleX (uint16_t)
    //   [2..3] LightAngleY (uint16_t)
    //   [4..5] RoughZposFine (uint16_t)
    //   [6..7] Zpos (uint16_t)
    //   [8..9] Shadow (uint16_t)
    //   [10..11] AmbShadow (uint16_t)
    //   [12..13] SIgradient (uint16_t)
    //   [14..15] OTrap (uint16_t)
    for (int i = 0; i < n; i++) {
        const uint8_t* src = sl4Data + static_cast<size_t>(i) * 16;
        uint16_t angleX, angleY;
        std::memcpy(&angleX, src + 0, 2);
        std::memcpy(&angleY, src + 2, 2);

        // Pascal evaluates the products as Single, then Round uses nearest-even.
        // Keep each multiplication in float; lround incorrectly rounds ties away.
        sl5[i].NormalX = static_cast<int16_t>(std::nearbyint(
            SinTab[angleX & 0x7FFF] * 32767.0f));
        sl5[i].NormalY = static_cast<int16_t>(std::nearbyint(
            SinTab[angleY & 0x7FFF] * 32767.0f));
        sl5[i].NormalZ = static_cast<int16_t>(std::nearbyint(
            SinTab[(angleX + 8192) & 0x7FFF] *
            SinTab[(angleY + 8192) & 0x7FFF] * 32767.0f));

        // Copy remaining 12 bytes: RoughZposFine through OTrap
        // ptt(@PSL5.RoughZposFine)^ := ptt(@PSL4.RoughZposFine)^;
        // ptt is array[0..11] of Byte — 12 bytes from RoughZposFine onward
        std::memcpy(&sl5[i].RoughZposFine, src + 4, 12);
    }
}

// ============================================================================
// ConvertSiLight3To4
// From FileHandling.pas lines 2229-2248
// Converts siLight3 (12 bytes) to siLight4 (16 bytes) in a temporary buffer.
// ============================================================================
static void ConvertSiLight3To4(const uint8_t* sl3Data, std::vector<uint8_t>& sl4Data, int n) {
    sl4Data.resize(static_cast<size_t>(n) * 16);

    // TsiLight3 layout (12 bytes):
    //   [0..1] LightAngleX
    //   [2..3] LightAngleY
    //   [4..5] Zpos
    //   [6..7] Shadow
    //   [8..9] AmbShadow
    //   [10..11] SIgradient
    //
    // TsiLight4 layout (16 bytes):
    //   [0..1] LightAngleX
    //   [2..3] LightAngleY
    //   [4..5] RoughZposFine = 0
    //   [6..7] Zpos
    //   [8..9] Shadow
    //   [10..11] AmbShadow
    //   [12..13] SIgradient
    //   [14..15] OTrap = 0

    for (int i = 0; i < n; i++) {
        const uint8_t* src = sl3Data + static_cast<size_t>(i) * 12;
        uint8_t* dst = sl4Data.data() + static_cast<size_t>(i) * 16;

        // Copy LightAngleX + LightAngleY (4 bytes)
        std::memcpy(dst, src, 4);
        // RoughZposFine = 0
        dst[4] = 0; dst[5] = 0;
        // Copy Zpos + Shadow (4 bytes)
        std::memcpy(dst + 6, src + 4, 4);
        // Copy AmbShadow + SIgradient (4 bytes)
        std::memcpy(dst + 10, src + 8, 4);
        // OTrap = 0
        dst[14] = 0; dst[15] = 0;
    }
}

// ============================================================================
// PostProcessSiLight5ForOldMID
// From FileHandling.pas lines 2263-2297
// Applies version-specific corrections to siLight5 buffer for MID < 20
// and MID == 35 DFog.
// ============================================================================
static void PostProcessSiLight5ForOldMID(TsiLight5* sl5, int n, int MID,
                                          const TMandHeader10& header) {
    if (MID < 20) {
        // PW starts at byte offset 10 from siLight5[0], which is the Shadow field.
        // siLight5 layout: NormalX(2) NormalY(2) NormalZ(2) RoughZposFine(2) Zpos(2) Shadow(2) ...
        // Offset 10 = Shadow.
        // PW+2 = AmbShadow, PW-6 = NormalZ (offset 4)
        // PW-2 = Zpos (offset 8)
        if (header.bCalculateHardShadow > 0) {
            for (int i = 0; i < n; i++) {
                // AmbShadow >>= 1
                sl5[i].AmbShadow >>= 1;
                // Negate NormalZ
                sl5[i].NormalZ = -sl5[i].NormalZ;
                // Shadow: if < 32767 then $400, else 0
                if (sl5[i].Shadow < 32767) {
                    sl5[i].Shadow = 0x0400;
                } else {
                    sl5[i].Shadow = 0;
                }
            }
        } else {
            for (int i = 0; i < n; i++) {
                // AmbShadow >>= 1
                sl5[i].AmbShadow >>= 1;
                // Negate NormalZ
                sl5[i].NormalZ = -sl5[i].NormalZ;
                // Shadow remap: if Zpos > 32767 (background) then 100,
                // else (32767 - (Shadow & 0x7FFF)) >> 5
                if (sl5[i].Zpos > 32767) {
                    sl5[i].Shadow = 100;
                } else {
                    sl5[i].Shadow = (32767 - (sl5[i].Shadow & 0x7FFF)) >> 5;
                }
            }
        }
    } else if (MID == 35 && header.bDFogIt > 0) {
        for (int i = 0; i < n; i++) {
            uint16_t s = sl5[i].Shadow;
            sl5[i].Shadow = (s & 0xFC00) | ((s & 0x3FF) >> 2);
        }
    }
}

// ============================================================================
// LoadM3IFile
// From FileHandling.pas lines 2198-2316
// Loads a .m3i render cache: header + siLight buffer + optional image.
// ============================================================================
M3ILoadResult LoadM3IFile(const std::string& filepath) {
    M3ILoadResult result;
    result.success = false;

    // Load the header/params via LoadM3PFile (which handles all migrations).
    // The .m3i file starts with the same header as .m3p (just uncompressed raw).
    result.params = LoadM3PFile(filepath);
    if (!result.params.success) {
        result.error = result.params.error;
        return result;
    }
    // LoadFullM3I reopens the uncompressed file for its pixel records. A zlib
    // parameter stream is supported by LoadM3PFile, but is not a raw M3I cache.
    if (result.params.is_compressed) {
        result.error = "Compressed M3I pixel data is unsupported";
        return result;
    }

    // Re-open file to read pixel data after the header.
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        result.error = "Cannot reopen file: " + filepath;
        return result;
    }
    const auto endPos = file.tellg();
    if (endPos < 0) {
        result.error = "Cannot determine M3I file size";
        return result;
    }
    const auto totalFileSize = static_cast<uint64_t>(endPos);
    file.seekg(0, std::ios::beg);

    // Read MID from first 4 bytes (original on-disk MandId before migration)
    int32_t MID = result.params.source_mand_id;

    // Determine header seek position (matches Pascal Seek logic)
    size_t headerSeekPos;
    if (MID < 4) headerSeekPos = 200;
    else if (MID < 7) headerSeekPos = sizeof(TMandHeader4); // 272
    else if (MID < 8) headerSeekPos = sizeof(TMandHeader9) - 124; // 580
    else if (MID < 20) headerSeekPos = sizeof(TMandHeader9);
    else headerSeekPos = sizeof(TMandHeader10); // 840

    // Pixel count: tile-aware computation matching Pascal GetTileSize
    int w = result.params.header.Width;
    int h = result.params.header.Height;
    if (w <= 0 || h <= 0) {
        result.error = "Invalid dimensions for siLight buffer";
        return result;
    }
    size_t tileW, tileH;
    if (!GetLegacyCacheTileDimensions(MID, result.params.header, tileW, tileH)) {
        result.error = "Invalid M3I tiling count: Pascal divisor is zero";
        return result;
    }
    const int j = static_cast<int>(tileW * tileH); // each axis is clamped to30000
    // SetImageSize allocates the tile size for tiled images; untiled images
    // retain Width*Height. LoadFullM3I reads an image only if lengths agree.
    const uint64_t fullImagePixelCount = MID >= 35 && result.params.header.TilingOptions != 0
        ? static_cast<uint64_t>(j) : static_cast<uint64_t>(w) * h;

    const uint64_t recordSize = MID < 18 ? kTsiLight3Size :
                                MID == 18 ? kTsiLight4Size : kTsiLight5Size;
    const uint64_t pixelBytes = static_cast<uint64_t>(j) * recordSize;
    if (headerSeekPos > totalFileSize || pixelBytes > totalFileSize - headerSeekPos) {
        result.error = "Truncated M3I siLight pixel data";
        return result;
    }

    file.seekg(static_cast<std::streamoff>(headerSeekPos), std::ios::beg);

    // Allocate siLight5 output
    result.siLight.resize(static_cast<size_t>(j));
    std::memset(result.siLight.data(), 0, static_cast<size_t>(j) * sizeof(TsiLight5));

    if (MID < 18) {
        // Read siLight3, convert to siLight4, then siLight4 to siLight5
        size_t sl3Size = static_cast<size_t>(j) * kTsiLight3Size;
        std::vector<uint8_t> sl3Data(sl3Size);
        file.read(reinterpret_cast<char*>(sl3Data.data()), static_cast<std::streamsize>(sl3Size));
        if (file.gcount() != static_cast<std::streamsize>(sl3Size)) {
            result.error = "Failed to read siLight3 data";
            result.siLight.clear();
            return result;
        }

        // Convert siLight3 -> siLight4
        std::vector<uint8_t> sl4Data;
        ConvertSiLight3To4(sl3Data.data(), sl4Data, j);

        // Convert siLight4 -> siLight5
        ConvertSiLight4To5(sl4Data.data(), result.siLight.data(), j);
    } else if (MID <= 18) {
        // MID == 18: read siLight4, convert to siLight5
        size_t sl4Size = static_cast<size_t>(j) * kTsiLight4Size;
        std::vector<uint8_t> sl4Data(sl4Size);
        file.read(reinterpret_cast<char*>(sl4Data.data()), static_cast<std::streamsize>(sl4Size));
        if (file.gcount() != static_cast<std::streamsize>(sl4Size)) {
            result.error = "Failed to read siLight4 data";
            result.siLight.clear();
            return result;
        }
        ConvertSiLight4To5(sl4Data.data(), result.siLight.data(), j);
    } else {
        // MID > 18: read siLight5 directly
        size_t sl5Size = static_cast<size_t>(j) * kTsiLight5Size;
        file.read(reinterpret_cast<char*>(result.siLight.data()), static_cast<std::streamsize>(sl5Size));
        if (file.gcount() != static_cast<std::streamsize>(sl5Size)) {
            result.error = "Failed to read siLight5 data";
            result.siLight.clear();
            return result;
        }
    }

    // Post-process siLight5 for old versions and DFog
    PostProcessSiLight5ForOldMID(result.siLight.data(), j, MID, result.params.header);
    result.has_silight = true;

    // Skip THeaderCustomAddon (already loaded by LoadM3PFile)
    // Pascal: Seek(f, FilePos(f) + SizeOf(THeaderCustomAddon));
    const uint64_t imageOffset = headerSeekPos + pixelBytes + sizeof(THeaderCustomAddon);

    // Read fullSizeImage if present (uint32_t per pixel)
    const uint64_t imageSize = static_cast<uint64_t>(j) * 4;
    if (fullImagePixelCount == static_cast<uint64_t>(j) && imageOffset <= totalFileSize &&
        imageSize <= totalFileSize - imageOffset) {
        file.seekg(static_cast<std::streamoff>(imageOffset), std::ios::beg);
        result.image.resize(static_cast<size_t>(j));
        file.read(reinterpret_cast<char*>(result.image.data()), static_cast<std::streamsize>(imageSize));
        if (file.gcount() == static_cast<std::streamsize>(imageSize)) {
            result.has_image = true;
        } else {
            // A file shortened between the extent check and read must not expose
            // a successful partial image; the already complete siLight is usable.
            result.image.clear();
        }
    }

    result.success = true;
    return result;
}

// ============================================================================
// SaveM3PFile
// ============================================================================
bool SaveM3PFile(const std::string& filepath,
                 const TMandHeader10& header,
                 const THeaderCustomAddon& addon) {
    std::ofstream file(filepath, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) return false;

    // Write header (840 bytes)
    TMandHeader10 saveHeader = header;
    // Zero out pointer fields for disk format
    std::memset(saveHeader.PHCustomF, 0, sizeof(saveHeader.PHCustomF));
    saveHeader.PCFAddon = 0;

    file.write(reinterpret_cast<const char*>(&saveHeader), sizeof(TMandHeader10));

    // Write addon (1136 bytes)
    THeaderCustomAddon saveAddon = addon;
    saveAddon.bHCAversion = 16; // Current version
    file.write(reinterpret_cast<const char*>(&saveAddon), sizeof(THeaderCustomAddon));

    return file.good();
}

} // namespace mb3d
