// Generated from Ads/_updateC2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10014 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__UPDATEC2 = 10014;
 template < > FormulaVec3 execute_formula < FORMULA__UPDATEC2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 RotMatrix3 R = rotMatrix3FromFloats ( params , 0 );
 const float Scale = params [ 9 ];
 const float AddX = params [ 10 ];
 const float AddY = params [ 11 ];
 const float AddZ = params [ 12 ];
 float Cx = params [ 13 ];
 float Cy = params [ 14 ];
 float Cz = params [ 15 ];
 float x = p . x , y = p . y , z = p . z;
 FormulaVec3 rotated = R * FormulaVec3 ( x , y , z );
 float rx = rotated . x;
 float ry = rotated . y;
 float rz = rotated . z;
 x = rx * Scale + AddX;
 y = ry * Scale + AddY;
 z = rz * Scale + AddZ;
 params [ FORMULA_STATE_W_SLOT ] *= Scale;
 if ( Cx >= x ) {
 float tmp = x;
 x = Cx;
 Cx = tmp;

}
 if ( Cy >= y ) {
 float tmp = y;
 y = Cy;
 Cy = tmp;

}
 if ( Cz >= z ) {
 float tmp = z;
 z = Cz;
 Cz = tmp;

}
 FormulaVec3 v = FormulaVec3 ( x , y , z );
 float ox = dot ( R . col0 , v ) - AddX;
 float oy = dot ( R . col1 , v ) - AddY;
 float oz = dot ( R . col2 , v ) - AddZ;
 params [ WRITE_J1_SLOT ] = Cx;
 params [ WRITE_J2_SLOT ] = Cy;
 params [ WRITE_J3_SLOT ] = Cz;
 return FormulaVec3 ( ox , oy , oz );

}

}}
