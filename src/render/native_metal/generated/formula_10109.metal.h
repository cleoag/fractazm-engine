// Generated from dIFS/Seashell.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10109 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_SEASHELL = 10109;
 template < > FormulaVec3 execute_formula < FORMULA_SEASHELL > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float state_x = p . x;
 float state_y = p . y;
 float state_z = p . z;
 ulong xAddBits;
 xAddBits = params . bits ( 4 );
 const int Apply_scale_add = static_cast < uint > ( xAddBits >> 32 );
 const ulong trapBits = ( xAddBits << 32 ) | static_cast < uint > ( static_cast < int > ( params [ 5 ] ) );
 float OTrapOnIts;
 native_std :: memcpy ( & OTrapOnIts , & trapBits , sizeof ( OTrapOnIts ) );
 const float OTrapOffset = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 RotMatrix3 Rot = rotMatrix3FromFloats ( params , 7 );
 const float Skew = params [ 16 ];
 const float Dist = params [ 17 ];
 const float Thick = params [ 18 ];
 const float SinZPower = params [ 19 ];
 const float ZPower = params [ 20 ];
 const float halfConst = 0.5f;
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;
 state_x = x;
 state_y = y;
 state_z = z;

}
 float rx = x * Rot . col0 . x + y * Rot . col1 . x + z * Rot . col2 . x;
 float ry = x * Rot . col0 . y + y * Rot . col1 . y + z * Rot . col2 . y;
 float rz = x * Rot . col0 . z + y * Rot . col1 . z + z * Rot . col2 . z;
 float initial_de = fabs ( rz - halfConst ) - halfConst;
 float power1 = - x87_log_power ( fabs ( rz - 1.0f ) , ZPower );
 float angle = x87_log_power ( fabs ( power1 ) , SinZPower ) * Skew;
 float cos_a = cos ( angle );
 float sin_a = sin ( angle );
 float A = rx * sin_a - ry * cos_a;
 float B = rx * cos_a + ry * sin_a + power1 * Dist;
 float dist = sqrt ( A * A + B * B );
 float absSkew = fabs ( Skew );
 if ( absSkew > 1.0f ) {
 dist /= sqrt ( absSkew );

}
 dist += power1 * Thick;
 float shape_de = initial_de >= dist ? initial_de : dist;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 params [ 63 ] = fabs ( OTrapOffset + params [ ITER_COUNT_SLOT ] * OTrapOnIts );
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 Seashell_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_SEASHELL > ( p , params , de );

}

}}
