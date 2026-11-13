// Generated from dIFS/tetratubeIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10219 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
struct Arm64TubeState {
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 FormulaVec3 original , transformed;
 FormulaParams params;
 bool apply;
 E a , b , c;
 Arm64TubeState ( FormulaVec3 p , FormulaParams values ) : original ( p ) , transformed ( p . x * values [ 3 ] + values [ 6 ] , p . y * values [ 3 ] + values [ 5 ] , p . z * values [ 3 ] + values [ 4 ] ) , params ( values ) , apply ( int ( values [ 11 ] ) != 0 ) , a ( transformed . y ) , b ( transformed . z ) , c ( transformed . x ) {
 if ( apply ) {
 params [ DIFS_VARYSCALE_SLOT ] *= params [ 3 ];
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.f;

}

}
 FormulaVec3 finish ( thread float & de ) {
 const E ac = float ( a * params [ 7 ] + c * params [ 8 ] );
 a = float ( a * params [ 8 ] + ( - ( c * params [ 7 ] ) ) );
 c = ac;
 const E ab = float ( a * params [ 9 ] + b * params [ 10 ] );
 a = float ( a * params [ 10 ] + ( - ( b * params [ 9 ] ) ) );
 b = ab;
 const E db = b - params [ 1 ] , dc = c - params [ 2 ];
 const E tube = sqrt ( db * db + dc * dc ) - params [ 0 ];
 b = float ( db );
 c = float ( dc );
 const E position = abs ( a ) - params [ 12 ];
 const E sphere = native_formula_length3 ( position , b , c ) - params [ 13 ];
 const bool useTube = sphere > tube;
 E distance = useTube ? tube : sphere;
 params [ MAX_FORMULA_PARAMS - 1 ] = params [ useTube ? 14 : 15 ];
 if ( ! apply ) distance /= params [ 3 ];
 de = float ( distance );
 return apply ? transformed : original;

}

}
;
 constant constexpr unsigned int FORMULA_TETRATUBEIFS = 10219;
 template < > FormulaVec3 execute_formula < FORMULA_TETRATUBEIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 Arm64TubeState state ( p , params );
 using E = Arm64TubeState :: E;
 E s0 = - state . c , s1 = state . b , s2 = state . a;
 struct NativeClosure0 {
 auto operator ( ) ( thread E & a , thread E & b ) const {
 const E t = a;
 a = b;
 b = t;

}

}
;
 const auto swap = NativeClosure0 {

}
;
 if ( s0 >= s1 ) swap ( s0 , s1 );
 if ( s0 >= s2 ) swap ( s0 , s2 );
 s0 = - s0;
 swap ( s0 , s1 );
 s0 = - s0;
 if ( s0 >= s2 ) swap ( s0 , s2 );
 s0 = - s0;
 swap ( s0 , s1 );
 if ( ! ( s0 >= s1 ) ) swap ( s0 , s1 );
 if ( ! ( s0 >= s2 ) ) swap ( s0 , s2 );
 swap ( s0 , s1 );
 if ( ! ( s0 >= s2 ) ) swap ( s0 , s2 );
 state . a = s2;
 state . b = s1;
 state . c = s0;
 return state . finish ( de );

}
 FormulaVec3 tetratubeIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_TETRATUBEIFS > ( p , params , de );

}

}}
