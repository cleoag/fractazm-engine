// Magnetic triplex iteration. W holds the reciprocal distance to (1,0,0).
// The original Fix_z_zero branch also advances the persistent bFirstIt counter (context +208, not ItResultI +64).
constexpr unsigned int FORMULA_MAGNET4D = 10340;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Magnet4DOriginal_x87(double*,const double*,int*,int);
#endif

template<>
cpu_double3 execute_formula<FORMULA_MAGNET4D>(cpu_double3 p, constant double* params, double &de) {
    int iteration=int(params[BFIRSTIT_SLOT]);
#if defined(_MSC_VER) && defined(_M_X64)
    // XYZW followed by the original nine Double stack temporaries.
    double state[13] = {p.x,p.y,p.z,de};
    Magnet4DOriginal_x87(state,params,&iteration,int(params[4]));
    params[BFIRSTIT_SLOT]=iteration;
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#else
    const numeric::X87Scope extendedScope;
    // Preserve the original Double spill boundaries and expression order.
    const numeric::X87Real zm=params[0], up=params[1], dn=params[2];
    const numeric::X87Real add=params[3], two=params[5];
    const numeric::X87Real cx=params[6], cy=params[7], cz=params[8];
    double x=p.x,y=p.y,z=p.z,x1,y1,z1;
    if (int(params[4])!=0 && iteration==0) {
        x=y=x1=y1=0.;z1=z;++iteration;
    } else {
        const numeric::X87Real xx=x,yy=y,zz=z;
        const double radius=double(xx*xx+yy*yy);
        const double factor=double(-(zz*zz/radius)+numeric::X87Real(1.));
        x1=double((xx*xx-yy*yy)*factor);
        y1=double(xx*yy*two*factor);
        z1=double(numeric::x87Sqrt(radius)*zz*two*zm);
    }
    x1=double((static_cast<numeric::X87Real>(x1)+cx*up)+add);
    y1=double(static_cast<numeric::X87Real>(y1)+cy*up);
    z1=double(static_cast<numeric::X87Real>(z1)+cz*up);
    const double x2=double((static_cast<numeric::X87Real>(x)*two+(add-numeric::X87Real(1.)))+cx*dn);
    const double y2=double(static_cast<numeric::X87Real>(y)*two+cy*dn);
    const double z2=double(static_cast<numeric::X87Real>(z)*two*zm+cz*dn);
    const numeric::X87Real ax=x1,ay=y1,az=z1,bx=x2,by=y2,bz=z2;
    double radius=double(bx*bx+(by*by+bz*bz));
    const double r1=double(numeric::x87Sqrt(ax*ax+ay*ay));
    const double r2=double(numeric::x87Sqrt(bx*bx+by*by));
    z=double((static_cast<numeric::X87Real>(r2)*az-static_cast<numeric::X87Real>(r1)*bz)/radius*zm);
    radius=double(((az*bz/r1)/r2+numeric::X87Real(1.))/radius);
    x=double((ax*bx+ay*by)*radius);
    y=double((ay*bx-ax*by)*radius);
    const numeric::X87Real xx=x,yy=y,zz=z;
    const double r=double(xx*xx+yy*yy);
    const double f=double(-(zz*zz/r)+numeric::X87Real(1.));
    x=double((xx*xx-yy*yy)*f);
    y=double(xx*yy*two*f);
    z=double(numeric::x87Sqrt(r)*zz*two*zm);
    const numeric::X87Real dx=static_cast<numeric::X87Real>(x)-numeric::X87Real(1.);
    de=double(numeric::X87Real(1.)/(dx*dx+(static_cast<numeric::X87Real>(y)*y+static_cast<numeric::X87Real>(z)*z)));
    params[BFIRSTIT_SLOT]=iteration;
    return cpu_double3(x,y,z);
#endif
}

constant FormulaInfo formula_info_Magnet4D = {
    .id = FORMULA_MAGNET4D,
    .name = "Magnet4D",
    .param_count = 6, // Z/up/down/add/fix/two; C follows at indices 6..9.
    .complexity = 545,
    .uses_transcendental = true
};
