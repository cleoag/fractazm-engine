#include "core/parity_trace.h"

#ifdef MB3D_ENABLE_PARITY_TRACE
#include "core/formulas.h"
#include "core/ray_marcher.h"

namespace mb3d {

const char* ParityCalcDEName(void* fn) {
    if (fn == nullptr) return "null";
    if (fn == reinterpret_cast<void*>(&CalcDEanalytic)) return "CalcDEanalytic";
    if (fn == reinterpret_cast<void*>(&CalcDEnoADE)) return "CalcDEnoADE";
    if (fn == reinterpret_cast<void*>(&CalcDEfull)) return "CalcDEfull";
    return "unknown";
}

const char* ParityMandFunctionName(void* fn) {
    if (fn == nullptr) return "null";
    if (fn == reinterpret_cast<void*>(&doHybridPas)) return "doHybridPas";
    if (fn == reinterpret_cast<void*>(&doHybrid4DPas)) return "doHybrid4DPas";
    if (fn == reinterpret_cast<void*>(&doInterpolHybridPas)) return "doInterpolHybridPas";
    if (fn == reinterpret_cast<void*>(&doInterpolHybridPas4D)) return "doInterpolHybridPas4D";
    return "unknown";
}

const char* ParityMandFunctionDEName(void* fn) {
    if (fn == nullptr) return "null";
    if (fn == reinterpret_cast<void*>(&doHybridPasDE)) return "doHybridPasDE";
    if (fn == reinterpret_cast<void*>(&doHybrid4DDEPas)) return "doHybrid4DDEPas";
    if (fn == reinterpret_cast<void*>(&doInterpolHybridPasDE)) return "doInterpolHybridPasDE";
    if (fn == reinterpret_cast<void*>(&doInterpolHybridPas4DDE)) return "doInterpolHybridPas4DDE";
    if (fn == reinterpret_cast<void*>(&doHybridIFS3D)) return "doHybridIFS3D";
    if (fn == reinterpret_cast<void*>(&doHybridIFS3DnoVecIni)) return "doHybridIFS3DnoVecIni";
    return "unknown";
}

const char* ParityHybridName(void* fn) {
    if (fn == nullptr) return "null";
    if (fn == reinterpret_cast<void*>(&HybridItIntPow2)) return "HybridItIntPow2";
    if (fn == reinterpret_cast<void*>(&HybridItIntPow3)) return "HybridItIntPow3";
    if (fn == reinterpret_cast<void*>(&HybridItIntPow4)) return "HybridItIntPow4";
    if (fn == reinterpret_cast<void*>(&HybridIntP5)) return "HybridIntP5";
    if (fn == reinterpret_cast<void*>(&HybridIntP6)) return "HybridIntP6";
    if (fn == reinterpret_cast<void*>(&HybridIntP7)) return "HybridIntP7";
    if (fn == reinterpret_cast<void*>(&HybridIntP8)) return "HybridIntP8";
    if (fn == reinterpret_cast<void*>(&HybridFloatPow)) return "HybridFloatPow";
    if (fn == reinterpret_cast<void*>(&HybridQuat)) return "HybridQuat";
    if (fn == reinterpret_cast<void*>(&HybridItTricorn)) return "HybridItTricorn";
    if (fn == reinterpret_cast<void*>(&HybridCube)) return "HybridCube";
    if (fn == reinterpret_cast<void*>(&HybridCubeDE)) return "HybridCubeDE";
    if (fn == reinterpret_cast<void*>(&HybridCubeSSE2)) return "HybridCubeSSE2";
    if (fn == reinterpret_cast<void*>(&HybridCubeSSE2DE)) return "HybridCubeSSE2DE";
    if (fn == reinterpret_cast<void*>(&HybridItIntPow2scale)) return "HybridItIntPow2scale";
    if (fn == reinterpret_cast<void*>(&HybridSuperCube2)) return "HybridSuperCube2";
    if (fn == reinterpret_cast<void*>(&HybridFolding)) return "HybridFolding";
    if (fn == reinterpret_cast<void*>(&TestHybrid)) return "TestHybrid";
    if (fn == reinterpret_cast<void*>(&HybridCustomIFStest)) return "HybridCustomIFStest";
    if (fn == reinterpret_cast<void*>(&AexionC)) return "AexionC";
    if (fn == reinterpret_cast<void*>(&EmptyFormula)) return "EmptyFormula";
    return "unknown";
}

} // namespace mb3d
#endif
