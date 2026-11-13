// Generated from 3Da/MengerKoch.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10033 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MENGERKOCH = 10033;
 template < > FormulaVec3 execute_formula < FORMULA_MENGERKOCH > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 RotMatrix3 R1 = rotMatrix3FromFloats ( params , 4 );
 RotMatrix3 R2 = rotMatrix3FromFloats ( params , 13 );
 const float Edge1 = params [ 22 ];
 const float Edge2 = params [ 23 ];
 float ax = abs ( p . x );
 float ay = abs ( p . y );
 float az = abs ( p . z );
 FormulaVec3 r1v = R1 * FormulaVec3 ( ax , ay , az );
 float rx = r1v . x;
 float ry = r1v . y;
 float rz = r1v . z;
 float max_val = rx;
 float mid_val = ry;
 float min_val = rz;
 if ( ! ( max_val >= mid_val ) ) {
 float t = max_val;
 max_val = mid_val;
 mid_val = t;

}
 if ( ! ( max_val >= min_val ) ) {
 float t = max_val;
 max_val = min_val;
 min_val = t;

}
 if ( ! ( mid_val >= min_val ) ) {
 float t = mid_val;
 mid_val = min_val;
 min_val = t;

}
 float edge_mid = abs ( mid_val - Edge2 );
 float edge_max = abs ( Edge1 - max_val );
 FormulaVec3 r2v = R2 * FormulaVec3 ( edge_max , edge_mid , min_val );
 float r2x = r2v . x;
 float r2y = r2v . y;
 float r2z = r2v . z;
 de = de * Scale;
 float xs = r2x * Scale;
 float ys = r2y * Scale;
 float zs = r2z * Scale;
 float k = Scale - 1.0f;
 float x_out = xs - k * CScaleX;
 float y_out = ys - k * CScaleY;
 float kCZ = k * CScaleZ;
 float threshold = 0.5f * kCZ;
 float z_out;
 if ( ! ( threshold > zs ) ) {
 z_out = zs - kCZ;

}
 else {
 z_out = zs;

}
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
