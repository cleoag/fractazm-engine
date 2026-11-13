// Generated from dIFS/RoundedBox.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9624 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ROUNDEDBOX = 9624;
 template < > FormulaVec3 execute_formula < FORMULA_ROUNDEDBOX > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Z_halfwidth = params [ 0 ];
 const float Y_halfwidth = params [ 1 ];
 const float X_halfwidth = params [ 2 ];
 const float Scale = params [ 3 ];
 const float Z_add = params [ 4 ];
 const float Y_add = params [ 5 ];
 const float X_add = params [ 6 ];
 const float Border = params [ 7 ];
 const int Apply_scale_add = int ( params [ 8 ] );
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;

}
 float qx = max ( abs ( x ) - X_halfwidth + Border , 0.0f );
 float qy = max ( abs ( y ) - Y_halfwidth + Border , 0.0f );
 float qz = max ( abs ( z ) - Z_halfwidth + Border , 0.0f );
 float shape_de = native_formula_length3 ( qx , qy , qz ) - Border;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 const int OTrap_option = int ( params [ 9 ] );
 const float OTrap_offset = params [ 10 ];
 const float OTrap_scale = params [ 11 ];
 const FormulaVec3 committed = Apply_scale_add ? FormulaVec3 ( x , y , z ) : p;
 float val;
 switch ( OTrap_option & 3 ) {
 case 0 : val = params [ MAX_FORMULA_PARAMS - 2 ];
 break;
 case 1 : val = committed . x;
 break;
 case 2 : val = committed . y;
 break;
 case 3 : val = committed . z;
 break;
 default : val = 0.0f;
 break;

}
 params [ 63 ] = val * OTrap_scale + OTrap_offset;
 return committed;

}
 FormulaVec3 RoundedBox_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ROUNDEDBOX > ( p , params , de );

}

}}
