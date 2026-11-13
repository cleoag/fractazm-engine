// Generated from 3Da/koch_cube.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10175 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_KOCH_CUBE = 10175;
 template < > FormulaVec3 execute_formula < FORMULA_KOCH_CUBE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Post_scale = params [ 0 ];
 const float XY_Stretch = params [ 1 ];
 const float ZFold = params [ 2 ];
 const float X_Add = params [ 3 ];
 const float Y_Add = params [ 4 ];
 const float Z_Add = params [ 5 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 6 );
 const float const3 = 3.0f;
 float x = abs ( p . x ) * const3;
 float y = abs ( p . y ) * const3;
 float z = abs ( p . z ) * const3;
 de = de * const3 / XY_Stretch * Post_scale;
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
 z += Z_Add;
 y += Y_Add;
 x += X_Add;
 FormulaVec3 rv = R * FormulaVec3 ( x , y , z );
 x = rv . x;
 y = rv . y;
 z = rv . z;
 z = ZFold - abs ( ZFold - z );
 float a = const3 + XY_Stretch;
 float b = const3 - XY_Stretch;
 float c = x - b;
 float d = x - a;
 if ( c < y ) {
 x = c;
 y = y - b;

}
 else if ( d > y ) {
 x = d;

}
 else {
 float t = y;
 y = d;
 x = t;

}
 x = x * Post_scale / XY_Stretch;
 y = y * Post_scale / XY_Stretch;
 z = z * Post_scale;
 return FormulaVec3 ( x , y , z );

}

}}
