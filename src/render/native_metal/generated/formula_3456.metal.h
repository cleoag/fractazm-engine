// Generated from Ads/_SinhX.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3456 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SINHX = 3456;
 template < > FormulaVec3 execute_formula < FORMULA__SINHX > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float fixX = params [ 0 ];
 const float fixLg = params [ 1 ];
 const float Mul = params [ 2 ];
 const float Base = params [ 3 ];
 float x = p . x;
 float temp = Mul * x + fixX;
 float sx = ( metal :: signbit ( temp ) ? - 1.0f : 1.0f );
 float v = abs ( temp ) + abs ( fixLg );
 float new_x = sx * Base * log2 ( v );
 return FormulaVec3 ( new_x , p . y , p . z );

}
 FormulaVec3 _SinhX_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float fX = params [ 0 ] , fL = params [ 1 ] , M = params [ 2 ] , B = params [ 3 ];
 float t = M * p . x + fX;
 float s = ( metal :: signbit ( t ) ? - 1.0f : 1.0f );
 return FormulaVec3 ( s * B * log2 ( abs ( t ) + abs ( fL ) ) , p . y , p . z );

}

}}
