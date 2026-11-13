// Generated from 4D/QuadrayBrot.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_1946 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_QUADRAYBROT = 1946;
 template < > FormulaVec3 execute_formula < FORMULA_QUADRAYBROT > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Cx = params [ 0 ];
 const float Cy = params [ 1 ];
 const float Cz = params [ 2 ];
 float x = p . x , y = p . y , z = p . z;
 float w = de;
 float ctx = abs ( Cx + Cy + Cz );
 float cty = abs ( - Cx - Cy + Cz );
 float ctz = abs ( - Cx + Cy - Cz );
 float ctw = abs ( Cx - Cy - Cz );
 float wz = 2.0f * w * z;
 float xy = 2.0f * x * y;
 float new_x = x * x - y * y + wz + ctx;
 float new_y = y * y - x * x + wz + cty;
 float new_z = z * z - w * w + xy + ctz;
 float new_w = w * w - z * z + xy + ctw;
 de = new_w;
 return FormulaVec3 ( new_x , new_y , new_z );

}
 FormulaVec3 QuadrayBrot_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Cx = params [ 0 ] , Cy = params [ 1 ] , Cz = params [ 2 ];
 float x = p . x , y = p . y , z = p . z , w = de;
 float x2 = x * x , y2 = y * y , z2 = z * z;
 float xy2 = 2.0f * x * y;
 float w2 = w * w;
 de = w2 - z2 + xy2 + abs ( Cx - Cy - Cz );
 return FormulaVec3 ( x2 - y2 + 2.0f * w * z + abs ( Cx + Cy + Cz ) , y2 - x2 + 2.0f * w * z + abs ( - Cx - Cy + Cz ) , z2 - w2 + xy2 + abs ( - Cx + Cy - Cz ) );

}

}}
