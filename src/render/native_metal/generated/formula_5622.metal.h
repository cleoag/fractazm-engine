// Generated from dIFS/SphereIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5622 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_SPHEREIFS = 5622;
 template < > FormulaVec3 execute_formula_dIFS < FORMULA_SPHEREIFS > ( FormulaVec3 p , FormulaParams params , thread float & varyScale , thread float & rout ) {
 const float Radius = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Z_size_recip = params [ 5 ];
 const float Y_size_recip = params [ 6 ];
 const float X_size_recip = params [ 7 ];
 const int Apply_scale_add = int ( params [ 8 ] );
 const float Inside_radius = params [ 12 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 varyScale *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;

}
 else {
 params [ DIFS_COMMIT_XYZ_SLOT ] = 0.0f;

}
 float sx = x * X_size_recip;
 float sy = y * Y_size_recip;
 float sz = z * Z_size_recip;
 float r_scaled = native_formula_length3 ( sx , sy , sz );
 float outer_dist = r_scaled - Radius;
 float inner_dist = Inside_radius - r_scaled;
 float sphere_dist = max ( outer_dist , inner_dist );
 float max_size = max ( abs ( X_size_recip ) , max ( abs ( Y_size_recip ) , abs ( Z_size_recip ) ) );
 float shape_de = sphere_dist / max_size;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 rout = shape_de;
 {
 int otrap_opt = int ( params [ 9 ] ) & 3;
 float otrap_scale = static_cast < float > ( params [ 11 ] );
 float otrap_offset = static_cast < float > ( params [ 10 ] );
 float source;
 switch ( otrap_opt ) {
 default : case 0 : source = params [ MAX_FORMULA_PARAMS - 2 ];
 break;
 case 1 : source = x;
 break;
 case 2 : source = y;
 break;
 case 3 : source = z;
 break;

}
 params [ MAX_FORMULA_PARAMS - 1 ] = source * otrap_scale + otrap_offset;

}
 return FormulaVec3 ( x , y , z );

}
 template < > FormulaVec3 execute_formula < FORMULA_SPHEREIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float vs = 1.0f , rout = 0.0f;
 auto r = execute_formula_dIFS < FORMULA_SPHEREIFS > ( p , params , vs , rout );
 de = rout;
 return r;

}
 FormulaVec3 SphereIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_SPHEREIFS > ( p , params , de );

}

}}
