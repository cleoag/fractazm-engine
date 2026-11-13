#include "core/extended80.h"
#include "core/extended_transcendentals.h"
#include "core/math3d.h"
#include <atomic>
#include <barrier>
#include <bit>
#include <cfenv>
#include <cstdio>
#include <limits>
#include <thread>
#include <vector>
#include "fixtures/ray_refinement_regressions.h"
#include "fixtures/packed_normal_regressions.h"
#include "fixtures/clip15_regressions.h"
#include "fixtures/extended_transcendental_reference.h"

int main() {
    using namespace mb3d;
    using namespace mb3d::numeric;
    int checks = 0, failures = 0;
    auto check = [&](bool pass, const char* message) {
        ++checks;
        if (!pass) { ++failures; std::fprintf(stderr, "FAIL: %s\n", message); }
    };
    check(sizeof(long double) == sizeof(double), "Apple ARM64 ABI");
    check(std::fegetround() == FE_TONEAREST, "hardware default rounding");
    for (const auto& row : rayRefinementRegressions) {
        TVec3D p{row.input[0],row.input[1],row.input[2]}, d{row.input[5],row.input[6],row.input[7]};
        double zz = row.input[3];
        const float stop = RefineRayStep(p,zz,d,row.input[4],row.input[8],row.input[9]);
        for (int i=0;i<3;++i) check(std::bit_cast<uint64_t>(p[i]) == row.expected[i], "independent ray coordinate bits");
        check(std::bit_cast<uint64_t>(zz) == row.expected[3], "independent ray depth bits");
        check(std::bit_cast<uint32_t>(stop) == row.expected[4], "independent ray stop bits");
    }
    for (const auto& row : packedNormalRegressions) {
        TVec3D p{std::bit_cast<double>(row.bits[0]),std::bit_cast<double>(row.bits[1]),std::bit_cast<double>(row.bits[2])};
        int16_t output[3]; MakeWNormalsFromDVec(output,p);
        for (int i=0;i<3;++i) check(output[i] == row.expected[i], "independent packed normal");
    }
    for (const auto& row : clip15Regressions) {
        uint16_t out; MinMaxClip15bit(std::bit_cast<float>(row.bits),out);
        check(out == row.expected, "independent clipping including signaling NaN");
    }
    {
        ExtendedScope scope;
        using E=Extended80;
        auto value=[](ExtendedReferenceBits v) {return E::fromBits(v.exponent,v.significand);};
        for(const auto& row:extendedReferences) {
            const E a=value(row.a),b=value(row.b),expected=value(row.expected);
            E actual;
            switch(row.operation) {
            case 0:actual=extendedSinCos(a).sin;break;
            case 1:actual=extendedSinCos(a).cos;break;
            case 2:actual=extendedAtan2(a,b);break;
            case 3:actual=extendedLog2(a);break;
            default:actual=extendedExp2(a);break;
            }
            const E magnitude=abs(expected)>E(1) ? abs(expected) : E(1);
            check(abs(actual-expected)<=E(0x1p-60)*magnitude,"independent extended transcendental reference");
            if(row.operation==2)
                check(extendedAtan2Wide(a,b)==expected,"wide atan2 rounds independent reference to ext80");
        }
        check(std::isnan(double(extendedLogPower(E(0),E(2)))),"log power retains original zero-base NaN");
        check(std::isnan(double(extendedLogPower(E(-1),E(2)))),"log power retains original negative-base NaN");
        check(extendedExp2(E(-16445))==E::fromBits(0,1),"extended minimum subnormal exp2");
        check(extendedLog2(E::fromBits(0,1))==E(-16445),"extended minimum subnormal logarithm");
        check(extendedExp2(E(-16446))==E(0),"extended half-subnormal rounds to even zero");
        check(extendedExp2(E(16384))==E(std::numeric_limits<double>::infinity()),"extended exp2 overflow");
        check(extendedSinCos(E(0x1p63)).cos==E(0x1p63) && std::isnan(double(extendedSinCos(E(0x1p63)).sin)),"selected empty-stack FSINCOS range behavior");
        check(std::signbit(double(extendedSinCos(E(-0.)).sin)),"sine negative zero");
        check(std::signbit(double(extendedAtan2(E(-0.),E(1)))),"atan2 negative zero");
        const Extended80 zero(0.0), minusZero(-0.0);
        check(std::bit_cast<uint64_t>(double(zero + zero)) == 0, "positive zero addition is defined");
        check(std::bit_cast<uint64_t>(double(minusZero + minusZero)) == 0x8000000000000000ULL,
              "negative zero addition preserves sign");
    }
    // Overlap different caller environments to catch accidentally process-global
    // SoftFloat state, and verify nested scopes restore the caller exactly.
    std::barrier barrier(4);
    std::atomic<int> threadFailures{0};
    std::vector<std::jthread> threads;
    for (unsigned i=0;i<4;++i) threads.emplace_back([&,i] {
        softfloat_roundingMode = i;
        extF80_roundingPrecision = 32;
        softfloat_exceptionFlags = i + 1;
        barrier.arrive_and_wait();
        for (int k=0;k<1000;++k) {
            {
                ExtendedScope scope;
                const Extended80 one(1), tiny(0x1p-63);
                if (double(-one) != -1 || double(abs(-one)) != 1) ++threadFailures;
                if (double(x87Pi()) != 0x1.921fb54442d18p+1) ++threadFailures;
                if (double((one + tiny) - one) != 0x1p-63) ++threadFailures;
                const Extended80 huge(1e300);
                if (double(sqrt(huge*huge)) != 1e300) ++threadFailures;
                if (extendedAtan2(one,one)!=x87Pi()/Extended80(4)) ++threadFailures;
                if (extendedAtan2Wide(one,one)!=x87Pi()/Extended80(4)) ++threadFailures;
                if (double(extendedExp2(Extended80(10)))!=1024.) ++threadFailures;
                if (double(extendedSinCos(Extended80(.5)).sin)!=0x1.eaee8744b05f0p-2) ++threadFailures;
                if (std::bit_cast<uint64_t>(double(Extended80(-0.0)*one)) != 0x8000000000000000ULL) ++threadFailures;
                { ExtendedScope nested; (void)double(one/Extended80(0)); }
                if (softfloat_roundingMode != softfloat_round_near_even || extF80_roundingPrecision != 80) ++threadFailures;
            }
            if (softfloat_roundingMode != i || extF80_roundingPrecision != 32 || softfloat_exceptionFlags != i+1) ++threadFailures;
        }
    });
    threads.clear();
    check(threadFailures == 0, "4000 overlapping/nested software arithmetic scopes restore TLS");
    std::printf("ARM64 numerical primitives: %d checks, %d failures\n",checks,failures);
    return failures ? 1 : 0;
}
