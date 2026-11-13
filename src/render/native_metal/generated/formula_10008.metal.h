// Generated from Ads/_DonutTransform.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10008 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__DONUTTRANSFORM = 10008;
 template < > FormulaVec3 execute_formula < FORMULA__DONUTTRANSFORM > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Offset = params [ 0 ];
 const float ROffset = params [ 1 ];
 const float Order = params [ 2 ];
 float angle = float ( atan2 ( p . x , p . y ) * Order );
 float full_radius = sqrt ( p . x * p . x + p . y * p . y );
 float radius = float ( full_radius );
 float t_radius = float ( full_radius - Offset );
 float s = sin ( float ( angle ) );
 float c = cos ( float ( angle ) );
 float new_x = s * radius - ROffset;
 float new_y = c * radius - ROffset;
 float new_z = p . z - ROffset;
 params [ FORMULA_STATE_W_SLOT ] = ( p . z * p . z - ROffset ) + float ( t_radius ) * float ( t_radius );
 const float cx = params [ 3 ] , cy = params [ 4 ] , cz = params [ 5 ];
 const float cangle = float ( atan2 ( cx , cy ) * Order );
 const float cfull_radius = sqrt ( cx * cx + cy * cy );
 const float cradius = float ( cfull_radius );
 const float ctradius = float ( cfull_radius - Offset );
 params [ WRITE_J1_SLOT ] = sin ( float ( cangle ) ) * cradius - ROffset;
 params [ WRITE_J2_SLOT ] = cos ( float ( cangle ) ) * cradius - ROffset;
 params [ WRITE_J3_SLOT ] = cz - ROffset;
 params [ WRITE_J4_SLOT ] = ( cz * cz - ROffset ) + float ( ctradius ) * float ( ctradius );
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
