// Generated from 3D/_sphereinv.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7092 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SPHEREINV = 7092;
 template < > FormulaVec3 execute_formula < FORMULA__SPHEREINV > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float ShiftX = params [ 0 ];
 const float ShiftY = params [ 1 ];
 const float ShiftZ = params [ 2 ];
 const float RadiusScale = params [ 3 ];
 const float MinRadius = params [ 4 ];
 const float FixedFactor = params [ 5 ];
 const int Mode = int ( params [ 6 ] );
 float x = p . x , y = p . y , z = p . z;
 float xs = x + ShiftX;
 float ys = y + ShiftY;
 float zs = z + ShiftZ;
 float r_scaled = ( xs * xs + ys * ys + zs * zs ) * RadiusScale;
 if ( Mode != 0 && r_scaled >= 1.0f ) {
 return FormulaVec3 ( x , y , z );

}
 float factor;
 if ( r_scaled <= MinRadius ) {
 if ( Mode == 2 ) {
 factor = 1.0f / ( 2.0f * MinRadius - r_scaled );

}
 else {
 factor = FixedFactor;

}

}
 else {
 factor = 1.0f / r_scaled;

}
 float new_x = xs * factor - ShiftX;
 float new_y = ys * factor - ShiftY;
 float new_z = zs * factor - ShiftZ;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
