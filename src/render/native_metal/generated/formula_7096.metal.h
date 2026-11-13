// Generated from Ads/_FoldingTetra3d.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7096 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__FOLDINGTETRA3D = 7096;
 template < > FormulaVec3 execute_formula < FORMULA__FOLDINGTETRA3D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int Mode1 = int ( params [ 0 ] );
 const int Mode2 = int ( params [ 1 ] );
 float x = p . x , y = p . y , z = p . z;
 if ( Mode1 != 0 ) {
 if ( x + y <= 0.0f ) {
 float t = x;
 x = - y;
 y = - t;

}
 if ( x + z <= 0.0f ) {
 float t = x;
 x = - z;
 z = - t;

}
 if ( y + z <= 0.0f ) {
 float t = y;
 y = - z;
 z = - t;

}
 if ( Mode2 != 0 ) {
 if ( x < y ) {
 float t = x;
 x = y;
 y = t;

}
 if ( x < z ) {
 float t = x;
 x = z;
 z = t;

}
 if ( y < z ) {
 float t = y;
 y = z;
 z = t;

}

}

}
 else {
 if ( x < y ) {
 float t = x;
 x = y;
 y = t;

}
 if ( x < z ) {
 float t = x;
 x = z;
 z = t;

}
 if ( y < z ) {
 float t = y;
 y = z;
 z = t;

}

}
 return FormulaVec3 ( x , y , z );

}

}}
