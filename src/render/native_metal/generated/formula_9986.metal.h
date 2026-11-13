// Generated from 4D/hyperd_1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9986 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_HYPERD_1 = 9986;
 template < > FormulaVec3 execute_formula < FORMULA_HYPERD_1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float W_add = params [ 0 ];
 int power = int ( params [ 1 ] );
 const float P1 = params [ 2 ];
 const float P2 = params [ 3 ];
 const float P3 = params [ 4 ];
 const float Cx = params [ 5 ];
 const float Cy = params [ 6 ];
 const float Cz = params [ 7 ];
 if ( power < 1 || power > 16 ) {
 power = 2;

}
 const float x = p . x , y = p . y , z = p . z;
 const float w = de;
 float x0 = x , y0 = y , z0 = z;
 float w0 = w + W_add;
 int count = power - 1;
 if ( count < 1 ) count = 1;
 for ( int i = 0;
 i < count;
 i ++ ) {
 float nx = x * x0 - y * y0 - z * z0 - w * w0 + ( z * w0 + w * z0 ) * P1;
 float ny = x * y0 + y * x0;
 float nz = x * z0 + z * x0 + ( y * w0 + w * y0 ) * P2;
 float nw = x * w0 + w * x0 + ( y * z0 + z * y0 ) * P3;
 x0 = nx;
 y0 = ny;
 z0 = nz;
 w0 = nw;

}
 float x_out = x0 + Cx;
 float y_out = y0 + Cy;
 float z_out = z0 + Cz;
 de = w0;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
