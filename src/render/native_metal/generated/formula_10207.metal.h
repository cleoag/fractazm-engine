// Generated from dIFS/borgIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10207 {
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
 constant constexpr unsigned int FORMULA_BORGIFS = 10207;
 template < > FormulaVec3 execute_formula < FORMULA_BORGIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 Arm64ApplyIFSState native ( p , params );
 using E = Arm64ApplyIFSState :: E;
 const auto rotated = native . rotate ( params + 7 );
 const E x = rotated . x , y = rotated . y , z = rotated . z;
 struct NativeClosure0 {
 auto operator ( ) ( E a ) const {
 if ( ( numeric :: signExponent ( a ) & 0x7fff ) != 0x7fff && abs ( a ) >= E ( 0x1p63f ) ) return a;
 return numeric :: extendedSinCos ( a ) . sin;

}

}
;
 auto sine = NativeClosure0 {

}
;
 const E s1 = sine ( ( x * y ) * params [ 5 ] ) , s2 = sine ( ( z * y ) * params [ 5 ] ) , s3 = sine ( ( z * x ) * params [ 5 ] );
 params [ MAX_FORMULA_PARAMS - 1 ] = float ( abs ( ( ( s3 - s2 ) - s1 ) * params [ 16 ] ) );
 const E sum = s1 + ( s2 + s3 );
 E cube = abs ( y ) >= abs ( z ) ? abs ( y ) : abs ( z );
 cube = abs ( x ) >= cube ? abs ( x ) : cube;
 cube -= params [ 0 ];
 return native . finish ( ( cube >= sum ? cube : sum ) / params [ 5 ] , de );

}
 FormulaVec3 borgIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_BORGIFS > ( p , params , de );

}

}}
