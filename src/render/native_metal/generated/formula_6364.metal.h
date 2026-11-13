// Generated from Ads/_gnarl2Dfast.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6364 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__GNARL2DFAST = 6364;
 static inline float gnarl2d_sin_approx ( float x ) {
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
 template < > FormulaVec3 execute_formula < FORMULA__GNARL2DFAST > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Strength = params [ 0 ];
 const float X_step = params [ 1 ];
 const float Y_step = params [ 2 ];
 const float Alpha = params [ 3 ];
 const float Beta = params [ 4 ];
 float x = p . x , y = p . y , z = p . z;
 float s1y = gnarl2d_sin_approx ( y * Beta );
 float s2y = gnarl2d_sin_approx ( Alpha * ( y + s1y ) );
 float s3y = gnarl2d_sin_approx ( y + s2y );
 float new_x = Strength * ( x - X_step * s3y );
 float s1x = gnarl2d_sin_approx ( x * Beta );
 float s2x = gnarl2d_sin_approx ( Alpha * ( x + s1x ) );
 float s3x = gnarl2d_sin_approx ( x + s2x );
 float new_y = Strength * ( y - Y_step * s3x );
 return FormulaVec3 ( new_x , new_y , z );

}

}}
