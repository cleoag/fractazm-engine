// Generated from 3D/TrifoxComplexAngles.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4225 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TRIFOXCOMPLEXANGLES = 4225;
 template < > FormulaVec3 execute_formula < FORMULA_TRIFOXCOMPLEXANGLES > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 if ( int ( params [ 0 ] ) != 0 && int ( params [ 4 ] ) != 0 ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 E radius ( 1 ) , base = sqrt ( E ( params [ FORMULA_ROUT_SLOT ] ) );
 uint exponent = static_cast < uint > ( int ( params [ 0 ] ) );
 while ( exponent ) {
 if ( exponent & 1 ) radius *= base;
 exponent >>= 1;
 if ( exponent ) base *= base;

}
 const E x ( p . x ) , y ( p . y ) , z ( p . z ) , angleScale ( static_cast < float > ( params [ 1 ] ) );
 E a = numeric :: extendedAtan2 ( y , x ) * angleScale;
 E b = numeric :: extendedAtan2 ( z , sqrt ( x * x + y * y ) ) * angleScale;
 if ( int ( params [ 3 ] ) != 0 ) {
 const E temporary = a;
 a = b;
 b = temporary;

}
 if ( int ( params [ 5 ] ) != 0 ) {
 const E inverse = E ( 1 ) / ( a * a + b * b + E ( static_cast < float > ( params [ 6 ] ) ) );
 a *= inverse;
 b *= inverse;

}
 E real ( 1 ) , imaginary ( 0 );
 exponent = static_cast < uint > ( int ( params [ 4 ] ) );
 while ( exponent ) {
 if ( exponent & 1 ) {
 const E nextReal = real * a - imaginary * b;
 imaginary = imaginary * a + real * b;
 real = nextReal;

}
 exponent >>= 1;
 if ( exponent ) {
 const E product = b * a;
 a = a * a - b * b;
 b = product + product;

}

}
 const E outputScale ( static_cast < float > ( params [ 2 ] ) );
 real *= outputScale;
 imaginary *= outputScale;
 if ( abs ( real ) >= E ( 0x1p63f ) || abs ( imaginary ) >= E ( 0x1p63f ) ) return FormulaVec3 ( static_cast < float > ( numeric :: extendedNaN ( ) ) );
 const auto first = numeric :: extendedSinCos ( real ) , second = numeric :: extendedSinCos ( imaginary );
 return FormulaVec3 ( static_cast < float > ( second . cos * first . cos * radius + E ( params [ 8 ] ) ) , static_cast < float > ( ( first . cos * second . sin ) * radius + E ( params [ 9 ] ) ) , static_cast < float > ( first . sin * radius * E ( static_cast < float > ( params [ 7 ] ) ) + E ( params [ 10 ] ) ) );

}
 const int power_R = int ( params [ 0 ] );
 const float Angle_scale = params [ 1 ];
 const float Angle_scale2 = params [ 2 ];
 const int Flip_Theta_Phi = int ( params [ 3 ] );
 const int power_Angle = int ( params [ 4 ] );
 const int Reciprocal_R_angle = int ( params [ 5 ] );
 const float Reciprocal_fixing = params [ 6 ];
 const float Z_multiplier = params [ 7 ];
 const float Cx = params [ 8 ];
 const float Cy = params [ 9 ];
 const float Cz = params [ 10 ];
 float x = p . x , y = p . y , z = p . z;
 float r_sq = x * x + y * y + z * z;
 float r = sqrt ( r_sq );
 float R = pow ( r , float ( power_R ) );
 float phi = atan2 ( z , sqrt ( x * x + y * y ) );
 float theta = atan2 ( y , x );
 phi *= Angle_scale;
 theta *= Angle_scale;
 float angle_a = ( Flip_Theta_Phi != 0 ) ? phi : theta;
 float angle_b = ( Flip_Theta_Phi != 0 ) ? theta : phi;
 if ( Reciprocal_R_angle != 0 ) {
 float inv = 1.0f / ( angle_a * angle_a + angle_b * angle_b + Reciprocal_fixing );
 angle_a *= inv;
 angle_b *= inv;

}
 float m = sqrt ( angle_a * angle_a + angle_b * angle_b );
 float arg = atan2 ( angle_b , angle_a );
 float n = float ( power_Angle );
 float mn = pow ( m , n );
 float final_re = mn * cos ( n * arg );
 float final_im = mn * sin ( n * arg );
 float a = final_re * Angle_scale2;
 float b = final_im * Angle_scale2;
 float cos_a = cos ( a );
 float sin_a = sin ( a );
 float cos_b = cos ( b );
 float sin_b = sin ( b );
 float x_out = R * cos_a * cos_b + Cx;
 float y_out = R * cos_a * sin_b + Cy;
 float z_out = R * sin_a * Z_multiplier + Cz;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
