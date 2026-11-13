// SierpinskiFull: literal x87 stack order from the shipped 407-byte formula.
constexpr unsigned int FORMULA_SIERPINSKIFULL = 10178;

template<>
cpu_double3 execute_formula<FORMULA_SIERPINSKIFULL>(cpu_double3 p, constant double* params, double &de) {
    cpu_double3 r = rotMatrix3FromFloats(params, 4) * p;
    double a=r.x, b=r.y, c=r.z;
    auto exchange = [](double& x, double& y) { double t=x; x=y; y=t; };
    auto positive = [&]() {
        if (!(a >= b)) exchange(a,b);
        if (!(a >= c)) exchange(a,c);
        exchange(a,b);
        if (!(a >= c)) exchange(a,c);
    };
    auto negative = [&]() {
        a=-a;
        if (a >= b) exchange(a,b);
        if (a >= c) exchange(a,c);
        a=-a;
        exchange(a,b);
        a=-a;
        if (a >= c) exchange(a,c);
        a=-a;
    };
    if (int(params[24]) == 0) {
        exchange(a,b);
        positive();
        negative();
    } else {
        negative();
        exchange(a,b);
        positive();
    }
    // fld Edge inserts a stack entry: st(2) is b, st(1) is a.
    if (!(0.0 >= params[22])) b=params[22]-fabs(params[22]-b);
    if (!(0.0 >= params[23])) a=params[23]-fabs(params[23]-a);
    r=rotMatrix3FromFloats(params, 13)*cpu_double3(a,b,c);
    const double scale=params[0], k=scale-1.0;
    de*=scale;
    return cpu_double3(r.x*scale-k*params[1],r.y*scale-k*params[2],r.z*scale-k*params[3]);
}

constant FormulaInfo formula_info_SierpinskiFull = {
    .id = FORMULA_SIERPINSKIFULL, .name = "SierpinskiFull",
    .param_count = 13, .complexity = 407, .uses_transcendental = true
};
