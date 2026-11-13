// Generated from Ads/_SinhZ.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7890 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SINHZ = 7890;
 template < > FormulaVec3 execute_formula < FORMULA__SINHZ > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float fixZ = params [ 0 ];
 const float fixLg = params [ 1 ];
 const float Mul = params [ 2 ];
 const float Base = params [ 3 ];
 float z = p . z;
 float temp = Mul * z + fixZ;
 float v = temp + sqrt ( temp * temp + 1.0f );
 float new_z = Base * log2 ( max ( v , 1e-30f ) ) - fixLg;
 return FormulaVec3 ( p . x , p . y , new_z );

}
 FormulaVec3 _SinhZ_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float fZ = params [ 0 ] , fL = params [ 1 ] , M = params [ 2 ] , B = params [ 3 ];
 float t = M * p . z + fZ;
 float v = t + sqrt ( t * t + 1.0f );
 return FormulaVec3 ( p . x , p . y , B * log2 ( max ( v , 1e-30f ) ) - fL );

}

}}
