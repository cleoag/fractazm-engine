#include "core/numeric_precision.h"
#include "core/extended_sine.h"
#include <array>
#include <atomic>
#include <barrier>
#include <bit>
#include <cfenv>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <thread>
#include <vector>

namespace {
using namespace mb3d;
using namespace mb3d::numeric;
using E = Extended80;
struct SoftState {
    uint_fast8_t rounding, precision, flags, tininess;
    static SoftState read() {
        return {softfloat_roundingMode, extF80_roundingPrecision,
                softfloat_exceptionFlags, softfloat_detectTininess};
    }
    void write() const {
        softfloat_roundingMode=rounding; extF80_roundingPrecision=precision;
        softfloat_exceptionFlags=flags; softfloat_detectTininess=tininess;
    }
    bool operator==(const SoftState&) const = default;
};
struct Checks {
    unsigned checks=0, failures=0;
    void check(bool pass,const char* label) {
        ++checks;
        if (!pass && failures++<20) std::fprintf(stderr,"FAIL %s\n",label);
    }
    void same(double actual,double expected,const char* label) {
        check(std::isnan(expected) ? std::isnan(actual) :
              std::bit_cast<uint64_t>(actual)==std::bit_cast<uint64_t>(expected),label);
    }
};
struct Conversion {
    uint16_t exponent;
    uint64_t significand;
    double value;
    uint16_t roundedExponent;
    uint64_t roundedSignificand;
};
uint64_t next(uint64_t& state) {
    state^=state<<13;state^=state>>7;state^=state<<17;return state;
}
#if defined(__SIZEOF_INT128__)
// Independent pre-port reduction oracle. Keep this test-only integer version
// so sharing the portable reducer between sin and sincos cannot hide a change.
ExtendedTrigReduction previousTrigReduction(E x,int exponent) {
    using U=__uint128_t;
    constexpr U pi=(U(0xc)<<64)|U(0x90fdaa22168c234cULL);
    U remainder=x.significand();
    unsigned quadrant=0;
    for(int i=0;i<exponent+4;++i) {
        remainder<<=1;quadrant=(quadrant<<1)&3;
        if(remainder>=pi) {remainder-=pi;quadrant|=1;}
    }
    const bool reflect=remainder>pi/2;
    if(reflect) {remainder=pi-remainder;quadrant=(quadrant+1)&3;}
    E residual(0);
    if(remainder) {
        const uint64_t high=uint64_t(remainder>>64);
        const int bits=high?128-std::countl_zero(high):64-std::countl_zero(uint64_t(remainder));
        const int shift=bits-64;
        uint64_t sig;
        int resultExponent=bits-1-67;
        if(shift<=0) sig=uint64_t(remainder)<<(-shift);
        else {
            sig=uint64_t(remainder>>shift);
            const U tail=remainder&((U(1)<<shift)-1),half=U(1)<<(shift-1);
            if(tail>half || (tail==half && (sig&1)))
                if(++sig==0) {sig=UINT64_C(0x8000000000000000);++resultExponent;}
        }
        residual=E::fromBits(uint16_t(16383+resultExponent),sig);
    }
    return {reflect?-residual:residual,quadrant};
}
#endif
}

int main() {
    Checks check;
    check.check(std::fegetround()==FE_TONEAREST,"native profile host nearest-even environment");
    const SoftState original=SoftState::read();
    std::vector<Conversion> conversions;
    E compatibleSmall,compatibleHuge;
    {
        ScopedNumericPrecision profile(PrecisionMode::Compatible);
        ExtendedScope scope;
        compatibleSmall=E::fromBits(0x3fff,UINT64_C(0x8000000000000001));
        compatibleHuge=E::fromBits(0x7ffe,UINT64_C(0x8000000000000000));
        const auto add=[&](uint16_t exponent,uint64_t significand) {
            const E value=E::fromBits(exponent,significand);
            const double rounded=double(value);
            const E roundTrip(rounded);
            conversions.push_back({exponent,significand,rounded,
                roundTrip.signExponent(),roundTrip.significand()});
        };
        // Independent SoftFloat conversion oracle, captured before entering
        // Native. Exercise ties, subnormal transitions, overflow, signed zero,
        // infinities and quiet/signaling NaN payloads without moving fixtures.
        const uint16_t exponents[]={0,1,0x3bcc,0x3bcd,0x3bce,0x3bff,0x3c00,0x3c01,
                                    0x3fff,0x43fe,0x43ff,0x7ffe};
        const uint64_t significands[]={0,1,0x7fffffffffffffffULL,0x8000000000000000ULL,0x8000000000000400ULL,
                                      0x8000000000000401ULL,0x8000000000000c00ULL,UINT64_MAX};
        for (auto exponent:exponents) for(auto significand:significands) {
            if (bool(significand>>63) != bool(exponent)) continue; // canonical encodings
            for(unsigned sign:{0u,0x8000u}) add(uint16_t(exponent|sign),significand);
        }
        for(unsigned sign:{0u,0x8000u}) for(uint64_t significand:
            {0x8000000000000000ULL,0xc000000000002800ULL,0x8000000000001800ULL})
            add(uint16_t(0x7fff|sign),significand);
        uint64_t state=0xbd9f3c720e6415a9ULL;
        for(unsigned i=0;i<512;++i)
            add(uint16_t((next(state)%0x7fff)|((i&1)<<15)),next(state)|UINT64_C(0x8000000000000000));
        // Retain platform-independent carry/reflection cases for compilers
        // without int128, where the differential oracle below is unavailable.
        struct ReductionCase {int exponent;uint64_t input;uint16_t resultExponent;uint64_t result;unsigned quadrant;};
        const ReductionCase reductions[]={
            {-4, 0x8000000000000000ULL, 0x3ffb, 0x8000000000000000ULL, 0},
            {-4, 0xffffffffffffffffULL, 0x3ffb, 0xffffffffffffffffULL, 0},
            {-3, 0x8000000000000001ULL, 0x3ffc, 0x8000000000000001ULL, 0},
            {0, 0xffffffffffffffffULL, 0x3ffd, 0xdbc095777a5cf729ULL, 1},
            {1, 0xc90fdaa22168c234ULL, 0xbfc0, 0xc000000000000000ULL, 2},
            {2, 0xc90fdaa22168c235ULL, 0x3fc0, 0x8000000000000000ULL, 0},
            {10, 0xfedcba9876543210ULL, 0x3ff7, 0x87d7cbeb9f908a80ULL, 2},
            {61, 0xffffffffffffffffULL, 0x3ffe, 0x8912470183982a01ULL, 2},
            {62, 0x8000000000000000ULL, 0xbffe, 0xc90d6e42bf395a68ULL, 3},
            {62, 0xc90fdaa22168c235ULL, 0x3ffc, 0x8000000000000000ULL, 0},
            {62, 0xffffffffffffffffULL, 0xbffd, 0xfff64e82774260cfULL, 1},
        };
        for(const auto& row:reductions) {
            const auto actual=reduceExtendedTrig(E::fromBits(uint16_t(16383+row.exponent),row.input),row.exponent);
            check.check(actual.quadrant==row.quadrant && actual.residual.signExponent()==row.resultExponent &&
                        actual.residual.significand()==row.result,"portable trig reduction fixed reference");
        }
#if defined(__SIZEOF_INT128__)
        // Every exponent in the x87 finite reduction interval; dense mantissa
        // samples cover carry, reflection, nearest-even rounding and quadrants.
        for(int exponent=-4;exponent<63;++exponent) for(unsigned i=0;i<256;++i) {
            const uint64_t sig=i==0?UINT64_C(0x8000000000000000):
                               i==1?UINT64_MAX:next(state)|UINT64_C(0x8000000000000000);
            const E x=E::fromBits(uint16_t(16383+exponent),sig);
            const auto expected=previousTrigReduction(x,exponent);
            const auto actual=reduceExtendedTrig(x,exponent);
            check.check(actual.quadrant==expected.quadrant &&
                        actual.residual.signExponent()==expected.residual.signExponent() &&
                        actual.residual.significand()==expected.residual.significand(),
                        "portable trig reduction preserves prior integer result");
        }
#endif
    }
    const SoftState poison{softfloat_round_min,32,softfloat_flag_underflow,softfloat_tininess_beforeRounding};
    poison.write();
    E nativeSaved;
    {
        ScopedNumericPrecision profile(PrecisionMode::Native);
        check.check(NativePrecisionEnabled(),"native scope enabled");
        ExtendedScope scope;
        check.check(SoftState::read()==poison,"native ExtendedScope leaves SoftFloat state untouched");
        for(const auto& row:conversions) {
            const E value=E::fromBits(row.exponent,row.significand);
            check.same(double(value),row.value,"native constant conversion matches nearest-even binary64");
            check.check(value.signExponent()==row.roundedExponent &&
                        value.significand()==row.roundedSignificand,"native getter encodes stored binary64");
        }
        const uint64_t doubles[]={0,0x8000000000000000ULL,1,0x8000000000000001ULL,
            0x000fffffffffffffULL,0x0010000000000000ULL,0x3ff0000000000001ULL,
            0x7fefffffffffffffULL,0x7ff0000000000000ULL,0xfff0000000000000ULL};
        for(auto bits:doubles) check.same(double(E(std::bit_cast<double>(bits))),
                                       std::bit_cast<double>(bits),"native constructor preserves double bits");
        for(double a:{-3.25,-0.0,0.125,1.0,3.0,1e300}) for(double b:{-2.0,0.0,0.3,7.0}) {
            volatile double x=a,y=b;
            check.same(double(E(a)+E(b)),x+y,"native add");
            check.same(double(E(a)-E(b)),x-y,"native subtract");
            check.same(double(E(a)*E(b)),x*y,"native multiply");
            check.same(double(E(a)/E(b)),x/y,"native divide");
            check.same(double(extendedProduct(E(a),E(b))),x*y,"native product helper");
            check.check((E(a)<E(b))==(x<y) && (E(a)==E(b))==(x==y),"native comparisons");
        }
        for(double x:{-0.0,0.125,0.75,1.0,1.5,2.0,10.0}) {
            check.same(double(sqrt(E(x))),std::sqrt(x),"native sqrt");
            check.same(double(abs(E(-x))),std::fabs(-x),"native abs");
            check.same(double(extendedLog2(E(x))),std::log2(x),"native log2");
            check.same(double(extendedExp2(E(x))),std::exp2(x),"native exp2");
            check.same(double(extendedLogPower(E(x),E(0.3))),std::pow(x,0.3),"native power");
            check.same(double(extendedAtan2(E(x),E(-0.5))),std::atan2(x,-0.5),"native atan2");
            check.same(double(extendedAtan2Wide(E(x),E(-0.5))),std::atan2(x,-0.5),"native wide atan2 bypass");
            check.same(double(extendedSin(E(x))),std::sin(x),"native scalar sine bypass");
            const auto trig=extendedSinCos(E(x));
            check.same(double(trig.sin),std::sin(x),"native sincos sine");
            check.same(double(trig.cos),std::cos(x),"native sincos cosine");
            check.same(double(extendedDivideSmall(E(x),3)),x/3.0,"native small division helper");
        }
        check.same(double(extendedSin(E(0x1p64))),std::sin(0x1p64),"native sine uses native large-argument reduction");
        check.same(double(extendedLogPower(E(-2),E(3))),-8.0,"native power uses native integer exponent semantics");
        check.check(E(2.5).integer()==2 && E(3.5).integer()==4 && E(-2.5).integer()==-2,"native integer nearest-even ties");
        check.check(int32_t(E(-3.9))==-3 && E(2147483647.0).integer()==INT32_MAX,"native valid integer conversions");
        for(double x:{2147483648.0,-2147483649.0,std::numeric_limits<double>::infinity(),
                      std::numeric_limits<double>::quiet_NaN()})
            check.check(int32_t(E(x))==INT32_MIN && E(x).integer()==INT32_MIN,"native integer invalid sentinel without UB");
        check.same(double(round(E(2.5))),2.0,"native round helper");
        check.same(double(trunc(E(-2.75))),-2.0,"native truncate helper");
        check.same(double(compatibleSmall+E(0)),1.0,"compatible object narrows safely in native operation");
        check.check(std::isinf(double(compatibleHuge)),"compatible huge object overflows safely in native conversion");
        nativeSaved=E(0.1);
        check.check(SoftState::read()==poison,"native arithmetic and transcendental paths do not mutate poisoned SoftFloat state");
        {
            ScopedNumericPrecision nested(PrecisionMode::Compatible);
            ExtendedScope extended;
            check.same(double(nativeSaved*E(3)),double(E(0.1)*E(3)),"native object promotes safely into compatible operation");
            check.check(softfloat_roundingMode==softfloat_round_near_even && extF80_roundingPrecision==80,
                        "nested compatible scope configures SoftFloat");
        }
        check.check(NativePrecisionEnabled() && SoftState::read()==poison,"nested compatible scope restores native caller and TLS");
    }
    check.check(!NativePrecisionEnabled() && SoftState::read()==poison,"native profile exit restores caller without SoftFloat writes");

    std::barrier start(4);
    std::atomic<unsigned> concurrentFailures{0};
    std::vector<std::jthread> workers;
    for(unsigned lane=0;lane<4;++lane) workers.emplace_back([&,lane] {
        const auto mode=lane&1?PrecisionMode::Native:PrecisionMode::Compatible;
        ScopedNumericPrecision profile(mode);
        const SoftState marker{uint_fast8_t(lane),32,uint_fast8_t(lane+1),uint_fast8_t(lane%2)};
        marker.write();start.arrive_and_wait();
        for(unsigned i=0;i<100;++i) {
            {
                ExtendedScope extended;
                const E one(1),tiny(0x1p-63);
                const double residual=double((one+tiny)-one);
                if(residual!=(mode==PrecisionMode::Native?0.0:0x1p-63)) ++concurrentFailures;
                if(mode==PrecisionMode::Native && !(SoftState::read()==marker)) ++concurrentFailures;
            }
            if(!(SoftState::read()==marker) || NativePrecisionEnabled()!=(mode==PrecisionMode::Native)) ++concurrentFailures;
        }
    });
    workers.clear();
    check.check(concurrentFailures==0,"concurrent native and compatible profiles remain isolated");
    check.check(SoftState::read()==poison,"worker arithmetic leaves parent SoftFloat state intact");
    original.write();
    std::printf("Native precision: %u checks, %u failures; %zu cross-profile conversion cases\n",
                check.checks,check.failures,conversions.size());
    return check.failures?1:0;
}
