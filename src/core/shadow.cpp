#include "core/render_control.h"
#include "core/shadow.h"
#include "core/ray_marcher.h"
#include "core/math3d.h"
#include "core/packed_math.h"
#include "core/parity_trace.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <sstream>
#include <iomanip>

namespace mb3d {

// ============================================================================
// HSminLengthToCutPlane
// Ported from CalcHardShadow.pas THardShadowCalcThread.HSminLengthToCutPlane
//
// Limits the shadow ray length at cutting planes.
// ============================================================================

static void HSminLengthToCutPlane(const void* direction, double& dLength,
                                   const void* position, TMCTparameter* mcp) {
    TVec3D HVec, vPos;
    std::memcpy(HVec, direction, sizeof(HVec));
    std::memcpy(vPos, position, sizeof(vPos));
    if ((mcp->iCutOptions & 1) != 0 && std::fabs(HVec[0]) > 1e-20) {
        double dTmp = (vPos[0] - mcp->dCOX) / HVec[0];
        if (dTmp > 0 && dTmp < dLength) dLength = dTmp;
    }
    if ((mcp->iCutOptions & 2) != 0 && std::fabs(HVec[1]) > 1e-20) {
        double dTmp = (vPos[1] - mcp->dCOY) / HVec[1];
        if (dTmp > 0 && dTmp < dLength) dLength = dTmp;
    }
    if ((mcp->iCutOptions & 4) != 0 && std::fabs(HVec[2]) > 1e-20) {
        double dTmp = (vPos[2] - mcp->dCOZ) / HVec[2];
        if (dTmp > 0 && dTmp < dLength) dLength = dTmp;
    }
}

static void CalcHSsoftPoint(
    TMCTparameter* mcp,
    TIteration3Dext* it3Dext,
    TsiLight5* psi,
    TLightVals* lvals,
    int y)
{
    float ZZ = static_cast<float>(std::fabs(mcp->mZZ));
    int iHSnr = 0;
    for (int itmp = 0; itmp < 6; itmp++) {
        if ((mcp->calcHardShadow & (4 << itmp)) != 0) {
            iHSnr = lvals->SortTab[itmp];
        }
    }

    TVec3D HSVec;
    std::memcpy(HSVec, mcp->HSvecs[iHSnr], sizeof(TVec3D));

    bool isPosLight = (lvals->iLightPos[iHSnr] & 1) != 0;
    float ZRSmul;
    if (isPosLight) {
        ZRSmul = ((lvals->iLightPos[iHSnr] & 6) == 2)
            ? 70.0f / mcp->SoftShadowRadius
            : 40.0f / mcp->SoftShadowRadius;
    } else {
        ZRSmul = 80.0f / mcp->SoftShadowRadius;
    }

    psi->Shadow |= 0xFC00;

    double MaxLHS = (mcp->iMandWidth + y) * 0.6 *
                    (1.0 + s05f * MinCD(ZZ, mcp->Zend * 0.4) * Clamp0D(mcp->FOVy) / mcp->iMandHeight) *
                    mcp->sHSmaxLengthMultiplier;
    double ZZ2 = mcp->mZZ;
    mcp->msDEstop = mcp->DEstop * (1.0f + ZZ * mcp->mctDEstopFactor);
    double dMaxL = MaxLHS;

    if (isPosLight) {
        TVec3D lightPos;
        AddSVecToPackedVec3D(lvals->PLValigned->LN[iHSnr], &mcp->Xmit, lightPos);
        SubtractPackedVec3D(HSVec, lightPos, &it3Dext->C1);
        double dTmp = SqrLengthOfPackedVec(HSVec);
        if (dTmp > lvals->sLmaxL[iHSnr] * mcp->sHSmaxLengthMultiplier) {
            return;
        }
        if (dTmp < std::pow(dMaxL * mcp->StepWidth, 2.0)) {
            dMaxL = std::sqrt(dTmp) / mcp->StepWidth;
        }
        NormalizePackedVectorTo(-mcp->StepWidth, HSVec);
    }

    double ZZ2mul = -DotOfPackedVectorsNormalize(HSVec, mcp->mVgradsFOV);
    if (mcp->iCutOptions != 0) {
        HSminLengthToCutPlane(HSVec, dMaxL, &it3Dext->C1, mcp);
    }
    if (dMaxL <= 0.0) {
        return;
    }

    TVec3D NVec;
    MakeDVecFromNormals(psi, NVec);
    RotatePackedVectorReverse(NVec, mcp->Vgrads);
    if (DotPackedVec3D(NVec, HSVec) > 0.0) {
        psi->Shadow &= 0x03FF;
        return;
    }

    TVec3D IC;
    std::memcpy(IC, &it3Dext->C1, sizeof(TVec3D));
    double dT1 = dMaxL;
    float ZRsoft = 1.0f;
    float RStepFactorDiff = 1.0f;  // Pascal Calc.pas CalcHS uses 1 for single-pixel
    double dTmp = mcp->CalcDE(it3Dext, mcp);

    while (dT1 >= 0.0) {
        RenderPoll();
        double RLastDE = dTmp;
        // Pascal Calc.pas CalcHS: MinCS(dTmp * sZstepDiv * RStepFactorDiff, MaxCS(msDEstop, 0.4) * mctMH04ZSD)
        // Single-pixel formula does NOT use MaxCS(s011) or msDEsub subtraction
        float RLastStepWidth = MinCS(
            static_cast<float>(dTmp * mcp->sZstepDiv * RStepFactorDiff),
            MaxCS(mcp->msDEstop, 0.4f) * mcp->mctMH04ZSD);
        dT1 -= RLastStepWidth;
        AddPackedVecWeight(&it3Dext->C1, HSVec, -RLastStepWidth);
        ZZ2 += RLastStepWidth * ZZ2mul;
        mcp->msDEstop = mcp->DEstop * (1.0f + static_cast<float>(std::fabs(ZZ2)) * mcp->mctDEstopFactor);
        dTmp = mcp->CalcDE(it3Dext, mcp);

        float frac = static_cast<float>((dMaxL - dT1) / MaxLHS);
        float threshold = frac * frac;
        threshold *= threshold;
        threshold *= threshold;
        ZRsoft = MinCS(
            ZRsoft,
            static_cast<float>((dTmp - mcp->msDEstop) * ZRSmul / (dMaxL - dT1 + s011f) + threshold));

        if (it3Dext->ItResultI >= mcp->MaxItsResult || dTmp <= mcp->msDEstop) {
            break;
        }
        if (dTmp > RLastDE + RLastStepWidth) {
            dTmp = RLastDE + RLastStepWidth;
        }
        if (RLastDE > dTmp + 1e-30) {
            float sTmp = RLastStepWidth / static_cast<float>(RLastDE - dTmp);
            RStepFactorDiff = (sTmp < 1.0f) ? MaxCS(s05f, sTmp) : 1.0f;
        } else {
            RStepFactorDiff = 1.0f;
        }
    }

    psi->Shadow = static_cast<uint16_t>((psi->Shadow & 0x03FF) |
                                        (static_cast<uint16_t>(std::round(Clamp01S(ZRsoft) * 63.4f)) << 10));
    std::memcpy(&it3Dext->C1, IC, sizeof(TVec3D));
}

static void CalcHShardPoint(
    TMCTparameter* mcp,
    TIteration3Dext* it3Dext,
    TsiLight5* psi,
    TLightVals* lvals,
    int y)
{
    float ZZ = static_cast<float>(std::fabs(mcp->mZZ));
    TVec3D IC;
    std::memcpy(IC, &it3Dext->C1, sizeof(TVec3D));

    psi->Shadow = static_cast<uint16_t>(psi->Shadow &
        static_cast<uint16_t>(((mcp->calcHardShadow & 0xFC) << 8) ^ 0xFFFF));

    double MaxLHS = (mcp->iMandWidth + y) * 0.6 *
                    (1.0 + s05f * MinCD(ZZ, mcp->Zend * 0.4) * Clamp0D(mcp->FOVy) / mcp->iMandHeight) *
                    mcp->sHSmaxLengthMultiplier;

    for (int itmp = 0; itmp < 6; itmp++) {
        if ((mcp->calcHardShadow & (4 << itmp)) == 0) continue;

        int itmp2 = lvals->SortTab[itmp];
        std::memcpy(&it3Dext->C1, IC, sizeof(TVec3D));
        double ZZ2 = mcp->mZZ;
        mcp->msDEstop = mcp->DEstop * (1.0f + ZZ * mcp->mctDEstopFactor);
        double dT1 = MaxLHS;

        if ((lvals->iLightPos[itmp2] & 1) != 0) {
            TVec3D lightPos;
            AddSVecToPackedVec3D(lvals->PLValigned->LN[itmp2], &mcp->Xmit, lightPos);
            SubtractPackedVec3D(mcp->HSvecs[itmp2], lightPos, &it3Dext->C1);
            double dTmp = SqrLengthOfPackedVec(mcp->HSvecs[itmp2]);
            if (dTmp > lvals->sLmaxL[itmp2] * mcp->sHSmaxLengthMultiplier) {
                psi->Shadow |= static_cast<uint16_t>(0x400 << itmp);
                continue;
            }
            if (dTmp < std::pow(dT1 * mcp->StepWidth, 2.0)) {
                dT1 = std::sqrt(dTmp) / mcp->StepWidth;
            }
            NormalizePackedVectorTo(-mcp->StepWidth, mcp->HSvecs[itmp2]);
        }

        double ZZ2mul = -DotOfPackedVectorsNormalize(mcp->HSvecs[itmp2], mcp->mVgradsFOV);
        if (mcp->iCutOptions != 0) {
            HSminLengthToCutPlane(mcp->HSvecs[itmp2], dT1, &it3Dext->C1, mcp);
        }
        if (dT1 <= 0.0) {
            continue;
        }

        TVec3D HSVec;
        MakeDVecFromNormals(psi, HSVec);
        RotatePackedVectorReverse(HSVec, mcp->Vgrads);
        if (DotPackedVec3D(HSVec, mcp->HSvecs[itmp2]) > 0.0) {
            psi->Shadow |= static_cast<uint16_t>(0x400 << itmp);
            continue;
        }

        float RStepFactorDiff = 1.0f;
        double dTmp = mcp->CalcDE(it3Dext, mcp);
        while (dT1 >= 0.0) {
            RenderPoll();
            double RLastDE = dTmp;
            // Pascal Calc.pas CalcHS single-pixel: MinCS(dTmp * sZstepDiv * RStepFactorDiff, MaxCS(msDEstop, 0.4) * mctMH04ZSD)
            // No MaxCS(s011) wrapper, no msDEsub subtraction
            dTmp = MinCS(
                static_cast<float>(dTmp * mcp->sZstepDiv * RStepFactorDiff),
                MaxCS(mcp->msDEstop, 0.4f) * mcp->mctMH04ZSD);
            float RLastStepWidth = static_cast<float>(dTmp);
            dT1 -= dTmp;
            AddPackedVecWeight(&it3Dext->C1, mcp->HSvecs[itmp2], -dTmp);
            ZZ2 += dTmp * ZZ2mul;
            mcp->msDEstop = mcp->DEstop * (1.0f + static_cast<float>(std::fabs(ZZ2)) * mcp->mctDEstopFactor);
            dTmp = mcp->CalcDE(it3Dext, mcp);

            if (it3Dext->ItResultI >= mcp->MaxItsResult || dTmp < mcp->msDEstop) {
                break;
            }
            if (dTmp > RLastDE + RLastStepWidth) {
                dTmp = RLastDE + RLastStepWidth;
            }
            if (RLastDE > dTmp + 1e-30) {
                float sTmp = RLastStepWidth / static_cast<float>(RLastDE - dTmp);
                RStepFactorDiff = (sTmp < 1.0f) ? MaxCS(s05f, sTmp) : 1.0f;
            } else {
                RStepFactorDiff = 1.0f;
            }
        }

        if (dT1 > 0.0) {
            psi->Shadow |= static_cast<uint16_t>(0x400 << itmp);
        }
    }

    std::memcpy(&it3Dext->C1, IC, sizeof(TVec3D));
}

void CalcHardShadowForPoint(
    TMCTparameter* mcp,
    TIteration3Dext* it3Dext,
    TsiLight5* psi,
    TLightVals* lvals,
    int y)
{
    RenderPoll();
    if (!mcp || !it3Dext || !psi || !lvals) return;
    if ((mcp->calcHardShadow & 1) == 0) return;

    if ((mcp->calcHardShadow & 256) != 0) {
        CalcHSsoftPoint(mcp, it3Dext, psi, lvals, y);
    } else {
        CalcHShardPoint(mcp, it3Dext, psi, lvals, y);
    }
}

// ============================================================================
// calcHSsoft -- Soft shadow calculation (single light, 6-bit intensity)
// Ported from CalcHardShadow.pas THardShadowCalcThread.calcHSsoft
//
// When bCalc1HSsoft bit 1 is set, instead of per-light binary shadow bits,
// all 6 HS bits encode a single 6-bit intensity value based on closest
// approach distance along the shadow ray.
// ============================================================================

static void calcHSsoft(TMCTparameter* mcp, TIteration3Dext* it3Dext,
                        TsiLight5* siLightBase, int width, int height,
                        int slOffset, TLightVals* lvals,
                        int y_start = 0, int y_end = -1,
                        int x_start = 0, int x_end = -1) {
    if (y_end < 0) y_end = height;
    if (x_end < 0) x_end = width;
    LongBool bInsideTmp = mcp->bInsideRendering;

    // Find which light is the HS light
    int iHSnr = 0;
    for (int itmp = 0; itmp < 6; itmp++) {
        if ((mcp->calcHardShadow & (4 << itmp)) != 0) {
            iHSnr = lvals->SortTab[itmp];
        }
    }

    TVec3D HSVec;
    std::memcpy(HSVec, mcp->HSvecs[iHSnr], sizeof(TVec3D));

    bool bisPosLight = (lvals->iLightPos[iHSnr] & 1) != 0;
    float ZRSmul;
    if (bisPosLight) {
        if ((lvals->iLightPos[iHSnr] & 6) == 2)
            ZRSmul = 70.0f / mcp->SoftShadowRadius;
        else
            ZRSmul = 40.0f / mcp->SoftShadowRadius;
    } else {
        ZRSmul = 80.0f / mcp->SoftShadowRadius;
    }

    for (int y = y_start; y < y_end; y++) {
        RenderPoll();
        TsiLight5* mPsiLight = siLightBase + y * slOffset;
        const int screenY = y + mcp->CalcRect.Top;
        double CAFY = (static_cast<double>(screenY) / mcp->iMandHeight - 0.5) * mcp->FOVy;
        mcp->CAFY = CAFY;

        for (int x = x_start; x < x_end; x++) {
            RenderWork();
            // Set all HS bits by default (no shadow = fully lit)
            mPsiLight->Shadow = mPsiLight->Shadow | 0xFC00;

            if (mPsiLight->Zpos < 32768 && mPsiLight->SIgradient < 32768) {
                mcp->bInsideRendering = bInsideTmp;
                mcp->bCalcInside = bInsideTmp;

                const int screenX = x + mcp->CalcRect.Left;
                RMCalculateVgradsFOV(mcp, screenX + 1);
                RMCalculateStartPos(mcp, screenX, screenY);

                // Reconstruct Z position from encoded value
                uint32_t rawZ = (static_cast<uint32_t>(mPsiLight->Zpos) << 8) |
                                (mPsiLight->RoughZposFine & 0xFF);
                rawZ <<= 8;  // PInteger(@mPsiLight.RoughZposFine)^ shr 8
                // Actually, the Pascal code does: PInteger(@mPsiLight.RoughZposFine)^ shr 8
                // TsiLight5 layout: RoughZposFine(2 bytes) then Zpos(2 bytes)
                // So PInteger(@RoughZposFine)^ reads 4 bytes starting at RoughZposFine
                // which is RoughZposFine | (Zpos << 16), then shr 8 gives
                // (RoughZposFine >> 8) | (Zpos << 8)
                uint32_t packedZ;
                std::memcpy(&packedZ, &mPsiLight->RoughZposFine, 4);
                packedZ >>= 8;

                double mZZ = (std::pow((8388351.5 - static_cast<double>(packedZ)) / mcp->ZcMul + 1.0, 2.0) - 1.0) / mcp->Zcorr;
                mcp->mZZ = mZZ;
                AddPackedVecWeight(&it3Dext->C1, mcp->mVgradsFOV, mZZ);

                mcp->msDEstop = mcp->DEstop * (1.0f + static_cast<float>(mZZ) * mcp->mctDEstopFactor);
                double dTmp = mcp->CalcDE(it3Dext, mcp);

                if (mcp->bInAndOutside && (mPsiLight->OTrap & 0x8000) != 0) {
                    mcp->bInsideRendering = 0;
                    mcp->bCalcInside = 0;
                    dTmp = mcp->CalcDE(it3Dext, mcp);
                }

                bool DElimited = (it3Dext->ItResultI < mcp->MaxItsResult) || (dTmp < mcp->msDEstop);
                if (DElimited) {
                    // Finer Z reconstruction for bin search
                    uint32_t packedZ2;
                    std::memcpy(&packedZ2, &mPsiLight->RoughZposFine, 4);
                    packedZ2 >>= 8;
                    double dT1 = (std::pow((8388351.9 - static_cast<double>(packedZ2)) / mcp->ZcMul + 1.0, 2.0) - 1.0) / mcp->Zcorr - mZZ;
                    RMdoBinSearch(mcp, dTmp, dT1);
                } else {
                    RMdoBinSearchIt(mcp, &mZZ);
                }

                mZZ = mZZ - 0.1;  // Pascal: mZZ := mZZ - 0.1 (no clamping)
                AddPackedVecWeight(&it3Dext->C1, mcp->mVgradsFOV, -0.1);
                mcp->msDEstop = mcp->DEstop * (1.0f + static_cast<float>(mZZ) * mcp->mctDEstopFactor);

                float MaxLHS = static_cast<float>(
                    (mcp->iMandWidth + screenY) * 0.6 *
                    (1.0 + 0.5 * std::min(mZZ, mcp->Zend * 0.4) *
                     std::max(0.0, double(mcp->FOVy)) / mcp->iMandHeight) *
                    mcp->sHSmaxLengthMultiplier);

                double ZZ2 = mZZ;
                mcp->msDEstop = mcp->DEstop * (1.0f + static_cast<float>(mZZ) * mcp->mctDEstopFactor);
                double dMaxL = MaxLHS;

                if (bisPosLight) {
                    // Calculate LightVec from position
                    TVec3D lightPos;
                    AddSVecToPackedVec3D(lvals->PLValigned->LN[iHSnr],
                                 &mcp->Xmit, lightPos);
                    SubtractPackedVec3D(HSVec, lightPos, &it3Dext->C1);
                    dTmp = SqrLengthOfPackedVec(HSVec);
                    if (dTmp > lvals->sLmaxL[iHSnr] * mcp->sHSmaxLengthMultiplier) {
                        mPsiLight++;
                        continue;
                    }
                    if (dTmp < std::pow(dMaxL * mcp->StepWidth, 2.0))
                        dMaxL = std::sqrt(dTmp) / mcp->StepWidth;
                    NormalizePackedVectorTo(-mcp->StepWidth, HSVec);
                }

                double ZZ2mul = -DotOfPackedVectorsNormalize(HSVec, mcp->mVgradsFOV);
                if (mcp->iCutOptions != 0)
                    HSminLengthToCutPlane(HSVec, dMaxL, &it3Dext->C1, mcp);

                // Check dot product of normals and light vec
                TVec3D NVec;
                MakeDVecFromNormals(mPsiLight, NVec);
                RotatePackedVectorReverse(NVec, mcp->Vgrads);
                if (DotPackedVec3D(NVec, HSVec) > 0) {
                    mPsiLight->Shadow = mPsiLight->Shadow & 0x3FF;
                    mPsiLight++;
                    continue;
                }

                float ZRsoft = 1.0f;
                double dT1 = dMaxL;
                if (dT1 > 0) {
                    mcp->msDEstop = mcp->DEstop * (1.0f + static_cast<float>(std::fabs(ZZ2)) * mcp->mctDEstopFactor);
                    float RStepFactorDiff = 2.0f;
                    dTmp = mcp->CalcDE(it3Dext, mcp);

                    do {
                        RenderPoll();
                        double RLastDE = dTmp;
                        float RLastStepWidth = MinCS(
                            MaxCS(s011f, static_cast<float>((dTmp - mcp->msDEsub * mcp->msDEstop) * mcp->sZstepDiv * RStepFactorDiff)),
                            MaxCS(mcp->msDEstop, 0.4f) * mcp->mctMH04ZSD);
                        dT1 -= RLastStepWidth;
                        AddPackedVecWeight(&it3Dext->C1, HSVec, -static_cast<double>(RLastStepWidth));
                        ZZ2 += RLastStepWidth * ZZ2mul;
                        mcp->msDEstop = mcp->DEstop * (1.0f + static_cast<float>(std::fabs(ZZ2)) * mcp->mctDEstopFactor);
                        dTmp = mcp->CalcDE(it3Dext, mcp);

                        // Soft shadow: track closest approach
                        double dist = dMaxL - dT1 + static_cast<double>(s011f);
                        double threshold = std::pow(std::pow((dMaxL - dT1) / MaxLHS, 2.0), 4.0); // Sqr(Sqr(Sqr(x)))
                        // Actually: Sqr(Sqr(Sqr(x))) = x^8
                        double frac = (dMaxL - dT1) / static_cast<double>(MaxLHS);
                        threshold = frac * frac;
                        threshold = threshold * threshold;
                        threshold = threshold * threshold;  // x^8
                        ZRsoft = MinCS(ZRsoft,
                            static_cast<float>((dTmp - mcp->msDEstop) * ZRSmul / dist + threshold));

                        if (it3Dext->ItResultI >= mcp->MaxItsResult || dTmp <= mcp->msDEstop) break;
                        if (dTmp > RLastDE + RLastStepWidth) dTmp = RLastDE + RLastStepWidth;
                        if (RLastDE > dTmp + 1e-30) {
                            float sT2 = RLastStepWidth / static_cast<float>(RLastDE - dTmp);
                            if (sT2 < 1.0f)
                                RStepFactorDiff = MaxCS(s05f, sT2);
                            else
                                RStepFactorDiff = 1.0f;
                        } else {
                            RStepFactorDiff = 1.0f;
                        }
                    } while (dT1 >= 0);
                }

                int softVal = static_cast<int>(std::round(Clamp01S(ZRsoft) * 63.4f));
                mPsiLight->Shadow = (mPsiLight->Shadow & 0x3FF) | (static_cast<uint16_t>(softVal) << 10);
            }
            mPsiLight++;
        }
    }

    mcp->bInsideRendering = bInsideTmp;
    mcp->bCalcInside = bInsideTmp;
}

// ============================================================================
// calcHShard -- Standard hard shadow calculation (per-light binary)
// Ported from CalcHardShadow.pas THardShadowCalcThread.Execute (main path)
//
// For each visible pixel, traces a shadow ray toward each enabled light.
// Sets bits 10-15 in the Shadow field if an object blocks the light.
// Includes the open-air threshold optimization with dLastZ tracking.
// ============================================================================

static void calcHShard(TMCTparameter* mcp, TIteration3Dext* it3Dext,
                             TsiLight5* siLightBase, int width, int height,
                             int slOffset, TLightVals* lvals,
                             int y_start = 0, int y_end = -1,
                             int x_start = 0, int x_end = -1) {
    if (y_end < 0) y_end = height;
    if (x_end < 0) x_end = width;
    LongBool bInsideTmp = mcp->bInsideRendering;

    for (int y = y_start; y < y_end; y++) {
        RenderPoll();
        TsiLight5* mPsiLight = siLightBase + y * slOffset;
        const int screenY = y + mcp->CalcRect.Top;
        double CAFY = (static_cast<double>(screenY) / mcp->iMandHeight - 0.5) * mcp->FOVy;
        mcp->CAFY = CAFY;

        // Per-row state for open-air optimization
        double dLastFound[6] = {0};
        int bZZoathr[6] = {0};
        double dLastZ = 0;

        for (int x = x_start; x < x_end; x++) {
            RenderWork();
            // Decrement threshold counters
            for (int itmp = 0; itmp < 6; itmp++) {
                if (bZZoathr[itmp] > 0) bZZoathr[itmp]--;
            }

            // Reset selected HS bits to 0 (no shadow)
            uint16_t resetMask = static_cast<uint16_t>(
                ((mcp->calcHardShadow & 0xFC) << 8) ^ 0xFFFF);
            mPsiLight->Shadow = mPsiLight->Shadow & resetMask;

            if (mPsiLight->Zpos < 32768 && mPsiLight->SIgradient < 32768) {
                // Extract normal vector
                TVec3D NVec;
                MakeDVecFromNormals(mPsiLight, NVec);
                RotatePackedVectorReverse(NVec, mcp->Vgrads);

                mcp->bInsideRendering = bInsideTmp;
                mcp->bCalcInside = bInsideTmp;

                const int screenX = x + mcp->CalcRect.Left;
                RMCalculateVgradsFOV(mcp, screenX + 1);
                RMCalculateStartPos(mcp, screenX, screenY);

                // Reconstruct Z position
                uint32_t packedZ;
                std::memcpy(&packedZ, &mPsiLight->RoughZposFine, 4);
                packedZ >>= 8;

                double mZZ = (std::pow((8388351.5 - static_cast<double>(packedZ)) / mcp->ZcMul + 1.0, 2.0) - 1.0) / mcp->Zcorr;
                mcp->mZZ = mZZ;
                AddPackedVecWeight(&it3Dext->C1, mcp->mVgradsFOV, mZZ);

                mcp->msDEstop = mcp->DEstop * (1.0f + static_cast<float>(mZZ) * mcp->mctDEstopFactor);
                double dTmp = mcp->CalcDE(it3Dext, mcp);

                if (mcp->bInAndOutside && (mPsiLight->OTrap & 0x8000) != 0) {
                    mcp->bInsideRendering = 0;
                    mcp->bCalcInside = 0;
                    dTmp = mcp->CalcDE(it3Dext, mcp);
                }

                bool DElimited = (it3Dext->ItResultI < mcp->MaxItsResult) || (dTmp < mcp->msDEstop);
                if (DElimited) {
                    uint32_t packedZ2;
                    std::memcpy(&packedZ2, &mPsiLight->RoughZposFine, 4);
                    packedZ2 >>= 8;
                    double dT1 = (std::pow((8388351.9 - static_cast<double>(packedZ2)) / mcp->ZcMul + 1.0, 2.0) - 1.0) / mcp->Zcorr - mZZ;
                    RMdoBinSearch(mcp, dTmp, dT1);
                } else {
                    RMdoBinSearchIt(mcp, &mZZ);
                }

                // Step forward to avoid self-intersection
                mZZ = mZZ - 0.1;  // Pascal: mZZ := mZZ - 0.1 (no clamping)
                AddPackedVecWeight(&it3Dext->C1, mcp->mVgradsFOV, -0.1);
                mcp->msDEstop = mcp->DEstop * (1.0f + static_cast<float>(mZZ) * mcp->mctDEstopFactor);

                // Save start position
                TVec3D IC;
                std::memcpy(IC, &it3Dext->C1, sizeof(TVec3D));

                // Calculate max shadow ray length
                float MaxLHS = static_cast<float>(
                    (mcp->iMandWidth + screenY) * 0.6 *
                    (1.0 + 0.5 * std::min(mZZ, mcp->Zend * 0.4) *
                     std::max(0.0, double(mcp->FOVy)) / mcp->iMandHeight) *
                    mcp->sHSmaxLengthMultiplier);

                // Open-air threshold
                double OAthr = (1.0 + (mZZ + MaxLHS) * mcp->mctDEstopFactor) * 3.3 / mcp->sZstepDiv;

                // Process each enabled light
                for (int itmp = 0; itmp < 6; itmp++) {
                    if ((mcp->calcHardShadow & (4 << itmp)) == 0) continue;

                    int itmp2 = lvals->SortTab[itmp];
                    const bool traceShadowPixel =
                        HasParityTraceContext() &&
                        x == GetParityTracePixelX() &&
                        y == GetParityTracePixelY();

                    // Open-air threshold optimization
                    bool bThr = bZZoathr[itmp] > 0;
                    if (bThr) {
                        bZZoathr[itmp] -= static_cast<int>(std::lrint(std::fabs(dLastZ - mZZ) * 3.3 / OAthr));
                        if (bZZoathr[itmp] <= 0) {
                            bThr = false;
                            bZZoathr[itmp] = 0;
                        } else {
                            dLastFound[itmp] = dLastFound[itmp] - std::fabs(dLastZ - mZZ) -
                                               (1.0 + mZZ * mcp->mctDEstopFactor);
                            if (dLastFound[itmp] <= 0) {
                                bThr = false;
                                bZZoathr[itmp] = 0;
                            }
                        }
                    }

                    // Restore start position
                    std::memcpy(&it3Dext->C1, IC, sizeof(TVec3D));
                    double ZZ2 = mZZ;
                    mcp->msDEstop = mcp->DEstop * (1.0f + static_cast<float>(mZZ) * mcp->mctDEstopFactor);
                    double dMaxL = static_cast<double>(MaxLHS);

                    TVec3D HSVec;

                    // Position light: calculate direction from surface to light
                    if ((lvals->iLightPos[itmp2] & 1) != 0) {
                        TVec3D lightPos;
                        AddSVecToPackedVec3D(lvals->PLValigned->LN[itmp2],
                                     &mcp->Xmit, lightPos);
                        SubtractPackedVec3D(HSVec, lightPos, &it3Dext->C1);
                        dTmp = SqrLengthOfPackedVec(HSVec);
                        if (dTmp > lvals->sLmaxL[itmp2] * mcp->sHSmaxLengthMultiplier) {
                            mPsiLight->Shadow = mPsiLight->Shadow | (0x400 << itmp);
                            continue;
                        }
                        if (dTmp < std::pow(dMaxL * mcp->StepWidth, 2.0))
                            dMaxL = std::sqrt(dTmp) / mcp->StepWidth;
                        NormalizePackedVectorTo(-mcp->StepWidth, HSVec);
                        std::memcpy(mcp->HSvecs[itmp2], HSVec, sizeof(TVec3D));
                    } else {
                        std::memcpy(HSVec, mcp->HSvecs[itmp2], sizeof(TVec3D));
                    }

                    double ZZ2mul = -DotOfPackedVectorsNormalize(mcp->HSvecs[itmp2], mcp->mVgradsFOV);
                    if (mcp->iCutOptions != 0)
                        HSminLengthToCutPlane(mcp->HSvecs[itmp2], dMaxL,
                                              &it3Dext->C1, mcp);

                    if (traceShadowPixel) {
                        std::ostringstream payload;
                        payload.setf(std::ios::fixed, std::ios::floatfield);
                        payload << std::setprecision(12)
                                << "itmp=" << itmp
                                << " sorted=" << itmp2
                                << " bThr=" << (bThr ? 1 : 0)
                                << " Shadow=" << mPsiLight->Shadow
                                << " dMaxL=" << dMaxL
                                << " OAthr=" << OAthr
                                << " ZZ2mul=" << ZZ2mul
                                << " iLightPos=" << lvals->iLightPos[itmp2]
                                << " iHScalced=" << lvals->iHScalced[itmp2]
                                << " iHSmask=" << lvals->iHSmask[itmp2]
                                << " iLightFuncDiff=" << lvals->iLightFuncDiff[itmp2]
                                << " HSVecX=" << mcp->HSvecs[itmp2][0]
                                << " HSVecY=" << mcp->HSvecs[itmp2][1]
                                << " HSVecZ=" << mcp->HSvecs[itmp2][2]
                                << " NDotL=" << DotPackedVec3D(NVec, mcp->HSvecs[itmp2]);
                        AppendParityTrace("shadow_light_begin", payload.str());
                    }

                    // Check if surface faces away from light
                    if (DotPackedVec3D(NVec, mcp->HSvecs[itmp2]) > 0) {
                        if (traceShadowPixel) {
                            std::ostringstream payload;
                            payload << "itmp=" << itmp
                                    << " sorted=" << itmp2
                                    << " reason=normal_dot"
                                    << " Shadow=" << (mPsiLight->Shadow | (0x400 << itmp));
                            AppendParityTrace("shadow_light_end", payload.str());
                        }
                        mPsiLight->Shadow = mPsiLight->Shadow | (0x400 << itmp);
                        bZZoathr[itmp] = 0;
                        continue;
                    }

                    double dT1 = dMaxL;
                    if (dT1 > 0) {
                        mcp->msDEstop = mcp->DEstop * (1.0f + static_cast<float>(std::fabs(ZZ2)) * mcp->mctDEstopFactor);
                        float RStepFactorDiff = 2.0f;
                        dTmp = mcp->CalcDE(it3Dext, mcp);

                        do {
                            RenderPoll();
                            if (bThr && dLastFound[itmp] > dT1) {
                                dT1 = -1.0;
                                break;
                            }

                            double RLastDE = dTmp;
                            dTmp = MinCS(
                                MaxCS(s011f, static_cast<float>((dTmp - mcp->msDEsub * mcp->msDEstop) * mcp->sZstepDiv * RStepFactorDiff)),
                                MaxCS(mcp->msDEstop, 0.4f) * mcp->mctMH04ZSD);
                            float RLastStepWidth = static_cast<float>(dTmp);
                            dT1 -= dTmp;
                            AddPackedVecWeight(&it3Dext->C1, mcp->HSvecs[itmp2], -dTmp);
                            ZZ2 += dTmp * ZZ2mul;
                            mcp->msDEstop = mcp->DEstop * (1.0f + static_cast<float>(std::fabs(ZZ2)) * mcp->mctDEstopFactor);
                            dTmp = mcp->CalcDE(it3Dext, mcp);

                            if (it3Dext->ItResultI >= mcp->MaxItsResult || dTmp <= mcp->msDEstop) break;
                            if (dTmp > RLastDE + RLastStepWidth) dTmp = RLastDE + RLastStepWidth;

                            if (!bThr) {
                                if (dTmp > OAthr) {
                                    if (bZZoathr[itmp] < 3) {
                                        dLastFound[itmp] = dT1;
                                        bZZoathr[itmp] = 3;
                                    }
                                } else {
                                    bZZoathr[itmp] = 0;
                                }
                            }

                            if (RLastDE > dTmp + 1e-30) {
                                float sT2 = RLastStepWidth / static_cast<float>(RLastDE - dTmp);
                                if (sT2 < 1.0f)
                                    RStepFactorDiff = MaxCS(s05f, sT2);
                                else
                                    RStepFactorDiff = 1.0f;
                            } else {
                                RStepFactorDiff = 1.0f;
                            }
                        } while (dT1 >= 0);
                    }

                    if (dT1 > 0) {
                        if (traceShadowPixel) {
                            std::ostringstream payload;
                            payload.setf(std::ios::fixed, std::ios::floatfield);
                            payload << std::setprecision(12)
                                    << "itmp=" << itmp
                                    << " sorted=" << itmp2
                                    << " reason=blocked"
                                    << " dT1=" << dT1
                                    << " Shadow=" << (mPsiLight->Shadow | (0x400 << itmp));
                            AppendParityTrace("shadow_light_end", payload.str());
                        }
                        mPsiLight->Shadow = mPsiLight->Shadow | (0x400 << itmp);
                        bZZoathr[itmp] = 0;
                    } else if (traceShadowPixel) {
                        std::ostringstream payload;
                        payload.setf(std::ios::fixed, std::ios::floatfield);
                        payload << std::setprecision(12)
                                << "itmp=" << itmp
                                << " sorted=" << itmp2
                                << " reason=clear"
                                << " dT1=" << dT1
                                << " Shadow=" << mPsiLight->Shadow;
                        AppendParityTrace("shadow_light_end", payload.str());
                    }
                } // end for each light

                dLastZ = mZZ;
            }
            mPsiLight++;
        }
    }

    mcp->bInsideRendering = bInsideTmp;
    mcp->bCalcInside = bInsideTmp;
}

// ============================================================================
// CalcHardShadows -- Main entry point
// Ported from CalcHardShadow.pas CalcHardShadowT + THardShadowCalcThread.Execute
//
// Dispatches to soft or hard shadow calculation based on header flags.
// Sets up iteration state and calls the appropriate implementation.
// ============================================================================

void CalcHardShadows(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals)
{
    RenderPoll();
    if (!siLight || !mcp || !lvals) return;
    if (width <= 0 || height <= 0) return;

    // Setup iteration state
    TIteration3Dext it3Dext;
    std::memset(&it3Dext, 0, sizeof(TIteration3Dext));
    mcp->pIt3Dext = &it3Dext;

    // Initialize iteration parameters (from Pascal: IniIt3D)
    IniIt3D(mcp, &it3Dext);

    // Extra steps for shadow DE (from Pascal: iDEAddSteps := 8)
    mcp->iDEAddSteps = 8;
    it3Dext.CalcSIT = 0;

    // Setup HS-related fields in lvals (from CalcHardShadowT)
    for (int x = 0; x < 6; x++) {
        int y = lvals->SortTab[x];
        lvals->iHScalced[y] = (mcp->calcHardShadow >> (x + 2)) & 1;
        lvals->iHSmask[y] = 0x400 << x;
        if ((mcp->calcHardShadow & 2) != 0 &&
            ((mcp->calcHardShadow >> (x + 2)) & 1) != 0) {
            lvals->iLightFuncDiff[y] = 0;
        }
        // Soft shadow: when enabled, all HS bits treated as one value
        if ((mcp->calcHardShadow & 256) != 0 && lvals->iHScalced[y] != 0) {
            lvals->iHSmask[y] = -1;
        }
    }

    // Point MCT params to the siLight buffer
    mcp->pSiLight = siLight;
    mcp->SLoffset = slOffset * static_cast<int>(sizeof(TsiLight5));
    mcp->PLVals = lvals;

    // Keep the full-frame origin of this tile. Worker ranges are buffer-local.
    mcp->CalcRect.Right = mcp->CalcRect.Left + width - 1;
    mcp->CalcRect.Bottom = mcp->CalcRect.Top + height - 1;

    // Dispatch: soft shadow (bit 8 set) vs hard shadow
    if ((mcp->calcHardShadow & 256) != 0) {
        calcHSsoft(mcp, &it3Dext, siLight, width, height, slOffset, lvals);
    } else {
        calcHShard(mcp, &it3Dext, siLight, width, height, slOffset, lvals);
    }
}

void CalcHardShadowsRows(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    int y_start, int y_end)
{
    RenderPoll();
    if (!siLight || !mcp || !lvals) return;
    if (width <= 0 || height <= 0) return;
    if (y_start >= y_end) return;

    // Setup iteration state (per-thread)
    TIteration3Dext it3Dext;
    std::memset(&it3Dext, 0, sizeof(TIteration3Dext));
    mcp->pIt3Dext = &it3Dext;

    IniIt3D(mcp, &it3Dext);

    mcp->iDEAddSteps = 8;
    it3Dext.CalcSIT = 0;

    // Setup HS-related fields in lvals
    for (int x = 0; x < 6; x++) {
        int y = lvals->SortTab[x];
        lvals->iHScalced[y] = (mcp->calcHardShadow >> (x + 2)) & 1;
        lvals->iHSmask[y] = 0x400 << x;
        if ((mcp->calcHardShadow & 2) != 0 &&
            ((mcp->calcHardShadow >> (x + 2)) & 1) != 0) {
            lvals->iLightFuncDiff[y] = 0;
        }
        if ((mcp->calcHardShadow & 256) != 0 && lvals->iHScalced[y] != 0) {
            lvals->iHSmask[y] = -1;
        }
    }

    mcp->pSiLight = siLight;
    mcp->SLoffset = slOffset * static_cast<int>(sizeof(TsiLight5));
    mcp->PLVals = lvals;

    mcp->CalcRect.Right = mcp->CalcRect.Left + width - 1;
    mcp->CalcRect.Bottom = mcp->CalcRect.Top + height - 1;

    if ((mcp->calcHardShadow & 256) != 0) {
        calcHSsoft(mcp, &it3Dext, siLight, width, height, slOffset, lvals, y_start, y_end);
    } else {
        calcHShard(mcp, &it3Dext, siLight, width, height, slOffset, lvals, y_start, y_end);
    }
}

void ReplayHardShadowsPrefix(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    TLightVals* lvals,
    int target_x,
    int target_y)
{
    if (!siLight || !mcp || !lvals) return;
    if (width <= 0 || height <= 0) return;
    if (target_x < 0 || target_x >= width) return;
    if (target_y < 0 || target_y >= height) return;

    TIteration3Dext it3Dext;
    std::memset(&it3Dext, 0, sizeof(TIteration3Dext));
    mcp->pIt3Dext = &it3Dext;

    IniIt3D(mcp, &it3Dext);
    mcp->iDEAddSteps = 8;
    it3Dext.CalcSIT = 0;

    for (int x = 0; x < 6; x++) {
        int y = lvals->SortTab[x];
        lvals->iHScalced[y] = (mcp->calcHardShadow >> (x + 2)) & 1;
        lvals->iHSmask[y] = 0x400 << x;
        if ((mcp->calcHardShadow & 2) != 0 &&
            ((mcp->calcHardShadow >> (x + 2)) & 1) != 0) {
            lvals->iLightFuncDiff[y] = 0;
        }
        if ((mcp->calcHardShadow & 256) != 0 && lvals->iHScalced[y] != 0) {
            lvals->iHSmask[y] = -1;
        }
    }

    mcp->pSiLight = siLight;
    mcp->SLoffset = slOffset * static_cast<int>(sizeof(TsiLight5));
    mcp->PLVals = lvals;
    // Prefix selection is local; retain the scene/tile camera origin.

    if ((mcp->calcHardShadow & 256) != 0) {
        calcHSsoft(mcp, &it3Dext, siLight, width, height, slOffset, lvals,
                   target_y, target_y + 1, 0, target_x + 1);
    } else {
        calcHShard(mcp, &it3Dext, siLight, width, height, slOffset, lvals,
                   target_y, target_y + 1, 0, target_x + 1);
    }
}

} // namespace mb3d
