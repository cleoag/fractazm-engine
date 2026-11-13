// Generated from Ads/_GenCWarp.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10022 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__GENCWARP = 10022;
 static float2 select_angle_args ( float Cx , float Cy , float Cz , int mode ) {
 switch ( mode ) {
 case 0 : return float2 ( Cx , Cy );
 case 1 : return float2 ( Cx , Cz );
 case 2 : return float2 ( Cy , Cx );
 case 3 : return float2 ( Cy , Cz );
 case 4 : return float2 ( Cz , Cx );
 case 5 : return float2 ( Cz , Cy );
 case 6 : return float2 ( Cx , sqrt ( Cy * Cy + Cz * Cz ) );
 case 7 : return float2 ( Cy , sqrt ( Cx * Cx + Cz * Cz ) );
 case 8 : return float2 ( Cz , sqrt ( Cx * Cx + Cy * Cy ) );
 case 9 : return float2 ( sqrt ( Cy * Cy + Cz * Cz ) , Cx );
 case 10 : return float2 ( sqrt ( Cx * Cx + Cz * Cz ) , Cy );
 case 11 : return float2 ( sqrt ( Cx * Cx + Cy * Cy ) , Cz );
 case 12 : return float2 ( Cx , max ( fabs ( Cz ) , fabs ( Cy ) ) );
 case 13 : return float2 ( Cy , max ( fabs ( Cz ) , fabs ( Cx ) ) );
 case 14 : return float2 ( Cz , max ( fabs ( Cx ) , fabs ( Cy ) ) );
 case 15 : return float2 ( max ( fabs ( Cz ) , fabs ( Cy ) ) , Cx );
 case 16 : return float2 ( max ( fabs ( Cz ) , fabs ( Cx ) ) , Cy );
 case 17 : return float2 ( max ( fabs ( Cx ) , fabs ( Cy ) ) , Cz );
 case 18 : return float2 ( Cx , fabs ( Cy ) );
 case 19 : return float2 ( Cy , fabs ( Cx ) );
 case 20 : return float2 ( Cz , fabs ( Cy ) );
 case 21 : return float2 ( fabs ( Cy ) , Cx );
 case 22 : return float2 ( fabs ( Cx ) , Cy );
 case 23 : return float2 ( fabs ( Cx ) , Cz );
 default : return float2 ( Cx , Cy );

}

}
 template < > FormulaVec3 execute_formula < FORMULA__GENCWARP > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float R00 = params [ 0 ];
 const float R01 = params [ 1 ];
 const float R02 = params [ 2 ];
 const float R10 = params [ 3 ];
 const float R11 = params [ 4 ];
 const float R12 = params [ 5 ];
 const float R20 = params [ 6 ];
 const float R21 = params [ 7 ];
 const float R22 = params [ 8 ];
 const float theta_pow = params [ 9 ];
 const float phi_pow = params [ 10 ];
 const int th_mode = ( int ) params [ 11 ];
 const int ph_mode = ( int ) params [ 12 ];
 const int recipe = ( int ) params [ 13 ];
 const float strength = params [ 14 ];
 const float xyz_strength = params [ 15 ];
 float Cx = params [ 17 ];
 float Cy = params [ 18 ];
 float Cz = params [ 19 ];
 float orig_Cx = Cx;
 float orig_Cy = Cy;
 float orig_Cz = Cz;
 float new_Cx = R00 * Cx + R01 * Cy + R02 * Cz;
 float new_Cy = R10 * Cx + R11 * Cy + R12 * Cz;
 float new_Cz = R20 * Cx + R21 * Cy + R22 * Cz;
 Cx = new_Cx;
 Cy = new_Cy;
 Cz = new_Cz;
 float2 theta_args = select_angle_args ( Cx , Cy , Cz , th_mode );
 float theta = atan2 ( theta_args . x , theta_args . y );
 theta *= theta_pow;
 float cos_theta = cos ( theta );
 float sin_theta = sin ( theta );
 float2 phi_args = select_angle_args ( Cx , Cy , Cz , ph_mode );
 float phi = atan2 ( phi_args . x , phi_args . y );
 phi *= phi_pow;
 float cos_phi = cos ( phi );
 float sin_phi = sin ( phi );
 float r = native_formula_length3 ( Cx , Cy , Cz );
 float warped_Cx , warped_Cy , warped_Cz;
 if ( recipe == 0 ) {
 warped_Cx = cos_theta * cos_phi * r;
 warped_Cy = sin_theta * cos_phi * r;
 warped_Cz = sin_phi * r;

}
 else if ( recipe == 1 ) {
 warped_Cx = cos_theta * sin_phi * r;
 warped_Cy = sin_theta * sin_phi * r;
 warped_Cz = cos_phi * r;

}
 else {
 warped_Cx = cos_theta * cos_phi * r;
 warped_Cy = sin_theta * r;
 warped_Cz = cos_theta * sin_phi * r;

}
 float inv_str = 1.0f - strength;
 Cx = inv_str * orig_Cx + strength * warped_Cx;
 Cy = inv_str * orig_Cy + strength * warped_Cy;
 Cz = inv_str * orig_Cz + strength * warped_Cz;
 float inv_xyz = 1.0f - xyz_strength;
 Cx = xyz_strength * p . x + inv_xyz * Cx;
 Cy = xyz_strength * p . y + inv_xyz * Cy;
 Cz = xyz_strength * p . z + inv_xyz * Cz;
 params [ WRITE_J1_SLOT ] = Cx;
 params [ WRITE_J2_SLOT ] = Cy;
 params [ WRITE_J3_SLOT ] = Cz;
 return p;

}
 FormulaVec3 _GenCWarp_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA__GENCWARP > ( p , params , de );

}

}}
