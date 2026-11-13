// Generated from 3Da/ABoxPlatinum.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10047 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXPLATINUM = 10047;
 template < > FormulaVec3 execute_formula < FORMULA_ABOXPLATINUM > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float ScaleOverMinR2 = params [ 1 ];
 const float minR2 = params [ 2 ];
 const float Fold = params [ 3 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 4 );
 const float InvXC = params [ 13 ];
 const float InvYC = params [ 14 ];
 const float InvZC = params [ 15 ];
 const float InvRadFactor = params [ 16 ];
 const float sinFoldX_a = params [ 19 ];
 const float cosFoldX_a = params [ 20 ];
 const float sinFoldX_b = params [ 17 ];
 const float cosFoldX_b = params [ 18 ];
 const float sinFoldY_a = params [ 21 ];
 const float cosFoldY_a = params [ 22 ];
 const float sinFoldY_b = params [ 23 ];
 const float cosFoldY_b = params [ 24 ];
 const int absSwitch = ( int ) params [ 25 ];
 float x = p . x;
 float y = p . y;
 float z = p . z;
 struct NativeClosure0 {
 decltype ( Fold ) Fold;
 auto operator ( ) ( float v ) const {
 return fabs ( v + Fold ) - fabs ( v - Fold ) - v;

}

}
;
 auto tglad_fold = NativeClosure0 {
 Fold
}
;
 float x1 = x * cosFoldX_a - z * sinFoldX_a;
 float z1 = x * sinFoldX_a + z * cosFoldX_a;
 float x2 = x1 * cosFoldX_b - y * sinFoldX_b;
 float y2 = x1 * sinFoldX_b + y * cosFoldX_b;
 float fold_x_rot = tglad_fold ( x2 );
 float x_after_b = fold_x_rot * cosFoldX_b - y2 * sinFoldX_b;
 float y_after_x = fold_x_rot * sinFoldX_b + y2 * cosFoldX_b;
 float x_after_x = x_after_b * cosFoldX_a - z1 * sinFoldX_a;
 float z_after_x = x_after_b * sinFoldX_a + z1 * cosFoldX_a;
 float y1 = y_after_x * cosFoldY_b - z_after_x * sinFoldY_b;
 float z1y = y_after_x * sinFoldY_b + z_after_x * cosFoldY_b;
 float y2r = y1 * cosFoldY_a - x_after_x * sinFoldY_a;
 float x2r = x_after_x * cosFoldY_a + y1 * sinFoldY_a;
 float fold_y_rot = tglad_fold ( y2r );
 float y_after_a = fold_y_rot * cosFoldY_a - x2r * sinFoldY_a;
 float fx_final = x2r * cosFoldY_a + fold_y_rot * sinFoldY_a;
 float fy_final = y_after_a * cosFoldY_b - z1y * sinFoldY_b;
 float fz_pre = y_after_a * sinFoldY_b + z1y * cosFoldY_b;
 float fz_final = tglad_fold ( fz_pre );
 if ( absSwitch & 4 ) fz_final = fabs ( fz_final );
 if ( absSwitch & 2 ) fy_final = fabs ( fy_final );
 if ( absSwitch & 1 ) fx_final = fabs ( fx_final );
 fx_final += InvXC;
 fy_final += InvYC;
 fz_final += InvZC;
 float rr = ( fx_final * fx_final + fy_final * fy_final + fz_final * fz_final ) * InvRadFactor;
 float m;
 if ( rr < minR2 ) {
 m = ScaleOverMinR2;

}
 else if ( rr >= 1.0f ) {
 m = Scale;

}
 else {
 m = Scale / rr;

}
 de *= m;
 fx_final = fx_final * m - InvXC * Scale;
 fy_final = fy_final * m - InvYC * Scale;
 fz_final = fz_final * m - InvZC * Scale;
 return R * FormulaVec3 ( fx_final , fy_final , fz_final ) + FormulaVec3 ( params [ 26 ] , params [ 27 ] , params [ 28 ] );

}

}}
