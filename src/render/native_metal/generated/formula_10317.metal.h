// Generated from 3D/TalisTpx.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10317 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TALITTPX = 10317;
 template < > FormulaVec3 execute_formula < FORMULA_TALITTPX > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 const numeric :: ExtendedScope scope;
 struct NativeClosure0 {
 auto operator ( ) ( E value ) const {
 return E ( float ( value ) );

}

}
;
 const auto store = NativeClosure0 {

}
;
 const E x ( p . x ) , y ( p . y ) , z ( p . z ) , zmul ( params [ 0 ] );
 E px = x , py = y , pz = z;
 int power = int ( params [ 7 ] );
 if ( power < 2 || power > 22 ) power = 8;
 const E originalRadius = store ( sqrt ( x * x + y * y ) );
 for ( int i = 1;
 i < power;
 ++ i ) {
 const E radius = sqrt ( px * px + py * py );
 const E nextZ = ( radius * z - originalRadius * pz ) * zmul;
 const E factor = store ( - ( ( pz * z / originalRadius ) / store ( radius ) ) + E ( 1 ) );
 const E nextX = ( x * px - y * py ) * factor;
 const E nextY = ( y * px + x * py ) * factor;
 px = store ( nextX );
 py = store ( nextY );
 pz = store ( nextZ );

}
 const E a1 ( params [ 4 ] ) , a2 ( params [ 5 ] ) , a3 ( params [ 6 ] );
 const E cx ( params [ 8 ] ) , cy ( params [ 9 ] ) , cz ( params [ 10 ] );
 px = store ( px * a1 + ( x * a2 + cx * a3 ) );
 py = store ( py * a1 + ( y * a2 + cy * a3 ) );
 pz = store ( pz * a1 + ( z * a2 + cz * a3 ) );
 const E qx = store ( E ( params [ 1 ] ) + x ) , qy = store ( E ( params [ 2 ] ) + y ) , qz = store ( E ( params [ 3 ] ) + z );
 const E square = store ( qx * qx + ( qy * qy + qz * qz ) );
 const E numeratorRadius = store ( sqrt ( px * px + py * py ) );
 const E denominatorRadius = store ( sqrt ( qx * qx + qy * qy ) );
 const E outZ = store ( ( ( denominatorRadius * pz - numeratorRadius * qz ) / square ) * zmul );
 const E factor = store ( ( ( ( pz * qz / denominatorRadius ) / numeratorRadius ) + E ( 1 ) ) / square );
 const E outX = store ( ( px * qx + py * qy ) * factor );
 const E outY = store ( ( qx * py - px * qy ) * factor );
 const E complement = E ( 1 ) - a3;
 return FormulaVec3 ( float ( cx * complement + outX ) , float ( cy * complement + outY ) , float ( cz * complement + outZ ) );

}
 FormulaVec3 TalisTpx_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_TALITTPX > ( p , params , de );

}

}}
