// Generated from 3Da/ABoxModKali.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8900 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXMODKALI = 8900;
 template < > FormulaVec3 execute_formula < FORMULA_ABOXMODKALI > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float innerScale = params [ 1 ];
 const float minR2 = params [ 2 ];
 const float Z_add = params [ 3 ];
 const float Y_add = params [ 4 ];
 const float X_add = params [ 5 ];
 const float fixedR2 = 1.0f;
 float vx = X_add - fabs ( p . x );
 float vy = Y_add - fabs ( p . y );
 float vz = Z_add - fabs ( p . z );
 float r2 = vx * vx + vy * vy + vz * vz;
 float scale;
 if ( r2 < minR2 ) {
 scale = innerScale;

}
 else if ( r2 < fixedR2 ) {
 scale = Scale / r2;

}
 else {
 scale = Scale;

}
 de = fabs ( de ) * scale;
 float x_out = vx * scale;
 float y_out = vy * scale;
 float z_out = vz * scale;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
