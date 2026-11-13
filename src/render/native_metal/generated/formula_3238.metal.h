// Generated from 3D/Aex-TorusPow.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3238 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_AEX_TORUSPOW = 3238;
 template < > FormulaVec3 execute_formula < FORMULA_AEX_TORUSPOW > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 const E x = p . x , y = p . y , z = p . z;
 const E theta = extendedAtan2 ( y , x );
 const auto thetaPower = extendedSinCos ( theta * params [ 0 ] );
 const auto thetaBase = extendedSinCos ( E ( float ( theta ) ) );
 const auto polar = extendedSinCos ( extendedAtan2 ( sqrt ( x * x + y * y ) , z ) * params [ 0 ] );
 const E a = float ( int ( params [ 4 ] ) == 0 ? polar . cos + params [ 3 ] : polar . sin );
 const E b = float ( int ( params [ 4 ] ) == 0 ? polar . sin : polar . cos + params [ 3 ] );
 const E dx = x + ( - ( E ( float ( thetaBase . cos ) ) * params [ 1 ] ) );
 const E dy = y + ( - ( E ( float ( thetaBase . sin ) ) * params [ 1 ] ) );
 const E radius = ( dx * dx + dy * dy ) + z * z;
 const E power = extendedExp2 ( ( E ( params [ 0 ] ) * params [ 5 ] ) * extendedLog2 ( abs ( radius ) ) , false );
 const E storedPower = float ( power );
 return FormulaVec3 ( float ( ( power * a ) * E ( float ( thetaPower . cos ) ) + params [ 7 ] ) , float ( ( storedPower * a ) * E ( float ( thetaPower . sin ) ) + params [ 8 ] ) , float ( ( storedPower * b + params [ 9 ] ) * params [ 2 ] ) );

}

}}
