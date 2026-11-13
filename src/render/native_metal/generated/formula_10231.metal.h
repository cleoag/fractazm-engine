// Generated from dIFS/boardIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10231 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BOARDIFS = 10231;
 template < > FormulaVec3 execute_formula < FORMULA_BOARDIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Bump = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Checker_scale = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const int Variation = int ( params [ 16 ] );
 const float OTrapClr1 = params [ 17 ];
 const float OTrapClr2 = params [ 18 ];
 const float OTrapOnIts = params [ 19 ];
 const float const_scale = params [ 20 ];
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
 float m00 = params [ 7 ] , m01 = params [ 8 ] , m02 = params [ 9 ];
 float m10 = params [ 10 ] , m11 = params [ 11 ] , m12 = params [ 12 ];
 float m20 = params [ 13 ] , m21 = params [ 14 ] , m22 = params [ 15 ];
 float rx = x * m00 + y * m01 + z * m02;
 float ry = x * m10 + y * m11 + z * m12;
 float rz = x * m20 + y * m21 + z * m22;
 x = rx;
 y = ry;
 z = rz;
 float u = x , v = y;
 if ( Variation == 1 ) {
 float r2 = x * x + y * y;
 float inv_r2 = 1.0f / r2;
 u = x * inv_r2;
 v = y * inv_r2;

}
 else if ( Variation == 2 ) {
 float denom = x * x + ( y + 1.0f ) * ( y + 1.0f );
 float inv_denom = 1.0f / denom;
 float r2 = x * x + y * y;
 float new_v = ( r2 - 1.0f ) * inv_denom;
 float new_u = - 2.0f * x * inv_denom;
 v = new_v;
 u = new_u;

}
 else if ( Variation == 3 ) {
 float r2 = x * x + y * y;
 float factor = 1.0f / log2 ( r2 );
 u = x * factor;
 v = y * factor;

}
 else if ( Variation == 4 ) {
 float r2 = x * x + y * y;
 float r = sqrt ( r2 );
 float theta = atan2 ( y , x );
 u = r;
 v = theta;

}
 float pattern = sin ( u * Checker_scale ) * sin ( v * Checker_scale );
 params [ MAX_FORMULA_PARAMS - 1 ] = params [ ITER_COUNT_SLOT ] * OTrapOnIts + ( ! ( pattern >= 0.0f ) ? OTrapClr1 : OTrapClr2 );
 float shape_de_val = pattern * const_scale / Checker_scale;
 float z_de = - z + Bump;
 float result = min ( z_de , shape_de_val );
 shape_de_val = max ( - z , result );
 if ( Apply_scale_add == 0 ) {
 shape_de_val /= Scale;

}
 de = shape_de_val;
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 boardIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_BOARDIFS > ( p , params , de );

}

}}
