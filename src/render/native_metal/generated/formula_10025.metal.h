// Generated from 3Da/MengerIFSplus.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10025 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MENGERIFS_PLUS = 10025;
 template < > FormulaVec3 execute_formula < FORMULA_MENGERIFS_PLUS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 const float dX = params [ 13 ];
 const float dY = params [ 14 ];
 const float dZ = params [ 15 ];
 float ax = abs ( p . x );
 float ay = abs ( p . y );
 float az = abs ( p . z );
 float max_val = ax , mid_val = ay , min_val = az;
 if ( max_val < mid_val ) {
 float t = max_val;
 max_val = mid_val;
 mid_val = t;

}
 if ( max_val < min_val ) {
 float t = max_val;
 max_val = min_val;
 min_val = t;

}
 if ( mid_val < min_val ) {
 float t = mid_val;
 mid_val = min_val;
 min_val = t;

}
 float pmax = max_val + dX;
 float pmid = mid_val + dY;
 float pmin = min_val + dZ;
 float r0 = pmax * params [ 10 ] + pmid * params [ 11 ] + pmin * params [ 12 ];
 float r1 = pmax * params [ 4 ] + pmid * params [ 5 ] + pmin * params [ 6 ];
 float r2 = pmax * params [ 7 ] + pmid * params [ 8 ] + pmin * params [ 9 ];
 r1 -= dX;
 r2 -= dY;
 r0 -= dZ;
 de *= Scale;
 float k = Scale - 1.0f;
 float kz = ( k / Scale ) * CScaleZ;
 return FormulaVec3 ( r1 * Scale - k * CScaleX , r2 * Scale - k * CScaleY , ( kz - abs ( r0 - kz ) ) * Scale );

}
 FormulaVec3 MengerIFSplus_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_MENGERIFS_PLUS > ( p , params , de );

}

}}
