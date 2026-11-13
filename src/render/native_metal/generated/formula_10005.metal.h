// Generated from Ads/_PartlyJuliaRoff.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10005 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__PARTLYJULIAROFF = 10005;
 template < > FormulaVec3 execute_formula < FORMULA__PARTLYJULIAROFF > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float X_sphere = params [ 0 ];
 const float Y_sphere = params [ 1 ];
 const float Z_sphere = params [ 2 ];
 const float Radius_sphere = params [ 3 ];
 float Cx = params [ 5 ];
 float Cy = params [ 6 ];
 float Cz = params [ 7 ];
 float xt = params [ 11 ] - X_sphere;
 float yt = params [ 12 ] - Y_sphere;
 float zt = params [ 13 ] - Z_sphere;
 float r = native_formula_length3 ( xt , yt , zt );
 if ( r > Radius_sphere ) {
 float rt = r - Radius_sphere;
 float r_safe = ( fabs ( r ) < 1e-10f ) ? copysign ( 1e-10f , r ) : r;
 float m = rt / r_safe;
 Cx += xt * m;
 Cy += yt * m;
 Cz += zt * m;

}
 params [ WRITE_J1_SLOT ] = Cx;
 params [ WRITE_J2_SLOT ] = Cy;
 params [ WRITE_J3_SLOT ] = Cz;
 return p;

}

}}
