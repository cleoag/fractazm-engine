// Generated from dIFS/NewMenger2IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10308 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_NEWMENGER2IFS = 10308;
 template < > FormulaVec3 execute_formula < FORMULA_NEWMENGER2IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float MengerScale = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float MengerDivisor = params [ 5 ];
 const int Apply_sa_AddIts = int ( params [ 6 ] );
 const float XOff = params [ 7 ];
 const float YOff = params [ 8 ];
 const float ZOff = params [ 9 ];
 const int MaxIter_raw = int ( params [ 10 ] );
 const float MengerReCenter = params [ 20 ];
 const float half_val = params [ 21 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 float state_x = p . x;
 float state_y = p . y;
 float state_z = p . z;
 if ( Apply_sa_AddIts != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;
 state_x = x;
 state_y = y;
 state_z = z;

}
 int MaxIter = MaxIter_raw & 0x1F;
 if ( MaxIter == 0 ) MaxIter = 1;
 x = x * half_val + half_val;
 y = y * half_val + half_val;
 z = z * half_val + half_val;
 const float x0 = x;
 const float y0 = y;
 const float z0 = z;
 float accumulated_scale = 1.0f;
 float best_de = fabs ( x0 - half_val ) - half_val;
 {
 float fx0 = fabs ( x0 - half_val ) - half_val;
 float fy0 = fabs ( y0 - half_val ) - half_val;
 float fz0 = fabs ( z0 - half_val ) - half_val;
 if ( fx0 > best_de ) best_de = fx0;
 if ( fy0 > best_de ) best_de = fy0;
 if ( fz0 > best_de ) best_de = fz0;

}
 for ( int i = 0;
 i < MaxIter;
 i ++ ) {
 float mz = x87_fprem ( z0 * MengerDivisor * accumulated_scale , MengerDivisor ) - MengerReCenter;
 float my = x87_fprem ( y0 * MengerDivisor * accumulated_scale , MengerDivisor ) - MengerReCenter;
 float mx = x87_fprem ( x0 * MengerDivisor * accumulated_scale , MengerDivisor ) - MengerReCenter;
 float rx_rot = mx * params [ 17 ] + my * params [ 18 ] + mz * params [ 19 ];
 float ry_rot = mx * params [ 11 ] + my * params [ 12 ] + mz * params [ 13 ];
 float rz_rot = mx * params [ 14 ] + my * params [ 15 ] + mz * params [ 16 ];
 float d1 = sqrt ( rx_rot * rx_rot + ry_rot * ry_rot ) - ZOff * rz_rot * rz_rot - fabs ( rz_rot ) * YOff - XOff;
 float d2 = sqrt ( rx_rot * rx_rot + rz_rot * rz_rot ) - ZOff * ry_rot * ry_rot - fabs ( ry_rot ) * YOff - XOff;
 float d3 = sqrt ( rz_rot * rz_rot + ry_rot * ry_rot ) - ZOff * rx_rot * rx_rot - fabs ( rx_rot ) * YOff - XOff;
 float min_d = d1;
 if ( d2 < min_d ) min_d = d2;
 if ( d3 < min_d ) min_d = d3;
 float scaled_d = - ( min_d / ( 1.0f + fabs ( ZOff ) + fabs ( YOff ) ) ) / accumulated_scale;
 if ( ! ( best_de > scaled_d ) ) {
 best_de = scaled_d;

}
 accumulated_scale *= MengerScale;

}
 float shape_de = best_de;
 if ( Apply_sa_AddIts == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 NewMenger2IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_NEWMENGER2IFS > ( p , params , de );

}

}}
