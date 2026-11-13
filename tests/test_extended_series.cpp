#include "core/extended_transcendentals.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <limits>

// Frozen implementations from 5d65c4d, src/core/extended_transcendentals.h.
// These deliberately retain the original SoftFloat operators, conversions,
// series order and stopping conditions. Do not route this oracle through the
// optimized product/divisor/context helpers; this checks preservation of the
// previously qualified algorithm, not a new independent mathematical oracle.
namespace baseline {
using mb3d::numeric::Extended80;
inline Extended80 extendedNaN() { return Extended80::fromBits(0xffff,0xc000000000000000ULL); }
inline Extended80 extendedLn2() { return Extended80::fromBits(0x3ffe,0xb17217f7d1cf79acULL); }
inline Extended80 x87Pi() { return Extended80::fromBits(0x4000,0xc90fdaa22168c235ULL); }
inline Extended80 extendedLog2(Extended80 x) {
    using E=Extended80;
    if(!(x>E(0)) || (x.signExponent()&0x7fff)==0x7fff) return extendedNaN();
    int exponent=(x.signExponent()&0x7fff)-16383;
    uint64_t sig=x.significand();
    if((x.signExponent()&0x7fff)==0) {
        const int shift=std::countl_zero(sig);
        sig<<=shift; exponent=1-16383-shift;
    }
    E m=E::fromBits(0x3fff,sig);
    if(m>E(1.5)) {m/=E(2);++exponent;}
    const E t=(m-E(1))/(m+E(1)),square=t*t;
    E term=t,sum=t;
    for(int k=1;k<32;++k) {
        term*=square;
        const E next=sum+term/E(2*k+1);
        if(next==sum) break;
        sum=next;
    }
    return E(exponent)+(E(2)*sum)/extendedLn2();
}
inline Extended80 extendedAtan2(Extended80 y, Extended80 x) {
    using E = Extended80;
    if (x!=x || y!=y) return x+y;
    const bool negativeY=std::signbit(static_cast<double>(y));
    const bool negativeX=std::signbit(static_cast<double>(x));
    const E pi=x87Pi(), infinity(std::numeric_limits<double>::infinity());
    E ax=abs(x), ay=abs(y), angle;
    if (ay==E(0)) angle=negativeX ? pi : E(0);
    else if (ax==E(0)) angle=pi/E(2);
    else if (ay==infinity || ax==infinity) {
        angle=ay==ax ? pi/E(4) : (ay==infinity ? pi/E(2) : E(0));
        if (negativeX) angle=pi-angle;
    } else {
        const bool invert=ay>ax;
        E t=invert ? ax/ay : ay/ax;
        const bool shift=t>E(0.5);
        if (shift) t=(t-E(1))/(t+E(1));
        // |t| <= 1/2: the first omitted term after 40 is < 6e-27.
        const E square=-(t*t);
        E term=t, sum=t;
        for(int k=1;k<40;++k) {
            term*=square;
            const E next=sum+term/E(2*k+1);
            if(next==sum) break;
            sum=next;
        }
        angle=shift ? pi/E(4)+sum : sum;
        if(invert) angle=pi/E(2)-angle;
        if(negativeX) angle=pi-angle;
    }
    return negativeY ? -angle : angle;
}

} // namespace baseline

namespace {
using E = mb3d::numeric::Extended80;
constexpr uint64_t integerBit = UINT64_C(0x8000000000000000);
constexpr uint64_t maximum = UINT64_MAX;

bool sameBits(E a, E b) {
    return a.signExponent() == b.signExponent() && a.significand() == b.significand();
}

struct Differential {
    uint64_t logChecks = 0, atanChecks = 0, failures = 0;
    void compare(const char* name, E a, E b, E actual, E expected) {
        if (sameBits(actual, expected)) return;
        if (failures++ < 12)
            std::fprintf(stderr,
                "%s mismatch input=%04x:%016llx,%04x:%016llx "
                "actual=%04x:%016llx expected=%04x:%016llx mode=%u precision=%u\n",
                name, unsigned(a.signExponent()), static_cast<unsigned long long>(a.significand()),
                unsigned(b.signExponent()), static_cast<unsigned long long>(b.significand()),
                unsigned(actual.signExponent()), static_cast<unsigned long long>(actual.significand()),
                unsigned(expected.signExponent()), static_cast<unsigned long long>(expected.significand()),
                unsigned(softfloat_roundingMode), unsigned(extF80_roundingPrecision));
    }
    void logarithm(E x) {
        const E expected = baseline::extendedLog2(x);
        const E actual = mb3d::numeric::extendedLog2(x);
        ++logChecks;
        compare("log2", x, E(0), actual, expected);
    }
    void angle(E y, E x) {
        const E expected = baseline::extendedAtan2(y, x);
        const E actual = mb3d::numeric::extendedAtan2(y, x);
        ++atanChecks;
        compare("atan2", y, x, actual, expected);
    }
    void quadrants(E y, E x) {
        angle(y, x); angle(-y, x); angle(y, -x); angle(-y, -x);
    }
};

uint64_t nextRandom(uint64_t& state) {
    state ^= state >> 12; state ^= state << 25; state ^= state >> 27;
    return state * UINT64_C(0x2545f4914f6cdd1d);
}

// Adjacent canonical positive normal representations, including binade
// boundaries. Callers choose exponents far enough from both format limits.
E neighbour(uint16_t exponent, uint64_t significand, int offset) {
    while (offset > 0) {
        if (significand == maximum) { ++exponent; significand = integerBit; }
        else ++significand;
        --offset;
    }
    while (offset < 0) {
        if (significand == integerBit) { --exponent; significand = maximum; }
        else --significand;
        ++offset;
    }
    return E::fromBits(exponent, significand);
}
} // namespace

int main() {
    mb3d::numeric::ExtendedScope scope;
    Differential test;
    // Output bits are the contract. The removed atan2-to-double sign probes
    // could set private underflow/overflow flags. Those flags are restored by
    // the formula's ExtendedScope and are intentionally not compared here.
    const std::array<E, 20> special{
        E(0.0), E(-0.0), E(1), E(-1), E(0.5), E(-0.5),
        E::fromBits(0, 1), E::fromBits(0x8000, 1),
        E::fromBits(0, integerBit - 1), E::fromBits(0x8000, integerBit - 1),
        E::fromBits(0x7ffe, maximum), E::fromBits(0xfffe, maximum),
        E::fromBits(0x7fff, integerBit), E::fromBits(0xffff, integerBit),
        E::fromBits(0x7fff, UINT64_C(0xc000000000000000)),
        E::fromBits(0xffff, UINT64_C(0xc000000000000001)),
        E::fromBits(0x7fff, integerBit + 1), E::fromBits(0xffff, integerBit + 1),
        E::fromBits(1, integerBit), E::fromBits(0x8001, integerBit)};
    for (const E x : special) {
        test.logarithm(x);
        for (const E y : special) test.angle(y, x);
    }

    // Subnormal normalization, including minimum ext80 magnitude and the
    // transition to a normal number. Their binary64 conversions underflow.
    for (unsigned bit = 0; bit < 63; ++bit) {
        const E tiny = E::fromBits(0, UINT64_C(1) << bit);
        test.logarithm(tiny); test.logarithm(-tiny);
        test.quadrants(tiny, E(1));
        test.quadrants(E(1), tiny);
        test.quadrants(tiny, tiny);
    }

    // Mantissa normalization and logarithm cancellation at 1, plus the 1.5
    // reduction threshold. For atan2, ratios around 0.5 trigger its shift and
    // ratios around 1 swap the division operands. Tiny signed y with x<0
    // additionally checks the two sides of the +/-pi branch cut.
    constexpr std::array<uint16_t, 12> scales{
        4, 20, 15308, 15309, 15360, 16360, 16382, 16383,
        16400, 17406, 17407, 0x7ffb};
    for (const uint16_t exponent : scales) {
        const E x = E::fromBits(exponent, integerBit);
        for (int offset = -4; offset <= 4; ++offset) {
            const E nearOne = neighbour(exponent, integerBit, offset);
            const E nearHalf = neighbour(exponent - 1, integerBit, offset);
            test.logarithm(nearOne);
            test.logarithm(neighbour(exponent, UINT64_C(0xc000000000000000), offset));
            test.quadrants(nearOne, x);
            test.quadrants(nearHalf, x);
        }
        const E tiny = E::fromBits(0, 1);
        test.quadrants(tiny, x);
        test.quadrants(E(0.0), x);
        test.quadrants(x, E(0.0));
    }

    uint64_t random = UINT64_C(0xd14675bfac9023e8);
    for (unsigned i = 0; i < 20000; ++i) {
        const unsigned exponent = 1 + nextRandom(random) % 0x7ffe;
        const E x = E::fromBits(uint16_t(exponent), nextRandom(random) | integerBit);
        test.logarithm((i & 31) == 0 ? -x : x);

        // Half the samples have nearby exponents so the full atan series is
        // exercised; independent extreme exponents also cover ratios that
        // underflow and sign conversions that used to overflow binary64.
        unsigned yExponent;
        switch (i & 3) {
        case 0: yExponent = 1 + nextRandom(random) % 0x7ffe; break;
        case 1: yExponent = exponent; break;
        case 2: {
            const int nearby = int(exponent) + int(nextRandom(random) % 9) - 4;
            yExponent = unsigned(nearby < 1 ? 1 : nearby > 0x7ffe ? 0x7ffe : nearby);
            break;
        }
        default: yExponent = 16383 - 30 + nextRandom(random) % 61; break;
        }
        E y = E::fromBits(uint16_t(yExponent), nextRandom(random) | integerBit);
        const unsigned signs = unsigned(nextRandom(random) & 3);
        test.angle((signs & 1) ? -y : y, (signs & 2) ? -x : x);
    }

    // Fresh contexts must use the current caller environment. Non-default
    // modes/precision exercise the exact SoftFloat fallback, not the fast path.
    constexpr std::array<uint_fast8_t, 6> modes{
        softfloat_round_near_even, softfloat_round_minMag, softfloat_round_min,
        softfloat_round_max, softfloat_round_near_maxMag, softfloat_round_odd};
    constexpr std::array<uint_fast8_t, 3> precisions{32, 64, 80};
    for (const auto precision : precisions) {
        extF80_roundingPrecision = precision;
        for (const auto mode : modes) {
            softfloat_roundingMode = mode;
            for (unsigned i = 0; i < 64; ++i) {
                const E x = E::fromBits(uint16_t(16383 - 4 + nextRandom(random) % 9),
                                      nextRandom(random) | integerBit);
                const E y = E::fromBits(uint16_t(16383 - 4 + nextRandom(random) % 9),
                                      nextRandom(random) | integerBit);
                test.logarithm(x);
                test.angle((i & 1) ? -y : y, (i & 2) ? -x : x);
            }
        }
    }
    std::printf("Extended series: %llu log2 + %llu atan2 bit-exact checks, %llu failures\n",
                static_cast<unsigned long long>(test.logChecks),
                static_cast<unsigned long long>(test.atanChecks),
                static_cast<unsigned long long>(test.failures));
    return test.failures ? 1 : 0;
}
