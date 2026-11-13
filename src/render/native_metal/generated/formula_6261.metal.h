// Generated from dIFS/UmbrellaIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6261 {
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
 constant constexpr unsigned int FORMULA_UMBRELLAIFS = 6261;
 template < > FormulaVec3 execute_formula < FORMULA_UMBRELLAIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 Arm64ApplyIFSState native ( p , params );
 using E = Arm64ApplyIFSState :: E;
 const bool flip = int ( params [ 7 ] ) != 0;
 const E a = flip ? native . transformed . z : native . transformed . x , b = native . transformed . y , c = flip ? native . transformed . x : native . transformed . z;
 E distance = ( abs ( ( a * a - ( c * b ) * b ) + params [ 5 ] ) - params [ 5 ] ) * E ( params [ 11 ] );
 if ( ! ( E ( 1 ) >= distance ) ) distance = sqrt ( sqrt ( distance ) );
 E clip = abs ( a ) - params [ 0 ] , candidate = abs ( b ) - params [ 0 ];
 if ( candidate >= clip ) clip = candidate;
 candidate = abs ( c ) - params [ 0 ];
 if ( candidate >= clip ) clip = candidate;
 if ( clip >= distance ) distance = clip;
 params [ MAX_FORMULA_PARAMS - 1 ] = float ( abs ( ( ( b * a - c ) * params [ 8 ] + E ( int ( params [ ITER_COUNT_SLOT ] ) ) * params [ 9 ] ) + params [ 10 ] ) );
 return native . finish ( distance , de );

}
 FormulaVec3 UmbrellaIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_UMBRELLAIFS > ( p , params , de );

}

}}
