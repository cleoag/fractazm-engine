// Generated from 3D/Benesi1Pow2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7083 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BENESI1POW2 = 7083;
 template < > FormulaVec3 execute_formula < FORMULA_BENESI1POW2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 const E x = p . x , y = p . y , z = p . z , yy = y * y , zz = z * z;
 const E sum = zz + yy , rootX = sqrt ( sum ) * x , scale = ( rootX + rootX ) / sum;
 const E yt = ( yy - zz ) * scale , zt = ( y * scale ) * z;
 return FormulaVec3 ( float ( x * x - sum + params [ 0 ] ) , float ( abs ( E ( params [ 1 ] ) ) - abs ( yt ) ) , float ( abs ( E ( params [ 2 ] ) ) - abs ( zt + zt ) ) );

}

}}
