// Generated from 4D/Barnsley2_4Dc.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10327 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BARNSLEY2_4DC = 10327;
 template < > FormulaVec3 execute_formula < FORMULA_BARNSLEY2_4DC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float dx = params [ 0 ];
 const float dy = params [ 1 ];
 const float dz = params [ 2 ];
 const float dw = params [ 3 ];
 const float w_add = params [ 4 ];
 const float xCy_mul = params [ 5 ];
 const float yCx_mul = params [ 6 ];
 const float threshold = params [ 7 ];
 const float Cx = params [ 8 ];
 const float Cy = params [ 9 ];
 const float Cz = params [ 10 ];
 const float Cw = params [ 11 ];
 float x = p . x , y = p . y , z = p . z , w = de;
 float condition = x * Cy * xCy_mul + y * Cx * yCx_mul;
 if ( condition >= threshold ) {
 x -= dx;
 y -= dy;
 z -= dz;
 w = w - dw + w_add;

}
 else {
 x += dx;
 y += dy;
 z += dz;
 w += dw + w_add;

}
 float nx = x * Cx - y * Cy - z * Cz + w * Cw;
 float ny = y * Cx + x * Cy - w * Cz - z * Cw;
 float nz = z * Cx - w * Cy + x * Cz - y * Cw;
 float nw = w * Cx + z * Cy + y * Cz + x * Cw;
 de = nw;
 return FormulaVec3 ( nx , ny , nz );

}

}}
