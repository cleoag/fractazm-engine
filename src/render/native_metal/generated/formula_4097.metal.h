// Generated from Ads/_dynamicfast.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4097 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__DYNAMICFAST = 4097;
 static inline float sin_approx ( float x ) {
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
 static inline float cos_approx ( float x ) {
 return sin_approx ( x + 1.5707963267948966f );

}
 template < > FormulaVec3 execute_formula < FORMULA__DYNAMICFAST > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Strength = params [ 0 ];
 const float Alpha = params [ 1 ];
 const float Beta = params [ 2 ];
 const float Gamma = params [ 3 ];
 const float Delta = params [ 4 ];
 float x = p . x , y = p . y , z = p . z;
 float new_x = Strength * ( x - ( Alpha * cos_approx ( y * Gamma ) + Beta * sin_approx ( z * Delta ) ) );
 float new_y = Strength * ( y - ( Alpha * cos_approx ( z * Gamma ) + Beta * sin_approx ( x * Delta ) ) );
 float new_z = Strength * ( z - ( Alpha * cos_approx ( x * Gamma ) + Beta * sin_approx ( y * Delta ) ) );
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
