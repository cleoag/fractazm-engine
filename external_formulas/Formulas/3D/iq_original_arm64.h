#pragma once

#ifdef MB3D_SOFTWARE_EXT80
enum class Arm64IQKind { Bulb, Normal, Pine };

// Shared 170/176-byte IQ kernels: FST keeps the extended radius for FYL2X,
// while the power and all four trigonometric components are Double slots.
inline cpu_double3 arm64IQ(cpu_double3 p,const double* params,Arm64IQKind kind,bool addYZ) {
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    const E x=p.x,y=p.y,z=p.z;
    const E radius=sqrt(x*x+(y*y+z*z));
    const E power=double(extendedExp2(E(params[0])*extendedLog2(abs(radius)),false));
    const bool yPole=kind==Arm64IQKind::Bulb;
    const auto polar=extendedSinCos(extendedAtan2(sqrt((yPole ? x*x : y*y)+z*z),
                                                yPole ? y : x)*params[0]);
    const auto azimuth=extendedSinCos(extendedAtan2(yPole ? x : y,z)*params[1]);
    const E ps=double(polar.sin),pc=double(polar.cos);
    const E azs=double(azimuth.sin),azc=double(azimuth.cos);
    E ox,oy,oz;
    if(kind==Arm64IQKind::Bulb) {
        ox=(ps*azs)*power; oy=pc*power; oz=((ps*azc)*params[2])*power;
    } else if(kind==Arm64IQKind::Normal) {
        ox=ps*power; oy=(pc*azs)*power; oz=((pc*azc)*params[2])*power;
    } else {
        ox=pc*power; oy=(ps*azc)*power; oz=((ps*azs)*params[2])*power;
    }
    ox+=params[6];
    if(addYZ) { oy+=params[7]; oz+=params[8]; }
    return cpu_double3(double(ox),double(oy),double(oz));
}
#endif
