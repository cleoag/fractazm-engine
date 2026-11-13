#pragma once
#ifdef MB3D_METAL_NATIVE_FLOAT
#include "native_arithmetic.h"
#else
#include "soft_extended.h"

#ifdef __METAL_VERSION__
#define MB3D_SINGLE_THREAD thread
#else
#define MB3D_SINGLE_THREAD
#endif
namespace mb3d_metal_exact {
// Binary32 operands are exact in binary64. Their product has <=48 bits;
// division/sqrt have sufficient guard precision to decide the binary32 result.
// Explicit stores retain gradual underflow without Metal's native float mode.
struct F32 {
    U32 bits;
    F32() : bits(0) {}
    explicit F32(int n) : bits(F64(n).to_f32_bits()) {}
    explicit F32(unsigned n) : bits(F64(n).to_f32_bits()) {}
    explicit F32(F64 n) : bits(n.to_f32_bits()) {}
    static F32 from_bits(U32 b) { F32 r; r.bits=b; return r; }
    static F32 from_f64(F64 n) { return F32(n); }
    F64 to_f64() const { return F64::from_f32_bits(bits); }
    int to_i32_rtz() const { return to_f64().to_i32_rtz(); }
    int to_i32_rne() const { return E80(to_f64()).to_i32_rne(); }
#ifndef __METAL_VERSION__
    static F32 from_float(float f) { U32 b; std::memcpy(&b,&f,4); return from_bits(b); }
    float to_float() const { float f; std::memcpy(&f,&bits,4); return f; }
#endif
};
inline F32 operator+(F32 a,F32 b) { return F32(a.to_f64()+b.to_f64()); }
inline F32 operator-(F32 a,F32 b) { return F32(a.to_f64()-b.to_f64()); }
inline F32 operator*(F32 a,F32 b) { return F32(a.to_f64()*b.to_f64()); }
inline F32 operator/(F32 a,F32 b) { return F32(a.to_f64()/b.to_f64()); }
inline F32 operator-(F32 a) { return F32::from_bits(a.bits^0x80000000U); }
inline bool operator==(F32 a,F32 b) { return a.to_f64()==b.to_f64(); }
inline bool operator!=(F32 a,F32 b) { return !(a==b); }
inline bool operator<(F32 a,F32 b) { return a.to_f64()<b.to_f64(); }
inline bool operator>(F32 a,F32 b) { return b<a; }
inline bool operator<=(F32 a,F32 b) { return a<b||a==b; }
inline bool operator>=(F32 a,F32 b) { return b<=a; }
inline MB3D_SINGLE_THREAD F32& operator+=(MB3D_SINGLE_THREAD F32& a,F32 b) { a=a+b;return a; }
inline MB3D_SINGLE_THREAD F32& operator-=(MB3D_SINGLE_THREAD F32& a,F32 b) { a=a-b;return a; }
inline MB3D_SINGLE_THREAD F32& operator*=(MB3D_SINGLE_THREAD F32& a,F32 b) { a=a*b;return a; }
inline MB3D_SINGLE_THREAD F32& operator/=(MB3D_SINGLE_THREAD F32& a,F32 b) { a=a/b;return a; }
inline F32 sqrt(F32 a) { return F32(sqrt(a.to_f64())); }
inline F32 abs(F32 a) { return F32::from_bits(a.bits&0x7fffffffU); }
inline bool isfinite(F32 a) { return isfinite(a.to_f64()); }
inline bool isnan(F32 a) { return isnan(a.to_f64()); }
inline bool isinf(F32 a) { return isinf(a.to_f64()); }
inline bool iszero(F32 a) { return (a.bits&0x7fffffffU)==0; }
inline bool signbit(F32 a) { return (a.bits>>31)!=0; }
inline F32 min(F32 a,F32 b) { return b<a?b:a; }
inline F32 max(F32 a,F32 b) { return a<b?b:a; }
}

#undef MB3D_SINGLE_THREAD

#endif // MB3D_METAL_NATIVE_FLOAT
