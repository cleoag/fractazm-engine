// Generated from 3D/IQ-bulb_old.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10260 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_IQBULB_OLD = 10260;
 template < > FormulaVec3 execute_formula < FORMULA_IQBULB_OLD > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 const E x = p . x , y = p . y , z = p . z , radius = native_formula_length3 ( x , y , z );
 const E power = float ( extendedExp2 ( E ( params [ 0 ] ) * extendedLog2 ( abs ( radius ) ) , false ) );
 const E zPower = int ( params [ 3 ] ) == 0 ? power : E ( float ( extendedExp2 ( E ( params [ 1 ] ) * extendedLog2 ( abs ( E ( float ( radius ) ) ) ) , false ) ) );
 const E theta = extendedAtan2Wide ( sqrt ( x * x + z * z ) , radius + y );
 const auto polar = extendedSinCos ( ( theta + theta ) * params [ 1 ] );
 E phi = extendedAtan2 ( z , x );
 if ( int ( params [ 4 ] ) == 0 && E ( 0 ) > x ) {
 const E shift = x87Pi ( ) * params [ 5 ];
 if ( E ( 0 ) > z ) phi += shift;
 else phi -= shift;

}
 const auto azimuth = extendedSinCos ( phi * params [ 1 ] );
 const E ps = float ( polar . sin ) , pc = float ( polar . cos );
 const E azs = float ( azimuth . sin ) , azc = float ( azimuth . cos );
 return FormulaVec3 ( float ( ( power * ps ) * azs + params [ 6 ] ) , float ( power * pc + params [ 7 ] ) , float ( ( ( zPower * azc ) * ps ) * params [ 2 ] + params [ 8 ] ) );

}

}}
