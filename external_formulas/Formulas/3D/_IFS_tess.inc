// _IFS_tess: original 796-byte pretransform. Its x87 flag tests and
// misaligned constant reads are observable behavior of the shipped formula.
constexpr unsigned int FORMULA_IFSTESS = 10275;
template<>
cpu_double3 execute_formula<FORMULA_IFSTESS>(cpu_double3 p, constant double* params, double &de) {
    FormulaWorkBudget work("_IFS_tess");
#if defined(_MSC_VER) && defined(_M_X64) && !defined(MB3D_FORCE_PORTABLE_FORMULA_LOOPS)
    double state[] = {p.x,p.y,p.z};
    work.checkNativeStatus(IFSTessOriginal_x87(state,params,work.nativeCounter()));
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80) && !defined(MB3D_FORCE_PORTABLE_FORMULA_LOOPS)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    p.x=double(E(p.x)+params[0]);
    p.y=double(E(p.y)+params[1]);
    p.z=double(E(p.z)+params[2]);
    E x=p.y,y=p.z,saved=p.x;
    const E atom=params[5],disp=params[4],norm=params[10];
    const auto sc=extendedSinCos(E(params[6])*params[11]);
    const E ca=double(sc.cos),sa=double(sc.sin);
    const int variation=int(params[8]);
    const auto rescale=[&] {
        return cpu_double3(double(saved*params[3]-params[0]),
                           double(E(p.y)*params[3]-params[1]),
                           double(E(p.z)*params[3]-params[2]));
    };
    // AND AH,41h clears CF: JA means C0 or C3 is set (<= or unordered),
    // while JBE means both are clear (>). Do not treat these as FCOM flags.
    if(!(atom>abs(saved)) || ((atom>abs(x)) && (atom>abs(y)))) return rescale();
    unsigned int depth=0;
    for(;;) {
        work.beforeStep();
        if(variation>=4) {
            if((atom>abs(x)) && (atom>abs(y))) {
                // Original 00C5 -> 0211 stores from an empty x87 stack.
                x=extendedNaN(); y=-y;
            } else {
                E k=abs(x);
                if(k!=E(0)) k=double(k/x);
                const E xt=double(-(k*disp)+x),yt=double(-(disp*params[9])+y);
                if(E(0)>k) {
                    x=double(xt*ca+yt*sa); y=double(-(xt*sa)+yt*ca);
                } else {
                    x=double(xt*ca+(-(yt*sa))); y=double(xt*sa+yt*ca);
                }
            }
        } else {
            bool negative=false;
            if(y>x) { y=double(y+(-(disp*norm))); negative=true; }
            else x=double(x+(-(disp*norm)));
            const E xt=double(-(x*sa)+(-(y*ca))),yt=double(x*ca+(-(y*sa)));
            x=xt; y=yt;
            if(negative) {
                if(variation==1) { x=-x; y=-y; }
                else if(variation==2) { const E t=x; x=y; y=t; }
                else if(variation==3) { const E t=x; x=-y; y=-t; }
            }
        }
        x=double(x/norm); y=double(y/norm);
        if((atom>abs(x)) && (atom>abs(y))) {
            p.y=double(x); p.z=double(y); saved=double(saved/norm);
            return rescale();
        }
        // The loader skips Integer=50: PVar+1C reads Double 500's high word.
        // Both depth comparisons in the original are unsigned, after the body.
        const auto constantDepth=uint32_t(std::bit_cast<uint64_t>(params[12])>>32);
        if(depth>=constantDepth || depth>=static_cast<unsigned int>(int(params[7]))) break;
        ++depth; saved=double(saved/norm);
    }
    return cpu_double3(double(E(params[12])*p.x),double(E(params[12])*p.y),
                       double(E(params[12])*p.z));
#else
    // Portable arithmetic fallback; native extended-precision parity is
    // verified separately. Constants retain the original decimal literals.
    p.x+=params[0]; p.y+=params[1]; p.z+=params[2];
    double x=p.y, y=p.z, saved=p.x;
    const double atom=params[5], disp=params[4], norm=params[10];
    const double angle=params[6]*params[11], ca=cos(angle), sa=sin(angle);
    const int variation=int(params[8]);
    auto rescale = [&]() { return cpu_double3(saved*params[3]-params[0],
        p.y*params[3]-params[1],p.z*params[3]-params[2]); };
    if (atom>fabs(saved) || (!(atom>fabs(x)) && !(atom>fabs(y)))) return rescale();
    unsigned int depth=0;
    for (;;) {
    work.beforeStep();
    if (variation>=4) {
        if (!(atom>fabs(x)) && !(atom>fabs(y))) {
            // Original jumps into FSTP with an empty x87 stack.
            x=NAN; y=-y;
        } else {
            double k=fabs(x);
            if (k!=0.) k/=x;
            const double xt=x-k*disp, yt=y-params[9]*disp;
            if (k<0.) { x=xt*ca-yt*sa; y=xt*sa+yt*ca; }
            else { x=xt*ca+yt*sa; y=-xt*sa+yt*ca; }
        }
    } else {
        double k=1.;
        if (y>x) x-=disp*norm;
        else { y-=disp*norm; k=-1.; }
        const double xt=-x*sa-y*ca, yt=x*ca-y*sa;
        x=xt; y=yt;
        if (!(0.>k)) {
            if (variation==1) { x=-x; y=-y; }
            else if (variation==2) { double t=x; x=y; y=t; }
            else if (variation==3) { double t=x; x=-y; y=-t; }
        }
    }
    x/=norm; y/=norm;
    if (!(atom>fabs(x)) && !(atom>fabs(y))) {
        p.y=x; p.z=y; saved/=norm;
        return rescale();
    }
    // Pascal skips the unsupported Integer constant line. PVar+18 holds
    // Double 500, whose high word (PVar+1C) is the unsigned loop bound.
    if (depth>=0x407f4000u || depth>=static_cast<unsigned int>(int(params[7]))) break;
    ++depth; saved/=norm;
    }
    const double clip=params[12];
    return cpu_double3(p.x*clip,p.y*clip,p.z*clip);
#endif
}
constant FormulaInfo formula_info_ifstess = {
    .id=FORMULA_IFSTESS, .name="_IFS_tess", .param_count=9,
    .complexity=796, .uses_transcendental=true
};
