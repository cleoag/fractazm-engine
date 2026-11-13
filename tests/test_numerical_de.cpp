#include "core/numerical_de.h"
#include <atomic>
#include <barrier>
#include <bit>
#include <cfenv>
#include <cstdint>
#include <cstdio>
#include <thread>
#include <vector>

extern "C" {
#if defined(_MSC_VER)
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL __thread
#endif
#include "softfloat.h"
#undef THREAD_LOCAL
}

namespace {
#include "fixtures/radius_de_reference.h"
static_assert(sizeof(radiusDEReferences) / sizeof(radiusDEReferences[0]) == 2240);

struct Case {
    uint64_t input[6];
    uint64_t raw, denominator;
};
// Independently captured x87/Rosetta and software-extended chains agree here.
// These are the first and critical 54th numerical-DE inputs of the diagnosed ray.
constexpr Case reflectionCases[] = {
    {{0x40F399CF13949B90ULL, 0x3FE868151D9DBA71ULL, 0x403F1D819BD2E4FFULL,
      0x405435C55B0AC97AULL, 0x3F70624DE0000000ULL, 0x3F2F751060000000ULL},
     0x4075597F4CFD8B6EULL, 0x40253BF71D986422ULL},
    {{0x40804822751856FBULL, 0x3FCED566132F1AA3ULL, 0x3FC3EE74CFFC130CULL,
      0x3FFCB7268AACEE4BULL, 0x3F70624DE0000000ULL, 0x3F2F751060000000ULL},
     0x40219C9619109BD8ULL, 0x3FF7B054C6FFE053ULL}
};

double evaluate(const uint64_t (&input)[6], double* denominator) {
    return mb3d::NumericalDistanceEstimate(
        std::bit_cast<double>(input[0]), std::bit_cast<double>(input[1]),
        std::bit_cast<double>(input[2]), std::bit_cast<double>(input[3]),
        std::bit_cast<double>(input[4]), std::bit_cast<double>(input[5]), denominator);
}

struct SoftState {
    uint_fast8_t rounding, precision, flags, tininess;
    static SoftState read() {
        return {softfloat_roundingMode, extF80_roundingPrecision,
                softfloat_exceptionFlags, softfloat_detectTininess};
    }
    void write() const {
        softfloat_roundingMode = rounding;
        extF80_roundingPrecision = precision;
        softfloat_exceptionFlags = flags;
        softfloat_detectTininess = tininess;
    }
    bool operator==(const SoftState&) const = default;
};

struct Checks {
    unsigned checks = 0, failures = 0;
    void bits(double actual, uint64_t expected, const char* lane, unsigned row) {
        ++checks;
        const auto value = std::bit_cast<uint64_t>(actual);
        if (value == expected) return;
        if (failures++ < 20)
            std::fprintf(stderr, "FAIL %s row=%u actual=0x%016llX expected=0x%016llX\n",
                lane, row, static_cast<unsigned long long>(value),
                static_cast<unsigned long long>(expected));
    }
    void check(bool pass, const char* message) {
        ++checks;
        if (!pass && failures++ < 20) std::fprintf(stderr, "FAIL %s\n", message);
    }
};
} // namespace

int main() {
    Checks result;
    result.check(std::fegetround() == FE_TONEAREST, "host nearest-even environment");
    unsigned rawNaNs = 0, denominatorNaNs = 0, rowIndex = 0;
    for (const auto& row : radiusDEReferences) {
        double denominator = std::bit_cast<double>(UINT64_C(0x7ff8000000001234));
        const double raw = evaluate(row.input, &denominator);
        // Compare every bit, including NaN sign, quiet bit and payload. There
        // is deliberately no classification-only allowance for these rows.
        result.bits(raw, row.expected[1], "frozen raw_de", rowIndex);
        result.bits(denominator, row.expected[2], "frozen denominator", rowIndex);
        rawNaNs += (row.expected[1] & UINT64_C(0x7fffffffffffffff)) > UINT64_C(0x7ff0000000000000);
        denominatorNaNs += (row.expected[2] & UINT64_C(0x7fffffffffffffff)) > UINT64_C(0x7ff0000000000000);
        ++rowIndex;
    }
    for (unsigned i = 0; i < 2; ++i) {
        double denominator = 0;
        result.bits(evaluate(reflectionCases[i].input, &denominator), reflectionCases[i].raw,
                    "reflection raw_de", i);
        result.bits(denominator, reflectionCases[i].denominator, "reflection denominator", i);
        result.bits(evaluate(reflectionCases[i].input, nullptr), reflectionCases[i].raw,
                    "nullable denominator", i);
    }

    // Different caller environments overlap to detect process-global state,
    // and both finite and invalid-domain calls must restore every TLS field.
    const SoftState original = SoftState::read();
    const SoftState mainMarker{softfloat_round_near_maxMag, 32, softfloat_flag_overflow,
                               softfloat_tininess_beforeRounding};
    mainMarker.write();
    std::barrier start(4);
    std::atomic<unsigned> threadFailures{0};
    std::vector<std::jthread> threads;
    for (unsigned lane = 0; lane < 4; ++lane) threads.emplace_back([&, lane] {
        const SoftState marker{uint_fast8_t(lane), uint_fast8_t(lane % 2 ? 64 : 32),
                               uint_fast8_t(lane + 1), uint_fast8_t(lane % 2)};
        marker.write();
        start.arrive_and_wait();
        for (unsigned repeat = 0; repeat < 64; ++repeat) {
            for (const auto& row : reflectionCases) {
                double denominator = 0;
                const auto raw = std::bit_cast<uint64_t>(evaluate(row.input, &denominator));
                if (raw != row.raw || std::bit_cast<uint64_t>(denominator) != row.denominator ||
                    !(SoftState::read() == marker)) ++threadFailures;
            }
            double denominator = 0;
            const auto& invalid = radiusDEReferences[0];
            if (std::bit_cast<uint64_t>(evaluate(invalid.input, &denominator)) != invalid.expected[1] ||
                std::bit_cast<uint64_t>(denominator) != invalid.expected[2] ||
                !(SoftState::read() == marker)) ++threadFailures;
        }
    });
    threads.clear();
    result.check(threadFailures == 0, "768 concurrent calls preserve results and caller SoftFloat TLS");
    result.check(SoftState::read() == mainMarker, "worker calls preserve main-thread SoftFloat state");
    original.write();

    std::printf("Numerical DE: 2240 frozen rows, %u raw NaNs and %u denominator NaNs checked bitwise; "
                "%u checks, %u failures\n", rawNaNs, denominatorNaNs, result.checks, result.failures);
    return result.failures ? 1 : 0;
}
