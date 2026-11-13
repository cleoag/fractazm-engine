// Generated from dIFS/gearIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8904 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_GEARIFS = 8904;
 template < > FormulaVec3 execute_formula_dIFS < FORMULA_GEARIFS > ( FormulaVec3 p , FormulaParams params , thread float & varyScale , thread float & rout ) {
 const float Angle = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Cyl_Radius = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float RepeatAngle = params [ 7 ];
 const float Thick1 = params [ 8 ];
 const float ThickZ = params [ 9 ];
 const float Cyl_R2 = params [ 10 ];
 const float Thick2 = params [ 11 ];
 const float K_360_OVER_PI = params [ 12 ];
 const float K_PI_OVER_360 = params [ 13 ];
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
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 const E sector = ( extendedAtan2 ( E ( y ) , E ( x ) ) * K_360_OVER_PI ) / RepeatAngle;
 const E tooth = - ( ( ( abs ( ( sector - round ( sector ) ) * RepeatAngle ) / Angle - E ( 1 ) ) * Angle ) * K_PI_OVER_360 );
 const E radius = sqrt ( E ( x ) * x + E ( y ) * y );
 const E shell1 = abs ( ( radius - Cyl_Radius ) + Thick1 ) - Thick1;
 E distance = shell1 >= tooth ? shell1 : tooth;
 const E shell2 = abs ( ( E ( float ( radius ) ) - Cyl_R2 ) + Thick2 ) - Thick2;
 if ( ! ( shell2 >= distance ) ) distance = shell2;
 const E plane = abs ( E ( z ) ) - ThickZ;
 if ( plane >= distance ) distance = plane;
 if ( ! Apply_scale_add ) distance /= Scale;
 rout = float ( distance );
 return FormulaVec3 ( x , y , z );

}
 template < > FormulaVec3 execute_formula < FORMULA_GEARIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float vs = 1.0f , rout = 0.0f;
 auto r = execute_formula_dIFS < FORMULA_GEARIFS > ( p , params , vs , rout );
 de = rout;
 return r;

}
 FormulaVec3 gearIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_GEARIFS > ( p , params , de );

}

}}
