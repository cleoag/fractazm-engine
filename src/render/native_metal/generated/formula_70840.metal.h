// Generated from dIFS/wavespIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_70840 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_WAVESPIFS = 70840;
 template < > FormulaVec3 execute_formula < FORMULA_WAVESPIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float N_of_spirals = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Amplitude = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float R_exponent = params [ 7 ];
 const int Pow_R_or_Log_R = int ( params [ 8 ] );
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;

}
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 const E square = abs ( E ( x ) * x + E ( y ) * y ) , infinity = native_std :: numeric_limits < float > :: infinity ( );
 const E logarithm = square == E ( 0 ) ? - infinity : square == infinity ? infinity : extendedLog2 ( square );
 E radial = E ( R_exponent ) * logarithm;
 if ( Pow_R_or_Log_R == 0 ) radial = extendedExp2 ( radial , false );
 const E angle = radial + extendedAtan2 ( E ( y ) , E ( x ) ) * N_of_spirals;
 const auto exponent = numeric :: signExponent ( angle ) & 0x7fff;
 const E wave = exponent >= 16383 + 63 && exponent < 0x7fff ? angle : extendedSinCos ( angle ) . sin;
 E distance = - ( abs ( wave ) * Amplitude + E ( z ) );
 if ( ! Apply_scale_add ) distance /= Scale;
 de = float ( distance );
 return Apply_scale_add ? FormulaVec3 ( x , y , z ) : p;

}
 FormulaVec3 wavespIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_WAVESPIFS > ( p , params , de );

}

}}
