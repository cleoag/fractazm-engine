// Generated from 3D/Ruckerbulb.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5516 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_RUCKERBULB = 5516;
 template < > FormulaVec3 execute_formula < FORMULA_RUCKERBULB > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 using E = Extended80;
 const ExtendedScope scope;
 const E x ( p . x ) , y ( p . y ) , z ( p . z );
 const E radius = native_formula_length3 ( x , y , z );
 const E radiusPower = float ( extendedExp2 ( E ( params [ 0 ] ) * extendedLog2 ( abs ( radius ) ) , false ) );
 const E radiusZ = int ( params [ 3 ] ) == 0 ? radiusPower : E ( float ( extendedExp2 ( E ( params [ 1 ] ) * extendedLog2 ( abs ( E ( float ( radius ) ) ) ) , false ) ) );
 E polar = extendedAtan2 ( - x , z );
 if ( int ( params [ 4 ] ) == 0 && x < E ( 0 ) ) polar -= x87Pi ( ) * E ( params [ 5 ] );
 const auto za = extendedSinCos ( polar * E ( params [ 1 ] ) );
 const auto xy = extendedSinCos ( extendedAtan2 ( y , x ) * E ( params [ 0 ] ) );
 const E cosZ = float ( za . cos ) , sinZ = float ( za . sin );
 const E cosXY = float ( xy . cos ) , sinXY = float ( xy . sin );
 return FormulaVec3 ( float ( - ( radiusPower * cosZ * cosXY ) + E ( params [ 6 ] ) ) , float ( - ( radiusPower * cosZ * sinXY ) + E ( params [ 7 ] ) ) , float ( radiusZ * sinZ * E ( params [ 2 ] ) + E ( params [ 8 ] ) ) );

}

}}
