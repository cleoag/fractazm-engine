// Generated from 3D/_MobiusTpx.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10276 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MOBIUSTPX = 10276;
 static FormulaVec3 triplex_mul ( FormulaVec3 u , FormulaVec3 v ) {
 float rxy_u = sqrt ( u . x * u . x + u . y * u . y );
 float rxy_v = sqrt ( v . x * v . x + v . y * v . y );
 float prod = rxy_u * rxy_v;
 float rx , ry;
 if ( prod == 0.0f ) {
 rx = - v . z * u . z;
 ry = v . z * u . z;

}
 else {
 float alpha = v . z * u . z / prod;
 float factor = 1.0f - alpha;
 rx = ( u . x * v . x - u . y * v . y ) * factor;
 ry = ( v . x * u . y + u . x * v . y ) * factor;

}
 float rz = rxy_v * u . z + rxy_u * v . z;
 return FormulaVec3 ( rx , ry , rz );

}
 template < > FormulaVec3 execute_formula < FORMULA_MOBIUSTPX > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 FormulaVec3 a = FormulaVec3 ( params [ 0 ] , params [ 1 ] , params [ 2 ] );
 FormulaVec3 b = FormulaVec3 ( params [ 3 ] , params [ 4 ] , params [ 5 ] );
 FormulaVec3 c = FormulaVec3 ( params [ 6 ] , params [ 7 ] , params [ 8 ] );
 FormulaVec3 d = FormulaVec3 ( params [ 9 ] , params [ 10 ] , params [ 11 ] );
 const float EPSILON = params [ 12 ];
 const float OVERFLOW = params [ 13 ];
 FormulaVec3 S = p;
 FormulaVec3 cS = triplex_mul ( c , S );
 FormulaVec3 T1 = cS + d;
 float T1_mag2 = T1 . x * T1 . x + T1 . y * T1 . y + T1 . z * T1 . z;
 if ( T1_mag2 <= EPSILON ) {
 return FormulaVec3 ( OVERFLOW , OVERFLOW , OVERFLOW );

}
 FormulaVec3 aS = triplex_mul ( a , S );
 FormulaVec3 T0 = aS + b;
 float inv_T1_mag2 = 1.0f / T1_mag2;
 float rxy_T0 = sqrt ( T0 . x * T0 . x + T0 . y * T0 . y );
 float rxy_T1 = sqrt ( T1 . x * T1 . x + T1 . y * T1 . y );
 float prod = rxy_T0 * rxy_T1;
 float rx , ry;
 if ( prod == 0.0f ) {
 rx = T1 . z * T0 . z;
 ry = - T1 . z * T0 . z;

}
 else {
 float alpha_div = T0 . z * T1 . z / prod;
 float factor_div = 1.0f + alpha_div;
 rx = ( T0 . x * T1 . x + T0 . y * T1 . y ) * inv_T1_mag2 * factor_div;
 ry = ( T1 . x * T0 . y - T0 . x * T1 . y ) * inv_T1_mag2 * factor_div;

}
 float rz = ( rxy_T1 * T0 . z - rxy_T0 * T1 . z ) * inv_T1_mag2;
 return FormulaVec3 ( rx , ry , rz );

}

}}
