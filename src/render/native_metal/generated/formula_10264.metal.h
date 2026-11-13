// Generated from 3D/Quad3Db.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10264 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_QUAD3DB = 10264;
 template < > FormulaVec3 execute_formula < FORMULA_QUAD3DB > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float X_mul = params [ 0 ];
 const float Y_mul = params [ 1 ];
 const float Z_mul = params [ 2 ];
 const float HRadius = params [ 3 ];
 const float ZRadius = params [ 4 ];
 const int Variants = ( int ) params [ 5 ];
 const float Cx = params [ 6 ];
 const float Cy = params [ 7 ];
 const float Cz = params [ 8 ];
 float x = p . x , y = p . y , z = p . z;
 if ( ! metal :: isfinite ( x ) || ! metal :: isfinite ( y ) || ! metal :: isfinite ( z ) ) {
 return FormulaVec3 ( 0.0f , 0.0f , 0.0f );

}
 float R2 = x * x + y * y + HRadius;
 if ( ! metal :: isfinite ( R2 ) ) {
 return FormulaVec3 ( 0.0f , 0.0f , 0.0f );

}
 if ( R2 < 0.0f ) {
 return FormulaVec3 ( 0.0f , 0.0f , 0.0f );

}
 if ( R2 == 0.0f ) {
 return FormulaVec3 ( Cx , Cy , Cz );

}
 int mode_x = Variants & 3;
 int mode_y = ( Variants >> 2 ) & 3;
 int mode_z = ( Variants >> 4 ) & 3;
 float val_x = ( x * x - y * y ) * X_mul * ( ZRadius - z * z / R2 );
 if ( ! metal :: isfinite ( val_x ) ) {
 return FormulaVec3 ( 0.0f , 0.0f , 0.0f );

}
 float new_x;
 if ( mode_x == 0 ) {
 new_x = abs ( val_x ) + Cx;

}
 else if ( mode_x == 1 ) {
 new_x = abs ( val_x + Cx );

}
 else if ( mode_x == 2 ) {
 new_x = abs ( val_x + Cx ) - abs ( val_x - Cx ) + ( - val_x );
 new_x = abs ( val_x + Cx ) - abs ( val_x - Cx ) - val_x;

}
 else {
 new_x = val_x + Cx;

}
 float val_y = x * y * Y_mul * ( ZRadius - z * z / R2 );
 if ( ! metal :: isfinite ( val_y ) ) {
 return FormulaVec3 ( 0.0f , 0.0f , 0.0f );

}
 float new_y;
 if ( mode_y == 0 ) {
 new_y = abs ( val_y ) + Cy;

}
 else if ( mode_y == 1 ) {
 new_y = abs ( val_y + Cy );

}
 else if ( mode_y == 2 ) {
 new_y = abs ( val_y + Cy ) - abs ( val_y - Cy ) - val_y;

}
 else {
 new_y = val_y + Cy;

}
 float val_z = z * sqrt ( R2 ) * Z_mul;
 if ( ! metal :: isfinite ( val_z ) ) {
 return FormulaVec3 ( 0.0f , 0.0f , 0.0f );

}
 float new_z;
 if ( mode_z == 0 ) {
 new_z = abs ( val_z ) + Cz;

}
 else if ( mode_z == 1 ) {
 new_z = abs ( val_z + Cz );

}
 else if ( mode_z == 2 ) {
 new_z = abs ( val_z + Cz ) - abs ( val_z - Cz ) - val_z;

}
 else {
 new_z = val_z + Cz;

}
 if ( ! metal :: isfinite ( new_x ) || ! metal :: isfinite ( new_y ) || ! metal :: isfinite ( new_z ) ) {
 return FormulaVec3 ( 0.0f , 0.0f , 0.0f );

}
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
