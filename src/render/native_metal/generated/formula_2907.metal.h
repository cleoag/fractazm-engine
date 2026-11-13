// Generated from 3D/BT1Pine.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_2907 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BT1PINE = 2907;
 template < > FormulaVec3 execute_formula < FORMULA_BT1PINE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 E x = p . x , y = p . y , z = p . z;
 const E c = params [ 6 ] , s = params [ 7 ] , h = params [ 8 ];
 const int count = int ( params [ 5 ] ) & 0xff;
 for ( int i = 0;
 i < ( count ? count : 256 );
 ++ i ) {
 const E t = float ( ( x * c + ( - ( z * s ) ) ) * h );
 const E zz = float ( abs ( x * s + z * c ) );
 const E xx = float ( abs ( t + ( - ( y * h ) ) ) );
 y = float ( abs ( t + y * h ) );
 x = xx;
 z = zz;
 const E reverse = float ( ( x + y ) * h );
 const E yy = float ( ( - x + y ) * h );
 const E rx = float ( reverse * c + z * s );
 z = float ( - ( reverse * s ) + z * c );
 x = rx;
 y = yy;
 x = float ( x * params [ 0 ] - params [ 1 ] );
 y = float ( y * params [ 0 ] );
 z = float ( z * params [ 0 ] );

}
 const E xx = float ( x * x ) , yy = float ( y * y ) , zz = float ( z * z );
 const E t = float ( ( x + x ) / sqrt ( yy + zz ) );
 return FormulaVec3 ( float ( ( xx - yy ) - zz + E ( params [ 9 ] ) * params [ 2 ] ) , float ( ( ( t + t ) * z ) * y + E ( params [ 10 ] ) * params [ 4 ] ) , float ( t * ( yy - zz ) + E ( params [ 11 ] ) * params [ 3 ] ) );

}

}}
