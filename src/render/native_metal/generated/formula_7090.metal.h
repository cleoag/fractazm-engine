// Generated from 3D/BPine_only.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7090 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BPINE_ONLY = 7090;
 template < > FormulaVec3 execute_formula < FORMULA_BPINE_ONLY > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float CxScale = params [ 0 ];
 const float CzScale = params [ 1 ];
 const float CyScale = params [ 2 ];
 const float Cx = params [ 3 ];
 const float Cy = params [ 4 ];
 const float Cz = params [ 5 ];
 float x = p . x , y = p . y , z = p . z;
 float x2 = x * x;
 float y2 = y * y;
 float z2 = z * z;
 float rho = sqrt ( y2 + z2 );
 float factor;
 if ( rho == 0.0f && ( y != 0.0f || z != 0.0f ) ) {
 factor = 0.0f;

}
 else {
 factor = 2.0f * x / rho;

}
 float new_x = x2 - y2 - z2 + Cx * CxScale;
 float new_y = factor * 2.0f * z * y + Cy * CyScale;
 float new_z = factor * ( y2 - z2 ) + Cz * CzScale;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
