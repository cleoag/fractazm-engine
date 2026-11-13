// Generated from Ads/_BenesiUnSpheric.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_1260 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BENESIUNSPHERIC = 1260;
 template < > FormulaVec3 execute_formula < FORMULA__BENESIUNSPHERIC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 float x = p . x , y = p . y , z = p . z;
 float y2 = y * y , z2 = z * z , x2 = x * x;
 float rCyz_ratio = y2 / z2;
 float rCyz;
 if ( rCyz_ratio < 1.0f ) rCyz = 1.0f / sqrt ( rCyz_ratio + 1.0f );
 else rCyz = 1.0f / sqrt ( 1.0f / rCyz_ratio + 1.0f );
 float new_y = y * rCyz;
 float new_z = z * rCyz;
 float rCxyz_ratio = ( new_y * new_y + new_z * new_z ) / x2;
 float rCxyz;
 if ( rCxyz_ratio < 1.0f ) rCxyz = 1.0f / sqrt ( rCxyz_ratio + 1.0f );
 else rCxyz = 1.0f / sqrt ( 1.0f / rCxyz_ratio + 1.0f );
 float combined = rCxyz * Scale;
 FormulaVec3 result;
 result . x = x * combined;
 result . y = new_y * combined;
 result . z = new_z * combined;
 return result;

}
 FormulaVec3 _BenesiUnSpheric_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float S = params [ 0 ];
 float x = p . x , y = p . y , z = p . z;
 float y2 = y * y , z2 = z * z , x2 = x * x;
 float r1 = y2 / z2;
 float rCyz = ( r1 < 1.0f ) ? rsqrt ( r1 + 1.0f ) : rsqrt ( 1.0f / r1 + 1.0f );
 float ny = y * rCyz , nz = z * rCyz;
 float r2 = ( ny * ny + nz * nz ) / x2;
 float rCxyz = ( r2 < 1.0f ) ? rsqrt ( r2 + 1.0f ) : rsqrt ( 1.0f / r2 + 1.0f );
 float c = rCxyz * S;
 return FormulaVec3 ( x * c , ny * c , nz * c );

}

}}
