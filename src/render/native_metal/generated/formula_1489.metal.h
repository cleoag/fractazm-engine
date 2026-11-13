// Generated from dIFS/discoballIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_1489 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_DISCOBALLIFS = 1489;
 float discoballIFS_shape_de ( FormulaVec3 p , FormulaParams params ) {
 const float x = p . x * params [ 2 ] + params [ 5 ] , y = p . y * params [ 2 ] + params [ 4 ];
 const float z = p . z * params [ 2 ] + params [ 3 ];
 const int apply = int ( params [ 6 ] );
 float angle = ( params [ 11 ] - atan2 ( static_cast < float > ( x ) , static_cast < float > ( y ) ) ) * params [ 9 ];
 angle = rint ( angle * params [ 0 ] ) / params [ 0 ] * params [ 10 ];
 const float c = float ( cos ( angle ) ) , sn = float ( sin ( angle ) );
 const float u = float ( static_cast < float > ( x ) * c + static_cast < float > ( y ) * sn );
 angle = ( params [ 11 ] - atan2 ( static_cast < float > ( u ) , static_cast < float > ( z ) ) ) * params [ 9 ];
 angle = rint ( angle * params [ 1 ] ) / params [ 1 ] * params [ 10 ];
 const float c2 = float ( cos ( angle ) ) , sn2 = float ( sin ( angle ) );
 float distance = abs ( static_cast < float > ( u ) * c2 + static_cast < float > ( z ) * sn2 - params [ 7 ] ) - params [ 8 ];
 if ( ! apply ) distance /= params [ 2 ];
 return float ( distance );

}
 template < > FormulaVec3 execute_formula_dIFS < FORMULA_DISCOBALLIFS > ( FormulaVec3 p , FormulaParams params , thread float & varyScale , thread float & rout ) {
 const int apply = int ( params [ 6 ] );
 rout = discoballIFS_shape_de ( p , params );
 params [ DIFS_COMMIT_XYZ_SLOT ] = apply != 0 ? 1.0f : 0.0f;
 if ( apply != 0 ) varyScale *= params [ 2 ];
 return FormulaVec3 ( p . x * params [ 2 ] + params [ 5 ] , p . y * params [ 2 ] + params [ 4 ] , p . z * params [ 2 ] + params [ 3 ] );

}
 template < > FormulaVec3 execute_formula < FORMULA_DISCOBALLIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float varyScale = 1.0f;
 return execute_formula_dIFS < FORMULA_DISCOBALLIFS > ( p , params , varyScale , de );

}
 FormulaVec3 discoballIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_DISCOBALLIFS > ( p , params , de );

}

}}
