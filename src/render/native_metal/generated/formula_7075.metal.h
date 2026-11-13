// Generated from 3D/MsltoeFoldQuat.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7075 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MSLTOEFOLDQUAT = 7075;
 template < > FormulaVec3 execute_formula < FORMULA_MSLTOEFOLDQUAT > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 E x = p . x , y = p . y , z = p . z , xx = x * x , yy = y * y , zz = z * z;
 if ( ! ( xx > zz ) ) {
 native_std :: swap ( x , z );
 native_std :: swap ( xx , zz );

}
 if ( ! ( xx > yy ) ) {
 native_std :: swap ( x , y );
 native_std :: swap ( xx , yy );

}
 if ( ! ( yy > zz ) ) {
 native_std :: swap ( y , z );
 native_std :: swap ( yy , zz );

}
 const E xy = x * y , xz = x * z;
 return FormulaVec3 ( float ( ( xx - yy ) - zz + params [ 0 ] ) , float ( xy + xy + params [ 1 ] ) , float ( xz + xz + params [ 2 ] ) );

}

}}
