// Generated from 3D/BT2Pine.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_129 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BT2PINE = 129;
 template < > FormulaVec3 execute_formula < FORMULA_BT2PINE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float xOff = params [ 1 ];
 const float pixel_x = params [ 2 ];
 const float pixel_y = params [ 3 ];
 const float pixel_z = params [ 4 ];
 const float c1 = 0.816496580927726f;
 const float s1 = 0.577350269189626f;
 const float h = 0.707106781186548f;
 const float Cx = params [ 8 ];
 const float Cy = params [ 9 ];
 const float Cz = params [ 10 ];
 float x = p . x , y = p . y , z = p . z;
 float tx = ( x * c1 - z * s1 ) * h;
 y = y * h;
 float z_sq = ( x * s1 + z * c1 );
 z_sq = z_sq * z_sq;
 float ty_sq = ( tx + y ) * ( tx + y );
 float tx_sq = ( tx - y ) * ( tx - y );
 x = fabs ( sqrt ( ty_sq + z_sq ) - xOff ) * Scale * h;
 y = fabs ( sqrt ( tx_sq + z_sq ) - xOff ) * Scale * h;
 z = fabs ( sqrt ( tx_sq + ty_sq ) - xOff ) * Scale;
 float tmp = y + x;
 y = y - x;
 x = tmp * c1 + z * s1;
 z = z * c1 - tmp * s1;
 float xt = x * x;
 float yt = y * y;
 float zt = z * z;
 float t = 2.0f * x / sqrt ( yt + zt );
 float new_x = xt - yt - zt + Cx * pixel_x;
 float new_y = 2.0f * t * z * y + Cy * pixel_z;
 float new_z = t * ( yt - zt ) + Cz * pixel_y;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
