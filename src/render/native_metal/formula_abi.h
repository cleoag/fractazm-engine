#pragma once
#include "smoothing_abi.h"
// Native GPU arithmetic is FP32. Binary64 parameter words below are opaque
// metadata for source-defined integer/mask operations, never an FP64 emulator.
#if defined(__METAL_VERSION__)
#include <metal_stdlib>
#else
#include <cstddef>
#include <cstdint>
#include <type_traits>
#endif

namespace mb3d_native {
#if defined(__METAL_VERSION__)
using U32 = uint;
using I32 = int;
#else
using U32 = std::uint32_t;
using I32 = std::int32_t;
#endif
enum : U32 { kFormulaAbiVersion = 1, kFormulaSlots = 6, kFormulaParameters = 64 };
// FP32 range exits one DE query may record (orbit_driver): the value orbit,
// three gradient samples, and one range retry of those four plus itself.
enum : U32 { kNativeRangeExitsPerDistance = 9 };
enum FormulaNamespace : U32 { InternalFormula = 0, ExternalFormula = 1 };
enum OrbitDriver : U32 {
    Hybrid3D = 0, Hybrid3DAnalytic = 1, Hybrid4D = 2, Hybrid4DAnalytic = 3,
    Interpolated3D = 4, Interpolated3DAnalytic = 5,
    Interpolated4D = 6, Interpolated4DAnalytic = 7,
    HybridDifs = 8, HybridDifsNoInitialization = 9
};
enum FormulaFault : U32 {
    FormulaOk = 0, InvalidFormulaAbi = 1, UnsupportedFormula = 2,
    UnsupportedDriver = 3, NonFiniteFormula = 4, FormulaWorkExhausted = 5,
    InvalidFormulaParameter = 6, MissingFormulaResource = 7,
    InvalidHybridCycle = 8, FormulaResourceBounds = 9
};

struct NativeFormulaSlot {
    U32 version, namespace_id, id;
    I32 de_option;
    U32 param_count, traits, wrapper_flags, difs_style;
    U32 internal_power, enabled, reserved[2];
    float params[kFormulaParameters];
    U32 param_bits[kFormulaParameters][2]; // little-endian original double
    U32 raw_words[64];                    // exact PVar[-256..-1]
    float negative_params[32];            // PVar[-8], PVar[-16], ...
    float constants[32];                  // PVar[0], PVar[8], ...
};

struct NativeFormulaState {
    float xyz[4], c[4], j[4], original_j[4];
    float derivative[4], sse[4];
    float rout, rold, rstop, rstop_d, otrap, vary_scale, dfree1, dfree2, smooth;
    I32 iteration, first, tmp, max_iterations, hybrid_counts[kFormulaSlots];
    U32 julia, inside, calc_smooth, current_slot;
};

struct NativeFormulaFault {
    U32 code, formula_id, slot, iteration, work, limit, reserved[2];
};

// DE combination (CPU FormulaType 1..6, Calc.pas CalcDEfull): the program
// buffer then holds a second program for the second hybrid part. Both copies
// carry the same combination words; FormulaType 0 keeps them all zero.
enum FormulaCombination : U32 {
    SingleFormula = 0, CombineMinimum = 1, CombineMaximum = 2, CombineMaximumInverse = 3,
    CombineLinearBlend = 4, CombineQuadraticBlend = 5, CombineMix = 6
};
struct NativeFormulaProgram {
    U32 version, driver, slot_count, start_slot, repeat_slot, end_slot;
    I32 de_option, max_iterations;
    U32 julia, inside, calc_smooth, work_limit, local_work_limit;
    U32 formula_type, mix_color; // FormulaCombination, DEmixColorOption
    float mix_power;             // sFmixPow
    float radius_stop, log_radius_stop, de_scale, de_offset;
    float de_offset_floor, gradient_radius_stop, stop_scale;
    float combination_smooth;    // sDEcombSmooth
    float julia_value[4], rotation[4][4], smoothing_power[kFormulaSlots];
    I32 hybrid_counts[kFormulaSlots];
    NativeFormulaSlot slots[kFormulaSlots];
};

// Raw RGB FP32 samples, interleaved, with the source's real extra seam column
// and bottom row. Offsets/counts are scalar elements; stride is width+1.
enum NativeMapFlags : U32 { FormulaMapWords=1, FormulaMapEmpty=2 };
struct NativeMapDescriptor {
    I32 map_id;
    U32 width, height, channels, offset, element_count, flags, reserved;
};

struct NativeOrbitResult {
    NativeFormulaState state;
    NativeFormulaFault fault;
    float distance;
    // maximum is the CPU MaxItsResult of a combined DE; zero for FormulaType 0,
    // whose maximum stays the program's own.
    U32 completed, escaped, maximum;
};

#if defined(__METAL_VERSION__)
struct NativeVolumeLight;
// Local shader view only. Host pointers never occur in uploaded descriptors.
struct NativeResources {
    device const NativeMapDescriptor* maps;
    device const float* pixels;
    U32 map_count, pixel_count;
    // Thread-local observers only, never uploaded or persisted in formula state.
    thread NativeSmoothingCounters* smoothing;
    thread uint* range_exits; // FP32 range exits of native_run_orbit
    // dIFS orbits whose result came from lanes carried in from earlier calls.
    thread uint* carried_reads;
    // RMdoColorOnIt's temporary RStop for the queries it encloses; zero keeps
    // the program's. Native general geometry sets and clears it around one query.
    float radius_stop;
    // Volumetric light map for primary DoDynFog sub-steps (volume_light_abi.h);
    // only the camera primary kernels bind it, so other kernels fail closed.
    constant NativeVolumeLight* volume;
    device const float* volume_map;
};
#else
static_assert(sizeof(float) == 4);
static_assert(sizeof(NativeFormulaSlot) == 1328);
static_assert(sizeof(NativeFormulaState) == 188);
static_assert(sizeof(NativeFormulaFault) == 32);
static_assert(sizeof(NativeFormulaProgram) == 8192);
static_assert(sizeof(NativeMapDescriptor) == 32);
static_assert(sizeof(NativeOrbitResult) == 236);
static_assert(std::is_standard_layout_v<NativeFormulaProgram>);
static_assert(std::is_trivially_copyable_v<NativeFormulaProgram>);
static_assert(offsetof(NativeFormulaSlot, params) == 48);
static_assert(offsetof(NativeFormulaSlot, param_bits) == 304);
#endif
} // namespace mb3d_native
