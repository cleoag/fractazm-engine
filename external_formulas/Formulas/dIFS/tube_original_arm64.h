#pragma once

#ifdef MB3D_SOFTWARE_EXT80
// Dodeca/tetra tubes share a permuted SSE preamble and a Double-spilling
// rotation/distance tail. The local fold coordinates never replace orbit XYZ.
struct Arm64TubeState {
    using E=mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    cpu_double3 original,transformed;
    double* params;
    bool apply;
    E a,b,c;
    Arm64TubeState(cpu_double3 p,double* values)
        : original(p),transformed(p.x*values[3]+values[6],p.y*values[3]+values[5],p.z*values[3]+values[4]),
          params(values),apply(int(values[11])!=0),a(transformed.y),b(transformed.z),c(transformed.x) {
        if(apply) { params[DIFS_VARYSCALE_SLOT]*=params[3]; params[DIFS_COMMIT_XYZ_SLOT]=1.; }
    }
    cpu_double3 finish(double& de) {
        const E ac=double(a*params[7]+c*params[8]);
        a=double(a*params[8]+(-(c*params[7]))); c=ac;
        const E ab=double(a*params[9]+b*params[10]);
        a=double(a*params[10]+(-(b*params[9]))); b=ab;
        const E db=b-params[1],dc=c-params[2];
        const E tube=sqrt(db*db+dc*dc)-params[0];
        // FST retains the unrounded offsets for tube; sphere reloads Double.
        b=double(db); c=double(dc);
        const E position=abs(a)-params[12];
        const E sphere=sqrt((position*position+b*b)+c*c)-params[13];
        const bool useTube=sphere>tube;
        E distance=useTube ? tube : sphere;
        params[MAX_FORMULA_PARAMS-1]=params[useTube ? 14 : 15];
        if(!apply) distance/=params[3];
        de=double(distance);
        return apply ? transformed : original;
    }
};
#endif
