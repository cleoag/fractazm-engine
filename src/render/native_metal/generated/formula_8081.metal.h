// Generated from 3D/RuckerBrot1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8081 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_RUCKERBROT1 = 8081;
 template < > FormulaVec3 execute_formula < FORMULA_RUCKERBROT1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float XMul = params [ 0 ];
 const float YMul = params [ 1 ];
 const float ZMul = params [ 2 ];
 const float CzMul = params [ 3 ];
 const float eps = params [ 4 ];
 float x = p . x , y = p . y , z = p . z;
 const float Cx = params [ 5 ];
 const float Cy = params [ 6 ];
 const float Cz = params [ 7 ];
 float x2 = x * x;
 float y2 = y * y;
 float z2 = z * z;
 float r2 = x2 + ( y2 + z2 );
 float T = r2 / ( sqrt ( x2 + z2 ) + eps );
 float U = ( x2 - y2 ) / ( x2 + y2 + eps );
 float new_x = ( U * T ) * fabs ( x ) * XMul + Cx;
 float V = x * y * fabs ( x ) / ( x2 + y2 + eps );
 float new_y = V * T * YMul + Cy;
 float new_z = z * sqrt ( r2 ) * ZMul + fabs ( Cz ) * CzMul;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
