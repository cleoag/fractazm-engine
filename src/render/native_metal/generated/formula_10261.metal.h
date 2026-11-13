// Generated from Ads/_cylripple.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10261 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_CYLRIPPLE = 10261;
 template < > FormulaVec3 execute_formula < FORMULA_CYLRIPPLE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 auto R = rotMatrix3FromFloats ( params , 0 );
 const float long_freq = params [ 9 ];
 const float long_ampl = params [ 10 ];
 const float trans_freq = params [ 11 ];
 const float trans_ampl = params [ 12 ];
 const int byC_L = ( int ) params [ 13 ];
 const int byC_T = ( int ) params [ 14 ];
 const float Cx = params [ 15 ];
 const float Cy = params [ 16 ];
 float x = p . x , y = p . y , z = p . z;
 FormulaVec3 rotated = R * FormulaVec3 {
 x , y , z
}
;
 float rx = rotated . x;
 float ry = rotated . y;
 float rz = rotated . z;
 x = rx;
 y = ry;
 z = rz;
 float rho , theta;
 float u;
 if ( byC_L == 1 ) {
 rho = sqrt ( x * x + y * y );

}
 else {
 rho = sqrt ( Cx * Cx + Cy * Cy );

}
 u = sin ( rho * long_freq ) * long_ampl;
 float v;
 if ( byC_T == 1 ) {
 theta = atan2 ( y , x );

}
 else {
 theta = atan2 ( Cy , Cx );

}
 v = sin ( theta * trans_freq ) * trans_ampl;
 float new_rho = ( 1.0f + u + v ) * rho;
 x = new_rho * cos ( theta );
 y = new_rho * sin ( theta );
 float ox = x * R . col0 . x + y * R . col0 . y + z * R . col0 . z;
 float oy = x * R . col1 . x + y * R . col1 . y + z * R . col1 . z;
 float oz = x * R . col2 . x + y * R . col2 . y + z * R . col2 . z;
 return FormulaVec3 ( ox , oy , oz );

}

}}
