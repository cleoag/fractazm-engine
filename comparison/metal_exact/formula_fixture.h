#pragma once
#include "soft_extended.h"

// Restricted boxIFS device contract, derived from the immutable
// mb3d/M3Formulas/boxIFS.m3f CODE and its translated source
// external_formulas/Formulas/dIFS/boxIFS.inc. The optional x87 Twist path is
// explicitly unsupported. Geometry keeps the source SSE2 operation order,
// MAXSD unordered/tie operand selection and separate multiply/add rounding.
// OTrap uses the prepared Single values and the original 64-significand-bit
// extended multiply/add, rounded to binary64 only at the explicit state store.
#ifdef __METAL_VERSION__
#define EXACT_BOX_THREAD thread
using ExactBoxU64 = ulong;
using ExactBoxU32 = uint;
#else
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>
#define EXACT_BOX_THREAD
using ExactBoxU64 = std::uint64_t;
using ExactBoxU32 = std::uint32_t;
#endif

namespace mb3d_exact {

enum : ExactBoxU32 {
    box_complete = 0,
    box_unsupported_twist = 1,
    box_invalid_input = 2,
};

// Scalars cross the device boundary as IEEE binary64 storage bits. Params
// 0..12 are materialized runtime parameters; params[15] is caller RStopD.
// The 20 state lanes follow tests/fixtures/boxifs_regressions.h exactly:
// XYZ,W,JuliaXYZW,sampleXYZ,Rout,VaryScale,bFirstIt,Dfree1,Dfree2,Deriv123,ItResultI.
struct alignas(16) ExactBoxInput {
    ExactBoxU64 params[16];
    ExactBoxU64 state[20];
    ExactBoxU32 apply, trap_option, twist_bits, reserved;
};
struct alignas(16) ExactBoxOutput {
    ExactBoxU64 state[20];
    ExactBoxU32 status, reserved[3];
};

// Real supplies from_bits(bits), .bits and binary64 operators. No floating
// operations, native float conversion or CPU callback occur in this function.
template<class Real>
inline ExactBoxOutput exact_box(EXACT_BOX_THREAD const ExactBoxInput& input) {
    ExactBoxOutput output{};
    for (ExactBoxU32 i = 0; i < 20; ++i) output.state[i] = input.state[i];
    if (input.reserved || input.trap_option > 3) {
        output.status = box_invalid_input;
        return output;
    }
    // The original tests the entire Single storage word, including -0.
    if (input.twist_bits != 0) {
        output.status = box_unsupported_twist;
        return output;
    }
    Real p[16];
    for (ExactBoxU32 i = 0; i < 16; ++i) p[i] = Real::from_bits(input.params[i]);
    const Real x = Real::from_bits(input.state[0]) * p[3] + p[6];
    const Real y = Real::from_bits(input.state[1]) * p[3] + p[5];
    const Real z = Real::from_bits(input.state[2]) * p[3] + p[4];
    const Real ax = Real::from_bits(x.bits & ExactBoxU64(0x7fffffffffffffff));
    const Real ay = Real::from_bits(y.bits & ExactBoxU64(0x7fffffffffffffff));
    const Real az = Real::from_bits(z.bits & ExactBoxU64(0x7fffffffffffffff));
    const Real dx = (ax - p[2]) - z * p[7];
    const Real dy = ay - p[1];
    const Real dz = az - p[0];
    Real distance = dz > dx ? dz : dx;
    distance = distance > dy ? distance : dy;
    if (input.apply == 0) distance = distance / p[3];
    else {
        output.state[0] = x.bits;
        output.state[1] = y.bits;
        output.state[2] = z.bits;
        output.state[12] = (Real::from_bits(input.state[12]) * p[3]).bits;
    }
    output.state[11] = distance.bits;
    const Real trap_source = input.trap_option == 0 ? p[15] :
        Real::from_bits(output.state[input.trap_option - 1]);
    using Extended = mb3d_metal_exact::E80;
    output.state[14] = (Extended(trap_source) * Extended(p[12]) + Extended(p[11])).to_f64().bits;
    return output;
}

#ifndef __METAL_VERSION__
static_assert(sizeof(ExactBoxInput) == 304 && alignof(ExactBoxInput) == 16);
static_assert(sizeof(ExactBoxOutput) == 176 && alignof(ExactBoxOutput) == 16);
static_assert(offsetof(ExactBoxInput, state) == 128 && offsetof(ExactBoxInput, apply) == 288);
static_assert(offsetof(ExactBoxOutput, status) == 160);

namespace fixture_host {
namespace original_box {
#include "tests/fixtures/boxifs_regressions.h"
}

inline constexpr std::size_t box_total_rows = std::size(original_box::formulaRegressions);
inline constexpr std::size_t box_lanes = 20;
static_assert(box_total_rows == 1610);

inline std::vector<ExactBoxInput> box_inputs() {
    std::vector<ExactBoxInput> inputs;
    inputs.reserve(box_total_rows);
    for (const auto& row : original_box::formulaRegressions) {
        if (row.option_count != 13) throw std::runtime_error("Unexpected original boxIFS option count");
        ExactBoxInput input{};
        for (std::size_t i = 0; i < 16; ++i) {
            // The production parameter loader materializes Twist/OTrap as Single.
            const double value = (i == 8 || i == 11 || i == 12) ?
                double(float(row.options[i])) : row.options[i];
            input.params[i] = std::bit_cast<std::uint64_t>(value);
        }
        input.params[15] = std::bit_cast<std::uint64_t>(4.0);
        const double x = row.input[0], y = row.input[1], z = row.input[2];
        const double state[20] = {x,y,z,row.input[3],.2,-.3,.4,0,
            x+.125,y-.25,z+.5,(x*x+y*y)+z*z,1,-1,0,0,-2,.375,3,0};
        for (std::size_t i = 0; i < box_lanes; ++i) input.state[i] = std::bit_cast<std::uint64_t>(state[i]);
        input.apply = static_cast<std::uint32_t>(static_cast<std::int32_t>(row.options[9]));
        input.trap_option = static_cast<std::uint32_t>(static_cast<std::int32_t>(row.options[10])) & 3;
        input.twist_bits = std::bit_cast<std::uint32_t>(float(row.options[8]));
        inputs.push_back(input);
    }
    return inputs;
}

inline std::vector<std::array<double, box_lanes>> box_expected() {
    std::vector<std::array<double, box_lanes>> values(box_total_rows);
    for (std::size_t i = 0; i < box_total_rows; ++i)
        std::copy_n(original_box::formulaRegressions[i].output, box_lanes, values[i].begin());
    return values;
}

struct BoxCounts { std::size_t total = 0, supported = 0, unsupported_twist = 0; };
inline BoxCounts box_counts() {
    BoxCounts counts;
    for (const auto& row : original_box::formulaRegressions) {
        ++counts.total;
        if (std::bit_cast<std::uint32_t>(float(row.options[8])) == 0) ++counts.supported;
        else ++counts.unsupported_twist;
    }
    if (counts.supported != 1540 || counts.unsupported_twist != 70)
        throw std::runtime_error("Original boxIFS fixture capability denominator changed");
    return counts;
}

struct BoxChecks {
    std::size_t rows = 0, values = 0, failed_rows = 0, failed_values = 0;
    std::size_t class_failures = 0, signed_zero_failures = 0, exact_bits = 0;
    std::size_t unsupported_rows = 0, unsupported_rejection_failures = 0;
};

inline BoxChecks check_box_outputs(const std::vector<ExactBoxOutput>& outputs) {
    const auto inputs = box_inputs();
    if (outputs.size() != inputs.size()) throw std::runtime_error("Incomplete boxIFS output buffer");
    BoxChecks checks;
    for (std::size_t i = 0; i < inputs.size(); ++i) {
        const auto& actual = outputs[i];
        if (inputs[i].twist_bits) {
            ++checks.unsupported_rows;
            bool valid = actual.status == box_unsupported_twist;
            for (std::size_t lane = 0; lane < box_lanes; ++lane)
                valid = valid && actual.state[lane] == inputs[i].state[lane];
            for (auto value : actual.reserved) valid = valid && value == 0;
            if (!valid) ++checks.unsupported_rejection_failures;
            continue;
        }
        ++checks.rows;
        bool failed = actual.status != box_complete;
        for (auto value : actual.reserved) failed = failed || value != 0;
        for (std::size_t lane = 0; lane < box_lanes; ++lane) {
            ++checks.values;
            const double reference = original_box::formulaRegressions[i].output[lane];
            const double value = std::bit_cast<double>(actual.state[lane]);
            const bool same_class = std::isnan(reference) ? std::isnan(value) :
                std::isinf(reference) ? value == reference : std::isfinite(value);
            const bool zero_sign = reference == 0.0 && value == 0.0 &&
                std::signbit(reference) != std::signbit(value);
            const bool passes = same_class && !zero_sign &&
                (!std::isfinite(reference) || std::abs(value-reference) <= 2e-12*std::max(1.0,std::abs(reference)));
            if (!same_class) ++checks.class_failures;
            if (zero_sign) ++checks.signed_zero_failures;
            if (actual.state[lane] == std::bit_cast<std::uint64_t>(reference)) ++checks.exact_bits;
            if (!passes) { ++checks.failed_values; failed = true; }
        }
        if (failed) ++checks.failed_rows;
    }
    return checks;
}
} // namespace fixture_host
#endif
} // namespace mb3d_exact

#undef EXACT_BOX_THREAD
