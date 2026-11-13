// Two persistent complex lanes. Storage and arithmetic follow original SSE2 code.
constexpr unsigned int FORMULA_KALILINCOMBSSE2SM = 10352;
template<> struct ExternalFormulaUsesKaliState<FORMULA_KALILINCOMBSSE2SM> {
    static constexpr bool value=true;
};

inline void kaliSquare(cpu_double2& re, cpu_double2& im) {
    const cpu_double2 oldIm=im;
    im=im*re; re=re*re-oldIm*oldIm; im=im+im;
}
inline void kaliMultiply(cpu_double2& re, cpu_double2& im, cpu_double2 baseRe, cpu_double2 baseIm) {
    const cpu_double2 newIm=im*baseRe+re*baseIm;
    re=re*baseRe-im*baseIm;im=newIm;
}
template<>
cpu_double3 execute_formula<FORMULA_KALILINCOMBSSE2SM>(cpu_double3 p, constant double* params, double &de) {
    (void)de;
    cpu_double2 im(params[FORMULA_STATE_X_SLOT],params[FORMULA_STATE_Y_SLOT]);
    cpu_double2 re(params[FORMULA_STATE_Z_SLOT],params[FORMULA_STATE_W_SLOT]);
    if (params[BFIRSTIT_SLOT]==0.) {
        im=cpu_double2(p.x,p.y);re=cpu_double2(p.z,p.z);
        p.z=0.;params[BFIRSTIT_SLOT]=1.;
    }
    const cpu_double2 baseRe=re,baseIm=im;
    const uint32_t power=static_cast<uint32_t>(static_cast<int32_t>(params[0]));
    uint32_t half=power>>1;
    kaliSquare(re,im);
    if (half==3) kaliMultiply(re,im,baseRe,baseIm);
    // Original SHR is unsigned; JS examines the subsequent subtraction's sign.
    half-=2;if ((half&0x80000000u)==0) kaliSquare(re,im);
    half-=2;if ((half&0x80000000u)==0) kaliSquare(re,im);
    if (power&1) kaliMultiply(re,im,baseRe,baseIm);
    im+=cpu_double2(params[3],params[4]);re+=cpu_double2(params[5],params[5]);
    params[FORMULA_STATE_X_SLOT]=im.x;params[FORMULA_STATE_Y_SLOT]=im.y;
    params[FORMULA_STATE_Z_SLOT]=re.x;params[FORMULA_STATE_W_SLOT]=re.y;
    const double realSum=re.y+re.x,imagSum=im.y+im.x;
    const cpu_double2 mod2=re*re+im*im;
    const double numerator=(mod2.x+mod2.y)*params[2];
    const double denominator=(imagSum*imagSum+realSum*realSum)+params[1];
    const double scale=sqrt(numerator/denominator);
    return cpu_double3(realSum*scale,imagSum*scale,p.z);
}
constant FormulaInfo formula_info_KaliLinCombSSE2sm = {
    .id=FORMULA_KALILINCOMBSSE2SM,.name="KaliLinCombSSE2sm",.param_count=6,
    .complexity=323,.uses_transcendental=true
};
