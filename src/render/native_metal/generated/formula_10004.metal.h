// Generated from Ads/_PartlyJulia.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10004 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__PARTLYJULIA = 10004;
 template < > FormulaVec3 execute_formula < FORMULA__PARTLYJULIA > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float X_above = params [ 0 ];
 const float X_below = params [ 1 ];
 const float X_add = params [ 2 ];
 const float Y_above = params [ 3 ];
 const float Y_below = params [ 4 ];
 const float Y_add = params [ 5 ];
 const float Z_above = params [ 6 ];
 const float Z_below = params [ 7 ];
 const float Z_add = params [ 8 ];
 float Cx = params [ 9 ];
 float Cy = params [ 10 ];
 float Cz = params [ 11 ];
 const float px = params [ 15 ] , py = params [ 16 ] , pz = params [ 17 ];
 if ( px > X_above || ! ( px > X_below ) ) Cx = px + X_add;
 if ( py > Y_above || ! ( py > Y_below ) ) Cy = py + Y_add;
 if ( pz > Z_above || ! ( pz > Z_below ) ) Cz = pz + Z_add;
 params [ WRITE_J1_SLOT ] = Cx;
 params [ WRITE_J2_SLOT ] = Cy;
 params [ WRITE_J3_SLOT ] = Cz;
 return p;

}

}}
