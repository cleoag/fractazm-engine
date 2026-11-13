// Generated from dIFS/gyroidIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3154 {
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
 constant constexpr unsigned int FORMULA_GYROIDIFS = 3154;
 template < > FormulaVec3 execute_formula < FORMULA_GYROIDIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 Arm64ApplyIFSState native ( p , params );
 using E = Arm64ApplyIFSState :: E;
 const auto rotated = native . rotate ( params + 7 );
 const E x = rotated . x , y = rotated . y , z = rotated . z;
 const E ax = x * params [ 5 ] , ay = y * params [ 5 ] , az = z * params [ 5 ];
 const E range = numeric :: fromBits ( 16383 + 63 , 0x8000000000000000ULL );
 if ( abs ( ax ) >= range || abs ( ay ) >= range || abs ( az ) >= range ) {
 params [ MAX_FORMULA_PARAMS - 1 ] = float ( abs ( extendedNaN ( ) ) );
 return native . finish ( extendedNaN ( ) , de );

}
 const auto sx = extendedSinCos ( ax ) , sy = extendedSinCos ( ay ) , sz = extendedSinCos ( az );
 const E t1 = sy . sin * sx . cos , t2 = sz . cos * sx . sin , t3 = sz . sin * sy . cos;
 params [ MAX_FORMULA_PARAMS - 1 ] = float ( abs ( ( ( t1 - t2 ) - t3 ) * params [ 16 ] ) );
 E distance = t3 + ( t2 + t1 ) , clip = abs ( z );
 if ( abs ( y ) >= clip ) clip = abs ( y );
 if ( abs ( x ) >= clip ) clip = abs ( x );
 clip -= params [ 0 ];
 if ( clip >= distance ) distance = clip;
 return native . finish ( distance / params [ 5 ] , de );

}
 FormulaVec3 gyroidIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_GYROIDIFS > ( p , params , de );

}

}}
