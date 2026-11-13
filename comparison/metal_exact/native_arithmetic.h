#pragma once

// Native render profile: every arithmetic value is hardware binary32. The
// integer code below only converts the unchanged ABI's binary64/ext80 storage
// words at boundaries; it does not implement floating arithmetic. Compatible
// arithmetic headers are excluded from this translation unit entirely.
#ifdef __METAL_VERSION__
#include <metal_stdlib>
#define MB3D_NATIVE_THREAD thread
#define MB3D_NATIVE_MATH metal
#else
#if defined(_MSC_VER)
#include <intrin.h>
#endif
#include <cmath>
#include <cstdint>
#include <cstring>
#define MB3D_NATIVE_THREAD
#define MB3D_NATIVE_MATH std
#endif

namespace mb3d_metal_exact {
#ifdef __METAL_VERSION__
using U64 = unsigned long;
#else
using U64 = unsigned long long;
#endif
using U32 = unsigned int;
inline float native_float(U32 bits) {
#ifdef __METAL_VERSION__
    return as_type<float>(bits);
#else
    float value;std::memcpy(&value,&bits,4);return value;
#endif
}
inline U32 native_bits(float value) {
#ifdef __METAL_VERSION__
    return as_type<U32>(value);
#else
    U32 bits;std::memcpy(&bits,&value,4);return bits;
#endif
}
inline U64 native_round_shift(U64 value,unsigned shift) {
    if(shift>64)return 0;
    if(shift==64)return value>0x8000000000000000ULL;
    const U64 q=value>>shift,mask=(1ULL<<shift)-1,midpoint=1ULL<<(shift-1);
    const U64 tail=value&mask;
    return q+(tail>midpoint||(tail==midpoint&&(q&1)));
}
// Canonical finite significand has its leading bit at `leading` (52 or 63).
// A single nearest-even rounding converts it to the ABI boundary's FP32 value.
inline float native_decode_finite(U32 sign,int exponent,U64 significand,unsigned leading) {
    if(!significand)return native_float(sign);
    int biased=exponent+127;
    if(biased>254)return native_float(sign|0x7f800000u);
    if(biased<=0) {
        const int shift=int(leading)-22-biased;
        if(shift>64)return native_float(sign);
        return native_float(sign|U32(native_round_shift(significand,unsigned(shift))));
    }
    U64 rounded=native_round_shift(significand,leading-23);
    if(rounded>=0x1000000ULL){rounded>>=1;++biased;}
    if(biased>254)return native_float(sign|0x7f800000u);
    return native_float(sign|(U32(biased)<<23)|(U32(rounded)&0x7fffffu));
}
inline float native_decode_double(U64 bits) {
    const U32 sign=U32(bits>>32)&0x80000000u;
    const unsigned exponent=unsigned((bits>>52)&2047);
    const U64 fraction=bits&0xfffffffffffffULL;
    if(exponent==2047)return native_float(sign|(fraction?0x7fc00000u:0x7f800000u));
    // Every binary64 subnormal underflows to binary32 zero.
    if(!exponent)return native_float(sign);
    return native_decode_finite(sign,int(exponent)-1023,fraction|0x10000000000000ULL,52);
}
inline U64 native_encode_double(float value) {
    const U32 bits=native_bits(value),fraction=bits&0x7fffffu;
    const unsigned exponent=(bits>>23)&255;
    const U64 sign=U64(bits&0x80000000u)<<32;
    if(exponent==255)return sign|(fraction?0x7ff8000000000000ULL:0x7ff0000000000000ULL);
    if(exponent)return sign|(U64(exponent+896)<<52)|(U64(fraction)<<29);
    if(!fraction)return sign;
#ifdef __METAL_VERSION__
    const unsigned shift=metal::clz(fraction)-8;
#elif defined(_MSC_VER)
    unsigned long leading_bit=0;
    _BitScanReverse(&leading_bit,fraction); // fraction is nonzero here.
    const unsigned shift=23u-unsigned(leading_bit);
#else
    const unsigned shift=unsigned(__builtin_clz(fraction))-8;
#endif
    return sign|(U64(897-shift)<<52)|((U64(fraction)<<shift&0x7fffffULL)<<29);
}
inline int native_integer(float value,bool nearest) {
    if(nearest)value=MB3D_NATIVE_MATH::rint(value);
    // Check the hardware conversion's range first; +INT_MAX rounds to 2^31.
    if(!MB3D_NATIVE_MATH::isfinite(value)||value>=2147483648.0f||value< -2147483648.0f)
        return (-2147483647-1);
    return int(value);
}

struct F64 {
    float value;
    F64():value(0){}
    explicit F64(int n):value(float(n)){}
    explicit F64(unsigned n):value(float(n)){}
    static F64 from_native(float n){F64 r;r.value=n;return r;}
    static F64 from_bits(U64 bits){return from_native(native_decode_double(bits));}
    static F64 from_i32(int n){return F64(n);}
    static F64 from_u32(unsigned n){return F64(n);}
    static F64 from_f32_bits(U32 bits){return from_native(native_float(bits));}
    U64 to_bits()const{return native_encode_double(value);}
    U32 to_f32_bits()const{return native_bits(value);}
    int to_i32_rtz()const{return native_integer(value,false);}
#ifndef __METAL_VERSION__
    static F64 from_double(double n){return from_native(float(n));}
    double to_double()const{return double(value);}
#endif
};
struct E80 {
    float value;
    E80():value(0){}
    explicit E80(F64 n):value(n.value){}
    explicit E80(int n):value(float(n)){}
    static E80 from_native(float n){E80 r;r.value=n;return r;}
    static E80 from_bits(U32 signExponent,U64 significand){
        const U32 sign=(signExponent&0x8000u)<<16;
        const unsigned exponent=signExponent&0x7fffu;
        if(exponent==0x7fff)return from_native(native_float(sign|
            ((significand&0x7fffffffffffffffULL)?0x7fc00000u:0x7f800000u)));
        if(!exponent)return from_native(native_float(sign));
        return from_native(native_decode_finite(sign,int(exponent)-16383,significand,63));
    }
    F64 to_f64()const{return F64::from_native(value);}
    U32 to_f32_bits()const{return native_bits(value);}
    int to_i32_rtz()const{return native_integer(value,false);}
    int to_i32_rne()const{return native_integer(value,true);}
};
struct F32 {
    float value;
    F32():value(0){}
    explicit F32(int n):value(float(n)){}
    explicit F32(unsigned n):value(float(n)){}
    explicit F32(F64 n):value(n.value){}
    static F32 from_native(float n){F32 r;r.value=n;return r;}
    static F32 from_bits(U32 bits){return from_native(native_float(bits));}
    static F32 from_f64(F64 n){return F32(n);}
    F64 to_f64()const{return F64::from_native(value);}
    U32 to_bits()const{return native_bits(value);}
    int to_i32_rtz()const{return native_integer(value,false);}
    int to_i32_rne()const{return native_integer(value,true);}
#ifndef __METAL_VERSION__
    static F32 from_float(float n){return from_native(n);}
    float to_float()const{return value;}
#endif
};

#define MB3D_NATIVE_OPERATORS(T) \
inline T operator+(T a,T b){return T::from_native(a.value+b.value);} \
inline T operator-(T a,T b){return T::from_native(a.value-b.value);} \
inline T operator*(T a,T b){return T::from_native(a.value*b.value);} \
inline T operator/(T a,T b){return T::from_native(a.value/b.value);} \
inline T operator-(T a){return T::from_native(-a.value);} \
inline bool operator==(T a,T b){return a.value==b.value;} \
inline bool operator!=(T a,T b){return a.value!=b.value;} \
inline bool operator<(T a,T b){return a.value<b.value;} \
inline bool operator>(T a,T b){return a.value>b.value;} \
inline bool operator<=(T a,T b){return a.value<=b.value;} \
inline bool operator>=(T a,T b){return a.value>=b.value;} \
inline MB3D_NATIVE_THREAD T& operator+=(MB3D_NATIVE_THREAD T& a,T b){a=a+b;return a;} \
inline MB3D_NATIVE_THREAD T& operator-=(MB3D_NATIVE_THREAD T& a,T b){a=a-b;return a;} \
inline MB3D_NATIVE_THREAD T& operator*=(MB3D_NATIVE_THREAD T& a,T b){a=a*b;return a;} \
inline MB3D_NATIVE_THREAD T& operator/=(MB3D_NATIVE_THREAD T& a,T b){a=a/b;return a;} \
inline T sqrt(T a){return T::from_native(MB3D_NATIVE_MATH::sqrt(a.value));} \
inline T abs(T a){return T::from_native(MB3D_NATIVE_MATH::fabs(a.value));} \
inline bool isfinite(T a){return MB3D_NATIVE_MATH::isfinite(a.value);} \
inline bool isnan(T a){return MB3D_NATIVE_MATH::isnan(a.value);} \
inline bool isinf(T a){return MB3D_NATIVE_MATH::isinf(a.value);} \
inline bool iszero(T a){return a.value==0.0f;} \
inline bool signbit(T a){return (native_bits(a.value)>>31)!=0;} \
inline T min(T a,T b){return b<a?b:a;} \
inline T max(T a,T b){return a<b?b:a;}
MB3D_NATIVE_OPERATORS(F64)
MB3D_NATIVE_OPERATORS(E80)
MB3D_NATIVE_OPERATORS(F32)
#undef MB3D_NATIVE_OPERATORS

inline F64 quiet_nan(){return F64::from_f32_bits(0x7fc00000u);}
inline F64 infinity(bool sign){return F64::from_f32_bits((U32(sign)<<31)|0x7f800000u);}
inline F64 zero(bool sign){return F64::from_f32_bits(U32(sign)<<31);}
inline E80 ext_nan(){return E80(quiet_nan());}
inline E80 ext_inf(bool sign){return E80(infinity(sign));}
inline E80 ext_zero(bool sign){return E80(zero(sign));}
inline F64 native_log(F64 n){return F64::from_native(MB3D_NATIVE_MATH::log(n.value));}
inline F64 native_length3(F64 x,F64 y,F64 z){
    if(!isfinite(x)||!isfinite(y)||!isfinite(z))return quiet_nan();
    const F64 magnitude=max(abs(x),max(abs(y),abs(z)));
    if(iszero(magnitude))return F64(0);
    // Keep the ordinary hardware dot/sqrt path; scale only when an otherwise
    // representable norm would overflow/underflow its squared intermediates.
    if(magnitude.value>=1.0e-18f&&magnitude.value<=1.0e18f)
        return sqrt((x*x+y*y)+z*z);
    x=x/magnitude;y=y/magnitude;z=z/magnitude;
    return magnitude*sqrt((x*x+y*y)+z*z);
}
inline E80 extended_log2(E80 n){return E80::from_native(MB3D_NATIVE_MATH::log2(n.value));}
inline E80 extended_ln2(){return E80::from_native(0.6931471805599453094f);}
struct ExtendedNumericalDE {F64 raw,denominator;};
inline ExtendedNumericalDE extended_numerical_de(F64 radius,F64 dxSquared,
        F64 dySquared,F64 dzSquared,F64 scale,F64 offset){
    const F64 denominator=sqrt((dzSquared+dySquared)+dxSquared)+offset;
    return {((native_log(radius)*radius)*scale)/denominator,denominator};
}
} // namespace mb3d_metal_exact
#undef MB3D_NATIVE_THREAD
#undef MB3D_NATIVE_MATH
