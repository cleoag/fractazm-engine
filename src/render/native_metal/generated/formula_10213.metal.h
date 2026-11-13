// Generated from dIFS/loxodrome2IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10213 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
struct Arm64ApplyIFSState {
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 FormulaVec3 original , transformed;
 FormulaParams params;
 bool apply;
 Arm64ApplyIFSState ( FormulaVec3 p , FormulaParams values , int applyIndex = 6 ) : original ( p ) , transformed ( p . x * values [ 1 ] + values [ 4 ] , p . y * values [ 1 ] + values [ 3 ] , p . z * values [ 1 ] + values [ 2 ] ) , params ( values ) , apply ( int ( values [ applyIndex ] ) != 0 ) {
 if ( apply ) {
 params [ DIFS_VARYSCALE_SLOT ] *= params [ 1 ];
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.f;

}

}
 FormulaVec3 rotate ( thread const float * m ) const {
 const E x = transformed . x , y = transformed . y , z = transformed . z;
 return FormulaVec3 ( float ( ( x * float ( m [ 0 ] ) + y * float ( m [ 1 ] ) ) + z * float ( m [ 2 ] ) ) , float ( ( x * float ( m [ 3 ] ) + y * float ( m [ 4 ] ) ) + z * float ( m [ 5 ] ) ) , float ( ( x * float ( m [ 6 ] ) + y * float ( m [ 7 ] ) ) + z * float ( m [ 8 ] ) ) );

}
 FormulaVec3 finish ( E distance , thread float & de ) const {
 if ( ! apply ) distance /= params [ 1 ];
 de = float ( distance );
 return apply ? transformed : original;

}

}
;
 constant constexpr unsigned int FORMULA_LOXODROME2IFS = 10213;
 template < > FormulaVec3 execute_formula < FORMULA_LOXODROME2IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 Arm64ApplyIFSState native ( p , params );
 using E = Arm64ApplyIFSState :: E;
 const E x = native . transformed . x , y = native . transformed . y , z = native . transformed . z;
 E distance = params [ 16 ];
 if ( E ( params [ 17 ] ) >= abs ( z ) ) {
 E v = abs ( z );
 const int style = int ( params [ 12 ] );
 if ( style < 0 ) v = extendedSinCos ( v * params [ 19 ] ) . sin;
 if ( style != 1 && style != - 1 ) v = - v + E ( 1 );
 const E infinity = native_std :: numeric_limits < float > :: infinity ( );
 const E logarithm = v == E ( 0 ) ? - infinity : v == infinity ? infinity : extendedLog2 ( v );
 E angle = E ( params [ 5 ] ) * logarithm;
 if ( metal :: signbit ( native . transformed . z ) ) angle = - angle;
 const auto sc = extendedSinCos ( angle + params [ 8 ] );
 const E sn = float ( sc . sin ) , cs = float ( sc . cos );
 const E sector = ( extendedAtan2 ( x * sn + ( - ( y * cs ) ) , x * cs + y * sn ) * params [ 13 ] ) / params [ 7 ];
 const E tooth = abs ( ( sector - round ( sector ) ) * params [ 7 ] ) / params [ 0 ] - E ( 1 );
 distance = - ( ( tooth * params [ 0 ] ) * params [ 14 ] );

}
 const E quadric = ( sqrt ( abs ( ( ( x * x + y * y ) + ( z * z ) * params [ 10 ] ) - params [ 11 ] ) ) - params [ 9 ] ) * params [ 15 ];
 if ( quadric >= distance ) distance = quadric;
 const E plane = abs ( z ) - params [ 18 ];
 if ( plane >= distance ) distance = plane;
 return native . finish ( distance , de );

}
 FormulaVec3 loxodrome2IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_LOXODROME2IFS > ( p , params , de );

}

}}
