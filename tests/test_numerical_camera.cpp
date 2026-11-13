#include "core/numerical_camera.h"
#include "core/numeric_precision.h"
#include "core/extended_transcendentals.h"
#include "fixtures/numerical_camera_reference.h"
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstring>
#if defined(__APPLE__) && defined(__aarch64__)
#include "../comparison/metal_exact/scene_types.h"
#endif

namespace {
unsigned checks=0,failures=0;
void check(bool pass,const char* what,const char* row="") {
    ++checks;
    if(!pass&&++failures<=20)std::fprintf(stderr,"FAIL %s %s\n",what,row);
}
void expect(const double* actual,const uint64_t* expected,const char* what,const char* row) {
    for(unsigned i=0;i<3;++i)check(std::bit_cast<uint64_t>(actual[i])==expected[i],what,row);
}
struct SoftState {
    uint_fast8_t rounding,precision,flags,tininess;
    static SoftState read() {
        return {softfloat_roundingMode,extF80_roundingPrecision,
                softfloat_exceptionFlags,softfloat_detectTininess};
    }
    void write() const {
        softfloat_roundingMode=rounding;extF80_roundingPrecision=precision;
        softfloat_exceptionFlags=flags;softfloat_detectTininess=tininess;
    }
    bool operator==(const SoftState&) const=default;
};
void nativeReference(double ay,double ax,const double* m,double* rotated) {
    const double x=-std::sin(ax),y=std::sin(ay),z=std::cos(ay)*std::cos(ax);
    const double inverse=1/std::sqrt((x*x+z*z)+y*y);
    const double a=x*inverse,b=y*inverse,c=z*inverse;
    for(unsigned i=0;i<3;++i)rotated[i]=(m[i]*a+m[i+3]*b)+m[i+6]*c;
}

#if defined(__APPLE__) && defined(__aarch64__)
ExactTrig deviceTrig(double angle) {
    const mb3d::numeric::ExtendedScope scope;
    const auto v=mb3d::numeric::extendedSinCos(mb3d::numeric::Extended80(angle));
    return {std::bit_cast<uint64_t>(double(v.sin)),std::bit_cast<uint64_t>(double(v.cos)),
            v.sin.significand(),v.cos.significand(),v.sin.signExponent(),v.cos.signExponent(),{0,0}};
}
void testDeviceCamera(const NumericalCameraReference& row,const double* input) {
    using namespace mb3d_metal_exact;
    ExactScene scene{};
    std::memcpy(scene.vgrads,row.input+2,sizeof(scene.vgrads));
    const auto tx=deviceTrig(input[1]),ty=deviceTrig(input[0]);
    check(valid_camera_trig(tx)&&valid_camera_trig(ty),"valid extended camera ABI",row.name);
    const auto v=camera_ray(tx,ty,scene);
    check(v.x.bits==row.expected[3],"Metal camera x matches independent x87",row.name);
    check(v.y.bits==row.expected[4],"Metal camera y matches independent x87",row.name);
    check(v.z.bits==row.expected[5],"Metal camera z matches independent x87",row.name);
    for(unsigned control=0;control<6;++control) {
        auto bad=tx;
        if(control==0)bad.reserved[0]=1;
        if(control==1)bad.sin_sign_exponent=0x10000;
        if(control==2)bad.cos_sign_exponent=0x7fff;
        if(control==3)bad.sin_significand^=UINT64_C(0x8000000000000000);
        if(control==4)bad.cos_value^=1;
        if(control==5)bad.sin_value=UINT64_C(0x7ff8000000000000);
        check(!valid_camera_trig(bad),"malformed extended camera ABI rejected",row.name);
    }
}
#endif
} // namespace

int main() {
    const SoftState original=SoftState::read();
    const SoftState marker{softfloat_round_min,32,softfloat_flag_underflow,
                           softfloat_tininess_beforeRounding};
    marker.write();
    const mb3d::ScopedNumericPrecision compatible(mb3d::PrecisionMode::Compatible);
    unsigned nativeDifferences=0;
    for(const auto& row:numericalCameraReferences) {
        double input[11],view[3],rotated[3],sphere[3];
        std::memcpy(input,row.input,sizeof(input));
        mb3d::NumericalBuildViewVectorDFOV(input[0],input[1],view);
        expect(view,row.expected,"perspective build",row.name);
        mb3d::NumericalBuildRotateViewVectorDFOV(input[0],input[1],input+2,rotated);
        expect(rotated,row.expected+3,"combined perspective rotation",row.name);
        mb3d::NumericalRotateVectorReverse(view,input+2);
        expect(view,row.expected+3,"separate binary64 store and rotation",row.name);
        mb3d::NumericalBuildViewVectorDSphereFOV(input[0],input[1],sphere);
        expect(sphere,row.expected+6,"spherical build",row.name);
        check(SoftState::read()==marker,"compatible camera restores caller SoftFloat TLS",row.name);
#if defined(__APPLE__) && defined(__aarch64__)
        testDeviceCamera(row,input);
#endif
        {
            const mb3d::ScopedNumericPrecision native(mb3d::PrecisionMode::Native);
            double expected[3];nativeReference(input[0],input[1],input+2,expected);
            mb3d::NumericalBuildRotateViewVectorDFOV(input[0],input[1],input+2,view);
            for(unsigned i=0;i<3;++i) {
                check(std::bit_cast<uint64_t>(view[i])==std::bit_cast<uint64_t>(expected[i]),
                      "native camera uses hardware double operation order",row.name);
                nativeDifferences+=std::bit_cast<uint64_t>(view[i])!=row.expected[3+i];
            }
            check(SoftState::read()==marker,"native camera leaves SoftFloat TLS untouched",row.name);
        }
        check(mb3d::CurrentPrecisionMode()==mb3d::PrecisionMode::Compatible,
              "nested native profile restores compatible",row.name);
    }
    check(nativeDifferences>0,"fixtures distinguish native from compatible arithmetic");
    original.write();
    std::printf("Numerical camera: 70 independent x87 cases; %u checks, %u failures\n",checks,failures);
    return failures?1:0;
}
