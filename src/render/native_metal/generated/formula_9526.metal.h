// Generated from 3Da/Menger3.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9526 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MENGER3 = 9526;
 template < > FormulaVec3 execute_formula < FORMULA_MENGER3 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 RotMatrix3 RM1 = rotMatrix3FromFloats ( params , 4 );
 RotMatrix3 RM2 = rotMatrix3FromFloats ( params , 13 );
 float R1_00 = RM1 . m [ 0 ] [ 0 ];
 float R1_01 = RM1 . m [ 0 ] [ 1 ];
 float R1_02 = RM1 . m [ 0 ] [ 2 ];
 float R1_10 = RM1 . m [ 1 ] [ 0 ];
 float R1_11 = RM1 . m [ 1 ] [ 1 ];
 float R1_12 = RM1 . m [ 1 ] [ 2 ];
 float R1_20 = RM1 . m [ 2 ] [ 0 ];
 float R1_21 = RM1 . m [ 2 ] [ 1 ];
 float R1_22 = RM1 . m [ 2 ] [ 2 ];
 float R2_00 = RM2 . m [ 0 ] [ 0 ];
 float R2_01 = RM2 . m [ 0 ] [ 1 ];
 float R2_02 = RM2 . m [ 0 ] [ 2 ];
 float R2_10 = RM2 . m [ 1 ] [ 0 ];
 float R2_11 = RM2 . m [ 1 ] [ 1 ];
 float R2_12 = RM2 . m [ 1 ] [ 2 ];
 float R2_20 = RM2 . m [ 2 ] [ 0 ];
 float R2_21 = RM2 . m [ 2 ] [ 1 ];
 float R2_22 = RM2 . m [ 2 ] [ 2 ];
 float ax = abs ( p . x );
 float ay = abs ( p . y );
 float az = abs ( p . z );
 float rx = ax * R1_00 + ay * R1_01 + az * R1_02;
 float ry = ax * R1_10 + ay * R1_11 + az * R1_12;
 float rz = ax * R1_20 + ay * R1_21 + az * R1_22;
 float max_val = rx;
 float mid_val = ry;
 float min_val = rz;
 if ( max_val < mid_val ) {
 float t = max_val;
 max_val = mid_val;
 mid_val = t;

}
 if ( max_val < min_val ) {
 float t = max_val;
 max_val = min_val;
 min_val = t;

}
 if ( mid_val < min_val ) {
 float t = mid_val;
 mid_val = min_val;
 min_val = t;

}
 float r0_R2 = max_val * params [ 13 ] + mid_val * params [ 14 ] + min_val * params [ 15 ];
 float r1_R2 = max_val * params [ 16 ] + mid_val * params [ 17 ] + min_val * params [ 18 ];
 float r2_R2 = max_val * params [ 19 ] + mid_val * params [ 20 ] + min_val * params [ 21 ];
 de = de * Scale;
 float k = Scale - 1.0f;
 float x_out = r0_R2 * Scale - k * CScaleX;
 float y_out = r1_R2 * Scale - k * CScaleY;
 float z_scaled = r2_R2 * Scale;
 float threshold = 0.5f * k * CScaleZ;
 float z_out;
 if ( z_scaled > threshold ) {
 z_out = z_scaled - k * CScaleZ;

}
 else {
 z_out = z_scaled;

}
 FormulaVec3 result;
 result . x = x_out;
 result . y = y_out;
 result . z = z_out;
 return result;

}
 FormulaVec3 Menger3_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 float a1 = params [ 4 ] * 3.14159265358979f / 180.0f;
 float a2 = params [ 5 ] * 3.14159265358979f / 180.0f;
 float a3 = params [ 6 ] * 3.14159265358979f / 180.0f;
 float b1 = params [ 7 ] * 3.14159265358979f / 180.0f;
 float b2 = params [ 8 ] * 3.14159265358979f / 180.0f;
 float b3 = params [ 9 ] * 3.14159265358979f / 180.0f;
 float s1 = sin ( a1 );
 float c1 = cos ( a1 );
 float s2 = sin ( a2 );
 float c2 = cos ( a2 );
 float s3 = sin ( a3 );
 float c3 = cos ( a3 );
 float R1_00 = c2 * c3;
 float R1_01 = - c2 * s3;
 float R1_02 = s2;
 float R1_10 = s1 * s2 * c3 + c1 * s3;
 float R1_11 = c1 * c3 - s1 * s2 * s3;
 float R1_12 = - s1 * c2;
 float R1_20 = s1 * s3 - c1 * s2 * c3;
 float R1_21 = c1 * s2 * s3 + s1 * c3;
 float R1_22 = c1 * c2;
 float t1 = sin ( b1 );
 float u1 = cos ( b1 );
 float t2 = sin ( b2 );
 float u2 = cos ( b2 );
 float t3 = sin ( b3 );
 float u3 = cos ( b3 );
 float R2_00 = u2 * u3;
 float R2_01 = - u2 * t3;
 float R2_02 = t2;
 float R2_10 = t1 * t2 * u3 + u1 * t3;
 float R2_11 = u1 * u3 - t1 * t2 * t3;
 float R2_12 = - t1 * u2;
 float R2_20 = t1 * t3 - u1 * t2 * u3;
 float R2_21 = u1 * t2 * t3 + t1 * u3;
 float R2_22 = u1 * u2;
 float ax = abs ( p . x );
 float ay = abs ( p . y );
 float az = abs ( p . z );
 float rx = ax * R1_00 + ay * R1_01 + az * R1_02;
 float ry = ax * R1_10 + ay * R1_11 + az * R1_12;
 float rz = ax * R1_20 + ay * R1_21 + az * R1_22;
 float max_val = rx;
 float mid_val = ry;
 float min_val = rz;
 if ( max_val < mid_val ) {
 float t = max_val;
 max_val = mid_val;
 mid_val = t;

}
 if ( max_val < min_val ) {
 float t = max_val;
 max_val = min_val;
 min_val = t;

}
 if ( mid_val < min_val ) {
 float t = mid_val;
 mid_val = min_val;
 min_val = t;

}
 float r0_R2 = max_val * R2_20 + mid_val * R2_21 + min_val * R2_22;
 float r1_R2 = max_val * R2_00 + mid_val * R2_01 + min_val * R2_02;
 float r2_R2 = max_val * R2_10 + mid_val * R2_11 + min_val * R2_12;
 de = de * Scale;
 float k = Scale - 1.0f;
 float threshold = 0.5f * k * CScaleZ;
 FormulaVec3 result;
 result . x = r1_R2 * Scale - k * CScaleX;
 result . y = r0_R2 * Scale - k * CScaleY;
 float z_scaled = r2_R2 * Scale;
 result . z = ( z_scaled > threshold ) ? z_scaled - k * CScaleZ : z_scaled;
 return result;

}

}}
