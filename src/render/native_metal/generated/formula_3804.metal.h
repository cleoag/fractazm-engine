// Generated from Ads/_invbisp.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3804 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__INVBISP = 3804;
 template < > FormulaVec3 execute_formula < FORMULA__INVBISP > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 using namespace numeric;
 ExtendedScope scope;
 const E x = p . x , y = p . y , z = p . z , a = params [ 0 ];
 const E radius = x * x + ( y * y + z * z ) , plus = radius + a * a , az = a * z , twice = az + az;
 const E root = float ( sqrt ( abs ( plus * plus + ( - ( twice * twice ) ) ) ) );
 const E minus = float ( E ( float ( radius ) ) + ( - ( a * a ) ) ) , numerator = float ( twice );
 const E perpendicular = sqrt ( root * root - minus * minus ) , v = numerator / root;
 const E log2e = numeric :: fromBits ( 0x3fff , 0xb8aa3b295c17f0bcULL );
 const E argument = v + sqrt ( v * v + 1 ) , infinity = native_std :: numeric_limits < float > :: infinity ( );
 const E logarithm = argument == infinity ? infinity : argument == E ( 0 ) ? - infinity : extendedLog2 ( argument );
 return FormulaVec3 ( float ( E ( params [ 1 ] ) - extendedAtan2 ( perpendicular , minus ) ) , float ( log2e * logarithm ) , float ( extendedAtan2 ( y , x ) ) );

}
 FormulaVec3 _invbisp_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA__INVBISP > ( p , params , de );

}

}}
