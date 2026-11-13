// Generated from Ads/_gnarl2D_2fast.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7921 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__GNARL2D_2FAST = 7921;
 static inline float gnarl2d2_sin_approx ( float x ) {
 const float inv_pi = 0.31830988618379067f;
 const float D = 3.1415926535897932f;
 const float C = - 5.167375251323450f;
 const float B = 2.543310591226590f;
 const float A = - 0.557168173477934f;
 float t = x * inv_pi;
 int n = int ( rint ( t ) );
 float frac = t - float ( n );
 float f2 = frac * frac;
 float poly = frac * ( ( ( A * f2 + B ) * f2 + C ) * f2 + D );
 return ( n & 1 ) ? - poly : poly;

}
 static inline float gnarl2d2_cos_approx ( float x ) {
 return gnarl2d2_sin_approx ( x + 1.5707963267948966f );

}
 template < > FormulaVec3 execute_formula < FORMULA__GNARL2D_2FAST > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Strength = params [ 0 ];
 const float X_step = params [ 1 ];
 const float Alpha = params [ 3 ];
 const float Beta = params [ 4 ];
 const float Gamma = params [ 5 ];
 float x = p . x , y = p . y , z = p . z;
 float inner1 = Alpha * gnarl2d2_sin_approx ( x + Beta * gnarl2d2_cos_approx ( x ) );
 float outer1 = gnarl2d2_sin_approx ( ( y + x ) * Gamma - inner1 );
 float new_x = Strength * ( y + X_step * outer1 );
 float inner2 = gnarl2d2_sin_approx ( Alpha * ( y + Beta * gnarl2d2_cos_approx ( y ) ) );
 float outer2 = gnarl2d2_sin_approx ( ( x - y ) * Gamma + inner2 );
 float new_y = Strength * ( x + X_step * outer2 );
 return FormulaVec3 ( new_x , new_y , z );

}

}}
