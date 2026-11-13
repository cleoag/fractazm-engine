// Generated from dIFS/trifoliumIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10304 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TRIFOLIUMIFS = 10304;
 template < > FormulaVec3 execute_formula < FORMULA_TRIFOLIUMIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Petal_fix_1 = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Petal_fix_2 = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const RotMatrix3 Rot = rotMatrix3FromFloats ( params , 7 );
 const float OTrapOnDist = params [ 16 ];
 const float OTrapOnIts = params [ 17 ];
 const float OTrapOffset = params [ 18 ];
 const float const_neg3 = - 3.0f;
 const float const_half = 0.5f;
 const float const_clip = 5.0f;
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;

}
 float rx = x * Rot . col0 . x + y * Rot . col1 . x + z * Rot . col2 . x;
 float ry = x * Rot . col0 . y + y * Rot . col1 . y + z * Rot . col2 . y;
 float rz = x * Rot . col0 . z + y * Rot . col1 . z + z * Rot . col2 . z;
 float pos_accum = 0.0f;
 float neg_accum = 0.0f;
 float r2_xy = rx * rx + ry * ry;
 float lhs = r2_xy * r2_xy;
 float z3_term = fabs ( Petal_fix_1 * rz * rz * rz );
 pos_accum = lhs + z3_term;
 float rhs = Petal_fix_2 * rx * ( rx * rx + const_neg3 * ry * ry );
 if ( rhs > 0.0f ) {
 pos_accum += rhs;

}
 else {
 neg_accum -= rhs;

}
 float de_raw = sqrt ( sqrt ( pos_accum ) ) - sqrt ( sqrt ( neg_accum ) );
 float shape_de = de_raw * const_half;
 float z_clip = fabs ( rz ) - const_clip;
 if ( z_clip > shape_de ) {
 shape_de = z_clip;

}
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 params [ 63 ] = fabs ( native_formula_length3 ( rx , ry , rz ) * OTrapOnDist + params [ ITER_COUNT_SLOT ] * OTrapOnIts + OTrapOffset );
 return Apply_scale_add ? FormulaVec3 ( x , y , z ) : p;

}
 FormulaVec3 trifoliumIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_TRIFOLIUMIFS > ( p , params , de );

}

}}
