// Generated from 4D/_Exp4D.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_124 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__EXP4D = 124;
 template < > FormulaVec3 execute_formula < FORMULA__EXP4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float threshold = params [ 0 ];
 float x = p . x , y = p . y , z = p . z , w = de;
 float r = native_formula_length3 ( y , z , w );
 float e = exp ( x );
 float x_out = e * cos ( r );
 float factor = e * sin ( r );
 if ( r > threshold ) {
 factor /= r;

}
 float y_out = factor * y;
 float z_out = factor * z;
 float w_out = factor * w;
 de = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
