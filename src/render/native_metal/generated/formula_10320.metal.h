// Generated from Ads/_TranslateRnd.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10320 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
inline int incrementFormulaCounter ( FormulaParams params ) {
 return native_increment_formula_counter ( params );

}
 inline uint formulaCounterRandomWord ( int count ) {
 const uint low = ( static_cast < uint > ( count ) - 0x3D07u ) & 0xFFFFu;
 return ( low * low ) & 0xFFFFu;

}
 constant constexpr unsigned int FORMULA__TRANSLATERND = 10320;
 template < > FormulaVec3 execute_formula < FORMULA__TRANSLATERND > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const uint word = formulaCounterRandomWord ( incrementFormulaCounter ( params ) );
 float xyz [ ] = {
 p . x , p . y , p . z
}
;
 for ( int axis = 0;
 axis < 3;
 ++ axis ) {
 const uint seed = static_cast < uint > ( native_int32_parameter ( params , 3 + axis ) );
 const float randomValue = ( ( seed & 0xFFFFu ) * word ) & 0x3FFFu;
 xyz [ axis ] = float ( xyz [ axis ] + static_cast < float > ( params [ axis ] ) * ( static_cast < float > ( randomValue ) * params [ 7 ] - params [ 6 ] ) );

}
 return FormulaVec3 ( xyz [ 0 ] , xyz [ 1 ] , xyz [ 2 ] );

}

}}
