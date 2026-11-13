#include "core/render_control.h"
#include "core/calc_2d.h"
#include "core/ray_marcher.h"
#include "core/math3d.h"
#include "core/packed_math.h"
#include <cmath>
#include <cstring>
#include <algorithm>

namespace mb3d {

// ============================================================================
// MinMaxClip15bit -- clamp float to [0, 32767] and store as uint16
// ============================================================================

// ============================================================================
// Calc2DSliceRows -- process a range of rows for 2D slice rendering
// Ported from CalcThread2D.pas: T2DcalcThread.Execute
// ============================================================================

void Calc2DSliceRows(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp,
    int y_start, int y_end)
{
    TIteration3Dext it3Dext;
    std::memset(&it3Dext, 0, sizeof(TIteration3Dext));
    IniIt3D(mcp, &it3Dext);

    mcp->bInsideRendering = 0;
    mcp->bCalcInside = 0;

    // slOffset is in TsiLight5 units (number of TsiLight5 elements per row)
    // In the Pascal code: mPsiLight := TPsiLight5(Integer(pSiLight) + (y - CalcRect.Top) * SLoffset)
    // Here slOffset = width (elements per row)

    for (int y = y_start; y < y_end; y++) {
        RenderPoll();
        TsiLight5* rowSI = siLight + y * slOffset;

        // CC = Ystart + Vgrads[1] * y
        TVec3D CC;
        CopyAddPackedVecWeight(CC, mcp->Ystart, mcp->Vgrads[1], static_cast<double>(y + mcp->CalcRect.Top));

        for (int x = 0; x < width; x++) {
            RenderWork();
            TsiLight5* psi = &rowSI[x];

            it3Dext.CalcSIT = 1; // True

            // Initialize pixel data to default values
            // PInteger(@mPsiLight.NormalX)^ := 0;  -> NormalX=0, NormalY=0
            psi->NormalX = 0;
            psi->NormalY = 0;

            // PInteger(@mPsiLight.RoughZposFine)^ := $4E200000;
            // This sets RoughZposFine and Zpos to specific defaults
            // $4E200000 = RoughZposFine=0x0000, Zpos=0x4E20 (20000)
            psi->RoughZposFine = 0x0000;
            psi->Zpos = 0x4E20;

            // PInteger(@mPsiLight.Shadow)^ := $13880010;
            // Shadow=0x0010 (16), AmbShadow=0x1388 (5000)
            psi->Shadow = 0x0010;
            psi->AmbShadow = 0x1388;

            psi->NormalZ = -32768;

            // C1,C2,C3 = CC + Vgrads[0] * x
            CopyAddPackedVecWeight(&it3Dext.C1, CC, mcp->Vgrads[0], static_cast<double>(x + mcp->CalcRect.Left));

            // Evaluate fractal formula
            if (mcp->DEoption > 19) {
                if (mcp->mMandFunctionDE) {
                    mcp->mMandFunctionDE(reinterpret_cast<TPIteration3D>(&it3Dext.C1));
                }
            } else {
                if (mcp->mMandFunction) {
                    mcp->mMandFunction(reinterpret_cast<TPIteration3D>(&it3Dext.C1));
                }
            }

            // Color based on iteration
            if (mcp->ColorOnIt != 0) {
                mcp->pIt3Dext = &it3Dext;
                mcp->mPsiLight = psi;
                RMdoColorOnIt(mcp);
            }

            // Color based on orbit trap
            mcp->pIt3Dext = &it3Dext;
            mcp->mPsiLight = psi;
            RMdoColor(mcp);

            if (mcp->FormulaType > 0) {
                // Color on DE
                float s = std::fabs(static_cast<float>(
                    mcp->CalcDE(&it3Dext, mcp) * 40.0));
                if (it3Dext.ItResultI < mcp->MaxItsResult) {
                    MinMaxClip15bit(s, psi->SIgradient);
                    psi->NormalY = 5000;
                } else {
                    psi->SIgradient = static_cast<uint16_t>(
                        std::lround(Min0MaxCS(s, 32767.0f))) + 32768;
                }
            } else {
                // Color on iteration count
                if (it3Dext.ItResultI < mcp->iMaxIt) {
                    float s = it3Dext.SmoothItD * 32767.0f / mcp->iMaxIt;
                    MinMaxClip15bit(s, psi->SIgradient);
                    psi->NormalY = 5000;
                } else {
                    psi->SIgradient = psi->OTrap + 32768;
                }
            }
        }
    }
}

// ============================================================================
// Calc2DSlice -- full image 2D slice rendering
// ============================================================================

void Calc2DSlice(
    TsiLight5* siLight,
    int width, int height,
    int slOffset,
    TMCTparameter* mcp)
{
    Calc2DSliceRows(siLight, width, height, slOffset, mcp, 0, height);
}

} // namespace mb3d
