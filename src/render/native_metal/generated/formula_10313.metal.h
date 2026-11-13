// Generated from dIFSa/totoricalIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10313 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TOTORICALIFS = 10313;
 template < > FormulaVec3 execute_formula < FORMULA_TOTORICALIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float R = params [ 0 ];
 const float mulA1 = params [ 1 ];
 const float subA1 = params [ 2 ];
 const float mulA2 = params [ 3 ];
 const float subA2 = params [ 4 ];
 const float mulRadius = params [ 5 ];
 const float subRadius = params [ 6 ];
 const int FixATo1 = int ( params [ 7 ] );
 const float FoldAngles = params [ 8 ];
 const float c_halfpi = 1.5707963267948966f;
 const float c_invpi = 0.318309886183790671f;
 float x = p . x;
 float y = p . y;
 float z = p . z;
 float rxy = sqrt ( z * z + y * y );
 float d = rxy - R;
 float r2 = sqrt ( d * d + x * x );
 float angle1 = atan2 ( d , - x );
 float angle1_raw = atan2 ( d , - x );
 float angle1_proc = fabs ( angle1_raw - c_halfpi );
 float angle2_raw = atan2 ( y , z );
 float angle2_proc = fabs ( angle2_raw );
 if ( FixATo1 != 0 ) {
 angle2_proc *= c_invpi;
 angle1_proc *= c_invpi;

}
 float a2_scaled = angle2_proc * mulA1 - subA1;
 if ( FoldAngles != 0.0f ) {
 float period = 4.0f * FoldAngles;
 float reduced = a2_scaled - round ( a2_scaled / period ) * period;
 reduced = - reduced;
 a2_scaled = reduced + fabs ( FoldAngles - reduced ) - fabs ( reduced + FoldAngles );

}
 float a1_scaled = angle1_proc * mulA2 - subA2;
 if ( FoldAngles != 0.0f ) {
 float period = 4.0f * FoldAngles;
 float reduced = a1_scaled - round ( a1_scaled / period ) * period;
 reduced = - reduced;
 a1_scaled = reduced + fabs ( FoldAngles - reduced ) - fabs ( reduced + FoldAngles );

}
 float r_out = r2 * mulRadius - subRadius;
 return FormulaVec3 ( a2_scaled , a1_scaled , r_out );

}
 FormulaVec3 totoricalIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_TOTORICALIFS > ( p , params , de );

}

}}
