// Generated from Ads/_MapTranslate.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10006 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__MAPTRANSLATE = 10006;
 inline FormulaVec3 mapTranslateSample ( FormulaVec3 p , FormulaParams params , FormulaVec3 sample ) {
 const numeric :: X87Scope extendedScope;
 return FormulaVec3 ( float ( ( numeric :: X87Real ) p . x + ( ( numeric :: X87Real ) sample . x + params [ 1 ] ) * params [ 4 ] ) , float ( ( numeric :: X87Real ) p . y + ( ( numeric :: X87Real ) sample . y + params [ 2 ] ) * params [ 5 ] ) , float ( ( numeric :: X87Real ) p . z + ( ( numeric :: X87Real ) sample . z + params [ 3 ] ) * params [ 6 ] ) );

}
 template < > FormulaVec3 execute_formula < FORMULA__MAPTRANSLATE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 ( void ) de;
 return mapTranslateSample ( p , params , native_formula_map_sphere ( params , p . x , p . y , p . z , native_int32_parameter ( params , 0 ) ) );

}
 template < > FormulaVec3 execute_formula_with_sphere_map < FORMULA__MAPTRANSLATE > ( FormulaVec3 p , FormulaParams params , thread float & de , bool map ) {
 if ( ! map ) return execute_formula < FORMULA__MAPTRANSLATE > ( p , params , de );
 TVec3D input = {
 p . x , p . y , p . z
}
 , sample {

}
;
 native_formula_map ( params , & sample , & input , native_int32_parameter ( params , 0 ) , false );
 return mapTranslateSample ( p , params , FormulaVec3 ( sample [ 0 ] , sample [ 1 ] , sample [ 2 ] ) );

}

}}
