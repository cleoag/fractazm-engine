#define MB3D_METAL_NATIVE_FLOAT 1
#include "native_arithmetic.h"
#include "scene_types.h"
#include "formula_math.h"
#include <bit>
#include <cstdio>
#include <initializer_list>
#include <limits>

using namespace mb3d_metal_exact;
int main() {
    static_assert(sizeof(F64)==sizeof(float)&&sizeof(E80)==sizeof(float)&&sizeof(F32)==sizeof(float));
    unsigned failures=0,checks=0;
    auto check=[&](bool value,const char* name){++checks;if(!value){if(failures<10)std::fprintf(stderr,"FAIL %s\n",name);++failures;}};
    auto checkDouble=[&](U64 bits){
        const double value=std::bit_cast<double>(bits);
        const float expected=float(value),actual=F64::from_bits(bits).value;
        check(std::isnan(expected)?std::isnan(actual):native_bits(actual)==native_bits(expected),"binary64 to native boundary");
    };
    // Random encodings cover the entire exponent range, including exceptional
    // inputs. Arithmetic is deliberately not compared with compatible results.
    U64 rng=0x7e83a9d291b0f416ULL;
    for(unsigned i=0;i<100000;++i){rng^=rng<<13;rng^=rng>>7;rng^=rng<<17;checkDouble(rng);}
    const U64 boundaries[]={0,0x8000000000000000ULL,0x3ff0000010000000ULL,
        0x3ff0000030000000ULL,0x380fffffe0000000ULL,0x3810000000000000ULL,
        0x3690000000000000ULL,0x3690000000000001ULL,0x36a0000000000000ULL,
        0x47efffffe0000000ULL,0x47effffff0000000ULL,0x7ff0000000000000ULL,
        0xfff0000000000000ULL,0x7ff8000000000001ULL};
    for(U64 bits:boundaries){checkDouble(bits);checkDouble(bits^0x8000000000000000ULL);}
    for(unsigned i=0;i<100000;++i){
        rng^=rng<<13;rng^=rng>>7;rng^=rng<<17;
        const float value=native_float(U32(rng));const U64 bits=native_encode_double(value);
        const double decoded=std::bit_cast<double>(bits);
        check(std::isnan(value)?std::isnan(decoded):bits==std::bit_cast<U64>(double(value)),"native to binary64 boundary");
    }
    for(U32 bits:{1u,0x7fffffu,0x800000u,0x80000001u,0x807fffffu,0x3f800000u})
        check(native_encode_double(native_float(bits))==std::bit_cast<U64>(double(native_float(bits))),"subnormal storage roundtrip");
    check(E80::from_bits(0x3ffe,0xb17217f7d1cf79acULL).value==float(0.6931471805599453094L),"extended constant converts to native");
    check(E80::from_bits(0x3fff,0x8000008000000000ULL).value==1.0f,"extended halfway rounds even");
    check(E80::from_bits(0x3fff,0x8000008000000001ULL).value==std::nextafter(1.0f,2.0f),"extended above halfway rounds up");
    check((F64::from_native(16777216.0f)+F64(1)).value==16777216.0f,"native arithmetic has FP32 precision");
    check((E80::from_native(16777216.0f)+E80(1)).value==16777216.0f,"extended wrapper has FP32 precision");
    check((F32::from_native(16777216.0f)+F32(1)).value==16777216.0f,"single arithmetic uses native FP32");
    check(native_log(F64(3)).value==std::log(3.0f),"log uses native float overload");
    check(extended_log2(E80(3)).value==std::log2(3.0f),"log2 uses native float overload");
    const F64 large=F64::from_native(2.0e19f),tiny=F64::from_native(1.0e-25f);
    check(isinf(sqrt(large*large+large*large)),"unscaled large norm overflows FP32 intermediate");
    check(isfinite(native_length3(large,large,large)),"scaled large norm stays representable");
    check(native_length3(large,large,large).value==large.value*std::sqrt(3.0f),"scaled large norm retains magnitude");
    check(iszero(sqrt(tiny*tiny+tiny*tiny)),"unscaled tiny norm underflows FP32 intermediate");
    check(native_length3(tiny,tiny,tiny).value==tiny.value*std::sqrt(3.0f),"scaled tiny norm retains magnitude");
    check(isnan(native_length3(quiet_nan(),F64(1),F64(1))),"scaled norm retains invalid-input fault");
    const auto sphere=SphereIFSLiteralFormula::step({large,large,large},F64(1),60,ExactScene{});
    check(sphere.finite&&isfinite(sphere.shape),"exterior SphereIFS iteration remains finite in FP32");
    for(float x:{0.5f,1.5f,2.5f,-0.5f,-1.5f,-2.5f})
        check(E80::from_native(x).to_i32_rne()==int(std::rint(x)),"nearest-even packing conversion");
    for(float x:{std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN(),2147483648.0f,-4294967296.0f}){
        check(E80::from_native(x).to_i32_rne()==std::numeric_limits<int>::min(),"invalid round avoids integer cast UB");
        check(F64::from_native(x).to_i32_rtz()==std::numeric_limits<int>::min(),"invalid truncation avoids integer cast UB");
    }
    ExactTrig trig{};trig.sin_value=0x3fe0000000000000ULL;trig.cos_value=0x3febb67ae8584caaULL;
    check(valid_camera_trig(trig),"native camera accepts binary64 prepared operands");
    check(camera_sine(trig).value==0.5f,"native camera reads binary64 operand");
    trig.cos_significand=0x8000000000000000ULL;trig.cos_sign_exponent=0x3fff;
    check(!valid_camera_trig(trig),"native camera rejects compatible prepared operands");
    std::printf("Native FP32 arithmetic: %u checks, %u failures\n",checks,failures);
    return failures?1:0;
}
