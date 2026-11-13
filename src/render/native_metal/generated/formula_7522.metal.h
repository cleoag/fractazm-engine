// Generated from dIFSa/Trans-KochIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7522 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TRANS_KOCHIFS = 7522;
 template < > FormulaVec3 execute_formula < FORMULA_TRANS_KOCHIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Add_X = params [ 0 ];
 const float Add_Y = params [ 1 ];
 const float Add_Z = params [ 2 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 3 );
 const int PolyFold_Order = int ( params [ 12 ] );
 const float sin_Koch = params [ 13 ];
 const float cos_Koch = params [ 14 ];
 const float KochSub = params [ 15 ];
 const float KochScale = params [ 16 ];
 const float KochFixX = params [ 17 ];
 const float KochFixZ = params [ 18 ];
 const float INV_2PI = 0.159154943f;
 const float TWO_PI = 6.283185307f;
 const float PI = 3.14159265358979f;
 float ax = p . x + Add_X;
 float ay = p . y + Add_Y;
 float az = p . z + Add_Z;
 FormulaVec3 rv = R * FormulaVec3 ( ax , ay , az );
 float rot_x = rv . x;
 float rot_y = rv . y;
 float rot_z = rv . z;
 float angle = PI - atan2 ( rot_y , rot_x );
 int n = int ( rint ( angle * INV_2PI * float ( PolyFold_Order ) ) );
 float sector = float ( n ) / float ( PolyFold_Order ) * TWO_PI;
 float cs = cos ( sector );
 float ss = sin ( sector );
 float new_y = rot_y * cs + rot_x * ss;
 float new_x = rot_x * cs - rot_y * ss;
 if ( ( n & 1 ) == 0 ) {
 new_y = - new_y;

}
 new_x = abs ( new_x );
 float x = new_x;
 float y = new_y;
 float z = rot_z;
 float x1 = x * cos_Koch - z * sin_Koch;
 float z1 = x * sin_Koch + z * cos_Koch;
 x = abs ( cos_Koch * KochSub - x1 );
 z = z1;
 float x2 = x * cos_Koch + z * sin_Koch;
 float z2 = z * cos_Koch - x * sin_Koch;
 x = abs ( sin_Koch * KochSub - x2 );
 z = z2;
 x *= KochScale;
 y *= KochScale;
 z *= KochScale;
 de *= KochScale;
 x -= KochFixX;
 z -= KochFixZ;
 return FormulaVec3 ( x , y , z );

}
 FormulaVec3 Trans_KochIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_TRANS_KOCHIFS > ( p , params , de );

}

}}
