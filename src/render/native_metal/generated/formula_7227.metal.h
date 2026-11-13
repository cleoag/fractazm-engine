// Generated from 3Da/koch_surf.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7227 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_KOCH_SURF = 7227;
 template < > FormulaVec3 execute_formula < FORMULA_KOCH_SURF > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float PFOrder = params [ 0 ];
 const float PFShift_deg = params [ 1 ];
 const float PFShift_x = params [ 2 ];
 const float PFShift_y = params [ 3 ];
 const float YOff1 = params [ 4 ];
 const float YOff2 = params [ 5 ];
 const float Scale = params [ 6 ];
 const float CScale_Z = params [ 7 ];
 const float CScale_Y = params [ 8 ];
 const float CScale_X = params [ 9 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 10 );
 const int Variation = int ( params [ 19 ] );
 const float TWO_PI = 6.283185307f;
 const float DEG2RAD = 0.017453293f;
 float PFShift_rad = PFShift_deg * DEG2RAD;
 float x = p . x + PFShift_x;
 float y = p . y + PFShift_y;
 float z = p . z;
 float angle = atan2 ( y , x ) + PFShift_rad;
 int n = int ( rint ( angle / TWO_PI * PFOrder ) );
 float rot_angle = - float ( n ) / PFOrder * TWO_PI + PFShift_rad;
 float cs = cos ( rot_angle );
 float ss = sin ( rot_angle );
 float comp1 = y * ss - x * cs;
 float comp2 = x * ss + y * cs;
 if ( Variation & 2 ) {
 comp2 = abs ( comp2 );

}
 else if ( ( n & 1 ) == 0 ) {
 comp2 = - comp2;

}
 y = YOff1 - abs ( comp2 - PFShift_y - YOff1 );
 x = comp1 - PFShift_x;
 x += YOff2;
 if ( z > x ) {
 float tmp = x;
 x = z;
 z = tmp;

}
 x -= YOff2;
 if ( Variation & 1 ) {
 x += YOff2;
 if ( z > x ) {
 float tmp = x;
 x = z;
 z = tmp;

}
 x -= YOff2;

}
 else {
 x -= YOff2;
 if ( z > x ) {
 float tmp = x;
 x = z;
 z = tmp;

}
 x += YOff2;

}
 float vx = x + CScale_X;
 float vy = y + CScale_Y;
 float vz = z + CScale_Z;
 FormulaVec3 rv = R * FormulaVec3 ( vx , vy , vz );
 float rx = rv . x;
 float ry = rv . y;
 float rz = rv . z;
 de = de * Scale;
 FormulaVec3 result;
 result . x = rx * Scale - CScale_X;
 result . y = ry * Scale - CScale_Y;
 result . z = rz * Scale - CScale_Z;
 return result;

}
 FormulaVec3 koch_surf_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_KOCH_SURF > ( p , params , de );

}

}}
