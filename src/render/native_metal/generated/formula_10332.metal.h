// Generated from 4D/Barnsley4Dc2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10332 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BARNSLEY4DC2 = 10332;
 template < > FormulaVec3 execute_formula < FORMULA_BARNSLEY4DC2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float dx = params [ 0 ];
 const float dy = params [ 1 ];
 const float dz = params [ 2 ];
 const float dw = params [ 3 ];
 const float w_add = params [ 4 ];
 const float T1 = params [ 5 ];
 const float T2 = params [ 6 ];
 const int conjMode = int ( params [ 7 ] );
 const int flipMode = int ( params [ 8 ] );
 const float half_val = 0.5f;
 const float Cx = params [ 11 ];
 const float Cy = params [ 12 ];
 const float Cz_val = params [ 13 ];
 const float Cw = params [ 14 ];
 float x = p . x , y = p . y , z = p . z , w = de;
 float x1 = x - w;
 float y1 = y + z;
 float z1 = x + w;
 float w1 = y - z;
 x = x1;
 y = y1;
 z = z1;
 w = w1;
 if ( x >= T1 ) {
 x -= dx;
 y -= dy;

}
 else {
 x += dx;
 y += dy;

}
 if ( z >= T2 ) {
 z -= dz;
 w -= dw;

}
 else {
 z += dz;
 w += dw;

}
 float cx1 = Cx , cy1 = Cy;
 if ( conjMode != 0 ) cy1 = - cy1;
 if ( flipMode != 0 ) {
 float tmp = cx1;
 cx1 = cy1;
 cy1 = tmp;

}
 float nx = x * cx1 - y * cy1;
 float ny = x * cy1 + y * cx1;
 x = nx;
 y = ny;
 float cz1 = Cz_val , cw1 = Cw;
 if ( conjMode != 0 ) cw1 = - cw1;
 if ( flipMode != 0 ) {
 float tmp = cz1;
 cz1 = cw1;
 cw1 = tmp;

}
 float nz = z * cz1 - w * cw1;
 float nw = x * cw1 + w * cz1 + w_add;
 z = nz;
 w = nw;
 float fx = ( x + z ) * half_val;
 float fy = ( y + w ) * half_val;
 float fz = ( y - w ) * half_val;
 float fw = ( - x + z ) * half_val;
 de = fw;
 return FormulaVec3 ( fx , fy , fz );

}

}}
