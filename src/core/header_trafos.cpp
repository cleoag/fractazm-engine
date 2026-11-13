#include "core/render_context.h"
#include "core/header_trafos.h"
#include "core/parity_trace.h"
#include "core/math3d.h"
#include "core/formulas.h"
#include "core/formula_registry.h"
#include "core/external_formulas.h"
#include "core/lightmap.h"
#include "core/ray_marcher.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <limits>
#include <sstream>
#include <iomanip>
#include <stdexcept>

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void FormulaAngleDouble_x87(double*, double*, const double*);
extern "C" void FormulaAngleSingle_x87(float*, float*, const double*);
extern "C" int64_t FormulaIntegerOption_x87(const double*);
#endif

namespace mb3d {

static void IniSMatrix4(TSMatrix4& m);
static void MultiplySMatrix4(TSMatrix4& m1, const TSMatrix4& m2);
static void BuildRotMatrix4d(const double* angles, TSMatrix4& ms4);

struct TSortItem {
    int iZ;
    int wX;
};

static inline float PascalColorByteToUnit(uint32_t v) {
    return static_cast<float>(v) * (1.0f / 255.0f);
}

static inline float PascalColorByteToUnitSqr(uint32_t v) {
    return static_cast<float>(v * v) * 0.0000153787f;
}

static inline void PascalColToSVec(TSVec& result, uint32_t c, bool sqr) {
    if (sqr) {
        result[0] = PascalColorByteToUnitSqr(c & 0xFFu);
        result[1] = PascalColorByteToUnitSqr((c >> 8) & 0xFFu);
        result[2] = PascalColorByteToUnitSqr((c >> 16) & 0xFFu);
    } else {
        result[0] = PascalColorByteToUnit(c & 0xFFu);
        result[1] = PascalColorByteToUnit((c >> 8) & 0xFFu);
        result[2] = PascalColorByteToUnit((c >> 16) & 0xFFu);
    }
    result[3] = 0.0f;
}

static inline void PascalColAToSVec(TSVec& result, uint32_t c, bool sqr) {
    PascalColToSVec(result, c, sqr);
    if (sqr) {
        result[3] = PascalColorByteToUnitSqr(c >> 24);
    } else {
        result[3] = PascalColorByteToUnit(c >> 24);
    }
}

static void SortLights(TPLightVals Lvals) {
    static const int stab[4] = {0, 2, 1, 3};
    auto quickSortItems = [&](auto&& self, TSortItem* items, int left, int right) -> void {
        int lpos = left - 1;
        int rpos = right;
        int pivot = items[right].iZ;
        for (;;) {
            do { ++lpos; } while (items[lpos].iZ < pivot);
            do { --rpos; } while ((rpos > lpos) && (items[rpos].iZ > pivot));
            if (lpos >= rpos) break;
            std::swap(items[lpos], items[rpos]);
        }
        std::swap(items[lpos], items[right]);
        if (lpos - 1 > left) self(self, items, left, lpos - 1);
        if (right > lpos + 1) self(self, items, lpos + 1, right);
    };

    TSortItem list[6];
    TLightVals lvtmp = *Lvals;
    TSVec tmpLN[6];
    TSVec tmpsLCols[6];
    TPLightMap tmpLightMaps[6];

    for (int i = 0; i < 6; ++i) {
        if ((Lvals->iLightPos[i] & 6) > 0) {
            list[i].iZ = static_cast<int>(std::lrint(Lvals->sPosLP[i]));
        } else {
            list[i].iZ = 40000 + stab[Lvals->iLightOption[i] & 3];
        }
        list[i].wX = i;
        std::memcpy(tmpLN[i], Lvals->PLValigned->LN[i], sizeof(TSVec));
        std::memcpy(tmpsLCols[i], Lvals->PLValigned->sLCols[i], sizeof(TSVec));
        tmpLightMaps[i] = Lvals->LLightMaps[i];
    }

    quickSortItems(quickSortItems, list, 0, 5);

    for (int i = 0; i < 6; ++i) {
        int j = list[i].wX;
        Lvals->SortTab[j] = static_cast<uint8_t>(i);
        Lvals->iLightOption[i] = lvtmp.iLightOption[j];
        Lvals->iHSenabled[i] = lvtmp.iHSenabled[j];
        Lvals->iHScalced[i] = lvtmp.iHScalced[j];
        Lvals->iHSmask[i] = lvtmp.iHSmask[j];
        Lvals->iLightPos[i] = lvtmp.iLightPos[j];
        Lvals->iLightAbs[i] = lvtmp.iLightAbs[j];
        Lvals->iLightPowFunc[i] = lvtmp.iLightPowFunc[j];
        Lvals->iLightFuncDiff[i] = lvtmp.iLightFuncDiff[j];
        Lvals->sLmaxL[i] = lvtmp.sLmaxL[j];
        Lvals->sPosLightZpos[i] = lvtmp.sPosLightZpos[j];
        Lvals->sPosLightXpos[i] = lvtmp.sPosLightXpos[j];
        Lvals->sPosLightYpos[i] = lvtmp.sPosLightYpos[j];
        Lvals->sPosLP[i] = lvtmp.sPosLP[j];
        Lvals->LLightMaps[i] = tmpLightMaps[j];
        std::memcpy(Lvals->PLValigned->LN[i], tmpLN[j], sizeof(TSVec));
        std::memcpy(Lvals->PLValigned->sLCols[i], tmpsLCols[j], sizeof(TSVec));
    }
}

// ============================================================================
// CalcStepWidth -- from DivUtils.pas
// ============================================================================

void CalcStepWidth(TMandHeader10* Header) {
    Header->dStepWidth = 2.1345 / (Header->dZoom * Header->Width);
}

// ============================================================================
// NotZero -- ensure value is never exactly zero
// ============================================================================

double NotZero(double d) {
    if (d == 0.0) return 1e-300;
    return d;
}

float CalcXoff(const TMandHeader10* Header) {
    double xoff = -0.065 / MaxCD(1e-100, Header->StereoScreenWidth);
    float result = 0.5f;
    switch (Header->bStereoMode) {
        case 1: result = static_cast<float>(0.5 + xoff); break;
        case 3: result = static_cast<float>(0.5 * (1.0 - xoff)); break;
        case 4: result = static_cast<float>(0.5 * (1.0 + xoff)); break;
        default: break;
    }
    return result;
}

static void StereoChange(TMandHeader10* Header, int mode, TVec3D& midPos, TMatrix3* pMatrix) {
    CalcStepWidth(Header);
    TMatrix3 matrix{};
    const void* packedMatrix = static_cast<const void*>(pMatrix);
    std::memcpy(&matrix, packedMatrix, sizeof(matrix));
    TVec3D v{matrix[0][0], matrix[0][1], matrix[0][2]};
    NormaliseVectorToInPlace(Header->dStepWidth, v);
    TVec3D headerMid{Header->dXmid, Header->dYmid, Header->dZmid};
    double eyedist = 0.065 * Header->Width * Header->StereoScreenDistance /
                     MaxCD(1e-100, Header->StereoMinDistance * Header->StereoScreenWidth);
    double xadd = 0.0;
    switch (mode) {
        case 1:
            xadd = -eyedist * (Header->StereoScreenDistance - Header->StereoMinDistance) /
                   Header->StereoScreenDistance;
            break;
        case 3:
            xadd = 0.5 * eyedist * (Header->StereoScreenDistance - Header->StereoMinDistance) /
                   Header->StereoScreenDistance;
            break;
        case 4:
            xadd = -0.5 * eyedist * (Header->StereoScreenDistance - Header->StereoMinDistance) /
                   Header->StereoScreenDistance;
            break;
        default:
            break;
    }
    mCopyAddVecWeight(midPos, headerMid, v, xadd);
}

struct TileSize {
    int width;
    int height;
};

static TileSize GetTileSize(const TMandHeader10* Header) {
    if (Header->TilingOptions == 0) {
        return {Header->Width, Header->Height};
    }
    int crop = std::max(1, (Header->TilingOptions >> 28) & 3);
    int countX = Header->TilingOptions & 0x7F;
    int countY = (Header->TilingOptions >> 7) & 0x7F;
    TileSize result{
        Header->Width / std::max(1, countX) + 2 * crop,
        Header->Height / std::max(1, countY) + 2 * crop
    };
    result.width = std::max(1, std::min(30000, result.width));
    result.height = std::max(1, std::min(30000, result.height));
    return result;
}

static void GetTilingInfosFromHeader(const TMandHeader10* Header, TRect& tileRect, int& crop) {
    int i = Header->TilingOptions;
    crop = std::max(1, (i >> 28) & 3);
    int countX = i & 0x7F;
    int countY = (i >> 7) & 0x7F;
    int posX = (i >> 14) & 0x7F;
    int posY = (i >> 21) & 0x7F;
    int sizeX = Header->Width / std::max(1, countX);
    int sizeY = Header->Height / std::max(1, countY);
    tileRect.Left = posX * sizeX - crop;
    tileRect.Top = posY * sizeY - crop;
    tileRect.Right = (posX + 1) * sizeX + crop - 1;
    tileRect.Bottom = (posY + 1) * sizeY + crop - 1;
}

// Port of DivUtils.pas GetPaintTileSizes
void GetPaintTileSizes(const TMandHeader10* Header,
                       int& wid, int& hei, int& Xplus, int& Yplus) {
    if (Header->TilingOptions == 0) {
        Xplus = 0;
        Yplus = 0;
        wid = Header->Width;
        hei = Header->Height;
    } else {
        int i = Header->TilingOptions;
        int Crop = std::max(1, (i >> 28) & 3);
        int countX = std::max(1, i & 0x7F);
        int countY = std::max(1, (i >> 7) & 0x7F);
        int posX = (i >> 14) & 0x7F;
        int posY = (i >> 21) & 0x7F;
        int sizeX = Header->Width / countX;
        int sizeY = Header->Height / countY;
        Xplus = posX * sizeX - Crop;
        Yplus = posY * sizeY - Crop;
        wid = sizeX + 2 * Crop;
        hei = sizeY + 2 * Crop;
    }
}

int GetTilingCrop(const TMandHeader10* Header) {
    if (Header->TilingOptions == 0) return 0;
    return (Header->TilingOptions >> 28) & 3;
}

static int GetDiffMapNr(const TLightingParas9* lightPars) {
    return static_cast<int>(lightPars->bColorMap |
           (static_cast<int>(lightPars->Lights[1].AdditionalByteEx) << 8));
}

static int GetFormulaDEoption(const THAformula& formula) {
    if (IsInternalFormula(formula.iFnr)) {
        return GetDEoptionForFormula(formula.iFnr);
    }
    // External formula (iFnr >= 20): look up DEoption from registry by CustomFname
    const char* name = reinterpret_cast<const char*>(formula.CustomFname);
    const ExternalFormulaInfo* extInfo = FindExternalFormula(name);
    if (extInfo) {
        return extInfo->DEoption;
    }
    return -1;
}

static double GetInternalFormulaDEscale(const THAformula& formula) {
    static const double ds[10] = {1.0, 1.0, 1.0, 1.0, 0.2, 0.2, 0.5, 0.2, 1.0, 1.0};
    if (!IsInternalFormula(formula.iFnr)) return 1.0;
    return ds[formula.iFnr];
}

static double GetInternalFormulaADEscale(const THAformula& formula) {
    (void)formula;
    return 1.0;
}

static double GetInternalFormulaSIpow(const THAformula& formula) {
    if (!IsInternalFormula(formula.iFnr)) return 2.0;
    if (formula.iFnr == 0) {
        const double power = formula.dOptionValue[0]; // packed file field
        return std::min(8.0, std::max(2.0, power));
    }
    if (formula.iFnr == 1 || formula.iFnr == 9) {
        return NotZero(formula.dOptionValue[0]);
    }
    return 2.0;
}

// ============================================================================
// Formula constant buffer system
// ============================================================================

// Pascal Round(Double) returns Int64 in the caller's floating-point mode.
static int64_t RoundFormulaInt64(double value) {
#if defined(_MSC_VER) && defined(_M_X64)
    return FormulaIntegerOption_x87(&value);
#else
    const double rounded = std::nearbyint(value);
    if (!std::isfinite(rounded) || rounded >= 0x1p63 || rounded < -0x1p63)
        return INT64_MIN; // x87's masked invalid-conversion result
    return static_cast<int64_t>(rounded);
#endif
}

// INTEGER/2INTEGER then store the low 32 bits. FOLDING clamps Int64 first.
static int32_t RoundFormulaInteger(double value) {
    const uint32_t bits = static_cast<uint32_t>(RoundFormulaInt64(value));
    int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

static int RoundFormulaPower(double value) {
    return static_cast<int>(std::clamp<int64_t>(RoundFormulaInt64(value), 2, 8));
}

// Initialize the PAligned16 constants at positive offsets from pConstPointer
static void InitPAligned16Constants(uint8_t* base) {
    auto setD = [](uint8_t* p, int offset, double v) {
        std::memcpy(p + offset, &v, sizeof(double));
    };
    auto setI64 = [](uint8_t* p, int offset, int64_t v) {
        std::memcpy(p + offset, &v, sizeof(int64_t));
    };

    setI64(base,   0, 0x7FFFFFFFFFFFFFFF);  // abs mask
    setI64(base,   8, 0x7FFFFFFFFFFFFFFF);
    setD(base,    16, -2.0);
    setD(base,    24, 1e-100);
    setD(base,    32, 1.0);          // fixedR^2 for cube
    setD(base,    40, 1.0);
    setD(base,    48, -1.0);
    setD(base,    56, -1.0);
    setD(base,    64, 2.0);
    setD(base,    72, 2.0);
    setI64(base,  80, (int64_t)0x8000000000000000LL);
    setI64(base,  88, (int64_t)0x8000000000000000LL);
    setD(base,    96, -1.0);
    setD(base,   104, 2.0);
    setD(base,   112, 0.5);
    setD(base,   120, 3.0);
    setD(base,   128, 4.0);
    setD(base,   136, 5.0);
    setD(base,   144, 6.0);
    setD(base,   152, 7.0);
    setD(base,   160, 8.0);
    setD(base,   168, 10.0);
    setD(base,   176, 15.0);
    setD(base,   184, 21.0);
    setD(base,   192, 28.0);
    setD(base,   200, 35.0);
    setD(base,   208, 70.0);
}

// Build the PVar constant buffer for a formula slot.
// This mirrors the Pascal FillCustomVBufWithVars procedure.
void BuildFormulaConstBuffer(int slot, const THeaderCustomAddon* pHCA,
                             int formulaIndex) {
    if (slot < 0 || slot >= 6) return;

    uint8_t* buf = GetFormulaConstBuffers().buf[slot];
    std::memset(buf, 0, 1024);

    // pConstPointer16 sits at offset 512 in our buffer
    // This gives 512 bytes of negative offset space and 512 bytes positive
    uint8_t* pConst = buf + 512;
    GetFormulaConstBuffers().pConstPointer[slot] = pConst;

    // Copy PAligned16 constants to positive offsets
    InitPAligned16Constants(pConst);

    // Store 0.5 at offset -8 (always present)
    double half = 0.5;
    std::memcpy(pConst - 8, &half, sizeof(double));

    const THAformula& formula = pHCA->Formulas[formulaIndex];
    const ExternalFormulaInfo* extInfo = nullptr;
    if (!IsInternalFormula(formula.iFnr)) {
        const char* name = reinterpret_cast<const char*>(formula.CustomFname);
        extInfo = FindExternalFormula(name);
        if (extInfo && extInfo->constantCount > 0 && extInfo->constants) {
            for (int constIdx = 0; constIdx < extInfo->constantCount; ++constIdx) {
                const FormulaConstEntry& entry = extInfo->constants[constIdx];
                std::memcpy(pConst + constIdx * 8, &entry.rawBits, sizeof(entry.rawBits));
            }
        }
    }
    const uint8_t* optValsBytes = reinterpret_cast<const uint8_t*>(formula.dOptionValue);
    const uint8_t* optTypes = formula.byOptionType;
    int optCount = formula.iOptionCount;

    // p walks backwards from pConst - 8 (the 0.5 is already placed)
    // In Pascal: p starts at pConstPointer16, Dec(p, 2) puts it at -8, stores 0.5
    // Then it processes options, each consuming negative space
    uint8_t* p = pConst - 8;  // after the 0.5

    auto readPackedOptionDouble = [&](int idx) {
        double value = 0.0;
        std::memcpy(&value, optValsBytes + idx * sizeof(double), sizeof(double));
        return value;
    };
    double optVals[16] = {};
    for (int idx = 0; idx < std::min(16, optCount); ++idx) {
        optVals[idx] = readPackedOptionDouble(idx);
    }

    // Pascal's negative parameter region is limited to 256 bytes. Its FOLDING
    // entry reserves 40 bytes but advances by 36 (a 32-bit function pointer).
    // Keep that logical cursor separate from our native pointer-sized storage.
    constexpr int optionBytes[] = {8,4,4,16,8,72,36,16,40,8,0,32,64,8,
                                   16,8,8,16,36,8,8,8,16};
    int pascalBytes = 8;
    int i = 0;
    while (i < std::min(16, optCount)) {
        const unsigned type = optTypes[i];
        if (type >= std::size(optionBytes))
            throw std::invalid_argument("Unsupported formula option type");
        if (pascalBytes + optionBytes[type] > 256) {
            // The original skips only this declaration. A skipped compound
            // does not consume its following saved options or any storage.
            ++i;
            continue;
        }
        const int inputs = type == 12 ? 6 : type == 18 ? 4 :
                           (type == 5 || type == 6) ? 3 : type == 19 ? 2 : 1;
        if (i + inputs > 16 || (type == 7 && i == 0))
            throw std::invalid_argument("Formula option reads outside its 16-value record");
        pascalBytes += type == 8 ? 36 : optionBytes[type];
        switch (optTypes[i]) {
            case 0: { // DOUBLE
                p -= 8;
                double val = readPackedOptionDouble(i);
                std::memcpy(p, &val, sizeof(double));
                break;
            }
            case 1: { // SINGLE
                p -= 4;
                float val = static_cast<float>(readPackedOptionDouble(i));
                std::memcpy(p, &val, sizeof(float));
                break;
            }
            case 2: { // INTEGER
                p -= 4;
                int32_t val = RoundFormulaInteger(readPackedOptionDouble(i));
                std::memcpy(p, &val, sizeof(int32_t));
                break;
            }
            case 3: { // DOUBLEANGLE (sin, cos pair as doubles)
                p -= 8;
                double angle = readPackedOptionDouble(i);
                const double radians = angle * Pid180;
                double sinVal, cosVal;
                if (NativePrecisionEnabled()) {
                    sinVal=std::sin(radians);cosVal=std::cos(radians);
                } else {
#if defined(_MSC_VER) && defined(_M_X64)
                // FSIN/FCOS also preserve the original out-of-range behavior.
                // Single stores round directly from x87, without a Double spill.
                FormulaAngleDouble_x87(&sinVal, &cosVal, &radians);
#elif defined(MB3D_SOFTWARE_EXT80)
                const numeric::ExtendedScope scope;
                const auto values=numeric::extendedSinCos(numeric::Extended80(radians));
                // Separate FSIN and FCOS each retain the operand on C2.
                sinVal=std::isfinite(radians) && std::abs(radians)>=0x1p63 ? radians : static_cast<double>(values.sin);
                cosVal=static_cast<double>(values.cos);
#else
                sinVal = static_cast<double>(std::sin(radians));
                cosVal = static_cast<double>(std::cos(radians));
#endif
                }
                std::memcpy(p, &sinVal, sizeof(double));
                p -= 8;
                std::memcpy(p, &cosVal, sizeof(double));
                break;
            }
            case 4: { // SINGLEANGLE (sin, cos pair as singles)
                p -= 4;
                double angle = readPackedOptionDouble(i);
                const double radians = angle * Pid180;
                float sinVal, cosVal;
                if (NativePrecisionEnabled()) {
                    sinVal=static_cast<float>(std::sin(radians));
                    cosVal=static_cast<float>(std::cos(radians));
                } else {
#if defined(_MSC_VER) && defined(_M_X64)
                // FSIN/FCOS also preserve the original out-of-range behavior.
                // Single stores round directly from x87, without a Double spill.
                FormulaAngleSingle_x87(&sinVal, &cosVal, &radians);
#elif defined(MB3D_SOFTWARE_EXT80)
                const numeric::ExtendedScope scope;
                const auto values=numeric::extendedSinCos(numeric::Extended80(radians));
                sinVal=std::isfinite(radians) && std::abs(radians)>=0x1p63 ? static_cast<float>(radians) : static_cast<float>(values.sin);
                cosVal=static_cast<float>(values.cos);
#else
                sinVal = static_cast<float>(std::sin(radians));
                cosVal = static_cast<float>(std::cos(radians));
#endif
                }
                std::memcpy(p, &sinVal, sizeof(float));
                p -= 4;
                std::memcpy(p, &cosVal, sizeof(float));
                break;
            }
            case 5: { // 3DOUBLEANGLES: 3x3 rotation matrix as doubles
                TMatrix3 m;
                BuildRotMatrix(readPackedOptionDouble(i) * Pid180,
                               readPackedOptionDouble(i + 1) * Pid180,
                               readPackedOptionDouble(i + 2) * Pid180, m);
                double* pd = &m[0][0];
                for (int j = 0; j < 9; ++j) {
                    p -= 8;
                    std::memcpy(p, pd, sizeof(double));
                    ++pd;
                }
                i += 2;
                break;
            }
            case 6: { // 3SINGLEANGLES: 3x3 rotation matrix as singles
                TMatrix3 m;
                BuildRotMatrix(readPackedOptionDouble(i) * Pid180,
                               readPackedOptionDouble(i + 1) * Pid180,
                               readPackedOptionDouble(i + 2) * Pid180, m);
                double* pd = &m[0][0];
                for (int j = 0; j < 9; ++j) {
                    p -= 4;
                    float val = static_cast<float>(*pd);
                    std::memcpy(p, &val, sizeof(float));
                    ++pd;
                }
                i += 2;
                break;
            }
            case 7: { // BOXSCALE: Scale/Sqr(MinR), Sqr(MinR)
                double minR = std::max(1e-40, optVals[i]);
                double minRsqr = minR * minR;
                double prevScale = (i > 0) ? optVals[i - 1] : 2.0;
                p -= 8;
                double scaleDivMinRsqr = prevScale / minRsqr;
                std::memcpy(p, &scaleDivMinRsqr, sizeof(double));
                p -= 8;
                std::memcpy(p, &minRsqr, sizeof(double));
                break;
            }
            case 8: { // FOLDING: R, 2R, -R, -2R + function pointer
                double fold = optVals[i];
                p -= 8;
                std::memcpy(p, &fold, sizeof(double));
                double fold2 = 2.0 * fold;
                p -= 8;
                std::memcpy(p, &fold2, sizeof(double));
                double negFold = -fold;
                p -= 8;
                std::memcpy(p, &negFold, sizeof(double));
                double negFold2 = -2.0 * fold;
                p -= 8;
                std::memcpy(p, &negFold2, sizeof(double));
                // Store function pointer for the sub-formula
                p -= sizeof(void*);
                if (i > 1) {
                    int power = RoundFormulaPower(optVals[i - 2]);
                    ThybridIteration func = fHIntFunctions[power - 2];
                    std::memcpy(p, &func, sizeof(func));
                }
                break;
            }
            case 9: { // DSQUARE: store squared value
                p -= 8;
                double val = optVals[i] * optVals[i];
                std::memcpy(p, &val, sizeof(double));
                break;
            }
            case 10: { // NOVARIABLE: no storage consumed
                break;
            }
            case 11: { // FOLDING16: R, R, -R, -R (for SSE2-style box fold)
                double fold = optVals[i];
                p -= 8;
                std::memcpy(p, &fold, sizeof(double));
                p -= 8;
                std::memcpy(p, &fold, sizeof(double));
                double negFold = -fold;
                p -= 8;
                std::memcpy(p, &negFold, sizeof(double));
                p -= 8;
                std::memcpy(p, &negFold, sizeof(double));
                break;
            }
            case 12: { // 6SINGLEANGLES: 4d rotation matrix as singles
                double angles[6];
                for (int j = 0; j < 6; ++j) {
                    angles[j] = optVals[i + j] * Pid180;
                }
                TSMatrix4 ms4;
                BuildRotMatrix4d(angles, ms4);
                float* ps = &ms4[0][0];
                for (int j = 0; j < 16; ++j) {
                    p -= 4;
                    std::memcpy(p, ps, sizeof(float));
                    ++ps;
                }
                i += 5;
                break;
            }
            case 13: { // DRECIPRO: store reciprocal
                p -= 8;
                double val = 1.0 / MaxAbsCD(1e-40, optVals[i]);
                std::memcpy(p, &val, sizeof(double));
                break;
            }
            case 14: { // 2DOUBLES: store value twice
                p -= 8;
                double val = optVals[i];
                std::memcpy(p, &val, sizeof(double));
                p -= 8;
                std::memcpy(p, &val, sizeof(double));
                break;
            }
            case 15: { // DSQRRECI: 1/max(1e-40, val^2)
                p -= 8;
                double val = 1.0 / MaxCD(1e-40, optVals[i] * optVals[i]);
                std::memcpy(p, &val, sizeof(double));
                break;
            }
            case 16:
            case 17: { // 2SINGLES / 4SINGLES
                int l = (optTypes[i] == 17) ? 4 : 2;
                while (l-- > 0) {
                    p -= 4;
                    float val = static_cast<float>(optVals[i]);
                    std::memcpy(p, &val, sizeof(float));
                }
                break;
            }
            case 18: { // 3SCALESANGLES: scaled rotation matrix as singles
                TMatrix3 m;
                BuildRotMatrix(optVals[i + 1] * Pid180, optVals[i + 2] * Pid180,
                               optVals[i + 3] * Pid180, m);
                ScaleMatrix(optVals[i], m);
                double* pd = &m[0][0];
                for (int j = 0; j < 9; ++j) {
                    p -= 4;
                    float val = static_cast<float>(*pd);
                    std::memcpy(p, &val, sizeof(float));
                    ++pd;
                }
                i += 3;
                break;
            }
            case 19: { // SCALESROT: scaled 2d rotation as singles
                p -= 4;
                float sinVal = static_cast<float>(std::sin(optVals[i + 1] * Pid180) * optVals[i]);
                std::memcpy(p, &sinVal, sizeof(float));
                p -= 4;
                float cosVal = static_cast<float>(std::cos(optVals[i + 1] * Pid180) * optVals[i]);
                std::memcpy(p, &cosVal, sizeof(float));
                i += 1;
                break;
            }
            case 20: { // 2INTEGER
                p -= 4;
                int32_t val = RoundFormulaInteger(optVals[i]);
                std::memcpy(p, &val, sizeof(int32_t));
                p -= 4;
                std::memcpy(p, &val, sizeof(int32_t));
                break;
            }
            case 21: { // SRECI2: single + reciprocal single
                p -= 4;
                float val = static_cast<float>(optVals[i]);
                std::memcpy(p, &val, sizeof(float));
                p -= 4;
                float rec = static_cast<float>(1.0 / MaxCS(1e-30f, static_cast<float>(optVals[i])));
                std::memcpy(p, &rec, sizeof(float));
                break;
            }
            case 22: { // DRECI2: double + reciprocal double
                p -= 8;
                double val = optVals[i];
                std::memcpy(p, &val, sizeof(double));
                p -= 8;
                double rec = 1.0 / MaxCD(1e-40, optVals[i]);
                std::memcpy(p, &rec, sizeof(double));
                break;
            }
            default: {
                // Unknown type -- store as double
                p -= 8;
                double val = optVals[i];
                std::memcpy(p, &val, sizeof(double));
                break;
            }
        }
        i++;
    }

    if (HasParityTraceContext()) {
        std::ostringstream payload;
        payload.setf(std::ios::fixed, std::ios::floatfield);
        payload << std::setprecision(12)
                << "slot=" << slot
                << " formulaIndex=" << formulaIndex
                << " iFnr=" << formula.iFnr
                << " optCount=" << optCount
                << " extConstCount=" << ((extInfo && extInfo->constants) ? extInfo->constantCount : 0);
        for (int traceIdx = 0; traceIdx < 16; ++traceIdx) {
            const int typeVal = (traceIdx < optCount) ? static_cast<int>(optTypes[traceIdx]) : 0;
            const double optVal = (traceIdx < optCount) ? optVals[traceIdx] : 0.0;
            payload << " type" << traceIdx << "=" << typeVal
                    << " opt" << traceIdx << "=" << optVal;
        }

        double pvarM2 = 0.0, pvarM1 = 0.0, pvar0 = 0.0, pvar1 = 0.0;
        std::memcpy(&pvarM2, pConst - 16, sizeof(double));
        std::memcpy(&pvarM1, pConst - 8, sizeof(double));
        std::memcpy(&pvar0, pConst, sizeof(double));
        std::memcpy(&pvar1, pConst + 8, sizeof(double));
        double power = std::numeric_limits<double>::quiet_NaN();
        double dZmul = std::numeric_limits<double>::quiet_NaN();
        if (optCount > 0) power = optVals[0];
        if (pConst != nullptr) dZmul = pvarM2;
        payload << " pvar=" << ParityPointerState(pConst)
                << " pvar_m2=" << pvarM2
                << " pvar_m1=" << pvarM1
                << " pvar_0=" << pvar0
                << " pvar_1=" << pvar1
                << " power=" << power
                << " dZmul=" << dZmul;
        AppendParityTrace("const_buffer", payload.str());
    }
}

// ============================================================================
// CalcPPZvals -- from DivUtils.pas
// ============================================================================

void CalcPPZvals(TMandHeader10& Header, double& Zcorr, double& ZcMul, double& ZZstmitDif) {
    CalcStepWidth(&Header);
    double fovRad = MaxCD(Header.dFOVy * Pid180, 1.0);
    Zcorr = std::sin(fovRad / Header.Height);
    double ze = (Header.dZend - Header.dZstart) * Zcorr / Header.dStepWidth;
    ZcMul = 8388352.0 / (std::sqrt(ze + 1.0) - 0.999999999);
    ZZstmitDif = Header.dZstart - Header.dZmid;
}

// ============================================================================
// GetDEstopFactor -- from HeaderTrafos.pas
// ============================================================================

double GetDEstopFactor(TMandHeader10* Header) {
    CalcStepWidth(Header);
    double ze = MaxCD(1e-16, (Header->dZend - Header->dZstart) / Header->dStepWidth);
    double x1;
    if (Header->bPlanarOptic == 2) {
        x1 = s001 * Header->Height / (std::sin(s001) * Pi);
    } else {
        x1 = s001 * Header->Height /
             (std::sin(s001) * MaxCD(d1d65535, Header->dFOVy * Pid180));
    }
    double x2 = Header->dStepWidth * (x1 + ze) / x1;
    return (x2 - Header->dStepWidth) / (Header->dStepWidth * MaxCD(d1d6, ze));
}

// ============================================================================
// NormVGrads -- from HeaderTrafos.pas
// ============================================================================

void NormVGrads(TMandHeader10* Header) {
    CalcStepWidth(Header);
    TMatrix3 result;
    TMatrix3 source{};
    std::memcpy(&source, static_cast<const void*>(Header->hVGrads), sizeof(source));
    NormaliseMatrixTo(result, Header->dStepWidth, source);
    std::memcpy(Header->hVGrads, result, sizeof(TMatrix3));
}

// ============================================================================
// CalcVGradsFromHeader8rots -- from HeaderTrafos.pas
// ============================================================================

void CalcVGradsFromHeader8rots(TMandHeader10* Header) {
    double Esin1, Ecos1, Esin2, Ecos2, Esin3, Ecos3;
    SinCosD(Header->dXWrot, Esin1, Ecos1);
    SinCosD(Header->dYWrot, Esin2, Ecos2);
    SinCosD(Header->dZWrot, Esin3, Ecos3);

    double step = 2.1345 / (Header->dZoom * Header->Width);
    if (Header->bCalc3D > 0) {
        const double fov = Header->dFOVy;
        double fov_clamped = std::max(-1.25, std::min(1.25, fov));
        step = step * (s001 + std::fabs(std::cos(fov_clamped * s05)));
    }

    for (int i = 0; i < 3; i++) {
        double x1 = 0, y1 = 0, z1 = 0;
        switch (i) {
            case 0: x1 = step; break;
            case 1: y1 = step; break;
            case 2: z1 = step; break;
        }
        // Rotate X axis
        double x2 = x1;
        double y2 = Ecos1 * y1 + Esin1 * z1;
        double z2 = Ecos1 * z1 - Esin1 * y1;
        // Rotate Y axis
        x1 = Ecos2 * x2 + Esin2 * z2;
        y1 = y2;
        z1 = Ecos2 * z2 - Esin2 * x2;
        // Rotate Z axis and store
        Header->hVGrads[i][0] = Ecos3 * x1 + Esin3 * y1;
        Header->hVGrads[i][1] = Ecos3 * y1 - Esin3 * x1;
        Header->hVGrads[i][2] = z1;
    }
}

// ============================================================================
// CalcSCstartAndSCmul -- from HeaderTrafos.pas
// ============================================================================

void CalcSCstartAndSCmul(TMandHeader10* Header, float& sCStart, float& sCmul,
                          LongBool bIsInside) {
    auto& Light = Header->Light;
    if (bIsInside) {
        double dTmp = (Light.TBoptions & 0x7F);
        dTmp = dTmp * dTmp * dTmp * dTmp; // Sqr(Sqr(...))
        sCStart = static_cast<float>(dTmp * 0.000158025 + 32768.0);
        double d2 = ((Light.TBoptions >> 7) & 0x7F);
        d2 = d2 * d2 * d2 * d2;
        sCmul = static_cast<float>((d2 - dTmp) * 0.5 * 0.00031605);
        if (std::fabs(sCmul) > 1e-10) sCmul = 1.0f / sCmul;
    } else {
        double p9 = (Light.TBpos[6] + 30) * 0.01111111111111111; // TBpos[9] in Pascal (0-based: 6)
        double p10 = (Light.TBpos[7] + 30) * 0.01111111111111111; // TBpos[10] in Pascal (0-based: 7)
        sCStart = static_cast<float>(p9 * p9 * 32767.0 - 10900.0);
        sCmul = static_cast<float>(p10 * p10 * 32767.0 - 10900.0 - sCStart);

        if ((Light.TBoptions & 0x10000) > 0) {
            double dTmp = sCStart + sCmul * (static_cast<int>(Light.FineColAdj2) - 30) * 0.0166666666666666;
            sCStart = static_cast<float>(sCStart + sCmul * (static_cast<int>(Light.FineColAdj1) - 30) * 0.0166666666666666);
            sCmul = static_cast<float>(dTmp - sCStart);
        }
        if (std::fabs(sCmul) > 0.0001f) {
            sCmul = 2.0f / sCmul;
        } else {
            if (sCmul < 0) sCmul = -2000.0f; else sCmul = 2000.0f;
        }
    }
}

// ============================================================================
// CalcHSVecsFromLights -- from HeaderTrafos.pas
// ============================================================================

void CalcHSVecsFromLights(TPLightVals Lvals, PMCTparameter MCTpars) {
    TMatrix3 M;
    TMatrix3 sourceVGrads{};
    std::memcpy(&sourceVGrads, MCTpars->Vgrads, sizeof(sourceVGrads));
    NormaliseMatrixTo(M, 1.0, sourceVGrads);
    for (int i = 0; i < 6; i++) {
        if ((Lvals->iLightPos[i] & 1) == 0) {
            // Global light: normalize to -StepWidth, then rotate
            TSVec ln{};
            std::memcpy(ln, Lvals->PLValigned->LN[i], sizeof(ln));
            float len = LengthOfSVec(ln);
            if (len > 1e-20f) {
                float scale = -static_cast<float>(MCTpars->StepWidth) / len;
                MCTpars->HSvecs[i][0] = ln[0] * scale;
                MCTpars->HSvecs[i][1] = ln[1] * scale;
                MCTpars->HSvecs[i][2] = ln[2] * scale;
            } else {
                MCTpars->HSvecs[i][0] = 0;
                MCTpars->HSvecs[i][1] = 0;
                MCTpars->HSvecs[i][2] = -MCTpars->StepWidth;
            }
            TVec3D hsvec{};
            std::memcpy(hsvec, MCTpars->HSvecs[i], sizeof(hsvec));
            RotateVectorReverse(hsvec, M);
            std::memcpy(MCTpars->HSvecs[i], hsvec, sizeof(hsvec));
        } else {
            // Position light: store absolute position relative to midpoint
            TSVec ln{};
            std::memcpy(ln, Lvals->PLValigned->LN[i], sizeof(ln));
            MCTpars->HSvecs[i][0] = ln[0] + MCTpars->Xmit;
            MCTpars->HSvecs[i][1] = ln[1] + MCTpars->Ymit;
            MCTpars->HSvecs[i][2] = ln[2] + MCTpars->Zmit;
        }
        if (HasParityTraceContext()) {
            std::ostringstream payload;
            payload.setf(std::ios::fixed, std::ios::floatfield);
            payload << std::setprecision(12)
                    << "light=" << i
                    << " iLightPos=" << Lvals->iLightPos[i]
                    << " LNx=" << Lvals->PLValigned->LN[i][0]
                    << " LNy=" << Lvals->PLValigned->LN[i][1]
                    << " LNz=" << Lvals->PLValigned->LN[i][2]
                    << " HSVecX=" << MCTpars->HSvecs[i][0]
                    << " HSVecY=" << MCTpars->HSvecs[i][1]
                    << " HSVecZ=" << MCTpars->HSvecs[i][2]
                    << " StepWidth=" << MCTpars->StepWidth;
            AppendParityTrace("shadow_vec_setup", payload.str());
        }
    }
}

// ============================================================================
// CalcPosLightsRelPos -- from HeaderTrafos.pas
// ============================================================================

void CalcPosLightsRelPos(TMandHeader10* Header, TPLightVals Lvals) {
    // Ported from HeaderTrafos.pas procedure CalcPosLightsRelPos
    TVec3D midPos{};
    std::memcpy(midPos, Lvals->lvMidPos, sizeof(midPos));
    StereoChange(Header, Header->bStereoMode, midPos, &Header->hVGrads);
    std::memcpy(Lvals->lvMidPos, midPos, sizeof(midPos));

    for (int i = 0; i < 6; i++) {
        if ((Header->Light.Lights[i].Loption & 4) != 0) { // positional light
            // DVecFromLightPos: decode light position from Double7B
            TVec3D lightPos;
            lightPos[0] = D7BtoDouble(Header->Light.Lights[i].LXpos);
            lightPos[1] = D7BtoDouble(Header->Light.Lights[i].LYpos);
            lightPos[2] = D7BtoDouble(Header->Light.Lights[i].LZpos);

            // Compute position relative to camera midpoint
            TVec3D relPos;
            SubtractVec3D(relPos, lightPos, Lvals->lvMidPos);

            // Store as SVec in PLValigned->LN[SortTab[i]]
            int si = Lvals->SortTab[i];
            Lvals->PLValigned->LN[si][0] = static_cast<float>(relPos[0]);
            Lvals->PLValigned->LN[si][1] = static_cast<float>(relPos[1]);
            Lvals->PLValigned->LN[si][2] = static_cast<float>(relPos[2]);
            Lvals->PLValigned->LN[si][3] = 0.0f;
        }
    }
}

// ============================================================================
// IniSMatrix4 / MultiplySMatrix4 -- 4×4 single-precision matrix helpers
// ============================================================================

static void IniSMatrix4(TSMatrix4& m) {
    std::memset(m, 0, sizeof(TSMatrix4));
    for (int i = 0; i < 4; ++i) {
        m[i][i] = 1.0f;
    }
}

static void MultiplySMatrix4(TSMatrix4& m1, const TSMatrix4& m2) {
    TSMatrix4 tmp;
    std::memcpy(tmp, m1, sizeof(TSMatrix4));
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            m1[row][col] = tmp[row][0] * m2[0][col] +
                           tmp[row][1] * m2[1][col] +
                           tmp[row][2] * m2[2][col] +
                           tmp[row][3] * m2[3][col];
        }
    }
}

// ============================================================================
// BuildSMatrix4 -- build 4D rotation matrix from 3 angles
// ============================================================================

static void BuildSMatrix4(double xa, double ya, double za, TSMatrix4& m) {
    // Pascal: Math3D.pas lines 2573-2599
    // Three sequential 4D W-coupling plane rotations: X-W, Y-W, Z-W
    double sinX, cosX, sinY, cosY, sinZ, cosZ;
    SinCosD(xa, sinX, cosX);
    SinCosD(ya, sinY, cosY);
    SinCosD(za, sinZ, cosZ);

    TSMatrix4 Smatrix4, SM4;

    // X-W plane rotation into Smatrix4
    IniSMatrix4(Smatrix4);
    Smatrix4[0][0] = static_cast<float>(cosX);
    Smatrix4[3][3] = static_cast<float>(cosX);
    Smatrix4[0][3] = static_cast<float>(-sinX);
    Smatrix4[3][0] = static_cast<float>(sinX);

    // Y-W plane rotation into SM4
    IniSMatrix4(SM4);
    SM4[1][1] = static_cast<float>(cosY);
    SM4[3][3] = static_cast<float>(cosY);
    SM4[1][3] = static_cast<float>(-sinY);
    SM4[3][1] = static_cast<float>(sinY);

    // Pascal: Multiply2SMatrix4(@SM4, @Smatrix4) => SM4 := SM4 * Smatrix4
    MultiplySMatrix4(SM4, Smatrix4);

    // Z-W plane rotation into Smatrix4 (reinitialize)
    IniSMatrix4(Smatrix4);
    Smatrix4[2][2] = static_cast<float>(cosZ);
    Smatrix4[3][3] = static_cast<float>(cosZ);
    Smatrix4[2][3] = static_cast<float>(-sinZ);
    Smatrix4[3][2] = static_cast<float>(sinZ);

    // Pascal: Multiply2SMatrix4(@Smatrix4, @SM4) => Smatrix4 := Smatrix4 * SM4
    MultiplySMatrix4(Smatrix4, SM4);

    // Final result
    std::memcpy(&m, &Smatrix4, sizeof(TSMatrix4));
}

static void BuildRotMatrix4d(const double* angles, TSMatrix4& ms4) {
    static constexpr int i1[6] = {1, 0, 0, 0, 1, 2};
    static constexpr int i2[6] = {2, 2, 1, 3, 3, 3};

    IniSMatrix4(ms4);
    for (int i = 0; i < 6; ++i) {
        double s1;
        double c1;
        SinCosD(angles[i], s1, c1);
        TSMatrix4 sm4;
        IniSMatrix4(sm4);
        sm4[i1[i]][i1[i]] = static_cast<float>(c1);
        sm4[i2[i]][i2[i]] = static_cast<float>(c1);
        sm4[i1[i]][i2[i]] = static_cast<float>(-s1);
        sm4[i2[i]][i1[i]] = static_cast<float>(s1);
        MultiplySMatrix4(sm4, ms4);
        std::memcpy(ms4, sm4, sizeof(TSMatrix4));
    }
}

// ============================================================================
// CheckHybridOptions -- validate/fix hybrid option bytes
// ============================================================================

static void CheckHybridOptions(THeaderCustomAddon* PCFA) {
    int x;
    int end1;
    int repeat1;
    int start2;
    int end2;
    int repeat2;

    if ((PCFA->bOptions1 & 3) == 1) {
        // Interpolation hybrid has a fixed two-formula layout in Pascal.
        end1 = 0;
        repeat1 = 0;
        start2 = 1;
        end2 = 5;
        repeat2 = 1;
    } else {
        x = 5;
        while ((x > 0) && (PCFA->Formulas[x].iItCount == 0)) --x;
        start2 = std::max(1, std::min(x, static_cast<int>(PCFA->bHybOpt2 & 7)));
        // Direct callers retain Pascal CustomFormulas.CheckHybridOptions:
        // mode 3 ends the first group immediately before the second group.
        // File loading separately reproduces FormulaGUI/MakeHeader readback,
        // where the absent fourth tab becomes mode 0 before this function.
        if ((PCFA->bOptions1 & 3) == 0) {
            end1 = x;
        } else {
            end1 = start2 - 1;
        }
        end2 = std::max(start2, x);
        repeat2 = std::max(start2, std::min(end2, static_cast<int>((PCFA->bHybOpt2 >> 8) & 7)));
        x = end1;
        while ((x > 0) && (PCFA->Formulas[x].iItCount <= 0)) --x;
        repeat1 = std::min(x, static_cast<int>(PCFA->bHybOpt1 >> 4));
        if ((PCFA->bOptions1 & 3) == 2) {
            x = 5;
            while ((x > start2) && (PCFA->Formulas[x].iItCount <= 0)) --x;
            repeat2 = std::min(x, repeat2);
        }
    }

    PCFA->bHybOpt1 = static_cast<uint8_t>(end1 | (repeat1 << 4));
    PCFA->bHybOpt2 = static_cast<uint16_t>(start2 | (end2 << 4) | (repeat2 << 8));
}

static LongBool IsSingleF(const THeaderCustomAddon* HA, int& firstIndex) {
    int count = 0;
    firstIndex = -1;
    for (int j = 0; j <= 5; ++j) {
        if (HA->Formulas[j].iItCount > 0) {
            ++count;
            if (firstIndex == -1) {
                firstIndex = j;
            }
        }
    }
    LongBool result = (count < 2);
    if (result && (HA->bOptions1 == 2)) {
        firstIndex = -1;
    }
    return result;
}

static void CheckDEoption(const int startNr, const int endNr, const THeaderCustomAddon* pHCA,
                          int& DEoption) {
    int difsShapes = 0;
    DEoption = -1;
    for (int i = startNr; i <= endNr; ++i) {
        if (pHCA->Formulas[i].iItCount <= 0) continue;
        int de = GetFormulaDEoption(pHCA->Formulas[i]);
        if (de < 0 || de > 20 || DEoption == de) continue;
        if (de == 20) ++difsShapes;
        if (((DEoption >= 0 && DEoption <= 19) && de > 19) ||
            ((DEoption > 19) && (de >= 0 && de <= 19))) {
            DEoption = -1;
            break;
        }
        switch (DEoption) {
            case -1: DEoption = de; break;
            case 2:  if (!(de == 2 || de == 5 || de == 6 || de == 11)) DEoption = 0; break;
            case 4:  if (!(de == 5 || de == 6)) DEoption = 0; break;
            case 5:
                if (de == 4) DEoption = 4;
                else if (de == 2 || de == 11) DEoption = 2;
                else if (de != 6) DEoption = 0;
                break;
            case 6:
                if (de == 2 || de == 4 || de == 5 || de == 11) DEoption = de;
                else DEoption = 0;
                break;
            case 11:
                if (de == 2 || de == 5) DEoption = 2;
                else if (de != 6) DEoption = 0;
                break;
            default:
                break;
        }
    }
    if (DEoption > 19) {
        DEoption = (difsShapes == 0) ? -1 : 20;
    }
}

static void CheckFormulaOptions(const int startNr, const int endNr, THeaderCustomAddon* pHCA,
                                int& DEoption, TMandFunctionDE& mandFunctionDE,
                                TMandFunction& mandFunction, int32_t* nHybrid,
                                const LongBool bDisableADE) {
    int difsShapes = 0;
    int lastValidF = startNr;
    int lastF = startNr;
    DEoption = -1;

    for (int i = startNr; i <= endNr; ++i) {
        if (nHybrid[i] == 0) continue;
        lastF = i;
        int de = GetFormulaDEoption(pHCA->Formulas[i]);
        if (de < 0 || de > 20) {
            nHybrid[i] = -std::abs(nHybrid[i]);
        } else {
            lastValidF = i;
        }
        if (de < 0 || DEoption == de) continue;
        if (de == 20) ++difsShapes;
        if (((DEoption >= 0 && DEoption <= 19) && de > 19) ||
            ((DEoption > 19) && (de >= 0 && de <= 19))) {
            DEoption = -1;
            std::fprintf(stderr, "Error: mixing dIFS with non-dIFS formulas is not supported\n");
            break;
        }
        switch (DEoption) {
            case -1:
            case 21:
            case 22:
                DEoption = de;
                break;
            case 2:
                if (!(de == 2 || de == 5 || de == 6 || de == 11)) DEoption = 0;
                break;
            case 4:
                if (!(de == 5 || de == 6)) DEoption = 0;
                break;
            case 5:
                if (de == 4) DEoption = 4;
                else if (de == 2 || de == 11) DEoption = 2;
                else if (de != 6) DEoption = 0;
                break;
            case 6:
                if (de == 2 || de == 4 || de == 5 || de == 11) DEoption = de;
                else DEoption = 0;
                break;
            case 11:
                if (de == 2 || de == 5) DEoption = 2;
                else if (de != 6) DEoption = 0;
                break;
            default:
                break;
        }
        if (bDisableADE) {
            switch (DEoption) {
                case 2:
                case 11:
                    DEoption = 0;
                    break;
                case 5:
                case 6:
                    DEoption = 4;
                    break;
                default:
                    break;
            }
        }
    }

    if (startNr < (pHCA->bHybOpt2 & 7)) {
        if (lastValidF < (pHCA->bHybOpt1 >> 4)) {
            pHCA->bHybOpt1 = (pHCA->bHybOpt1 & 7) | (lastValidF << 4);
        }
        if ((pHCA->bOptions1 == 2) && (pHCA->bOptions3 == 5) && (DEoption < 0 || DEoption > 20)) {
            for (int i = startNr; i <= endNr; ++i) {
                if (nHybrid[i] == 0) continue;
                nHybrid[i] = std::abs(nHybrid[i]);
                DEoption = std::max(0, GetFormulaDEoption(pHCA->Formulas[i]));
                ++difsShapes;
                break;
            }
        }
    } else {
        if (lastValidF < ((pHCA->bHybOpt2 >> 8) & 7)) {
            pHCA->bHybOpt2 = (pHCA->bHybOpt2 & 0x77) | (lastValidF << 8);
        }
        pHCA->bHybOpt2 = (pHCA->bHybOpt2 & 0x707) | (lastF << 4);
    }

    if (DEoption > 19) {
        if (difsShapes == 0) {
            DEoption = -1;
            std::fprintf(stderr, "Error: no dIFS shape formula selected\n");
        } else {
            DEoption = 20;
        }
    }

    mandFunction = fIsMemberAlternating;
    mandFunctionDE = fIsMemberAlternatingDE;
    if (DEoption >= 4 && DEoption <= 6) {
        mandFunctionDE = doHybrid4DDEPas;
        mandFunction = fIsMemberAlternating4D;
    } else if (DEoption == 20) {
        mandFunctionDE = doHybridIFS3D;
    }
}

// ============================================================================
// GetMCTparasFromHeader -- THE KEY FUNCTION
// Converts TMandHeader10 + THeaderCustomAddon into TMCTparameter
// Ported from HeaderTrafos.pas (~480 lines)
// ============================================================================

// HeaderTrafos.pas: all operands and intermediate results are Single.
float CalcRayStepLimiter(int width, int height, float stepDiv, float limiter) {
    const float sum = stepDiv + 0.001f;
    const float halfSize = static_cast<float>(std::max(width, height)) * 0.5f;
    const float scaledSize = halfSize * std::sqrt(sum);
    return scaledSize * MaxCS(s001, limiter);
}

// Pascal accumulates absolute repeat counts in signed 32-bit Integers.
// There is no source clamp. Reject an unrepresentable sum explicitly instead
// of invoking C++ signed overflow or silently changing the scale calculation.
static int AddHybridRepeatWeight(int sum, int32_t count, int slot) {
    const int64_t magnitude = count < 0 ? -static_cast<int64_t>(count) : count;
    const int64_t next = static_cast<int64_t>(sum) + magnitude;
    if (next > std::numeric_limits<int32_t>::max()) {
        throw std::invalid_argument("Unsupported ordinary hybrid repeat-weight sum at slot " +
            std::to_string(slot) + ": " + std::to_string(next) +
            " exceeds signed 32-bit range");
    }
    return static_cast<int>(next);
}

TMCTparameter GetMCTparasFromHeader(TMandHeader10& Header,
                                     THeaderCustomAddon* pHCA,
                                     LongBool bIniCalcMaps) {
    TMCTparameter Result;
    std::memset(&Result, 0, sizeof(TMCTparameter));

    InitExternalFormulaRegistry();

    THeaderCustomAddon* PCFA = pHCA;
    if (!PCFA) {
        Result.bMCTisValid = 0;
        return Result;
    }

    if ((PCFA->bOptions1 & 3) != 1) {
        for (int slot = 0; slot < 6; ++slot) {
            if (PCFA->Formulas[slot].iItCount == std::numeric_limits<int32_t>::min()) {
                // Raw file counts use signed integers, unlike the runtime
                // sign-bit encoding. Abs(INT_MIN) cannot fit in 31 bits.
                // In interpolation these same bits mean float -0.0, not a count.
                throw std::invalid_argument("Unsupported ordinary hybrid repeat count at slot " +
                    std::to_string(slot) + ": INT_MIN magnitude exceeds 31 bits");
            }
        }
    }

    // Rendering uses maxIt + 1 as an outside-orbit sentinel. The second
    // limit is only consumed by DE combinations; otherwise it is replaced.
    if (Header.Iterations == std::numeric_limits<int32_t>::max() ||
        ((PCFA->bOptions1 & 3) == 2 &&
         Header.iMaxItsF2 == std::numeric_limits<int32_t>::max())) {
        throw std::invalid_argument("Unsupported ordinary maximum iteration limit: "
                                    "INT_MAX leaves no room for the outside-orbit sentinel");
    }

    CheckHybridOptions(PCFA);

    Result.StartFrom1  = 0;
    Result.wEndTo      = PCFA->bHybOpt1 & 7;
    Result.RepeatFrom1 = PCFA->bHybOpt1 >> 4;
    Result.StartFrom2  = PCFA->bHybOpt2 & 7;
    Result.iEnd2       = (PCFA->bHybOpt2 >> 4) & 7;
    Result.RepeatFrom2 = (PCFA->bHybOpt2 >> 8) & 7;

    Result.CalcDE      = CalcDEfull;
    Result.iSmNormals  = (Header.iOptions >> 6) & 15;
    Result.msDEstop    = MaxCS(0.0001f, Header.sDEstop);
    Result.DEstop      = Result.msDEstop;
    Result.sZstepDiv   = MaxCS(0.0001f, Header.mZstepDiv);
    Result.iDEAddSteps = Header.bStepsafterDEStop;
    Result.iMandHeight = Header.Height;
    Result.iMandWidth  = Header.Width;

    if (Header.TilingOptions != 0) {
        int crop = 0;
        GetTilingInfosFromHeader(&Header, Result.CalcRect, crop);
    } else {
        Result.CalcRect.Left = 0;
        Result.CalcRect.Top = 0;
        Result.CalcRect.Right = Header.Width - 1;
        Result.CalcRect.Bottom = Header.Height - 1;
    }
    TileSize tileSize = GetTileSize(&Header);
    Result.SLoffset = tileSize.width * static_cast<int32_t>(sizeof(TsiLight5));
    Result.SLoffsetExt = Result.SLoffset;

    TVec3D midPos;
    StereoChange(&Header, Header.bStereoMode, midPos, &Header.hVGrads);
    Result.Xmit = midPos[0];
    Result.Ymit = midPos[1];
    Result.Zmit = midPos[2];

    // bOptions2 bits (Pascal numbering from 1):
    // bit1 (val 1): Disable analytical DE
    // bit2+3 (val 2+4): inside rendering flags
    // Pascal HeaderTrafos.pas:550-551 uses bitwise AND, not enum decode:
    //   bInsideRendering := bOptions2 and 6 <> 0  (set if bit1 or bit2 of the 2-bit field)
    //   bInAndOutside    := bOptions2 and 4 <> 0  (set if bit2 of the 2-bit field)
    Result.bInsideRendering = (PCFA->bOptions2 & 6) != 0;
    Result.bInAndOutside    = (PCFA->bOptions2 & 4) != 0;
    Result.bCalcInside      = Result.bInsideRendering;

    if (Header.bPlanarOptic == 2) {
        Result.FOVy = Pi;
    } else {
        Result.FOVy = Header.dFOVy * Pid180;
    }

    Result.iMaxIt       = Header.Iterations;
    Result.iMinIt       = std::min(Header.Iterations, static_cast<int32_t>(Header.MinimumIterations));
    Result.iMaxitF2     = Header.iMaxItsF2;
    Result.calc3D       = (Header.bCalc3D != 0);
    Result.ColorOption  = std::min(static_cast<int>(5), static_cast<int>(Header.byColor2Option));
    Result.ColorOnIt    = Header.bColorOnIt;
    Result.calcHardShadow = Header.bCalculateHardShadow | (Header.bCalc1HSsoft << 8);
    Result.dRStop       = static_cast<float>(Header.RStop * Header.RStop);
    Result.Rstop3D      = Result.dRStop * Result.dRStop * 64.0f;
    Result.iCutOptions  = Header.bCutOption;
    Result.dCOX         = Header.dCutX;
    Result.dCOY         = Header.dCutY;
    Result.dCOZ         = Header.dCutZ;
    Result.bDoJulia     = (Header.bIsJulia != 0) ? static_cast<LongBool>(-1) : static_cast<LongBool>(0);
    Result.dJUx         = Header.dJx;
    Result.dJUy         = Header.dJy;
    Result.dJUz         = Header.dJz;
    Result.dJUw         = Header.dJw;
    Result.dDEscale     = 1.0f;
    Result.bVaryDEstop  = (Header.bVaryDEstopOnFOV != 0);
    std::memcpy(Result.Vgrads, static_cast<const void*>(Header.hVGrads), sizeof(TMatrix3));
    Result.DEAOmaxL     = Header.sDEAOmaxL;
    Result.DEmixCol     = Header.DEmixColorOption;
    Result.FmixPow      = Header.sFmixPow;
    Result.SoftShadowRadius = ShortFloatToSingle(Header.MCSoftShadowRadius);
    Result.iSliceCalc   = Header.bSliceCalc;
    Result.MCTCameraOptic = std::min(2, static_cast<int>(Header.bPlanarOptic & 3));
    Result.sHSmaxLengthMultiplier = Header.HSmaxLengthMultiplier;
    Result.AOdither     = Header.AODEdithering;
    Result.bCalcAmbShadow = (Header.bCalcAmbShadowAutomatic & 1) != 0;

    double x1_fov = std::min(1.5, std::max(static_cast<double>(s001), Result.FOVy * s05));
    Result.mctPlOpticZ = static_cast<float>(std::cos(x1_fov) * x1_fov / std::sin(x1_fov));

    Result.DFogOnIt = Header.bDFogIt;
    if ((Header.bVolLightNr & 7) > 0) Result.DFogOnIt = 65535;
    int volN = std::min(5, std::max(0, (Header.bVolLightNr & 7) - 1));
    if ((Header.Light.Lights[volN].Loption & 4) != 0) {
        Result.VLmul = 1000.0f * ShortFloatToSingle(*reinterpret_cast<const ShortFloat*>(&Header.Light.Lights[volN].Lamp)) / Header.Width;
    } else {
        Result.VLmul = 300.0f * ShortFloatToSingle(*reinterpret_cast<const ShortFloat*>(&Header.Light.Lights[volN].Lamp)) / Header.Width;
    }
    Result.VLstepmul = static_cast<float>(std::sqrt(1.0 / (1.0 + 0.1 * ((Header.bVolLightNr >> 4) - 2))));

    // FOV calculations
    Result.FOVXoff = CalcXoff(&Header) * Header.Width;
    if (Header.bPlanarOptic == 2) {
        Result.FOVXmul = static_cast<float>(PiM2 / Header.Width);
    } else {
        Result.FOVXmul = static_cast<float>(Result.FOVy / Header.Height);
    }

    // Determine hybrid mode from bOptions1 bits 0-1
    // 0 = alternating, 1 = interpolating, 2 = DEcombinated, 3 = IFS
    LongBool bIsInterpolHybrid = (PCFA->bOptions1 & 3) == 1;
    int i52 = bIsInterpolHybrid ? 1 : 5;
    LongBool bIsDEcomb = (PCFA->bOptions1 & 3) == 2;
    if (bIsDEcomb) {
        Result.FormulaType = std::min(6, std::max(1, static_cast<int>(PCFA->bOptions3) + 1));
    } else {
        Result.FormulaType = 0;
    }
    LongBool bDisableADE = (PCFA->bOptions2 & 1) != 0;
    if (!bIsDEcomb) Result.iMaxitF2 = Result.iMaxIt;

    // Set up formula iteration counts
    if (!bIsInterpolHybrid) {
        for (int x = 0; x <= 5; x++) {
            Result.nHybrid[x] = PCFA->Formulas[x].iItCount;
        }
    }

    // Default DE settings
    Result.DEoption = -1;
    Result.DEoption2 = 0;
    Result.mMandFunction   = fIsMemberAlternating;
    Result.mMandFunctionDE = fIsMemberAlternatingDE;
    Result.mMandFunction2  = fIsMemberAlternating;
    Result.mMandFunctionDE2 = fIsMemberAlternatingDE;
    Result.IsCustomDE = 0;
    Result.IsCustomDE2 = 0;
    Result.CalcDE = CalcDEnoADE;

    // Set up function pointers based on hybrid mode
    if (bIsInterpolHybrid) {
        Result.wEndTo = 1;
        CheckDEoption(0, 1, PCFA, Result.DEoption);
        if (Result.DEoption >= 4 && Result.DEoption <= 6) {
            Result.mMandFunction = doInterpolHybridPas4D;
            Result.mMandFunctionDE = doInterpolHybridPas4DDE;
        } else {
            Result.mMandFunction = fIsMemberIpol;
            Result.mMandFunctionDE = fIsMemberIpolDE;
        }
        Result.mMandFunction2 = Result.mMandFunction;
        Result.mMandFunctionDE2 = Result.mMandFunctionDE;
        Result.IsCustomDE = (Result.DEoption == 2 || Result.DEoption == 5 ||
                             Result.DEoption == 6 || Result.DEoption == 11);
        Result.IsCustomDE2 = Result.IsCustomDE;
        Result.DEoption2 = Result.DEoption;
        Result.CalcDE = Result.IsCustomDE ? CalcDEanalytic : CalcDEnoADE;

        // Set up interpolation weights from iItCount
        // Pascal reads iItCount bytes as PSingle (float reinterpret)
        float w1, w2;
        std::memcpy(&w1, &PCFA->Formulas[0].iItCount, sizeof(float));
        std::memcpy(&w2, &PCFA->Formulas[1].iItCount, sizeof(float));
        // Pascal promotes the stored Singles to Double, sums them, then
        // multiplies by one Double reciprocal before storing Single weights.
        // Summing in float can overflow even for two finite input weights.
        const double weightSum = static_cast<double>(w1) + static_cast<double>(w2);
        if (weightSum < 1e-10) {
            w1 = 1.0f;
            w2 = 0.0f;
        } else {
            const double inverseWeightSum = 1.0 / weightSum;
            w1 = static_cast<float>(static_cast<double>(w1) * inverseWeightSum);
            w2 = static_cast<float>(static_cast<double>(w2) * inverseWeightSum);
        }
        std::memcpy(&Result.nHybrid[0], &w1, sizeof(float));
        std::memcpy(&Result.nHybrid[1], &w2, sizeof(float));
    } else {
        int firstFormulaIndex = -1;
        IsSingleF(PCFA, firstFormulaIndex);

        int32_t nHybridLocal[6];
        std::memcpy(nHybridLocal, Result.nHybrid, sizeof(nHybridLocal));
        int deoption = Result.DEoption;
        TMandFunctionDE mandFunctionDE = Result.mMandFunctionDE;
        TMandFunction mandFunction = Result.mMandFunction;
        CheckFormulaOptions(0, Result.wEndTo, PCFA, deoption,
                            mandFunctionDE, mandFunction,
                            nHybridLocal, bDisableADE);
        std::memcpy(Result.nHybrid, nHybridLocal, sizeof(nHybridLocal));
        Result.DEoption = deoption;
        Result.mMandFunctionDE = mandFunctionDE;
        Result.mMandFunction = mandFunction;
        if ((PCFA->bHybOpt1 >> 4) < Result.RepeatFrom1) {
            Result.RepeatFrom1 = std::min<int>(Result.RepeatFrom1, PCFA->bHybOpt1 >> 4);
        }
        Result.IsCustomDE = (Result.DEoption == 2 || Result.DEoption == 5 ||
                             Result.DEoption == 6 || Result.DEoption == 11 ||
                             Result.DEoption == 20);

        if (!bIsDEcomb) {
            Result.DEoption2 = 0;
        } else {
            int32_t nHybridLocal2[6];
            std::memcpy(nHybridLocal2, Result.nHybrid, sizeof(nHybridLocal2));
            int deoption2 = Result.DEoption2;
            TMandFunctionDE mandFunctionDE2 = Result.mMandFunctionDE2;
            TMandFunction mandFunction2 = Result.mMandFunction2;
            CheckFormulaOptions(Result.StartFrom2, 5, PCFA, deoption2,
                                mandFunctionDE2, mandFunction2,
                                nHybridLocal2, bDisableADE);
            std::memcpy(Result.nHybrid, nHybridLocal2, sizeof(nHybridLocal2));
            Result.DEoption2 = deoption2;
            Result.mMandFunctionDE2 = mandFunctionDE2;
            Result.mMandFunction2 = mandFunction2;
            Result.IsCustomDE2 = (Result.DEoption2 == 2 || Result.DEoption2 == 5 ||
                                  Result.DEoption2 == 6 || Result.DEoption2 == 11 ||
                                  Result.DEoption2 == 20);
            if (Result.FormulaType > 5) {
                if (Result.DEoption2 != 20) {
                    // Pascal reports this invalid mix, but subsequently overwrites
                    // bMCTisValid. Preserve the rejection instead of dispatching a
                    // non-dIFS callback with the mix driver's no-vector-init state.
                    throw std::invalid_argument("FORMULA_INPUT_REJECTED[invalid_de_mix] "
                        "DE mix requires the second hybrid type to be dIFS (DE option 20)");
                } else {
                    Result.mMandFunctionDE2 = doHybridIFS3DnoVecIni;
                }
            }
        }
        Result.CalcDE = bIsDEcomb ? CalcDEfull :
                        (Result.IsCustomDE ? CalcDEanalytic : CalcDEnoADE);
    }

    // Count valid formulas
    int n_valid = -1;
    if (bIsInterpolHybrid) {
        // Pascal explicitly rejects dIFS interpolation before counting its
        // two required formulas; a dIFS callback has a different state contract.
        if (Result.DEoption >= 20 && Result.DEoption <= 22) {
            throw std::invalid_argument("Unsupported ordinary interpolation: dIFS formulas cannot be interpolated");
        }
        int count = 0;
        for (int x = 0; x <= i52; x++) {
            if (PCFA->Formulas[x].iFnr >= 0) {
                ++count;
            }
        }
        n_valid = count - 2;
    } else {
        IsSingleF(PCFA, n_valid);
    }

    // Set up formula function pointers and hybrid parameters
    Result.bMCTisValid = (n_valid >= 0) && (Result.DEoption >= 0) &&
                          ((Result.DEoption2 >= 0) || !bIsDEcomb);

    if (Result.bMCTisValid) {
        Result.dColPlus = 0;
        Result.dDEscale = 1.0f;
        Result.dDEscale2 = 1.0f;

        // NI8: Separate part1/part2 accumulators for DEcomb
        int n_scale = 0, n_scale2 = 0;
        double x1_scale = 0, x1_scale2 = 0;
        double y1_color = 0, y1_color2 = 0;
        int i_color = 0, i_color2 = 0;
        bool hasSecondPart = false;

        // Set up formula pointers and build constant buffers for each slot
        for (int x = 0; x <= i52; x++) {
            if (bIsInterpolHybrid || Result.nHybrid[x] != 0) {
                // For external formulas, override byOptionType with correct values
                // from the .m3f definition. The .m3p file may have incorrect types
                // for multi-value options (e.g., .3SingleAngles).
                ApplyExternalFormulaOptionTypes(PCFA->Formulas[x]);
                // Build the PVar constant buffer (mirrors Pascal pConstPointer16)
                BuildFormulaConstBuffer(x, PCFA, x);
                Result.fHPVar[x] = GetFormulaConstBuffers().pConstPointer[x];

                int fnr = PCFA->Formulas[x].iFnr;

                // Determine if this slot is in the "second part" (for DEcomb)
                bool bSecondPart = x > Result.wEndTo;
                bool bTmpCustomDE = bSecondPart ? (Result.IsCustomDE2 != 0) : (Result.IsCustomDE != 0);

                // Get formula function using registry
                if (IsInternalFormula(fnr)) {
                    int power = 2;
                    if (fnr == 0) {
                        // ParseCFfromOld clamps dSIpow before Round; this differs
                        // from FOLDING's Round-then-clamp for huge input powers.
                        power = RoundFormulaPower(GetInternalFormulaSIpow(PCFA->Formulas[x]));
                    }
                    // For iFnr 4 (AmazingBox) without custom DE, use Cube (no w tracking)
                    bool useDE = bTmpCustomDE || (fnr != 4);
                    ThybridIteration func = GetInternalFormula(fnr, power, useDE);
                    Result.fHybrid[x] = reinterpret_cast<ThybridIteration2>(func);
                } else {
                    // External formula (iFnr >= 20) — look up in registry
                    const char* formulaName = reinterpret_cast<const char*>(PCFA->Formulas[x].CustomFname);
                    int extDEoption = 0;
                    ThybridIteration func = GetExternalFormula(formulaName, &extDEoption);
                    if (func) {
                        Result.fHybrid[x] = reinterpret_cast<ThybridIteration2>(func);
                    } else {
                        if (fnr >= 20) {
                            fprintf(stderr, "Warning: External formula '%.*s' not found in registry, using empty formula\n",
                                    32, formulaName);
                        }
                        Result.fHybrid[x] = reinterpret_cast<ThybridIteration2>(EmptyFormula);
                    }
                }

                // Compute fHln for smooth iterations
                double z1;
                if (IsInternalFormula(fnr)) {
                    z1 = GetInternalFormulaSIpow(PCFA->Formulas[x]);
                } else {
                    const ExternalFormulaInfo* extInfo = FindExternalFormula(
                        reinterpret_cast<const char*>(PCFA->Formulas[x].CustomFname));
                    // Pascal external-formula path uses dSIpow when it is > 1,
                    // otherwise it falls back to option[0].
                    if (extInfo && extInfo->SIpow > 1.0) {
                        z1 = extInfo->SIpow;
                    } else {
                        z1 = PCFA->Formulas[x].dOptionValue[0];
                    }
                }

                if (bIsInterpolHybrid || Result.nHybrid[x] > 0) {
                    Result.fHln[x] = static_cast<float>(1.0 / std::log(MaxCD(2.0, std::fabs(z1))));
                } else {
                    Result.fHln[x] = static_cast<float>(1.0 / std::log(2.0));
                }

                // Accumulate DE scale
                double iterWeight;
                if (bIsInterpolHybrid) {
                    float sw;
                    std::memcpy(&sw, &Result.nHybrid[x], sizeof(float));
                    iterWeight = sw;
                } else {
                    iterWeight = std::abs(Result.nHybrid[x]);
                }

                // NI8: Route to part1 or part2 accumulator based on bSecondPart
                hasSecondPart = hasSecondPart || bSecondPart;
                double& xs = bSecondPart ? x1_scale2 : x1_scale;
                int& ns = bSecondPart ? n_scale2 : n_scale;
                double& ys = bSecondPart ? y1_color2 : y1_color;
                int& is = bSecondPart ? i_color2 : i_color;

                int fDE = 0;
                if (IsInternalFormula(fnr)) {
                    fDE = GetDEoptionForFormula(fnr);
                } else {
                    const ExternalFormulaInfo* extInfo = FindExternalFormula(
                        reinterpret_cast<const char*>(PCFA->Formulas[x].CustomFname));
                    if (extInfo) {
                        fDE = extInfo->DEoption;
                    }
                }
                if (bTmpCustomDE && (bIsInterpolHybrid || Result.nHybrid[x] > 0) &&
                    (fDE == 5 || fDE == 11)) {
                    // w-based DE formulas: scale factor from PVar[-2] (scale parameter)
                    double* pConst = static_cast<double*>(GetFormulaConstBuffers().pConstPointer[x]);
                    double scaleVal = NotZero(*(pConst - 2));
                    if (scaleVal < 0) {
                        xs += 0.65 * iterWeight;
                    } else {
                        xs += (scaleVal * 1.2 + 1) / scaleVal * iterWeight;
                    }
                    ys += std::fabs(scaleVal) * iterWeight;
                    if (bIsInterpolHybrid) is++;
                    else is = AddHybridRepeatWeight(is, Result.nHybrid[x], x);
                    if (bIsInterpolHybrid) ns++;
                    else ns = AddHybridRepeatWeight(ns, Result.nHybrid[x], x);
                } else if (bIsInterpolHybrid || fDE >= 0) {
                    double deScale = 1.0;
                    if (IsInternalFormula(fnr)) {
                        deScale = bTmpCustomDE
                            ? GetInternalFormulaADEscale(PCFA->Formulas[x])
                            : GetInternalFormulaDEscale(PCFA->Formulas[x]);
                    } else {
                        const ExternalFormulaInfo* extInfo = FindExternalFormula(
                            reinterpret_cast<const char*>(PCFA->Formulas[x].CustomFname));
                        if (extInfo) {
                            deScale = bTmpCustomDE ? extInfo->ADEscale : extInfo->DEscale;
                        }
                    }
                    xs += deScale * iterWeight;
                    if (bIsInterpolHybrid) ns++;
                    else ns = AddHybridRepeatWeight(ns, Result.nHybrid[x], x);
                }

                // Handle negative nHybrid (formulas that don't count iterations)
                if (!bIsInterpolHybrid && Result.nHybrid[x] < 0) {
                    Result.nHybrid[x] = std::abs(Result.nHybrid[x]) | static_cast<int32_t>(0x80000000u);
                }
            } else {
                Result.fHybrid[x] = reinterpret_cast<ThybridIteration2>(EmptyFormula);
            }
        }

        // I19: fHln interpolation blend for interp mode
        if (bIsInterpolHybrid) {
            float w1i, w2i;
            std::memcpy(&w1i, &Result.nHybrid[0], sizeof(float));
            std::memcpy(&w2i, &Result.nHybrid[1], sizeof(float));
            Result.fHln[0] = Result.fHln[0] * w1i + Result.fHln[1] * w2i;
        }

        // Pascal computes DE scale and dColPlus per hybrid part, not from a
        // global average. This matters for Amazing Box / FoldIntPow scenes.
        int firstScaleCount = n_scale;
        int secondScaleCount = n_scale2;
        if (bIsInterpolHybrid) {
            if (firstScaleCount > 0) firstScaleCount = 1;
            if (secondScaleCount > 0) secondScaleCount = 1;
        }
        int ia = 0;
        if (!hasSecondPart) {
            if (firstScaleCount > 0) {
                Result.dDEscale = static_cast<float>(x1_scale / firstScaleCount);
                Result.dDEscale2 = Result.dDEscale;
            }
            if (i_color > 0) {
                Result.dColPlus = static_cast<float>(std::pow(std::fabs(y1_color / i_color), 0.25) * 40.0 - 49.0);
            }
        } else {
            if (n_scale > 0 && Result.DEoption != 20 && firstScaleCount > 0) {
                Result.dDEscale = static_cast<float>(x1_scale / firstScaleCount);
                if (i_color > 0) {
                    Result.dColPlus = static_cast<float>(std::pow(std::fabs(y1_color / i_color), 0.25) * 40.0 - 49.0);
                    ia = i_color;
                }
            }
            if (n_scale2 > 0 && Result.DEoption2 != 20 && secondScaleCount > 0) {
                Result.dDEscale2 = static_cast<float>(x1_scale2 / secondScaleCount);
                if (i_color2 > ia) {
                    Result.dColPlus = static_cast<float>(std::pow(std::fabs(y1_color2 / i_color2), 0.25) * 40.0 - 49.0);
                }
            } else if (firstScaleCount > 0) {
                Result.dDEscale2 = Result.dDEscale;
            }
        }

        // Ensure remaining slots have valid function pointers
        for (int x = i52 + 1; x <= 5; x++) {
            Result.fHybrid[x] = reinterpret_cast<ThybridIteration2>(EmptyFormula);
        }

        // Safety: ensure no null function pointers
        for (int x = 0; x <= 5; x++) {
            if (Result.fHybrid[x] == nullptr) {
                Result.fHybrid[x] = reinterpret_cast<ThybridIteration2>(EmptyFormula);
                Result.nHybrid[x] = 0;
            }
        }

        // Coloring parameters
        Result.mctColorMul = Header.sColorMul * 512.0f;

        // Z position and depth calculations
        double Zcorr_local, ZcMul_local, Zend_local;
        CalcPPZvals(Header, Zcorr_local, ZcMul_local, Zend_local);
        Result.Zcorr = Zcorr_local;
        Result.ZcMul = ZcMul_local;
        Result.sZZstmitDif = static_cast<float>(Zend_local);

        CalcStepWidth(&Header);
        Result.StepWidth = Header.dStepWidth;
        Result.Zend = MaxCD(1e-10, (Header.dZend - Header.dZstart) / Result.StepWidth);
        // First step random
        Result.bMCTFirstStepRandom = (Header.iOptions & 1) != 0;

        // Ray step limiter
        const int mandWidth = Result.iMandWidth;
        const int mandHeight = Result.iMandHeight;
        Result.mctMH04ZSD = CalcRayStepLimiter(
            mandWidth, mandHeight, Result.sZstepDiv, Header.sRaystepLimiter);

        // DEstop factor for FOV-based variation
        if (Result.bVaryDEstop) {
            Result.mctDEstopFactor = static_cast<float>(GetDEstopFactor(&Header));
        } else {
            Result.mctDEstopFactor = 0;
        }

        // LN RStop for smooth iterations
        Result.dLNRStop = static_cast<float>(std::log(std::log(Header.RStop)));

        // Normals on DE
        Result.NormalsOnDE = (Header.bNormalsOnDE > 0) || Result.IsCustomDE;

        // Camera start position calculation
        double x1_cam, y1_cam;
        if (Header.bPlanarOptic == 2) {
            x1_cam = 0;
            y1_cam = 0;
        } else {
            x1_cam = Result.iMandWidth * 0.5;
            y1_cam = Result.iMandHeight * 0.5;
        }

        double sw = Result.StepWidth;
        double z1_cam;
        if (Result.calc3D || Result.iSliceCalc == 1) {
            z1_cam = (Header.dZstart - Header.dZmid) / sw;
        } else if (Result.iSliceCalc == 3) {
            sw = sw * (1.0 + std::sin(Result.FOVy) * Result.Zend / Result.iMandHeight);
            z1_cam = (Header.dZend - Header.dZmid) / sw;
        } else {
            Result.iSliceCalc = 2;
            z1_cam = 0;
            sw = sw * (1.0 + std::sin(Result.FOVy) * (Header.dZmid - Header.dZstart) / (Result.StepWidth * Result.iMandHeight));
        }

        TMatrix3 sourceVGrads{};
        TMatrix3 normalizedVGrads{};
        std::memcpy(&sourceVGrads, static_cast<const void*>(Header.hVGrads), sizeof(sourceVGrads));
        NormaliseMatrixTo(normalizedVGrads, sw, sourceVGrads);
        std::memcpy(Result.Vgrads, normalizedVGrads, sizeof(normalizedVGrads));

        // Ystart = midpoint + z*Vgrads[2] - y*Vgrads[1] - x*Vgrads[0]
        Result.Ystart[0] = Result.Xmit + z1_cam * Result.Vgrads[2][0] - y1_cam * Result.Vgrads[1][0] - x1_cam * Result.Vgrads[0][0];
        Result.Ystart[1] = Result.Ymit + z1_cam * Result.Vgrads[2][1] - y1_cam * Result.Vgrads[1][1] - x1_cam * Result.Vgrads[0][1];
        Result.Ystart[2] = Result.Zmit + z1_cam * Result.Vgrads[2][2] - y1_cam * Result.Vgrads[1][2] - x1_cam * Result.Vgrads[0][2];

        // DE offset for 4-point numerical DE
        Result.mctDEoffset = static_cast<float>(std::min(static_cast<double>(Result.msDEstop) * 0.1, 0.004));

        // Formula initialization pointers
        for (int x = 0; x <= 5; x++) {
            Result.pInitialization[x] = nullptr;
        }

        // dIFS handling
        if (Result.DEoption == 20) Result.dDEscale = 1.0f;
        if (Result.DEoption2 == 20) Result.dDEscale2 = 1.0f;

        bool isIFS = (Result.DEoption == 20) || (Result.DEoption2 == 20);
        if (isIFS) {
            Result.iDEAddSteps = std::max(static_cast<int>(Result.iDEAddSteps),
                                          static_cast<int>(std::round(MinCS(1.0f, Result.sZstepDiv) * 2)) + 2);
            if (!bIsDEcomb || Result.FormulaType < 5) Result.iMinIt = 1;
            Result.bIsIFS = 1;
            Result.NormalsOnDE = 1;
        } else {
            Result.bIsIFS = 0;
        }

        // Max iterations for smooth iteration scale
        int64_t iMaxCombined64;
        const int iMaxIt = Result.iMaxIt;
        const int iMaxitF2 = Result.iMaxitF2;
        if (Result.FormulaType > 5) {
            iMaxCombined64 = static_cast<int64_t>(iMaxIt) + iMaxitF2;
        } else {
            iMaxCombined64 = std::max(iMaxIt, iMaxitF2);
        }
        if (iMaxCombined64 < std::numeric_limits<int32_t>::min() ||
            iMaxCombined64 >= std::numeric_limits<int32_t>::max()) {
            throw std::invalid_argument("Unsupported ordinary combined maximum iteration limit: " +
                std::to_string(iMaxCombined64) +
                " cannot represent the signed 32-bit smooth-iteration scale");
        }
        const int iMaxCombined = static_cast<int>(iMaxCombined64);
        Result.mctsM = 32767.0f / std::max(1, iMaxCombined + 1);
        // The Pascal literal is Extended; do not round its product to Single
        // before adding the stored color offset.
        Result.dColPlus = static_cast<float>(
            static_cast<long double>(Result.dColPlus) + iMaxCombined * 0.1L);

        // mctColVarDEstopMul: formula-specific DE weighting for color tab index
        // stability on zoom. Pascal HeaderTrafos.pas uses this same zero-based
        // accumulation for ordinary and interpolated hybrids; its older special
        // interpolation branch (starting with 0.6) is commented out.
        {
            auto readPrimaryScale = [&](int slot) -> double {
                if (slot < 0 || slot > 5 || GetFormulaConstBuffers().pConstPointer[slot] == nullptr) {
                    return 0.0;
                }
                double scale = 0.0;
                const uint8_t* pConst = static_cast<const uint8_t*>(GetFormulaConstBuffers().pConstPointer[slot]);
                std::memcpy(&scale, pConst - 16, sizeof(double));
                return scale;
            };

            Result.mctColVarDEstopMul = 0.6f;
            int ia_local = 0;
            int itmp_local = 0;
            double y1_local = 0.0;
            double z1_local = 0.0;
            double z2_local = 0.0;
            bool secondPart = false;

            for (int x = 0; x <= i52; x++) {
                if (bIsInterpolHybrid || Result.nHybrid[x] > 0) {
                    if ((x > Result.wEndTo) && !secondPart) {
                        secondPart = true;
                        Result.mctColVarDEstopMul = z1_local > 0.0
                            ? static_cast<float>(y1_local / z1_local)
                            : 0.6f;
                        z2_local = z1_local;
                        z1_local = 0.0;
                        y1_local = 0.0;
                        if ((Result.DEoption2 == 20) || !bIsDEcomb) {
                            break;
                        }
                    }

                    // Pascal stores normalized Single weights in these integer slots.
                    float interpolWeight = 0.0f;
                    if (bIsInterpolHybrid) {
                        std::memcpy(&interpolWeight, &Result.nHybrid[x], sizeof(interpolWeight));
                    }
                    const double x2 = bIsInterpolHybrid
                        ? static_cast<double>(interpolWeight)
                        : static_cast<double>(Result.nHybrid[x]);
                    int fDE = GetFormulaDEoption(PCFA->Formulas[x]);
                    if (fDE == 0 || fDE == 4) {
                        y1_local += x2 * 0.6;
                        z1_local += x2;
                    } else if ((fDE >= 1 && fDE <= 3) || (fDE >= 5 && fDE <= 40)) {
                        uint8_t optType0 = PCFA->Formulas[x].byOptionType[0];
                        if (optType0 == 0 || optType0 == 14) {
                            double scaleVal = readPrimaryScale(x);
                            if (scaleVal > 1.0) {
                                if (scaleVal < 1.2) {
                                    scaleVal = 1.1 + (scaleVal - 1.0) * (scaleVal - 1.0) * 2.5;
                                }
                                y1_local += x2 * 0.5 / (std::log(scaleVal) + 0.03);
                                z1_local += x2;
                            } else if (scaleVal < -1.0) {
                                if (scaleVal > -1.2) {
                                    scaleVal = -1.1 - (scaleVal + 1.0) * (scaleVal + 1.0) * 2.5;
                                }
                                y1_local += x2 * 0.5 / (std::log(-scaleVal) + 0.03);
                                z1_local += x2;
                            }
                        }
                    }
                } else if (!bIsInterpolHybrid && Result.nHybrid[x] < 0) {
                    if (x > Result.wEndTo) ++ia_local;
                    else ++itmp_local;
                }
            }

            if (((z1_local >= z2_local) && (z1_local > 1e-3)) ||
                ((ia_local > itmp_local) &&
                 (std::fabs(z1_local - z2_local) < 1e-3) &&
                 (z1_local > 1e-3))) {
                Result.mctColVarDEstopMul = static_cast<float>(y1_local / z1_local);
            } else if (z2_local < 1e-3) {
                Result.mctColVarDEstopMul = 0.6f;
            }
        }

        // BUG #16 fix: Pascal lines 919-923 — dIFS override of mctMH04ZSD and mctColVarDEstopMul
        // Must come after mctColVarDEstopMul loop to override its value.
        if ((Result.DEoption == 20) && (!bIsDEcomb || (Result.DEoption2 == 20))) {
            Result.mctMH04ZSD = static_cast<float>(std::max(Result.iMandWidth, Result.iMandHeight));
            Result.mctColVarDEstopMul = 0;
        }

        Result.sStepWm103 = static_cast<float>(Result.StepWidth * 1.03);

        // Build 4D rotation matrix
        TSMatrix4 smatrix4d{};
        BuildSMatrix4(Header.dXWrot, Header.dYWrot, Header.dZWrot, smatrix4d);
        std::memcpy(Result.Smatrix4d, smatrix4d, sizeof(smatrix4d));

        // Cut plane direction flags
        if (Result.iCutOptions > 0) {
            Result.iCutOptions |=
                ((Result.Vgrads[2][0] > 0 ? 1 : 0) << 4) |
                ((Result.Vgrads[2][1] > 0 ? 1 : 0) << 5) |
                ((Result.Vgrads[2][2] > 0 ? 1 : 0) << 6);
        }

        // DE scale adjustments
        Result.IsCustomDE1 = Result.IsCustomDE;
        if (Result.IsCustomDE) {
            Result.dDEscale = static_cast<float>(Result.dDEscale / Header.dStepWidth);
        } else {
            Result.dDEscale = Result.dDEscale * Result.mctDEoffset;
        }

        if (bIsDEcomb) {
            Result.NormalsOnDE = 1;
            Result.sDEcombSmooth = MaxCS(1e-30f, Header.sDEcombS) / static_cast<float>(Header.dStepWidth);
            if (Result.IsCustomDE2) {
                Result.dDEscale2 = static_cast<float>(Result.dDEscale2 / Header.dStepWidth);
            } else {
                Result.dDEscale2 = Result.dDEscale2 * Result.mctDEoffset;
            }
            // BUG #15 fix: Pascal line 966 — reset bIsIFS when DEcomb but DEoption2 != 20
            if (Result.DEoption2 != 20) Result.bIsIFS = 0;
        }

        Result.mctDEoffset006 =
            static_cast<float>(static_cast<double>(Result.mctDEoffset) * 0.06);
        Result.mctDEoffset =
            static_cast<float>(static_cast<double>(Result.mctDEoffset) * Result.StepWidth);

        if (bIniCalcMaps) {
            IniCalcMaps(&Header, PCFA);
        }

        // Normal calculation function pointer
        if (Result.NormalsOnDE) {
            Result.pCalcNormals = reinterpret_cast<void*>(RMCalculateNormals);
        } else {
            Result.pCalcNormals = reinterpret_cast<void*>(RMCalculateNormalsOnSmoothIt);
        }

        // DE subtraction for quality mode
        if ((Header.iOptions & 4) == 0) {
            Result.msDEsub = 0;
        } else {
            float zsd = Result.sZstepDiv;
            Result.sZstepDiv = zsd * zsd + (1.2f * zsd) * (1.0f - zsd);
            Result.msDEsub = MinCS(0.9f, std::sqrt(Result.sZstepDiv));
        }
    }

    return Result;
}

// ============================================================================
// MakeLightValsFromHeaderLight -- lighting parameter setup
// Ported from HeaderTrafos.pas (~330 lines)
// Ported from HeaderTrafos.pas
// ============================================================================

void MakeLightValsFromHeaderLight(TMandHeader10* Header, TPLightVals Lvals,
                                  double ImScale, int StereoMode) {
    if (!Lvals) return;

    auto& HLight = Header->Light;
    CalcStepWidth(Header);

    TVec3D midPos{};
    std::memcpy(midPos, Lvals->lvMidPos, sizeof(midPos));
    StereoChange(Header, StereoMode, midPos, &Header->hVGrads);
    std::memcpy(Lvals->lvMidPos, midPos, sizeof(midPos));

    // Low-level callers borrow fallback storage from their owning context.
    if (!Lvals->PLValigned) {
        auto& alignedStorage = CurrentRenderContext().fallback_light_values;
        std::memset(&alignedStorage, 0, sizeof(TLValigned));
        Lvals->PLValigned = &alignedStorage;
    } else {
        std::memset(Lvals->PLValigned, 0, sizeof(TLValigned));
    }

    for (int i = 0; i < 6; i++) {
        Lvals->PLValigned->LN[i][0] = 0.0f;
        Lvals->PLValigned->LN[i][1] = 0.0f;
        Lvals->PLValigned->LN[i][2] = -1.0f;
        Lvals->PLValigned->LN[i][3] = 0.0f;
    }

    Lvals->bColCycling = (HLight.TBoptions & 0x4000) != 0;
    Lvals->bFarFog     = (HLight.TBoptions & 0x40000) != 0;

    // Gamma
    int gammaIdx = (HLight.TBoptions >> 23) & 0x3F;
    if (gammaIdx == 32) {
        Lvals->iGammaH = 0;
        Lvals->sGamma = 0;
    } else if (gammaIdx < 32) {
        Lvals->iGammaH = -1;
        Lvals->sGamma = 1.0f - gammaIdx / 32.0f;
    } else {
        Lvals->iGammaH = 1;
        Lvals->sGamma = (gammaIdx - 32) / 31.0f;
    }

    // Depth and fog parameters
    Lvals->sDepth = HLight.TBpos[1] * 0.8e-6f;  // TBpos[4] in Pascal 1-based = [1] in 0-based

    // C13: bVolLight fog branch
    bool bVolLight = (Header->bVolLightNr & 7) != 0;
    Lvals->bVolLight = bVolLight;
    double dTmp;
    if (bVolLight) {
        Lvals->sDynFogMul = 0.0005f;
        dTmp = 50.0;
        Lvals->sShadGr = (HLight.TBpos[3] - 53) * 0.00002f;
    } else {
        Lvals->sShadGr = (HLight.TBpos[3] - 53) * ImScale * Header->mZstepDiv * 0.00065f;
        dTmp = 2.2 / Header->mZstepDiv;
        Lvals->sDynFogMul = Header->mZstepDiv * 0.015f;
        if (Header->bDFogIt > 0) {
            dTmp *= 0.25;
            Lvals->sShadGr *= 4.0f;
            Lvals->sDynFogMul *= 4.0f;
        }
    }

    float tb3val = static_cast<float>(HLight.TBpos[0] & 0xFFFF); // TBpos[3] in Pascal
    // NC7: Use 137 when not bVolLight and bDFogIt==0, 128 otherwise
    float sTmpShad = (bVolLight || Header->bDFogIt > 0) ? 128.0f : 137.0f;
    Lvals->sShad = static_cast<float>((static_cast<double>(sTmpShad) - std::sqrt(static_cast<double>(tb3val)) * 11.313708) * dTmp * 0.28);

    // C11: sShadZmul
    Lvals->sShadZmul = static_cast<float>(dTmp * 0.7 / (Header->dZend - Header->dZstart) *
        (128.0 - std::sqrt(static_cast<double>(HLight.TBpos[0] >> 16)) * 11.313708));

    Lvals->sAmbShad = static_cast<float>((HLight.TBpos[8] & 0xFF) / 53.0); // TBpos[11]

    // Smooth color start/mul
    CalcSCstartAndSCmul(Header, Lvals->sCStart, Lvals->sCmul, 0);
    CalcSCstartAndSCmul(Header, Lvals->sCiStart, Lvals->sCimul, 1);

    // Light setup
    TMatrix3 M;
    TMatrix3 sourceVGrads{};
    std::memcpy(&sourceVGrads, static_cast<const void*>(Header->hVGrads), sizeof(sourceVGrads));
    NormaliseMatrixTo(M, 1.0, sourceVGrads);

    // NI1: Set bCalcPixColSqr early so light color conversion can use it
    Lvals->bCalcPixColSqr = (HLight.AdditionalOptions & 1) != 0;
    float sLightScale = Lvals->bCalcPixColSqr ? 1.5f : 1.0f;

    // Pascal: HeaderTrafos.pas line 1337 — bCalc1HSsoft pre-adjustment before per-light loop
    if ((Header->bCalc1HSsoft & 1) != 0) {
        Header->bHScalculated = Header->bHScalculated & Header->bCalculateHardShadow;
    }

    // NI4: Save per-light lamp values for sLmaxL computation after Pascal-like
    // per-light scaling has been applied.
    float sLampTmp[6] = {0};

    for (int i = 0; i < 6; i++) {
        auto& light = HLight.Lights[i];

        Lvals->iLightPowFunc[i]  = 2 << (light.LFunction & 7);
        Lvals->iLightFuncDiff[i] = (light.LFunction >> 4) & 3;
        Lvals->iLightOption[i]   = light.Loption & 3;
        if (Lvals->iLightOption[i] == 3) Lvals->iLightOption[i] = 1;
        if (Lvals->iLightOption[i] == 2 && light.LightMapNr == 0) Lvals->iLightOption[i] = 1;

        Lvals->iLightPos[i] = ((light.Loption >> 2) & 7) |
                               ((light.LFunction & 128) >> 4);
        if (Lvals->iLightOption[i] > 0) Lvals->iLightPos[i] = 0;
        Lvals->iLightAbs[i] = (light.Loption >> 5) & 1;

        // Light color
        // NI1: When bCalcPixColSqr, use Sqr(byte) * (1/255) per DivUtils RGBColToSVecNoScaleSQR
        if (Lvals->bCalcPixColSqr) {
            float b0 = static_cast<float>(light.Lcolor[0]);
            float b1 = static_cast<float>(light.Lcolor[1]);
            float b2 = static_cast<float>(light.Lcolor[2]);
            Lvals->PLValigned->sLCols[i][0] = b0 * b0 / 255.0f;
            Lvals->PLValigned->sLCols[i][1] = b1 * b1 / 255.0f;
            Lvals->PLValigned->sLCols[i][2] = b2 * b2 / 255.0f;
        } else {
            Lvals->PLValigned->sLCols[i][0] = static_cast<float>(light.Lcolor[0]);
            Lvals->PLValigned->sLCols[i][1] = static_cast<float>(light.Lcolor[1]);
            Lvals->PLValigned->sLCols[i][2] = static_cast<float>(light.Lcolor[2]);
        }
        Lvals->PLValigned->sLCols[i][3] = 0;
        TSVec scaledColor{};
        std::memcpy(scaledColor, Lvals->PLValigned->sLCols[i], sizeof(scaledColor));
        ScaleSVec_inplace(scaledColor, sLightScale);
        std::memcpy(Lvals->PLValigned->sLCols[i], scaledColor, sizeof(scaledColor));

        ShortFloat lampValue{};
        std::memcpy(&lampValue, &light.Lamp, sizeof(lampValue));
        float sTmp = ShortFloatToSingle(lampValue);

        Lvals->iHSenabled[i] = 1 - ((light.Loption >> 6) & 1);
        Lvals->iHScalced[i]  = Lvals->iHSenabled[i] & (Header->bHScalculated >> (i + 2));
        Lvals->iHSmask[i]    = 0x400 << i;
        // Pascal: HeaderTrafos.pas line 1357 — soft-hard shadow mask override
        if (((Header->bCalc1HSsoft & 1) != 0) && (Lvals->iHScalced[i] != 0)) {
            Lvals->iHSmask[i] = -1;
        }

        sLampTmp[i] = sTmp;

        if ((Lvals->iLightPos[i] & 1) != 0) {
            // Position light
            ScaleSVec_inplace(Lvals->PLValigned->sLCols[i], sTmp * 1.3f * sLightScale);
            TVec3D lightPos;
            lightPos[0] = D7BtoDouble(light.LXpos);
            lightPos[1] = D7BtoDouble(light.LYpos);
            lightPos[2] = D7BtoDouble(light.LZpos);
            TVec3D relPos;
            SubtractVec3D(relPos, lightPos, Lvals->lvMidPos);
            Lvals->PLValigned->LN[i][0] = static_cast<float>(relPos[0]);
            Lvals->PLValigned->LN[i][1] = static_cast<float>(relPos[1]);
            Lvals->PLValigned->LN[i][2] = static_cast<float>(relPos[2]);
            Lvals->PLValigned->LN[i][3] = 0.0f;
            // BUG #13 fix: compute sLmaxL from SCALED sLCols for position lights
            {
                const float colSum = (Lvals->PLValigned->sLCols[i][0] +
                                      Lvals->PLValigned->sLCols[i][1] +
                                      Lvals->PLValigned->sLCols[i][3]) * (1.0f / 3.0f);
                Lvals->sLmaxL[i] = 800.0f * (colSum + 128.0f * sTmp);
            }
        } else {
            // Pascal builds the direction vector for every non-positional light,
            // even when iLightOption marks it as "off". Those LN values are still
            // consumed later after SortLights / hard-shadow remapping.
            double dTmpX = -D7BtoDouble(light.LXpos);
            double dTmpY = D7BtoDouble(light.LYpos);
            // Pascal normalizes before storing the Single vector. Normalizing
            // already-rounded components changes diffuse-light weights.
            BuildViewVectorFOVDouble(dTmpY, dTmpX, Lvals->PLValigned->LN[i]);
            for (int c = 0; c < 3; ++c) Lvals->PLValigned->LN[i][c] = -Lvals->PLValigned->LN[i][c];

            if (Lvals->iLightAbs[i] > 0) {
                // Rotate to object space
                TVec3D tmp;
                tmp[0] = Lvals->PLValigned->LN[i][0];
                tmp[1] = Lvals->PLValigned->LN[i][1];
                tmp[2] = Lvals->PLValigned->LN[i][2];
                RotateVectorReverse(tmp, M);
                double len = std::sqrt(tmp[0]*tmp[0] + tmp[1]*tmp[1] + tmp[2]*tmp[2]);
                if (len > 1e-20) {
                    Lvals->PLValigned->LN[i][0] = static_cast<float>(tmp[0] / len);
                    Lvals->PLValigned->LN[i][1] = static_cast<float>(tmp[1] / len);
                    Lvals->PLValigned->LN[i][2] = static_cast<float>(tmp[2] / len);
                }
            }

            // BUG #13 fix: compute sLmaxL from RAW sLCols BEFORE scaling
            {
                const float colSum = (Lvals->PLValigned->sLCols[i][0] +
                                      Lvals->PLValigned->sLCols[i][1] +
                                      Lvals->PLValigned->sLCols[i][3]) * (1.0f / 3.0f);
                Lvals->sLmaxL[i] = 800.0f * (colSum + 128.0f) * sTmp;
            }

            TSVec scaledColor{};
            std::memcpy(scaledColor, Lvals->PLValigned->sLCols[i], sizeof(scaledColor));
            ScaleSVec_inplace(scaledColor, sTmp);
            std::memcpy(Lvals->PLValigned->sLCols[i], scaledColor, sizeof(scaledColor));
        }

        // BUG #14 fix: multiply sLmaxL by 5 for light type 8
        if ((Lvals->iLightPos[i] & 14) == 8) {
            Lvals->sLmaxL[i] *= 5.0f;
        }

    }

    // Parity trace for lights (after sLmaxL computed inline above)
    for (int i = 0; i < 6; i++) {
        {
            std::ostringstream payload;
            payload << std::fixed << std::setprecision(12)
                    << "light=" << i
                    << " iLightOption=" << Lvals->iLightOption[i]
                    << " iLightPos=" << Lvals->iLightPos[i]
                    << " iLightAbs=" << Lvals->iLightAbs[i]
                    << " Lamp=" << sLampTmp[i]
                    << " LColX=" << Lvals->PLValigned->sLCols[i][0]
                    << " LColY=" << Lvals->PLValigned->sLCols[i][1]
                    << " LColZ=" << Lvals->PLValigned->sLCols[i][2]
                    << " LmaxL=" << Lvals->sLmaxL[i]
                    << " LNx=" << Lvals->PLValigned->LN[i][0]
                    << " LNy=" << Lvals->PLValigned->LN[i][1]
                    << " LNz=" << Lvals->PLValigned->LN[i][2];
            AppendParityTrace("light_setup_raw", payload.str());
        }
    }

    SortLights(Lvals);

    // Diffuse and specular multipliers
    Lvals->sDiff = static_cast<float>(HLight.TBpos[2] * 0.02); // TBpos[5] in Pascal; Double literal
    Lvals->sSpec = MaxCS(0.004f, (HLight.TBpos[4] & 0xFFF) * 0.02f); // TBpos[7]
    if (Lvals->bCalcPixColSqr) {
        Lvals->sSpec = (Lvals->sSpec * Lvals->sSpec + Lvals->sSpec) * 0.5f;
        // BUG #17 fix: Pascal line 1485 — sDiff squared-color adjustment
        Lvals->sDiff = (Lvals->sDiff * Lvals->sDiff + Lvals->sDiff) * 0.5f;
    }

    // Color table setup
    for (int i = 0; i < 10; i++) {
        TSVec colDif{};
        TSVec colSpe{};
        PascalColToSVec(colDif, HLight.LCols[i].ColorDif,
                        Lvals->bCalcPixColSqr);
        PascalColAToSVec(colSpe, HLight.LCols[i].ColorSpe,
                         Lvals->bCalcPixColSqr);
        std::memcpy(Lvals->PLValigned->ColDif[i], colDif, sizeof(colDif));
        std::memcpy(Lvals->PLValigned->ColSpe[i], colSpe, sizeof(colSpe));

        Lvals->ColPos[i] = HLight.LCols[i].Position;
    }

    for (int i = 0; i < 10; i++) {
        int j;
        if (i < 9) j = Lvals->ColPos[i + 1] - Lvals->ColPos[i];
        else j = 32767 - Lvals->ColPos[9];
        if (j > 1) Lvals->sCDiv[i] = 1.0f / j;
        else Lvals->sCDiv[i] = 1.0f;
    }

    // Inside colors
    for (int i = 0; i < 4; i++) {
        TSVec colInt{};
        PascalColAToSVec(colInt, HLight.ICols[i].Color,
                         Lvals->bCalcPixColSqr);
        std::memcpy(Lvals->PLValigned->ColInt[i], colInt, sizeof(colInt));
        Lvals->IColPos[i] = HLight.ICols[i].Position;
    }

    for (int i = 0; i < 4; i++) {
        int j;
        if (i < 3) j = Lvals->IColPos[i + 1] - Lvals->IColPos[i];
        else j = 32767 - Lvals->IColPos[3];
        if (j > 1) Lvals->sICDiv[i] = 1.0f / j;
        else Lvals->sICDiv[i] = 1.0f;
    }

    // NI1: Fog colors - squared when bCalcPixColSqr
    if (Lvals->bCalcPixColSqr) {
        float r = static_cast<float>(HLight.DynFogR);
        float g = static_cast<float>(HLight.DynFogG);
        float b = static_cast<float>(HLight.DynFogB);
        Lvals->PLValigned->sDynFogCol[0] = r * r / 255.0f;
        Lvals->PLValigned->sDynFogCol[1] = g * g / 255.0f;
        Lvals->PLValigned->sDynFogCol[2] = b * b / 255.0f;
    } else {
        Lvals->PLValigned->sDynFogCol[0] = static_cast<float>(HLight.DynFogR);
        Lvals->PLValigned->sDynFogCol[1] = static_cast<float>(HLight.DynFogG);
        Lvals->PLValigned->sDynFogCol[2] = static_cast<float>(HLight.DynFogB);
    }
    Lvals->PLValigned->sDynFogCol[3] = 0;

    // Ambient color (raw bytes, not normalized)
    float ambMul = (HLight.TBpos[5] & 0xFFF) / 90.0f; // TBpos[8]
    // I6: sqr adjustment for bCalcPixColSqr
    if (Lvals->bCalcPixColSqr) {
        ambMul = (ambMul * ambMul + ambMul) * 0.5f * sLightScale;
    }
    // NI1: Ambient color - squared when bCalcPixColSqr
    if (Lvals->bCalcPixColSqr) {
        float r = static_cast<float>(HLight.AmbCol[0]);
        float g = static_cast<float>(HLight.AmbCol[1]);
        float b = static_cast<float>(HLight.AmbCol[2]);
        Lvals->PLValigned->sAmbCol[0] = r * r / 255.0f * ambMul;
        Lvals->PLValigned->sAmbCol[1] = g * g / 255.0f * ambMul;
        Lvals->PLValigned->sAmbCol[2] = b * b / 255.0f * ambMul;
    } else {
        Lvals->PLValigned->sAmbCol[0] = static_cast<float>(HLight.AmbCol[0]) * ambMul;
        Lvals->PLValigned->sAmbCol[1] = static_cast<float>(HLight.AmbCol[1]) * ambMul;
        Lvals->PLValigned->sAmbCol[2] = static_cast<float>(HLight.AmbCol[2]) * ambMul;
    }
    Lvals->PLValigned->sAmbCol[3] = 0;

    // NI1: Depth colors - squared when bCalcPixColSqr
    if (Lvals->bCalcPixColSqr) {
        float r = static_cast<float>(HLight.DepthCol[0]);
        float g = static_cast<float>(HLight.DepthCol[1]);
        float b = static_cast<float>(HLight.DepthCol[2]);
        Lvals->PLValigned->sDepthCol[0] = r * r / 255.0f;
        Lvals->PLValigned->sDepthCol[1] = g * g / 255.0f;
        Lvals->PLValigned->sDepthCol[2] = b * b / 255.0f;
    } else {
        Lvals->PLValigned->sDepthCol[0] = static_cast<float>(HLight.DepthCol[0]);
        Lvals->PLValigned->sDepthCol[1] = static_cast<float>(HLight.DepthCol[1]);
        Lvals->PLValigned->sDepthCol[2] = static_cast<float>(HLight.DepthCol[2]);
    }
    Lvals->PLValigned->sDepthCol[3] = 0;

    Lvals->sColZmul = HLight.VarColZpos * -0.005f / static_cast<float>(Header->dStepWidth * Header->Width);
    Lvals->sStepWidth = static_cast<float>(Header->dStepWidth);
    Lvals->sRoughnessFactor = HLight.RoughnessFactor * (1.0f / 255.0f) * (1.0f / 255.0f);
    Lvals->sZZstmitDif = static_cast<float>(Header->dZstart - Header->dZmid);

    Lvals->SortTab[6] = 6;
    Lvals->SortTab[7] = 7;
}

// ============================================================================
// GetHAddOnFromInternFormula
// From HeaderTrafos.pas lines 238-281
// Sets byOptionType[], iOptionCount, dOptionValue[] defaults, and CustomFname
// for an internal formula (0..9) in the addon's formula slot.
// Combines SetCFoptionsFromOldF (CustomFormulas.pas lines 237-311) inline,
// since C++ does not use TCustomFormula at load time.
// ============================================================================
void GetHAddOnFromInternFormula(THeaderCustomAddon* pAddon, int f, int t) {
    // oa[0..9][0..1]: default option values per formula
    // From HeaderTrafos.pas line 242
    static const double oa[10][2] = {
        {8, -1},   // 0: Integer Power
        {8, -1},   // 1: Real Power
        {-1, 0},   // 2: Quaternion
        {-2, 1},   // 3: Tricorn
        {2, 0.5},  // 4: Amazing Box
        {2, 0.5},  // 5: Bulbox
        {2, -1},   // 6: Folding Int Pow
        {-1, 0},   // 7: test (hybrid)
        {0, 0},    // 8: testIFS
        {8, 1}     // 9: Aexion C
    };

    // testhybridOptionCount/Types/Vals from formulas.pas (active variant: X+SinY)
    static const int testhybridOptionCount = 6;
    static const int testhybridOptionTypes[6] = {2, 2, 1, 1, 1, 1};
    static const double testhybridOptionVals[6] = {1, 2, 0, 1, 1, 0};

    // testIFSOptionCount/Types/Vals from formulas.pas (active variant: Plane with otrap)
    static const int testIFSOptionCount = 10;
    static const int testIFSOptionTypes[10] = {0, 0, 0, 0, 0, 2, 2, 1, 1, 2};
    static const double testIFSOptionVals[10] = {0, 1, 0, 0, 1, 0, 0, 0, 1, 0};

    if (f > INTERN_FORMULA_MAX) return;

    THAformula& fa = pAddon->Formulas[t];
    fa.iFnr = f;

    // --- Inline SetCFoptionsFromOldF: set byOptionType[] and iOptionCount ---
    // From CustomFormulas.pas lines 237-311
    // First, clear all option types to 0 (=Double)
    for (int i = 0; i < 16; i++) fa.byOptionType[i] = 0;

    // IntPow and FoldingIntPow: option 0 is IntPow=NoVar (type 10)
    if (f == 0 || f == 6) fa.byOptionType[0] = 10;
    // AmazingBox and Bulbox: option 1 is BoxScale (type 7)
    if (f == 4 || f == 5) fa.byOptionType[1] = 7;

    int optCount = 2; // default

    if (f == 4) {
        // Amazing Box
        optCount = 3;
        fa.byOptionType[2] = 11; // Folding16 R,R,-R,-R
    } else if (f == 5) {
        // Bulbox
        optCount = 6;
        fa.byOptionType[2] = 11; // Folding16
        fa.byOptionType[3] = 0;  // Double
        fa.byOptionType[4] = 9;  // Squared
        fa.byOptionType[5] = 9;  // Squared
    } else if (f == 6) {
        // Folding Int Pow
        optCount = 3;
        fa.byOptionType[2] = 8;
    } else if (f == 7) {
        // test (hybrid)
        optCount = testhybridOptionCount;
        for (int i = 0; i < optCount; i++) {
            fa.byOptionType[i] = static_cast<uint8_t>(testhybridOptionTypes[i]);
        }
    } else if (f == 8) {
        // testIFS
        optCount = testIFSOptionCount;
        for (int i = 0; i < optCount; i++) {
            fa.byOptionType[i] = static_cast<uint8_t>(testIFSOptionTypes[i]);
        }
    } else if (f == 9) {
        // Aexion C
        optCount = 8;
        fa.byOptionType[2] = 2; // Integer
        fa.byOptionType[3] = 2; // Integer
        fa.byOptionType[6] = 2; // Integer
        fa.byOptionType[7] = 2; // Integer
    }

    fa.iOptionCount = optCount;

    // --- Set default option values from oa[] ---
    // From HeaderTrafos.pas lines 250-251
    for (int i = 0; i < 2; i++) fa.dOptionValue[i] = oa[f][i];
    for (int i = 2; i < 16; i++) fa.dOptionValue[i] = 0;

    // --- Formula-specific default overrides ---
    // From HeaderTrafos.pas lines 254-277
    if (f == 4) {
        fa.dOptionValue[2] = 1; // Box fold
    } else if (f == 5) {
        // BulBox
        fa.dOptionValue[2] = 1;
        fa.dOptionValue[3] = 1;
        fa.dOptionValue[4] = 2;
        fa.dOptionValue[5] = 2;
    } else if (f == 6) {
        fa.dOptionValue[2] = 2;
    } else if (f == 7) {
        for (int i = 0; i < testhybridOptionCount; i++)
            fa.dOptionValue[i] = testhybridOptionVals[i];
    } else if (f == 8) {
        for (int i = 0; i < testIFSOptionCount; i++)
            fa.dOptionValue[i] = testIFSOptionVals[i];
    } else if (f == 9) {
        // Aexion C
        fa.dOptionValue[2] = 1;
        fa.dOptionValue[3] = 1;
        fa.dOptionValue[4] = 8;
        fa.dOptionValue[5] = 1;
    }

    // --- Set CustomFname from InternFormulaNames ---
    // From HeaderTrafos.pas line 278: PutStringInCustomF(CustomFname, InternFormulaNames[f])
    std::memset(fa.CustomFname, 0, 32);
    const std::string& name = InternFormulaNames[static_cast<size_t>(f)];
    size_t len = std::min(name.size(), static_cast<size_t>(31));
    for (size_t i = 0; i < len; i++) {
        fa.CustomFname[i] = static_cast<uint8_t>(name[i]);
    }
    // CFdescription is a TCustomFormula field, not in THAformula — skipped
}

} // namespace mb3d
