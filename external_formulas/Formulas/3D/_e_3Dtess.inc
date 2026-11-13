// Original tessellation search and x87 stores on Win64.
// The portable branch is not yet numerically certified.
constexpr unsigned int FORMULA_E3DTESS = 10274;

template<>
cpu_double3 execute_formula<FORMULA_E3DTESS>(cpu_double3 p, constant double* params, double &de) {
    FormulaWorkBudget work("_e_3Dtess");
#if defined(_MSC_VER) && defined(_M_X64) && !defined(MB3D_FORCE_PORTABLE_FORMULA_LOOPS)
    double state[9]={p.x,p.y,p.z};
    work.checkNativeStatus(E3DtessOriginal_x87(
        state,params,int(params[6]),int(params[7]),work.nativeCounter()));
    return cpu_double3(state[0],state[1],state[2]);
#else
    double v[3]={p.x,p.y,p.z};
    for (int axis=0;axis<3;++axis)
        v[axis]=double(static_cast<long double>(v[axis])*params[5]+params[axis]);
    const int selector=int(params[6]),maxDepth=int(params[7]);
    double axiom=selector==1?params[8]:selector==2?params[10]:selector==3?params[11]:1.;
    double threshold=double((static_cast<long double>(axiom)+1)*axiom);
    int count=0;
    do {
        work.beforeStep();
        ++count;
        int dominant=0;
        double maximum=fabs(v[0]);
        // Original AND AH,41h / JLE skips replacement only for >.
        for (int axis=1;axis<3;++axis) if (!(maximum>fabs(v[axis]))) {
            maximum=fabs(v[axis]);dominant=axis;
        }
        if (threshold>maximum && !(axiom>maximum)) {
            const long double range=static_cast<long double>(threshold)-axiom;
            const double ratio=maximum/v[dominant];
            v[dominant]=double((-(static_cast<long double>(threshold)+axiom)*ratio*params[8]
                                +v[dominant])*params[9]*params[4]/range);
            for (int axis=0;axis<3;++axis) if (axis!=dominant) {
                v[axis]=double(static_cast<long double>(v[axis])/range);
                const long double rounded=rintl(static_cast<long double>(v[axis])+params[8]);
                v[axis]=double((-rounded+params[8]+v[axis])*params[9]*params[4]);
            }
            break;
        }
        const double previous=axiom;
        axiom=threshold;
        threshold=double((static_cast<long double>(threshold)-previous)*params[8]+threshold);
        if (count>=maxDepth || params[12]>static_cast<long double>(threshold)-axiom) break;
    } while (true);
    for (int axis=0;axis<3;++axis)
        v[axis]=double(static_cast<long double>(v[axis])*params[3]-params[axis]);
    return cpu_double3(v[0],v[1],v[2]);
#endif
}

constant FormulaInfo formula_info_e3dtess = {
    .id = FORMULA_E3DTESS,
    .name = "_e_3Dtess",
    .param_count = 8,
    .complexity = 0,
    .uses_transcendental = false
};
