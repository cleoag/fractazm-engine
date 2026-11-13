#pragma once

#if defined(_MSC_VER) && defined(_M_X64)
// Numeric PVar layout shared by the original SSE2 Apply-scale/add preamble.
// Keep the packed scale/add operands aligned and preserve conditional writes
// in the original code; local rotations must not overwrite the orbit XYZ.
struct OriginalApplyIFSState {
    alignas(16) unsigned char buffer[160]{};
    double state[7]; // XYZ, VaryScale, Rout, OTrap, packed iteration integer

    unsigned char* var() { return buffer + 128; }
    void real(int offset, double value) { std::memcpy(var()+offset,&value,8); }
    void integer(int offset, int32_t value) { std::memcpy(var()+offset,&value,4); }
    void matrix(int offset, const double* values) {
        for (int i=0;i<9;++i) {
            const float value=static_cast<float>(values[i]);
            std::memcpy(var()+offset-4*i,&value,4);
        }
    }
    OriginalApplyIFSState(cpu_double3 p, const double* params)
        : state{p.x,p.y,p.z,params[DIFS_VARYSCALE_SLOT],0.,params[MAX_FORMULA_PARAMS-1],0.} {
        real(-16,params[0]); real(-32,params[1]); real(-24,params[1]);
        real(-48,params[2]); real(-40,params[2]);
        real(-56,params[3]); real(-64,params[4]); real(-72,params[5]);
        integer(-76,static_cast<int32_t>(params[6]));
        const int32_t iteration=static_cast<int32_t>(params[ITER_COUNT_SLOT]);
        std::memcpy(state+6,&iteration,sizeof(iteration));
    }
    cpu_double3 finish(double* params, double& de) const {
        params[DIFS_VARYSCALE_SLOT]=state[3]; de=state[4];
        params[MAX_FORMULA_PARAMS-1]=state[5];
        if (static_cast<int32_t>(params[6]) != 0) params[DIFS_COMMIT_XYZ_SLOT]=1.;
        return cpu_double3(state[0],state[1],state[2]);
    }
};
#endif

#ifdef MB3D_SOFTWARE_EXT80
// The selected Apply-scale/add formulas share this SSE2 preamble and x87
// matrix sequence. Local rotated Double slots never replace the orbit XYZ.
struct Arm64ApplyIFSState {
    using E = mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    cpu_double3 original, transformed;
    double* params;
    bool apply;
    Arm64ApplyIFSState(cpu_double3 p,double* values,int applyIndex=6)
        : original(p), transformed(p.x*values[1]+values[4],
                                  p.y*values[1]+values[3],p.z*values[1]+values[2]),
          params(values), apply(int(values[applyIndex]) != 0) {
        if (apply) {
            params[DIFS_VARYSCALE_SLOT]*=params[1];
            params[DIFS_COMMIT_XYZ_SLOT]=1.;
        }
    }
    cpu_double3 rotate(const double* m) const {
        const E x=transformed.x,y=transformed.y,z=transformed.z;
        return cpu_double3(double((x*float(m[0])+y*float(m[1]))+z*float(m[2])),
                           double((x*float(m[3])+y*float(m[4]))+z*float(m[5])),
                           double((x*float(m[6])+y*float(m[7]))+z*float(m[8])));
    }
    cpu_double3 finish(E distance,double& de) const {
        if (!apply) distance/=params[1];
        de=double(distance);
        return apply ? transformed : original;
    }
};
#endif
