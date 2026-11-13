// Generated from Ads/_IntPowRotateC.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10020 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__INTPOWROTATEC = 10020;
 template < > FormulaVec3 execute_formula < FORMULA__INTPOWROTATEC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float x = params [ 13 ] , y = params [ 14 ] , z = params [ 15 ];
 const float r = native_formula_length3 ( z , x , y );
 const float inv = params [ 12 ] / r;
 x *= inv;
 y *= inv;
 z *= inv;
 if ( params [ 0 ] != 0.0f ) {
 const float t = x;
 x = y;
 y = t;

}
 complex_power_rotate ( x , y , z , ( int ) params [ 1 ] , ( int ) params [ 2 ] , params [ 12 ] , true );
 if ( params [ 3 ] != 0.0f ) complex_power_rotate ( x , y , z , ( int ) params [ 4 ] , ( int ) params [ 5 ] , params [ 12 ] , true );
 if ( params [ 6 ] != 0.0f ) complex_power_rotate ( x , y , z , ( int ) params [ 7 ] , ( int ) params [ 8 ] , params [ 12 ] , true );
 int lastType = static_cast < int > ( params [ 6 ] != 0.0f ? params [ 8 ] : params [ 3 ] != 0.0f ? params [ 5 ] : params [ 2 ] );
 if ( lastType == ( native_std :: numeric_limits < int > :: min ) ( ) ) lastType = 5;
 if ( lastType >= 6 ) lastType -= 6;
 params [ FORMULA_STATE_X_SLOT ] = x * r;
 params [ FORMULA_STATE_Y_SLOT ] = y * r;
 params [ FORMULA_STATE_Z_SLOT ] = ( z * r ) * params [ 9 ];
 params [ FORMULA_STATE_W_SLOT ] = lastType <= 0 ? 0.0f : lastType == 1 ? x : lastType == 2 ? y : z;
 params [ WRITE_J1_SLOT ] = x * r;
 params [ WRITE_J2_SLOT ] = y * r;
 params [ WRITE_J3_SLOT ] = ( z * r ) * params [ 9 ];
 return p;

}

}}
