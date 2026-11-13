// Generated from dIFS/lidinoidIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5011 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_LIDINOIDIFS = 5011;
 template < > FormulaVec3 execute_formula < FORMULA_LIDINOIDIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Size = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Density = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float TrapColor_Density = params [ 16 ];
 const float K_HALF = 0.5f;
 const float K_OFFSET = 0.15f;
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 float state_x = p . x;
 float state_y = p . y;
 float state_z = p . z;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;
 state_x = x;
 state_y = y;
 state_z = z;

}
 float rx = params [ 7 ] * x + params [ 8 ] * y + params [ 9 ] * z;
 float ry = params [ 10 ] * x + params [ 11 ] * y + params [ 12 ] * z;
 float rz = params [ 13 ] * x + params [ 14 ] * y + params [ 15 ] * z;
 float D = Density;
 float sxD , cxD , syD , cyD , szD , czD;
 sxD = sincos ( rx * D , cxD );
 syD = sincos ( ry * D , cyD );
 szD = sincos ( rz * D , czD );
 float sx , cx , sy , cy , sz , cz;
 sx = sincos ( rx , cx );
 sy = sincos ( ry , cy );
 sz = sincos ( rz , cz );
 float term1 = sy * ( cx * sxD );
 float term2 = cz * ( sx * szD );
 float term3 = sz * ( cy * syD );
 params [ MAX_FORMULA_PARAMS - 1 ] = fabs ( ( term1 - term2 - term3 ) * TrapColor_Density );
 float Part1 = term1 + term2 + term3;
 float Part2 = cxD * cyD + ( cyD * czD + czD * cxD );
 float lidinoid_value = K_HALF * ( Part1 - Part2 ) + K_OFFSET;
 float cube_dist = abs ( rz );
 if ( abs ( ry ) >= cube_dist ) cube_dist = abs ( ry );
 if ( abs ( rx ) >= cube_dist ) cube_dist = abs ( rx );
 cube_dist -= Size;
 float shape_de = ( cube_dist >= lidinoid_value ? cube_dist : lidinoid_value ) / Density;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 lidinoidIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_LIDINOIDIFS > ( p , params , de );

}

}}
