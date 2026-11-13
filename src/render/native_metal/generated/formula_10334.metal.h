// Generated from 4D/Makin4D_p8a.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10334 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MAKIN4D_P8A = 10334;
 template < > FormulaVec3 execute_formula < FORMULA_MAKIN4D_P8A > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: X87Scope extendedScope;
 const float w_add = params [ 0 ];
 float x = p . x , y = p . y , z = p . z , w = de;
 for ( int iter = 0;
 iter < 3;
 iter ++ ) {
 numeric :: X87Real lx = x , ly = y , lz = z , lw = w;
 numeric :: X87Real tw_re = lx * lz;
 tw_re -= ly * lw;
 tw_re += tw_re;
 numeric :: X87Real tw_im = lx * lw;
 tw_im += ly * lz;
 tw_im += tw_im;
 numeric :: X87Real s = lx * lx + ly * ly;
 numeric :: X87Real t = lz * lz + lw * lw;
 numeric :: X87Real tz_re , tz_im;
 if ( s == numeric :: X87Real ( 0.0f ) && t == numeric :: X87Real ( 0.0f ) && ( lx != numeric :: X87Real ( 0.0f ) || ly != numeric :: X87Real ( 0.0f ) || lz != numeric :: X87Real ( 0.0f ) || lw != numeric :: X87Real ( 0.0f ) ) ) {
 tz_re = numeric :: X87Real ( 0.0f );
 tz_im = numeric :: X87Real ( 0.0f );
 tw_re = numeric :: X87Real ( 0.0f );
 tw_im = numeric :: X87Real ( 0.0f );

}
 else if ( ! ( t >= s ) ) {
 numeric :: X87Real ratio = ( s - t ) / s;
 tz_re = ( lx * lx - ly * ly ) * ratio;
 tz_im = numeric :: X87Real ( 2.0f ) * lx * ly * ratio;

}
 else {
 numeric :: X87Real ratio = ( s - t ) / t;
 tz_re = ( lz * lz - lw * lw ) * ratio;
 tz_im = numeric :: X87Real ( 2.0f ) * lz * lw * ratio;

}
 volatile float x_store = static_cast < float > ( tz_re );
 volatile float y_store = static_cast < float > ( tz_im );
 volatile float z_store = static_cast < float > ( tw_re );
 volatile float w_store = static_cast < float > ( tw_im );
 x = x_store;
 y = y_store;
 z = z_store;
 w = w_store;

}
 w += w_add;
 de = w;
 return FormulaVec3 ( x , y , z );

}

}}
