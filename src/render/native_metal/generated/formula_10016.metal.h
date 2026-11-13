// Generated from Ads/_FoldingTetraKaleid.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10016 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__FOLDINGTETRAKALEID = 10016;
 template < > FormulaVec3 execute_formula < FORMULA__FOLDINGTETRAKALEID > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float k = params [ 0 ];
 int preRotation = cpu_bitcast_to_int ( params [ 1 ] );
 int undoTetra = cpu_bitcast_to_int ( params [ 2 ] );
 float h = 0.6666666666666667f;
 float x = p . x;
 float y = p . y;
 float z = p . z;
 bool swapped_xy = false;
 bool swapped_xz = false;
 bool swapped_yz = false;
 if ( x + y <= 0.0f ) {
 float tmp = x;
 x = - y;
 y = - tmp;
 swapped_xy = true;

}
 if ( x + z <= 0.0f ) {
 float tmp = x;
 x = - z;
 z = - tmp;
 swapped_xz = true;

}
 if ( z + y <= 0.0f ) {
 float tmp = z;
 z = - y;
 y = - tmp;
 swapped_yz = true;

}
 if ( preRotation != 0 ) {
 float dd = - ( x + y + z ) * h;
 x = - ( x + dd );
 y = - ( y + dd );
 z = - ( z + dd );

}
 float sum = x + y + z;
 if ( sum >= k ) {
 float dd = ( sum - k ) * h;
 x -= dd;
 y -= dd;
 z -= dd;

}
 if ( undoTetra != 0 ) {
 if ( ! swapped_yz ) {
 float tmp = z;
 z = - y;
 y = - tmp;

}
 if ( ! swapped_xz ) {
 float tmp = x;
 x = - z;
 z = - tmp;

}
 if ( ! swapped_xy ) {
 float tmp = x;
 x = - y;
 y = - tmp;

}

}
 return FormulaVec3 ( x , y , z );

}

}}
