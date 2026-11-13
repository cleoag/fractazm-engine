#ifndef MB3D_CORE_TYPES_H
#define MB3D_CORE_TYPES_H

#include <cstdint>
#include <cstring>
#include <cmath>

// ============================================================================
// Basic type aliases matching Pascal/Delphi types
// ============================================================================

// Pascal Shortint = signed 8-bit, Byte = unsigned 8-bit
// Pascal Smallint = signed 16-bit, Word = unsigned 16-bit
// Pascal Integer = signed 32-bit, Cardinal = unsigned 32-bit
// Pascal Single = 32-bit float, Double = 64-bit float
// Pascal LongBool = 32-bit boolean (0 = false, nonzero = true)
// Pascal WordBool = 16-bit boolean
// Pascal ByteBool = 8-bit boolean

using LongBool = int32_t;
using WordBool = int16_t;
using ByteBool = uint8_t;

// ============================================================================
// Math3D basic types (from Math3D.pas)
// ============================================================================

using TVec3D   = double[3];
using TVec4D   = double[4];
using TPos3D   = double[3];
using TSVec    = float[4];       // 4 floats, [3] often 0
using TSPoint  = float[2];

using TMatrix3   = double[3][3]; // [row][col]
using TSMatrix2  = float[2][2];
using TSMatrix3  = float[3][4];  // 3 rows x 4 cols (Pascal: array[0..2, 0..3])
using TSMatrix4  = float[4][4];

using TQuaternion = double[4];
using TComplex    = double[2];

using ShortFloat = int8_t[2];    // [0]=mantissa, [1]=exponent, both SIGNED
using Double7B   = uint8_t[7];

using T4Cardinal = uint32_t[4];
using T4SVec     = TSVec[4];
using T3SVec     = TSVec[3];
using TLNormals  = int16_t[3];

// Pointer type aliases (for documentation, in C++ these are just pointers)
using TPVec3D     = TVec3D*;
using TPVec4D     = TVec4D*;
using TPSVec      = TSVec*;
using TPPos3D     = TPos3D*;
using TPMatrix3   = TMatrix3*;
using TPSMatrix2  = TSMatrix2*;
using TPSMatrix3  = TSMatrix3*;
using TPSMatrix4  = TSMatrix4*;
using TPQuaternion = TQuaternion*;
using TPLNormals  = TLNormals*;
using TPSPoint    = TSPoint*;

// ============================================================================
// TypeDefinitions.pas types
// ============================================================================

using TRGB  = uint8_t[3];
using TRGBA = uint8_t[4];
using T3word = uint16_t[3];

// ============================================================================
// Function pointer types
// ============================================================================

// Forward declarations
struct TIteration3Dext;
struct TIteration3D;

using TPIteration3Dext = TIteration3Dext*;
using TPIteration3D    = TIteration3D*;

// ThybridIteration2 = procedure(var x, y, z, w: Double; PIteration3D: Pointer);
using ThybridIteration2 = void (*)(double& x, double& y, double& z, double& w, void* PIteration3D);

// TFormulaInitialization = procedure(PIteration3D: Pointer);
using TFormulaInitialization = void (*)(void* PIteration3D);

// TMandFunction = procedure(PIteration3D: TPIteration3D);
using TMandFunction = void (*)(TPIteration3D PIteration3D);

// TMandFunctionDE = function(PIteration3D: TPIteration3D): Double;
using TMandFunctionDE = double (*)(TPIteration3D PIteration3D);

// TCaldDEfunction = function(It3Dex: TPIteration3Dext; mctp: Pointer): Double;
using TCaldDEfunction = double (*)(TPIteration3Dext It3Dex, void* mctp);

// TLMSfunction = function(PVec3D: TPVec3D; MapNr: Integer): TVec3D;
// This returns by value in Pascal -- in C++ we use a struct wrapper or pointer
// For simplicity, use a function pointer that takes output param
using TLMSfunction = void (*)(TPVec3D result, TPVec3D PVec3D, int32_t MapNr);

// ThybridIteration (old style)
using ThybridIteration = void (*)(double& x, double& y, double& z, double& w, TPIteration3D PIt3D);

// ============================================================================
// Packed structures -- DISK FORMAT and runtime
// ============================================================================

#pragma pack(push, 1)

// ----------------------------------------------------------------------------
// TsiLight5 -- 18 bytes (surface lighting data per pixel)
// ----------------------------------------------------------------------------
struct TsiLight5 {
    int16_t  NormalX;        // 3 normals
    int16_t  NormalY;
    int16_t  NormalZ;
    uint16_t RoughZposFine;  // 1 Byte Roughness + 1 Byte LSB Zpos
    uint16_t Zpos;
    uint16_t Shadow;         // DEcount 10bit + 6 Light HS only yes/no
    uint16_t AmbShadow;
    uint16_t SIgradient;     // Smoothed Iteration gradient for coloring
    uint16_t OTrap;          // coloring on OrbitTrap
};
static_assert(sizeof(TsiLight5) == 18, "TsiLight5 must be 18 bytes");
using TPsiLight5 = TsiLight5*;

// ----------------------------------------------------------------------------
// TLCol8 -- 10 bytes (surface color)
// ----------------------------------------------------------------------------
struct TLCol8 {
    uint16_t Position;
    uint32_t ColorDif;
    uint32_t ColorSpe;       // transparency on byte4
};

// ----------------------------------------------------------------------------
// TICol8 -- 6 bytes (interior color)
// ----------------------------------------------------------------------------
struct TICol8 {
    uint16_t Position;
    uint32_t Color;          // transparency+Spec on byte4
};

// ----------------------------------------------------------------------------
// TLight8 -- 32 bytes (light source definition)
// FIELD ORDER IS CRITICAL
// ----------------------------------------------------------------------------
struct TLight8 {
    uint8_t  Loption;            // bit1: On/Off, bit2: lightmap, etc.
    uint8_t  LFunction;          // 4bit spec func + 2bit diff
    uint16_t Lamp;               // Light amplitude
    TRGB     Lcolor;             // RGB 24bit
    uint16_t LightMapNr;         // 0: no LM, 1..32000: LMnr
    Double7B LXpos;
    uint8_t  AdditionalByteEx;   // LVersionEx in Light[0]
    Double7B LYpos;
    uint8_t  FreeByte;
    Double7B LZpos;
};
static_assert(sizeof(TLight8) == 32, "TLight8 must be 32 bytes");

using T6Lights = TLight8[6];

// ----------------------------------------------------------------------------
// TLightingParas9 -- 408 bytes
// ----------------------------------------------------------------------------
struct TLightingParas9 {
    int16_t  VarColZpos;              // trackbar -> smallint
    uint8_t  RoughnessFactor;         // 0..255 = 0..1
    uint8_t  bColorMap;               // colormap for diff, 0 = off
    TRGB     DynFogCol2;
    uint8_t  AdditionalOptions;       // bit1: Internal Gamma of 2, etc.
    int32_t  TBpos[9];                // TBpos[3..11] in Pascal (0-indexed: 0..8)
    uint32_t TBoptions;               // various option bits
    uint8_t  FineColAdj1, FineColAdj2;
    uint8_t  PicOffsetX, PicOffsetY;
    TRGB     AmbCol;
    uint8_t  DynFogR;
    TRGB     AmbCol2;
    uint8_t  DynFogG;
    TRGB     DepthCol;
    uint8_t  DynFogB;
    TRGB     DepthCol2;
    uint8_t  PicOffsetZ;
    T6Lights Lights;                  // 6 * 32 = 192 bytes
    TLCol8   LCols[10];              // 10 * 10 = 100 bytes
    TICol8   ICols[4];               // 4 * 6 = 24 bytes
    uint8_t  BGbmp[24];              // Background image filename
};
static_assert(sizeof(TLightingParas9) == 408, "TLightingParas9 must be 408 bytes");
using TPLightingParas9 = TLightingParas9*;

// ----------------------------------------------------------------------------
// TLightingParas8 -- 272 bytes (older version)
// ----------------------------------------------------------------------------
struct TLight7 {
    uint8_t  Loption;
    uint8_t  LFunction;
    uint32_t Lcolor;
    int32_t  LXangle, LYangle;
};

struct TLightingParas8 {
    int32_t  TBpos[11];              // TBpos[1..11] in Pascal (0-indexed: 0..10)
    int32_t  TBoptions;
    uint32_t TBfineColAdjust;
    uint32_t DepthCol;               // 56 bytes to here
    uint32_t Cols[9];                // [1..9] in Pascal (0-indexed: 0..8)
    TLight7  Lights[4];             // 4 * 14 = 56 bytes
    TLCol8   LCols[10];             // 10 * 10 = 100 bytes
    TICol8   ICols[4];              // 4 * 6 = 24 bytes
};
using TPLightingParas8 = TLightingParas8*;

// ----------------------------------------------------------------------------
// TLightingParas7
// ----------------------------------------------------------------------------
struct TLightingParas7 {
    int32_t  TBpos[11];
    uint32_t TBoptions;
    uint32_t TBfineColAdjust;
    uint32_t DepthCol;
    uint32_t Cols[9];
    TLight7  Lights[4];
};

// ----------------------------------------------------------------------------
// TLightingParas (oldest version)
// ----------------------------------------------------------------------------
struct TLightingParas {
    int32_t  TBpos[11];
    uint32_t TBoptions;
    uint32_t TBfineColAdjust;
    uint32_t DepthCol;
    uint32_t Cols[9];                // diff1, spec1, amb1, diff2, spec2...
};

// ----------------------------------------------------------------------------
// THAformula -- 188 bytes
// ----------------------------------------------------------------------------
struct THAformula {
    int32_t  iItCount;               // 4
    int32_t  iFnr;                   // 4
    int32_t  iOptionCount;           // 4
    uint8_t  CustomFname[32];        // 32
    uint8_t  byOptionType[16];       // 16
    double   dOptionValue[16];       // 128
};
static_assert(sizeof(THAformula) == 188, "THAformula must be 188 bytes");

// ----------------------------------------------------------------------------
// THeaderCustomAddon -- 1136 bytes
// ----------------------------------------------------------------------------
struct THeaderCustomAddon {
    uint8_t  bHCAversion;
    uint8_t  bOptions1;              // type of hybrid
    uint8_t  bOptions2;              // bit1: Disable analytical DE, etc.
    uint8_t  bOptions3;              // bit1+2+3: type of DEcombination
    uint8_t  iFCount;
    uint8_t  bHybOpt1;              // end1, repeat1
    uint16_t bHybOpt2;              // start2, end2, repeat2
    THAformula Formulas[6];         // 8 + 188*6 = 1136
};
static_assert(sizeof(THeaderCustomAddon) == 1136, "THeaderCustomAddon must be 1136 bytes");

// ----------------------------------------------------------------------------
// TMandHeader10 -- 840 bytes (DISK FORMAT - main parameter file header)
// CRITICAL: Pointer fields must be uint32_t to maintain 840-byte size
// ----------------------------------------------------------------------------
struct TMandHeader10 {
    int32_t  MandId;                 // #0
    int32_t  Width, Height, Iterations; // #4, #8, #12
    uint16_t iOptions;               // #16
    uint8_t  bNewOptions;            // #18
    uint8_t  bColorOnIt;             // #19
    double   dZstart, dZend;         // #20
    double   dXmid, dYmid, dZmid;   // #36
    double   dXWrot, dYWrot, dZWrot; // #60   4D rotation
    double   dZoom, RStop;           // #84
    int32_t  iReflectsCalcTime;      // #100
    float    sFmixPow;              // #104
    double   dFOVy;                 // #108
    float    sTRIndex;              // #116
    float    sTRscattering;         // #120
    uint8_t  MCoptions;             // #124
    uint8_t  MCdiffReflects;        // #125
    uint8_t  bStereoMode;           // #126
    uint8_t  bSSAO24BorderMirrorSize; // #127
    int32_t  iAmbCalcTime;          // #128
    uint8_t  bNormalsOnDE;          // #132
    uint8_t  bCalculateHardShadow;  // #133
    uint8_t  bStepsafterDEStop;     // #134
    uint16_t MinimumIterations;     // #135
    uint16_t MClastY;               // #137
    uint8_t  bCalc1HSsoft;          // #139
    int32_t  iAvrgDEsteps, iAvrgIts; // #140
    uint8_t  bPlanarOptic;          // #148
    uint8_t  bCalcAmbShadowAutomatic; // #149
    float    sNaviMinDist;          // #150
    double   dStepWidth;            // #154
    uint8_t  bVaryDEstopOnFOV;      // #162
    uint8_t  bHScalculated;         // #163
    float    sDOFZsharp, sDOFclipR; // #164
    float    sDOFaperture;          // #172
    uint8_t  bCutOption;            // #176
    float    sDEstop;               // #177
    uint8_t  bCalcDOFtype;          // #181
    float    mZstepDiv;             // #182
    uint8_t  MCDepth;               // #186
    uint8_t  SSAORcount;            // #187
    uint8_t  AODEdithering;         // #188
    uint8_t  bImageScale;           // #189
    uint8_t  bIsJulia;              // #190
    double   dJx, dJy, dJz, dJw;   // #191
    uint8_t  bDFogIt;               // #223
    ShortFloat MCSoftShadowRadius;  // #224
    float    HSmaxLengthMultiplier; // #226
    float    StereoScreenWidth;     // #230
    float    StereoScreenDistance;  // #234
    float    StereoMinDistance;     // #238
    float    sRaystepLimiter;       // #242
    TMatrix3 hVGrads;               // #246  72 bytes
    uint8_t  bMCSaturation;         // #318
    float    sAmbShadowThreshold;   // #319
    int32_t  iCalcTime;             // #323
    int32_t  iCalcHStime;           // #327
    uint8_t  byCalcNsOnZBufAuto;    // #331
    float    SRamount;              // #332
    uint8_t  bCalcSRautomatic;      // #336
    uint8_t  SRreflectioncount;     // #337
    float    sColorMul;             // #338
    uint8_t  byColor2Option;        // #342
    uint8_t  bVolLightNr;           // #343
    uint8_t  bCalc3D;               // #344
    uint8_t  bSliceCalc;            // #345
    double   dCutX, dCutY, dCutZ;   // #346
    float    sTransmissionAbsorption; // #370
    float    sDEAOmaxL;             // #374
    float    sDEcombS;              // #378
    uint32_t PHCustomF[6];          // #382  DISK FORMAT: uint32_t, NOT native pointers!
    uint32_t PCFAddon;              // #406  DISK FORMAT: uint32_t, NOT native pointer!
    float    sDOFZsharp2;           // #410
    int32_t  iMaxIts;               // #414
    int32_t  iMaxItsF2;             // #418
    uint8_t  DEmixColorOption;      // #422
    uint8_t  MCcontrast;            // #423
    float    sM3dVersion;           // #424
    int32_t  TilingOptions;         // #428
    TLightingParas9 Light;          // #432  408 bytes -> total 840
};
static_assert(sizeof(TMandHeader10) == 840, "TMandHeader10 must be 840 bytes");
using TPMandHeader10 = TMandHeader10*;

// ----------------------------------------------------------------------------
// TMandHeader9 -- 704 bytes (older version)
// DISK FORMAT: pointer fields are uint32_t
// ----------------------------------------------------------------------------
struct TMandHeader9 {
    int32_t  MandId, Width, Height, Iterations, iOptions;
    double   dZstart, dZend;
    double   dXmid, dYmid, dZmid;
    double   dXrot, dYrot, dZrot;
    double   dZoom, RStop, dPow;
    double   dFOVy;
    double   dHardLightZaxis, dHardLightXaxis;
    uint8_t  bColorOnGrad;
    uint8_t  bCalculateHardShadow;
    uint8_t  bStepsafterDEStop;
    int32_t  MinimumIterations;
    uint8_t  iThreadPriority;
    int32_t  iAvrgDEsteps, iAvrgIts;
    int8_t   CalcBackgroundShadow;
    uint8_t  bCalcAmbShadowAutomatic;
    float    sDEcolVar;
    double   dStepWidth;
    uint8_t  bVaryDEstopOnFOV;
    uint8_t  bHScalculated;
    float    sDOFZsharp, sDOFradius, sDOFosize;
    uint8_t  bCutOption;
    float    sDEstop;
    uint8_t  bCalcDOFtype;
    uint8_t  newBuffer2[7];
    uint8_t  bImageScale;
    uint8_t  bIsJulia;
    double   dJx, dJy, dJz;
    uint8_t  newBuffer3[31];
    TMatrix3 hVGrads;
    int8_t   BGSoffset;
    float    sAmbShadowThreshold;
    int32_t  iCalcTime;
    uint8_t  newBuffer4[16];
    uint8_t  byRepeatFrom;
    uint8_t  bCalc3D;
    uint8_t  bSliceCalc;
    double   dCutX, dCutY, dCutZ;
    uint8_t  newBuffer5[8];
    uint32_t PCustomF;              // DISK FORMAT
    uint32_t PHCustomF[6];          // DISK FORMAT
    uint32_t PCFAddon;              // DISK FORMAT
    uint8_t  newBuffer6[22];
    TLightingParas8 Light;
};

// ----------------------------------------------------------------------------
// TMandHeader8 -- DISK FORMAT
// ----------------------------------------------------------------------------
struct TMandHeader8 {
    int32_t  MandId, Width, Height, Iterations, iOptions;
    double   dZstart, dZend;
    double   dXmid, dYmid, dZmid;
    double   dXrot, dYrot, dZrot;
    double   dZoom, RStop, dPow;
    double   dFOVy;
    double   dHardLightZaxis, dHardLightXaxis;
    uint8_t  bColorOnGrad;
    uint8_t  bCalculateHardShadow;
    uint8_t  bStepsafterDEStop;
    int32_t  MinimumIterations;
    uint8_t  iThreadPriority;
    int32_t  iAvrgDEsteps, iAvrgIts;
    int8_t   CalcBackgroundShadow;
    uint8_t  bCalcAmbShadowAutomatic;
    float    sDEcolVar;
    float    sAniStop, sAniStep;
    uint8_t  bVaryDEstopOnFOV;
    uint8_t  bHScalculated;
    float    sCutX, sCutY, sCutZ;
    uint8_t  bCutOption;
    float    sDEstop;
    float    sBBmulBulb;
    float    sBBRbulb;
    uint8_t  bImageScale;
    uint8_t  bIsJulia;
    double   dJx, dJy, dJz;
    double   dZmul;
    double   dMinR;
    uint8_t  iHformula[3];
    int32_t  iHcount[3];
    double   dHPow[3];
    double   dHZmul[3];
    double   dHMinR[3];
    int8_t   BGSoffset;
    float    sAmbShadowThreshold;
    int32_t  iCalcTime;
    uint8_t  obsoletebAniFileIndex;
    uint8_t  CustomFname[16];
    uint8_t  bCalc3D;
    uint8_t  bSliceCalc;
    double   dCutX, dCutY, dCutZ;
    double   dCustomOption1;
    uint32_t PCustomF;              // DISK FORMAT
    uint32_t PHCustomF[6];          // DISK FORMAT
    uint32_t PCFAddon;              // DISK FORMAT
    uint8_t  newBuffer[22];
    TLightingParas8 Light;
};

// ----------------------------------------------------------------------------
// TMandHeader4 -- oldest version
// ----------------------------------------------------------------------------
struct TMandHeader4 {
    int32_t  MandId, Width, Height, Iterations, iOptions;
    double   dZstart, dZend;
    double   dXmid, dYmid, dZmid;
    double   dXrot, dYrot, dZrot;
    double   dZoom, RStop, dPow;
    TLightingParas Light;
    double   dFOVy;
    double   dHardLightZaxis, dHardLightXaxis;
    uint8_t  bColorOnGrad;
    uint8_t  bCalculateHardShadow;
    uint8_t  bStepsafterDEStop;
    int32_t  MinimumIterations;
    uint8_t  iThreadPriority;
    int32_t  iAvrgDEsteps, iAvrgIts;
    int8_t   CalcBackgroundShadow;
    uint8_t  bCalcAmbShadowAutomatic;
    float    sAniStart, sAniStop, sAniStep;
    uint8_t  bAniOption;
    uint8_t  bHScalculated;
    float    sCutX, sCutY, sCutZ;
    uint8_t  bCutOption;
    uint8_t  newBuffer[3];
};

// ----------------------------------------------------------------------------
// TIteration3Dext -- runtime struct (has native pointers)
// DO NOT static_assert size -- varies by platform
// ----------------------------------------------------------------------------
struct TIteration3Dext {
    double   J4;             // -56  4d extension
    double   Rold;           // -48
    double   RStopD;         // -40
    double   x;              // -32  vector to iterate
    double   y;              // -24
    double   z;              // -16
    double   w;              // -8
    double   C1, C2, C3;    // 0    input start values
    double   J1, J2, J3;    // +24  julia start values
    void*    PVar;           // +48  pointer to user input values
    float    SmoothItD;      // +52
    double   Rout;           // +56  square of current vector length
    int32_t  ItResultI;      // +64  integer iteration count
    int32_t  maxIt;          // +68
    float    RStop;          // +72
    int32_t  nHybrid[6];     // +76  Hybrid counts
    void*    fHPVar[6];      // +100 pointer to constants+vars
    ThybridIteration2 fHybrid[6]; // +124 iteration function pointers
    ByteBool CalcSIT;        // +148
    uint8_t  bFree;          // +149
    uint16_t EndTo;          // +150
    LongBool DoJulia;        // +152
    float    LNRStop;        // +156
    int32_t  DEoption;       // +160
    float    fHln[6];        // +164  for SmoothIts
    uint16_t iRepeatFrom;    // +188
    uint16_t iStartFrom;     // +190
    double   OTrap;          // +192
    double   VaryScale;      // +200
    int32_t  bFirstIt;       // +208
    int32_t  bTmp;           // +212
    double   Dfree1;         // +216
    double   Dfree2;         // +224
    double   Deriv1;         // +232
    double   Deriv2;         // +240
    double   Deriv3;         // +248
    TSMatrix4 SMatrix4;      // +256  4x4 single matrix (64 bytes)
    double   Ju1, Ju2, Ju3, Ju4; // +320
    TLMSfunction PMapFunc;   // +352
    TLMSfunction PMapFunc2;  // +356
    TFormulaInitialization pInitialization[6]; // +360
    LongBool bIsInsideRender; // +384
    int32_t  OTrapMode;
    double   OTrapDE;
    // Legacy formula XMM0/XMM1 lanes. Keep the Pascal-compatible prefix intact.
    double   FormulaSse01[4];
};

// ----------------------------------------------------------------------------
// TIteration3D -- runtime struct (has native pointers)
// ----------------------------------------------------------------------------
struct TIteration3D {
    double   C1, C2, C3;
    double   J1, J2, J3;
    void*    PVar;
    float    SmoothItD;
    double   Rout;
    int32_t  ItResultI;
    int32_t  maxIt;
    float    RStop;
    int32_t  nHybrid[6];
    void*    fHPVar[6];
    ThybridIteration2 fHybrid[6];
    LongBool CalcSIT;
    LongBool DoJulia;
    float    LNRStop;
    int32_t  DEoption;
    float    fHln[6];
    int32_t  iRepeatFrom;
    double   OTrap;
    double   VaryScale;
    int32_t  bFirstIt;
    int32_t  bTmp;
    double   Dfree1;
    double   Dfree2;
    double   Deriv1;
    double   Deriv2;
    double   Deriv3;
};

// ----------------------------------------------------------------------------
// TMCTparameter -- runtime struct (has native pointers)
// MainCalcThread values
// ----------------------------------------------------------------------------

// Forward declarations for types used by pointers
struct TLightVals;
using TPLightVals = TLightVals*;
struct TCalcThreadStats;
using TPCalcThreadStats = TCalcThreadStats*;
struct TLightValsNavi;
using TPLightValsNavi = TLightValsNavi*;

// Windows RECT equivalent
struct TRect {
    int32_t Left, Top, Right, Bottom;
};

struct TMCTparameter {
    int32_t  iMinIt;
    int32_t  iMandWidth;
    int32_t  iThreadId;
    double   FOVy;               // #12
    double   Xmit, Ymit, Zmit;
    uint16_t iSmNormals;         // #44
    WordBool IsCustomDE;
    TPsiLight5 mPsiLight;
    float    msDEstop;
    float    sZstepDiv;
    int32_t  iDEAddSteps;
    TPIteration3Dext pIt3Dext;   // #64
    int32_t  iMandHeight;
    TCaldDEfunction CalcDE;
    TPLightVals PLVals;
    float    mctDEstopFactor;
    float    mctDEoffset;
    float    mctMH04ZSD;
    float    DEstop;
    float    sStepWm103;
    double   mZZ;
    TMandFunction mMandFunction;
    TMandFunctionDE mMandFunctionDE;
    int32_t  FormulaType;        // DEcombination mode
    float    dDEscale2;
    TMatrix3 Vgrads;
    double   StepWidth, Zend;
    double   dCOX, dCOY, dCOZ;
    float    dDEscale;
    float    dColPlus;
    TVec3D   Ystart;             // -> CamPos
    TVec3D   HSvecs[6];
    double   CAFX, CAFY;
    TPCalcThreadStats PCalcThreadStats;
    WordBool bVaryDEstop;
    WordBool NormalsOnDE;
    void*    pSiLight;
    LongBool IsCustomDE1;
    LongBool IsCustomDE2;
    uint8_t  ColorOption;
    uint8_t  ColorOnIt;
    uint16_t DFogOnIt;
    float    sRoughness;
    int32_t  DEoption2;
    TMandFunction mMandFunction2;
    TMandFunctionDE mMandFunctionDE2;
    int32_t  MaxItsResult;
    int32_t  iMaxitF2;
    TVec3D   mVgradsFOV;
    int32_t  DEoptionResult;
    int32_t  iSliceCalc;
    float    FOVXoff;
    float    FOVXmul;
    float    msDEsub;
    int32_t  calcHardShadow;
    int32_t  NaviStep, SLwidMNpix;
    TPLightValsNavi PLValsNavi;
    int32_t  iCutOptions;
    int32_t  MCTCameraOptic;
    LongBool bMCTFirstStepRandom;
    float    mctPlOpticZ;
    float    mctsM;
    float    mctColVarDEstopMul;
    float    mctColorMul;
    int32_t  iThreadCount;
    int32_t  FSIstart;
    int32_t  FSIoffset;
    LongBool bMCTisValid;
    void*    pCalcNormals;
    LongBool calc3D;
    float    sHSmaxLengthMultiplier;
    float    SoftShadowRadius;
    float    sZZstmitDif;
    float    DEAOmaxL;
    int32_t  AOdither;
    LongBool bCalcAmbShadow;
    TRect    CalcRect;
    int32_t  SLoffset;
    double   ZcMul;
    double   Zcorr;
    float    sDEcombSmooth;
    LongBool bInsideRendering;
    LongBool bCalcInside;
    LongBool bInAndOutside;
    LongBool bIsIFS;
    int32_t  DEmixCol;
    float    FmixPow;
    uint16_t RepeatFrom2;
    uint16_t StartFrom2;
    int32_t  iEnd2;
    float    VLmul;
    float    VLstepmul;
    float    Rstop3D;
    float    mctDEoffset006;

    double   dJUw;
    double   dJUx, dJUy, dJUz;
    void*    pAl16vars;
    float    sPlaceholder;
    double   sPlaceholder2;
    int32_t  iPlaceHolder;
    int32_t  iMaxIt;
    float    dRStop;
    int32_t  nHybrid[6];
    void*    fHPVar[6];
    ThybridIteration2 fHybrid[6];
    ByteBool bCalcSIT;
    uint8_t  bFree;
    uint16_t wEndTo;
    LongBool bDoJulia;
    float    dLNRStop;
    int32_t  DEoption;
    float    fHln[6];
    uint16_t RepeatFrom1;
    uint16_t StartFrom1;
    TSMatrix4 Smatrix4d;
    TFormulaInitialization pInitialization[6];

    bool     withZBuffer;
    void*    MCrecordExt;        // TPMCrecordExt
    int32_t  SLoffsetExt;
};
using PMCTparameter = TMCTparameter*;

// ----------------------------------------------------------------------------
// TLightMap -- runtime struct
// ----------------------------------------------------------------------------
struct TLightMap {
    int32_t  LMnumber;
    int32_t  LMframe;
    int32_t  LMWidth;
    int32_t  LMHeight;
    float    sLMXfactor;
    float    sLMYfactor;
    int32_t  iLMstart;
    float    sIntensity;
    int32_t  iMapType;
    TSVec    LMavrgCol;
    TSVec    LMavrgColSqr;
    TSMatrix3 PicRotMatrix;
    uint8_t  LMfilename[24];
    uint32_t* LMa;              // dynamic array in Pascal
};
using TPLightMap = TLightMap*;

// ----------------------------------------------------------------------------
// TLValigned -- runtime struct for aligned color buffers
// ----------------------------------------------------------------------------
struct TLValigned {
    TSVec    sDepthCol, sDepthCol2;
    TSVec    sAmbCol, sAmbCol2;
    TSVec    LN[6];
    TSVec    sLCols[6];
    TSVec    ColDif[10], ColSpe[10];
    TSVec    ColInt[4];
    TSVec    sDynFogCol;
    TSVec    sDynFogCol2;
};
using TPLValigned = TLValigned*;

// ----------------------------------------------------------------------------
// TLightVals -- runtime struct (has native pointers)
// DO NOT static_assert size
// ----------------------------------------------------------------------------
struct TLightVals {
    TPLValigned PLValigned;
    WordBool bColCycling;
    WordBool bNoColIpol;
    int32_t  iColOnOT;
    WordBool bFarFog;
    WordBool bVolLight;
    float    sGamma;
    float    sColZmul;
    float    sShadZ;
    float    sShadZmul;
    float    sDepth, sShadGr, sShad, sAmbShad;
    float    sCStart, sCiStart, sCmul, sCimul;
    int32_t  iLightOption[6];
    float    sDiff, sSpec;
    uint8_t  bDivOptions;
    ByteBool bAddBGlight;
    uint8_t  bDFogOptions;
    ByteBool bUseSmallBGpicForAmb;
    int32_t  iHSenabled[6];
    int32_t  iHScalced[6];
    int32_t  iHSmask[6];
    int32_t  iLightPos[6];
    int32_t  iLightAbs[6];
    int32_t  iLightPowFunc[6];
    int32_t  iLightFuncDiff[6];
    int32_t  ColPos[10];
    int32_t  IColPos[4];
    float    sCDiv[10];
    float    sICDiv[4];
    float    sLmaxL[6];
    LongBool bBackBMP;
    int32_t  iGammaH;
    LongBool bAmbRelObj;
    float    sIndLightReflect;
    float    sPosLightZpos[6];
    float    sPosLightXpos[6];
    float    sPosLightYpos[6];
    float    sPosLP[6];
    uint8_t  SortTab[8];
    TPLightMap LLightMaps[6];
    TPLightMap BGLightMap;
    TPLightMap BGsmallLM;
    TPLightMap DiffColLightMap;
    float    DCLMapOffX;
    float    DCLMapOffY;
    float    DCLMapRotSin;
    float    DCLMapRotCos;
    float    lvMapScale;
    LongBool bYCcomb;
    int32_t  iKFcount;
    int32_t  iDfunc;
    LongBool bDirectImageCoord;
    float    sDynFogMul;
    void*    lvCalcPixelColor;   // TCalcPixelColor function pointer
    LongBool bCalcPixColSqr;
    float    sRoughnessFactor;
    float    sZZstmitDif;
    float    ZposDynFog;
    float    sObjLightDecreaser;
    float    sAbsorpCoeff;
    float    sStepWidth;
    float    sDiffuseShadowing;
    int32_t  iExModes;
    LongBool bScaleAmbDiffDown;
    float    SRLightAmount;
    TVec3D   lvMidPos;
    TSVec    LColSbuf[43];
};

// ----------------------------------------------------------------------------
// TPaintParameter -- runtime struct
// ----------------------------------------------------------------------------
struct TPaintParameter {
    int32_t  ppWidth, ppHeight, ppYstart, ppYinc;
    int32_t  ppSLstart, ppSLoffset;
    int32_t  ppLocalCounter;
    int32_t  ppThreadID;
    TPLightVals PLVals;
    TPsiLight5  pPsiLight;
    uintptr_t   ppMessageHwnd;
    int32_t  ppPlanarOptic;
    float    sFOVy;
    TPMatrix3 pVgrads;
    double   Zcorr, ZcMul;
    double   ZZstmitDif, StepWidth;
    double   BackDist;
    TSMatrix3 m;
    float    ppPlOpticZ;
    int32_t  ppPLoffset;
    int32_t  ppYplus, ppXplus;
    int32_t  ppPaintWidth;
    int32_t  ppPaintHeight;
    float    ppXOff;
};

// ----------------------------------------------------------------------------
// TCTrecord -- 40 bytes
// ----------------------------------------------------------------------------
struct TCTrecord {
    int32_t  iItAvrCount, iDEAvrCount;
    int32_t  iActualXpos, iActualYpos;
    int32_t  isActive, MaxIts;
    int64_t  i64Its, i64DEsteps;
};
using TPCTrecord = TCTrecord*;

// ----------------------------------------------------------------------------
// TCalcThreadStats -- runtime struct (has native pointers)
// ----------------------------------------------------------------------------
struct TCalcThreadStats {
    int32_t  iProcessingType;
    int32_t  iAllProcessingOptions;
    int32_t  iTotalThreadCount;
    int32_t* pLBcalcStop;        // PLongBool
    uintptr_t pMessageHwnd;      // HWND
    uint32_t cCalcTime;
    TRect    ctCalcRect;          // 40 bytes to here
    int32_t  HandleType;
    int32_t  CTSid;
    TCTrecord CTrecords[64];     // 40*64 = 2560
    void*    CThandles[64];      // pointer*64
};

// ----------------------------------------------------------------------------
// TLightValsNavi -- runtime struct
// ----------------------------------------------------------------------------
struct TLValignedNavi {
    TSVec    sDepthCol, sDepthCol2;
    TSVec    sAmbCol;
    TSVec    sDynFogCol;
    TSVec    sLCols[3];
    TSVec    ColDif[4], ColSpe[4], ColInt[4];
};
using TPLValignedNavi = TLValignedNavi*;

struct TLightValsNavi {
    TPLValignedNavi PLValignedNavi;
    LongBool bColCycling;
    LongBool bFarFog;
    LongBool bGamma2;
    LongBool bBlendDFog;
    int32_t  iBackBMP;
    TPLightMap BGLightMap;
    TPSMatrix3 pBGRotMatrix;
    float    sColZmul;
    float    sZpos;
    float    sDepth, sShadGr, sShad, sAmbShad;
    float    sDiffuse, sCStart;
    float    sCiStart, sCmul, sCimul;
    float    sXpos, sYpos;
    int32_t  iXangle[3];
    int32_t  iYangle[3];
    uint8_t  bLightOption[3];
    float    sLightFuncSpec[3];
    int32_t  iLightFuncDiff[3];
    TSVec    AbsViewVec;
    TSVec    LColSbuf[21];
};

// ----------------------------------------------------------------------------
// TAObuf -- 5 bytes
// ----------------------------------------------------------------------------
struct TAObuf {
    uint8_t  Z[3];
    uint16_t AO;
};

// ----------------------------------------------------------------------------
// TBufDEcomb -- runtime struct
// ----------------------------------------------------------------------------
struct TBufDEcomb {
    int32_t  bufIt3DItResultI;
    float    bufIt3DSmoothItD;
    double   bufIt3DOtrap;
    TVec3D   bufVec3D;
    int32_t  bufMCTmaxItsResult;
    float    bufMCTdDEscale;
    LongBool bufMCTCalcSIT;
    TMandFunction bufMCTMandFunction;
    TMandFunctionDE bufMCTMandFunctionDE;
};

// ----------------------------------------------------------------------------
// TPaintLightVals
// ----------------------------------------------------------------------------
struct TPaintLightVals {
    TSVec    ViewVec;
    TSVec    AbsViewVec;
    TSVec    ObjPos;
    float    xPos, yPos;
    float    zPos, sFOVy;
    TPSMatrix3 PSmatrix;
    int32_t  iPlanarOptic;
    float    PlOpticZ;
    float    xOff;
    TSVec    CamPos;
    TSVec    PreDepthCol;
};
using TPPaintLightVals = TPaintLightVals*;

#pragma pack(pop)

// ============================================================================
// Constants from TypeDefinitions.pas
// ============================================================================

namespace mb3d {

constexpr double Pid180   = 3.14159265358979323846 / 180.0;
constexpr double M180dPi  = -180.0 / 3.14159265358979323846;
constexpr double PiM2     = 2.0 * 3.14159265358979323846;
constexpr float  s05      = 0.5f;
constexpr float  s001     = 0.01f;
constexpr double d1d65535 = 1.0 / 65535.0;
constexpr double d1d6     = 1.0 / 6.0;
constexpr double d1em100  = 1e-100;

} // namespace mb3d

#endif // MB3D_CORE_TYPES_H
