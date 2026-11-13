#pragma once
#include <metal_stdlib>
#include "formula_abi.h"
#include "formula_selector.h"
#include "riemann_reconstruction.metal.h"
#include "translate_add.metal.h"
// Only boundary bit conversions from this header are reused. Its arithmetic
// types contain float and never perform software binary64/extended operations.
#include "comparison/metal_exact/native_arithmetic.h"

namespace mb3d_native {
using namespace metal;
// Offline-library function constants select the uploaded six namespaced
// identities. Indices0..4 belong to the existing scene ABI;16 is the driver.
constant uint native_formula_id0 [[function_constant(10)]];
constant uint native_formula_id1 [[function_constant(11)]];
constant uint native_formula_id2 [[function_constant(12)]];
constant uint native_formula_id3 [[function_constant(13)]];
constant uint native_formula_id4 [[function_constant(14)]];
constant uint native_formula_id5 [[function_constant(15)]];
inline bool native_formula_selector_selected(uint identity){
    return native_formula_id0==identity||native_formula_id1==identity||native_formula_id2==identity||
        native_formula_id3==identity||native_formula_id4==identity||native_formula_id5==identity;
}
inline bool native_formula_specialization_matches(constant NativeFormulaProgram& p){
    if(!is_function_constant_defined(native_formula_id0)||!is_function_constant_defined(native_formula_id1)||
       !is_function_constant_defined(native_formula_id2)||!is_function_constant_defined(native_formula_id3)||
       !is_function_constant_defined(native_formula_id4)||!is_function_constant_defined(native_formula_id5))return false;
    uint ids[6]={native_formula_id0,native_formula_id1,native_formula_id2,native_formula_id3,native_formula_id4,native_formula_id5};
    for(uint i=0;i<6;++i){constant auto& s=p.slots[i];
        if(!ValidFormulaSelectorIdentity(s.namespace_id,s.id,s.enabled)||
           ids[i]!=FormulaPipelineSelector(s.namespace_id,s.id,s.enabled))return false;}
    return true;
}
constant constexpr uint MAX_FORMULA_PARAMS=64;
constant constexpr uint FORMULA_ORBIT_W_SLOT=47,FORMULA_STATE_X_SLOT=48,FORMULA_STATE_Y_SLOT=49;
constant constexpr uint FORMULA_STATE_Z_SLOT=50,FORMULA_STATE_W_SLOT=51;
constant constexpr uint WRITE_J1_SLOT=52,WRITE_J2_SLOT=53,WRITE_J3_SLOT=54,WRITE_J4_SLOT=55;
constant constexpr uint FORMULA_ROUT_SLOT=56,BFIRSTIT_SLOT=57,DIFS_ROLD_SLOT=58,ITER_COUNT_SLOT=59;
constant constexpr uint DIFS_VARYSCALE_SLOT=60,DIFS_COMMIT_XYZ_SLOT=61;
constant constexpr float M_PI=3.14159265358979323846f;
constant constexpr float M_LOG2E=1.44269504088896340736f;
constant constexpr int INT32_MIN=(-2147483647-1);
// Canonical three-square norms are explicitly scaled for Native FP32. This
// avoids overflow of intermediate squares when the final length is finite.
inline float native_formula_length3(float x,float y,float z){
    float3 v=float3(x,y,z);
    if(any(isnan(v)))return NAN;
    float m=max(max(abs(x),abs(y)),abs(z));
    if(m==0.0f||isinf(m))return m;
    if(m>=1.0e-18f&&m<=1.0e18f)return sqrt((x*x+y*y)+z*z);
    v/=m;
    return m*sqrt((v.x*v.x+v.y*v.y)+v.z*v.z);
}
// Scalar fields preserve source reference semantics. MSL vector components
// cannot bind to non-const references used by the original formula helpers.
struct FormulaVec3 {
    float x,y,z;
    FormulaVec3():x(0),y(0),z(0){}
    FormulaVec3(float a):x(a),y(a),z(a){}
    FormulaVec3(float a,float b,float c):x(a),y(b),z(c){}
    FormulaVec3(float3 v):x(v.x),y(v.y),z(v.z){}
    operator float3() const{return float3(x,y,z);}
    void operator+=(float3 v){x+=v.x;y+=v.y;z+=v.z;}
    thread float& operator[](int i){return i==0?x:i==1?y:z;}
    thread const float& operator[](int i)const{return i==0?x:i==1?y:z;}
};
struct FormulaParams {
    thread float* data;
    constant NativeFormulaSlot* slot;
    thread const NativeResources* resources;
    thread NativeFormulaFault* fault;
    thread int* counter;
    thread bool* counter_written;
    uint local_limit;
    thread float& operator[](uint i)const{return data[i];}
    operator thread float*()const{return data;}
    thread float* operator+(uint i)const{return data+i;}
    ulong bits(uint i)const{
        return i<slot->param_count?(ulong(slot->param_bits[i][1])<<32)|slot->param_bits[i][0]:mb3d_metal_exact::native_encode_double(data[i]);
    }
};
// Box inner-sphere scale Scale/MinR^2, read only by that branch. A zero saved
// MinR makes it a finite binary64 beyond FP32 (Scale/1e-80); the host uploads
// its binary32 rounding, infinity, under the parameter range policy. The
// scaled point then leaves binary32 and the driver takes the counted range
// exit. Never clamp it to a finite scale.
inline float native_aboxmod1_inner_scale(FormulaParams params) {
    return params[1];
}
inline bool native_aboxmod1_inner(float radius_squared,FormulaParams params) {
    // A positive binary64 MinR^2 can underflow during FP32 upload. Zero
    // radius is still inside it; preserve that branch and its fault guard.
    const ulong threshold=params.bits(2);
    return radius_squared<params[2] || (radius_squared==0.f && params[2]==0.f &&
        threshold>0UL && threshold<0x7ff0000000000000UL);
}
// Amazing Surf and ABoxSphereOffset4d use the original unordered !(r>=min)
// branch. Keep that distinction from ABoxMod1's ordered comparison, including
// zero radius against a positive binary64 threshold that underflows in FP32.
inline bool native_box_inner_unordered(float radius_squared,FormulaParams params) {
    return isnan(radius_squared) || native_aboxmod1_inner(radius_squared,params);
}
inline thread int& native_formula_counter(FormulaParams params){
    *params.counter_written=true;
    return *params.counter;
}
inline int native_int32_parameter(FormulaParams params,uint index){
    // ReadFormulaParams converted a declared int32 into an exact binary64
    // value. Decode that integer payload with shifts; never round via FP32.
    // Audited generated call sites use immutable declared integer slots.
    if(params.fault->code)return 0;
    if(index<params.slot->param_count){
        const ulong bits=(ulong(params.slot->param_bits[index][1])<<32)|params.slot->param_bits[index][0];
        const ulong magnitude=bits&0x7fffffffffffffffUL;
        if(!magnitude)return 0;
        const uint exponent=uint((bits>>52)&2047u);
        if(exponent>=1023u&&exponent<=1054u){
            const uint shift=1075u-exponent;
            const ulong significand=(1UL<<52)|(bits&0x000fffffffffffffUL);
            const ulong value=significand>>shift;
            const bool negative=(bits>>63)!=0;
            if(!(significand&((1UL<<shift)-1UL))&&value<=(negative?0x80000000UL:0x7fffffffUL))
                return as_type<int>(negative?0u-uint(value):uint(value));
        }
    }
    params.fault->code=InvalidFormulaParameter;params.fault->formula_id=params.slot->id;
    return 0;
}
inline float native_fistp_sector(float value,thread int& integer){
    // Preserve the original finite FISTP/FILD sector in Native FP32: nearest
    // even, int32 indefinite on overflow, and a positive zero after reload.
    // Nonfinite values keep the existing range/fault path instead of becoming
    // finite geometry through an integer-indefinite replacement.
    const float rounded=rint(value);
    if(!isfinite(rounded)){
        integer=INT32_MIN;
        return rounded;
    }
    integer=(rounded>=-2147483648.0f&&rounded<2147483648.0f)
        ?int(rounded):INT32_MIN;
    return float(integer);
}
inline int native_increment_formula_counter(FormulaParams params){
    // bFirstIt is an int32 word. Preserve wrap/low-bit dispatch independently
    // of the FP32 numeric scratch, which cannot represent every int above2^24.
    int next=as_type<int>(as_type<uint>(*params.counter)+1u);
    *params.counter=next;*params.counter_written=true;params[BFIRSTIT_SLOT]=float(next);
    return next;
}
struct FormulaWorkBudget {
    FormulaParams params;uint steps;
    FormulaWorkBudget(FormulaParams p,constant const char*):params(p),steps(0){}
    bool beforeStep(){
        if(params.fault->code)return false;
        if(steps>=params.local_limit||params.fault->work>=params.fault->limit){params.fault->code=FormulaWorkExhausted;params.fault->formula_id=params.slot->id;return false;}
        ++steps;++params.fault->work;return true;
    }
};
inline void native_store_packed_single(thread ulong& bits,thread float& numeric_value,int offset,float value){
    uint word=as_type<uint>(value);
    if(offset==24)bits=(bits&0xffffffff00000000UL)|word;
    else if(offset==28)bits=(bits&0xffffffffUL)|(ulong(word)<<32);
    else {numeric_value=NAN;return;}
    numeric_value=mb3d_metal_exact::native_decode_double(bits);
}

namespace numeric {
// Names retain source correspondence; representation and every operation are
// hardware FP32. Scope objects are inert per-call tags, not precision switches.
using Extended80=float;
using X87Real=float;
struct ExtendedScope {};
struct X87Scope {};
struct SinCos {float sin,cos;};
inline SinCos extendedSinCos(float x){return {metal::sin(x),metal::cos(x)};}
inline float extendedSin(float x){return metal::sin(x);}
inline float extendedCos(float x){return metal::cos(x);}
inline float extendedAtan2(float y,float x){return metal::atan2(y,x);}
inline float extendedAtan2Wide(float y,float x){return metal::atan2(y,x);}
inline float extendedLog2(float x){return metal::log2(x);}
inline float extendedExp2(float x,bool=false){return metal::exp2(x);}
inline float extendedLogPower(float x,float power){return metal::exp2(power*metal::log2(x));}
inline float extendedLn2(){return 0.69314718055994530942f;}
inline float extendedNaN(){return NAN;}
inline float extendedInfinity(bool negative=false){return negative?-INFINITY:INFINITY;}
inline float x87Pi(){return M_PI;}
inline float x87Abs(float x){return metal::abs(x);}
inline float x87Sqrt(float x){return metal::sqrt(x);}
inline float x87Round(float x){return metal::rint(x);}
inline float fromBits(uint exponent,ulong significand){
    return mb3d_metal_exact::E80::from_bits(exponent,significand).value;
}
inline uint signExponent(float x){
    uint bits=as_type<uint>(x),e=(bits>>23)&255u;
    if(e==255u)return ((bits>>16)&0x8000u)|0x7fffu;
    if(!e)return (bits>>16)&0x8000u;
    return ((bits>>16)&0x8000u)|(e+16256u);
}
}
namespace native_std {
template<class A,class B> struct pair { A first; B second; pair(A a,B b):first(a),second(b){} };
inline void memcpy(thread int* dst,thread const uint* src,ulong){*dst=as_type<int>(*src);}
inline void memcpy(thread ulong* dst,thread const float* src,ulong){*dst=mb3d_metal_exact::native_encode_double(*src);}
inline void memcpy(thread float* dst,thread const ulong* src,ulong){*dst=mb3d_metal_exact::native_decode_double(*src);}
template<class T> inline void swap(thread T& a,thread T& b){T saved=a;a=b;b=saved;}
template<class T> struct numeric_limits;
template<> struct numeric_limits<float>{
    static float quiet_NaN(){return NAN;}
    static float infinity(){return INFINITY;}
    static float max(){return 0x1.fffffep127f;}
    static float min(){return 0x1p-126f;}
    static float lowest(){return -max();}
};
template<> struct numeric_limits<int>{
    static int min(){return (-2147483647-1);}
    static int max(){return 2147483647;}
};
template<> struct numeric_limits<uint>{
    static uint min(){return 0;}
    static uint max(){return 0xffffffffu;}
};
template<class T> inline T bit_cast(float x);
template<> inline uint bit_cast<uint>(float x){return as_type<uint>(x);}
template<> inline int bit_cast<int>(float x){return as_type<int>(x);}
template<> inline ulong bit_cast<ulong>(float x){return mb3d_metal_exact::native_encode_double(x);}
template<class T> inline T bit_cast(ulong x);
template<> inline float bit_cast<float>(ulong x){return mb3d_metal_exact::native_decode_double(x);}
template<class T> inline T bit_cast(uint x);
template<> inline float bit_cast<float>(uint x){return as_type<float>(x);}
}
inline bool NativePrecisionEnabled(){return true;}
inline float x87_frndint(float x){return metal::rint(x);}
inline float x87_fprem(float a,float b){return (!isfinite(a)||!isfinite(b)||b==0)?NAN:a-trunc(a/b)*b;}
inline float x87_log_power(float x,float power){return numeric::extendedLogPower(x,power);}
inline int x87_fistp_int32(float x){return mb3d_metal_exact::native_integer(x,true);}
inline float sign(float x){return x>0?1.0f:x<0?-1.0f:0.0f;}
inline int cpu_bitcast_to_int(float x){return as_type<int>(x);}
inline float sincos(float angle,thread float& c){c=metal::cos(angle);return metal::sin(angle);}
inline void sincos_of_power_theta(float num,float den,float power,thread float& s,thread float& c){
    float angle=metal::atan2(num,den)*power;s=metal::sin(angle);c=metal::cos(angle);
}
struct RotMatrix3 {
    float m[3][3];float3 col0,col1,col2;
    float3 operator*(float3 v)const{
        return float3(m[0][0]*v.x+m[0][1]*v.y+m[0][2]*v.z,
                      m[1][0]*v.x+m[1][1]*v.y+m[1][2]*v.z,
                      m[2][0]*v.x+m[2][1]*v.y+m[2][2]*v.z);
    }
};
inline void finish_matrix3(thread RotMatrix3& r){
    r.col0=float3(r.m[0][0],r.m[1][0],r.m[2][0]);
    r.col1=float3(r.m[0][1],r.m[1][1],r.m[2][1]);
    r.col2=float3(r.m[0][2],r.m[1][2],r.m[2][2]);
}
inline RotMatrix3 rotMatrix3FromFloats(thread const float* p,int k){
    RotMatrix3 r{};for(int a=0;a<3;++a)for(int b=0;b<3;++b)r.m[a][b]=p[k+3*a+b];finish_matrix3(r);return r;
}
inline RotMatrix3 heightMapRotMatrix3FromFloats(thread const float* p,int k){
    RotMatrix3 r{};for(int a=0;a<3;++a)for(int b=0;b<3;++b)r.m[a][b]=p[k+((a+2)%3)*3+b];finish_matrix3(r);return r;
}
inline RotMatrix3 buildRotMatrix3(float x,float y,float z){
    x*=M_PI/180.0f;y*=M_PI/180.0f;z*=M_PI/180.0f;
    float sx=sin(x),sy=sin(y),sz=sin(z),cx=cos(x),cy=cos(y),cz=cos(z);
    RotMatrix3 r{};
    r.m[0][0]=cy*cz;r.m[0][1]=-cy*sz;r.m[0][2]=sy;
    r.m[1][0]=sx*sy*cz+cx*sz;r.m[1][1]=cx*cz-sx*sy*sz;r.m[1][2]=-sx*cy;
    r.m[2][0]=sx*sz-cx*sy*cz;r.m[2][1]=sx*cz+cx*sy*sz;r.m[2][2]=cx*cy;
    finish_matrix3(r);return r;
}
struct RotMatrix4{float m[4][4];};
inline RotMatrix4 rotMatrix4FromFloats(thread const float* p,int k){
    RotMatrix4 r{};for(int a=0;a<4;++a)for(int b=0;b<4;++b)r.m[a][b]=p[k+4*a+b];return r;
}
inline float4 operator*(RotMatrix4 r,float4 v){
    return float4(dot(float4(r.m[0][0],r.m[0][1],r.m[0][2],r.m[0][3]),v),
                  dot(float4(r.m[1][0],r.m[1][1],r.m[1][2],r.m[1][3]),v),
                  dot(float4(r.m[2][0],r.m[2][1],r.m[2][2],r.m[2][3]),v),
                  dot(float4(r.m[3][0],r.m[3][1],r.m[3][2],r.m[3][3]),v));
}
inline RotMatrix4 buildRotMatrix4(float a,float b,float c,float d,float e,float f){
    const int first[6]={1,0,0,0,1,2},second[6]={2,2,1,3,3,3};
    float angle[6]={a,b,c,d,e,f};RotMatrix4 result{};
    for(int j=0;j<4;++j)result.m[j][j]=1.0f;
    for(int k=0;k<6;++k){
        float s=sin(angle[k]*(M_PI/180.0f)),cs=cos(angle[k]*(M_PI/180.0f));
        RotMatrix4 r{};for(int j=0;j<4;++j)r.m[j][j]=1.0f;
        r.m[first[k]][first[k]]=cs;r.m[second[k]][second[k]]=cs;
        r.m[first[k]][second[k]]=-s;r.m[second[k]][first[k]]=s;
        RotMatrix4 out{};
        for(int i=0;i<4;++i)for(int j=0;j<4;++j)
            out.m[i][j]=((r.m[i][0]*result.m[0][j]+r.m[i][1]*result.m[1][j])+r.m[i][2]*result.m[2][j])+r.m[i][3]*result.m[3][j];
        result=out;
    }return result;
}
inline RotMatrix4 buildRotMatrix4d(float a,float b,float c,float d,float e,float f){return buildRotMatrix4(a,b,c,d,e,f);}
inline float external_sine_transform(float coordinate,thread const float* options,thread const float* constants,float accumulator,bool add){
    float t=(coordinate-options[0])*options[1];t*=constants[0];
    float v=2*(t-rint(t-0.5f));bool negative=false;
    if(v>1){v=2-v;negative=true;}v*=M_PI;
    if(v>=constants[1]){v-=M_PI;negative=!negative;}
    float result=v-((v*v)*v)*constants[2];if(negative)result=-result;
    result=result*options[2]+options[3];if(add)result+=accumulator;return result;
}
inline void external_pine_t1(thread float& x,thread float& y,thread float& z,float s23,float s13,float s12,float scale,float offset){
    float tx=(x*s23-z*s13)*s12,nz=abs(x*s13+z*s23);
    float nx=abs(tx-y*s12),ny=abs(tx+y*s12),tx2=(nx+ny)*s12;
    y=(ny-nx)*s12;x=tx2*s23+nz*s13;z=-tx2*s13+nz*s23;
    x=x*scale-offset;y*=scale;z*=scale;
}
struct ComplexPowerPair{float x,y;ComplexPowerPair(float a,float b):x(a),y(b){} void operator*=(float s){x*=s;y*=s;}};
inline ComplexPowerPair complex_power_mul(ComplexPowerPair z,ComplexPowerPair w){return {z.x*w.x-z.y*w.y,z.y*w.x+z.x*w.y};}
inline ComplexPowerPair complex_power_power(ComplexPowerPair z,int power,bool flip){
    if(flip)z={z.y,z.x};const ComplexPowerPair original=z;z=complex_power_mul(z,z);
    if(power>2){const ComplexPowerPair squared=z;
        if(power>=4){z=complex_power_mul(z,z);if(power>=8){z=complex_power_mul(z,z);if(power>=10)z=complex_power_mul(z,squared);}else if(power>=6)z=complex_power_mul(z,squared);}
        if(power&1)z=complex_power_mul(z,original);}
    return flip?ComplexPowerPair(z.y,z.x):z;
}
inline void complex_power_rotate(thread float& x,thread float& y,thread float& z,int power,int type,float one,bool sequential_type_dispatch=false){
    if(type==(-2147483647-1))type=sequential_type_dispatch?5:2;
    bool flip=type>=6;if(flip)type-=6;
    if(type<=2){float a=type<=1?x:y,b=type<=0?y:z;float r=sqrt(a*a+b*b),inv=one/r;
        ComplexPowerPair q=complex_power_power({a*inv,b*inv},power,flip);q*=r;
        if(type<=0){x=q.x;y=q.y;}else if(type==1){x=q.x;z=q.y;}else{y=q.x;z=q.y;}
    }else{float a=type<=4?x:y,b=type==3?y:z,other=type==3?z:type==4?y:x;
        float r=sqrt(a*a+b*b);ComplexPowerPair q=complex_power_power({r,other},power,flip);float scale=q.x/r;
        if(type==3){x=a*scale;y=b*scale;z=q.y;}else if(type==4){x=a*scale;z=b*scale;y=q.y;}else{y=a*scale;z=b*scale;x=q.y;}}
}
inline bool native_formula_fail(thread NativeFormulaFault& f,uint code,uint id=0){
    if(!f.code){f.code=code;f.formula_id=id;}return false;
}
} // namespace mb3d_native
#include "formula_maps.metal.h"
