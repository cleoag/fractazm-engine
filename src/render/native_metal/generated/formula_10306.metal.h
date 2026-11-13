// Generated from dIFS/helistairsIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10306 {
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
 constant constexpr unsigned int FORMULA_HELISTAIRSIFS = 10306;
 template < > FormulaVec3 execute_formula < FORMULA_HELISTAIRSIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 Arm64ApplyIFSState native ( p , params , 5 );
 using E = Arm64ApplyIFSState :: E;
 const auto r = native . rotate ( params + 7 );
 const E x = r . x , y = r . y , rz = r . z , n = int ( params [ 16 ] ) , height = params [ 17 ];
 const E thick = params [ 18 ] , position = params [ 19 ] , width = params [ 20 ];
 const E radius = float ( position > E ( 0 ) ? sqrt ( x * x + y * y ) : ( abs ( y ) > abs ( x ) ? abs ( y ) : abs ( x ) ) );
 const E angle = ( extendedAtan2 ( y , x ) * params [ 22 ] ) * n;
 const E step = x87_fistp_int32 ( angle );
 const E u = float ( ( ( angle - step ) * params [ 23 ] ) / n );
 const E formula_half = float ( ( - abs ( height ) ) * params [ 24 ] ) , period = float ( abs ( ( - abs ( height ) ) * n ) );
 const E local = rz + step * ( - abs ( height ) );
 const E z = float ( local - round ( local / period ) * period );
 E distance;
 if ( metal :: signbit ( params [ 17 ] ) ) {
 E d;
 if ( - z > u - formula_half ) d = z - formula_half;
 else if ( ! ( - z > u + formula_half ) ) d = z + formula_half;
 else d = - u;
 d = abs ( d ) - thick;
 const E radial = abs ( radius - abs ( position ) ) - width;
 distance = radial > d ? radial : d;

}
 else {
 const E vertical = abs ( abs ( z ) + formula_half ) , angular = metal :: signbit ( float ( z ) ) ? u : - u;
 distance = ( angular > vertical ? angular : vertical ) - thick;
 const E radial = radius - abs ( position );
 if ( radial - width > distance ) distance = radial - width;
 const E upper = E ( float ( radial ) ) + width;
 if ( ! ( upper > distance ) ) distance = upper;

}
 params [ MAX_FORMULA_PARAMS - 1 ] = float ( abs ( E ( params [ 0 ] ) + E ( int ( params [ ITER_COUNT_SLOT ] ) ) * params [ 6 ] ) );
 return native . finish ( distance , de );

}
 FormulaVec3 helistairsIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_HELISTAIRSIFS > ( p , params , de );

}

}}
