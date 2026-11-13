#include "soft_extended.h"
#ifndef MB3D_SOFTWARE_EXT80
#define MB3D_SOFTWARE_EXT80
#endif
#include "src/core/extended80.h"
#include <bit>
#include <cstdio>
#include <cstdint>
using namespace mb3d_metal_exact;
using mb3d::numeric::Extended80;
namespace {
std::uint64_t state=0x573ea4cbd9158f02ULL, checks=0, failures=0;
U64 next() { state ^= state >> 12; state ^= state << 25; state ^= state >> 27; return state * 2685821657736338717ULL; }
void check(const char* op,E80 actual,Extended80 expected,E80 a,E80 b) {
    ++checks;
    bool expected_nan = (expected.signExponent()&0x7fff)==0x7fff && (expected.significand()&0x7fffffffffffffffULL);
    bool good=expected_nan ? isnan(actual) : actual.significand==expected.significand() && actual.sign_exponent==expected.signExponent();
    if(!good && failures++<20) std::fprintf(stderr,"%s a=%04x/%016llx b=%04x/%016llx got=%04x/%016llx want=%04x/%016llx\n",op,a.sign_exponent,a.significand,b.sign_exponent,b.significand,actual.sign_exponent,actual.significand,expected.signExponent(),(U64)expected.significand());
}
void pair(E80 a,E80 b) {
    Extended80 da=Extended80::fromBits(a.sign_exponent,a.significand), db=Extended80::fromBits(b.sign_exponent,b.significand);
    check("add",a+b,da+db,a,b); check("sub",a-b,da-db,a,b); check("mul",a*b,da*db,a,b); check("div",a/b,da/db,a,b); check("sqrt",sqrt(a),sqrt(da),a,b);
    ++checks;
    double d=static_cast<double>(da);
    if(std::isnan(d) ? !isnan(a.to_f64()) : a.to_f64().bits != std::bit_cast<U64>(d)) ++failures;
    ++checks;
    float f=static_cast<float>(da);
    if(std::isnan(f) ? a.to_f32_bits()!=0x7fc00000U : a.to_f32_bits()!=std::bit_cast<U32>(f)) ++failures;
    ++checks; if(a.to_i32_rne()!=da.integer()) ++failures;
    ++checks; if(a.to_i32_rtz()!=static_cast<int32_t>(da)) ++failures;
    const bool ae[]={a==b,a!=b,a<b,a<=b,a>b,a>=b}; const bool de[]={da==db,da!=db,da<db,da<=db,da>db,da>=db};
    for(int i=0;i<6;++i) {++checks;if(ae[i]!=de[i])++failures;}
}
}
int main() {
    mb3d::numeric::ExtendedScope env;
    const E80 edges[]={E80::from_bits(0,0),E80::from_bits(0x8000,0),E80::from_bits(0,1),E80::from_bits(0,2),E80::from_bits(0,0x7fffffffffffffffULL),E80::from_bits(1,0x8000000000000000ULL),E80::from_bits(1,0x8000000000000001ULL),E80::from_bits(0x3fff,0x8000000000000000ULL),E80::from_bits(0x3fff,0x8000000000000001ULL),E80::from_bits(0x7ffe,0xffffffffffffffffULL),ext_inf(false),ext_inf(true),ext_nan(),E80::from_bits(0x3ffe,0xffffffffffffffffULL)};
    for(auto a:edges)for(auto b:edges){pair(a,b);pair(-a,b);}
    for(unsigned i=0;i<100000;++i){
        U32 ea=U32(next()%0x7fff),eb=U32(next()%0x7fff); U64 sa=next(),sb=next();
        if(ea)sa|=0x8000000000000000ULL;else sa&=0x7fffffffffffffffULL;
        if(eb)sb|=0x8000000000000000ULL;else sb&=0x7fffffffffffffffULL;
        E80 a=E80::from_bits(ea|(U32(next()&1)<<15),sa),b=E80::from_bits(eb|(U32(next()&1)<<15),sb);
        pair(a,b); if(i%4==0){b=E80::from_bits(a.sign_exponent^0x8000,a.significand|(next()&7));pair(a,b);}
    }
    for(unsigned i=0;i<50000;++i) { F64 a=F64::from_bits(next()),b=F64::from_bits(next()); pair(E80(a),E80(b)); }
    std::printf("{\"schema_version\":1,\"profile\":\"integer-x87-extended-rne-value-only-v1\",\"checks\":%llu,\"failures\":%llu}\n",(U64)checks,(U64)failures);
    return failures?1:0;
}
