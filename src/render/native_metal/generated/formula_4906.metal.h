// Generated from Ads/_BenesiSpheric.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4906 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BENESISPHERIC = 4906;
 template < > FormulaVec3 execute_formula < FORMULA__BENESISPHERIC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 float x = p . x , y = p . y , z = p . z;
 float y2 = y * y;
 float z2 = z * z;
 float x2 = x * x;
 float rCyz = y2 / z2;
 if ( rCyz < 1.0f ) rCyz = sqrt ( rCyz + 1.0f );
 else rCyz = sqrt ( 1.0f / rCyz + 1.0f );
 float rCxyz = ( y2 + z2 ) / x2;
 if ( rCxyz < 1.0f ) rCxyz = sqrt ( rCxyz + 1.0f );
 else rCxyz = sqrt ( 1.0f / rCxyz + 1.0f );
 float factor_x = rCxyz * Scale;
 float factor_yz = rCyz * rCxyz * Scale;
 FormulaVec3 result;
 result . x = x * factor_x;
 result . y = y * factor_yz;
 result . z = z * factor_yz;
 return result;

}
 FormulaVec3 _BenesiSpheric_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 float x = p . x , y = p . y , z = p . z;
 float y2 = y * y , z2 = z * z , x2 = x * x;
 float rCyz = y2 / z2;
 rCyz = ( rCyz < 1.0f ) ? sqrt ( rCyz + 1.0f ) : sqrt ( 1.0f / rCyz + 1.0f );
 float rCxyz = ( y2 + z2 ) / x2;
 rCxyz = ( rCxyz < 1.0f ) ? sqrt ( rCxyz + 1.0f ) : sqrt ( 1.0f / rCxyz + 1.0f );
 float fx = rCxyz * Scale;
 float fyz = rCyz * rCxyz * Scale;
 return FormulaVec3 ( x * fx , y * fyz , z * fyz );

}

}}
