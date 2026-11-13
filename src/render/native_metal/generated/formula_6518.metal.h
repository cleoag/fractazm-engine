// Generated from Ads/_Rotate4d.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6518 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__ROTATE4D = 6518;
 template < > FormulaVec3 execute_formula < FORMULA__ROTATE4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 RotMatrix4 rot = rotMatrix4FromFloats ( params , 0 );
 float4 v = {
 p . x , p . y , p . z , params [ FORMULA_STATE_W_SLOT ]
}
;
 float4 out = rot * v;
 params [ FORMULA_STATE_W_SLOT ] = out . w;
 return FormulaVec3 ( out . x , out . y , out . z );

}

}}
