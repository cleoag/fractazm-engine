// Generated from 3D/GenIntPower.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10300 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_GENINTPOWER = 10300;
 template < > FormulaVec3 execute_formula < FORMULA_GENINTPOWER > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int power = int ( params [ 0 ] );
 const float r2 = p . z * p . z + ( p . x * p . x + p . y * p . y );
 const float r = sqrt ( r2 );
 float rp = r2;
 if ( power >= 4 ) {
 rp *= rp;
 if ( power >= 8 ) {
 rp *= rp;
 if ( power >= 10 ) rp *= r2;

}
 else if ( power >= 6 ) rp *= r2;

}
 if ( power & 1 ) rp *= r;
 const float inv = 1.0f / r;
 float x = p . x * inv , y = p . y * inv , z = p . z * inv;
 complex_power_rotate ( x , y , z , int ( params [ 1 ] ) , int ( params [ 2 ] ) , 1.0f );
 if ( params [ 3 ] != 0.0f ) complex_power_rotate ( x , y , z , int ( params [ 4 ] ) , int ( params [ 5 ] ) , 1.0f );
 if ( params [ 6 ] != 0.0f ) complex_power_rotate ( x , y , z , int ( params [ 7 ] ) , int ( params [ 8 ] ) , 1.0f );
 return FormulaVec3 ( x * rp , y * rp , ( z * rp ) * params [ 9 ] );

}

}}
