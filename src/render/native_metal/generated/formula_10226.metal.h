// Generated from dIFS/diniIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10226 {
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
 constant constexpr unsigned int FORMULA_DINIIFS = 10226;
 template < > FormulaVec3 execute_formula < FORMULA_DINIIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 Arm64ApplyIFSState native ( p , params );
 using E = Arm64ApplyIFSState :: E;
 E x = native . transformed . x , y = native . transformed . y , z = native . transformed . z;
 if ( int ( params [ 9 ] ) == 1 ) {
 const E t = x;
 x = z;
 z = t;

}
 if ( int ( params [ 9 ] ) == - 1 ) {
 const E t = y;
 y = z;
 z = t;

}
 if ( int ( params [ 10 ] ) != 0 ) z = - z;
 E minimum = params [ 14 ];
 for ( int i = 0;
 i < 3;
 ++ i ) {
 const E nx = - x , ny = - y;
 const E angle = float ( extendedAtan2 ( y - x , y + x ) * params [ 11 ] );
 const E square = x * x + y * y , radius = float ( sqrt ( square ) );
 const E delta = E ( params [ 0 ] ) * params [ 0 ] - E ( float ( square ) );
 const E root = sqrt ( delta + abs ( delta ) ) , argument = ( root + params [ 0 ] ) / radius;
 const E infinity = native_std :: numeric_limits < float > :: infinity ( );
 const E logarithm = argument == E ( 0 ) ? - infinity : argument == infinity ? infinity : extendedLog2 ( argument );
 E distance = ( abs ( ( ( ( extendedLn2 ( ) * logarithm ) * params [ 0 ] + ( - root ) ) - angle ) - z ) - params [ 7 ] ) * abs ( radius * params [ 12 ] + params [ 13 ] );
 const E outer = radius - params [ 5 ] , inner = E ( params [ 8 ] ) - radius;
 if ( outer >= distance ) distance = outer;
 if ( inner >= distance ) distance = inner;
 if ( ny >= distance ) distance = ny;
 if ( ! ( distance >= minimum ) ) minimum = float ( distance );
 x = nx;
 y = ny;
 z = float ( z + E ( params [ 11 ] ) * x87Pi ( ) );

}
 return native . finish ( minimum , de );

}
 FormulaVec3 diniIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_DINIIFS > ( p , params , de );

}

}}
