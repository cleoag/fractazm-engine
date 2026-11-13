// Generated from dIFS/logspIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10229 {
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
 constant constexpr unsigned int FORMULA_LOGSPIFS = 10229;
 template < > FormulaVec3 execute_formula < FORMULA_LOGSPIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 Arm64ApplyIFSState native ( p , params );
 using E = Arm64ApplyIFSState :: E;
 const auto rotated = native . rotate ( params + 10 );
 E x = rotated . x , y = rotated . y , z = rotated . z;
 if ( metal :: signbit ( params [ 5 ] ) ) {
 const E shifted = x + E ( 1 ) , inverse = E ( 1 ) / ( shifted * shifted + y * y );
 const E a = float ( ( ( x * x + y * y ) - E ( 1 ) ) * inverse + params [ 4 ] );
 const E b = float ( ( ( - y ) + ( - y ) ) * inverse );
 x = float ( b * float ( params [ 20 ] ) + a * float ( params [ 19 ] ) );
 y = float ( b * float ( params [ 19 ] ) + ( - ( a * float ( params [ 20 ] ) ) ) );

}
 const E radius = sqrt ( x * x + y * y ) , infinity = native_std :: numeric_limits < float > :: infinity ( );
 const int style = int ( params [ 9 ] );
 E radial;
 if ( style == 1 ) radial = E ( params [ 0 ] ) / radius;
 else {
 const E logarithm = radius == E ( 0 ) ? - infinity : radius == infinity ? infinity : extendedLog2 ( radius );
 radial = E ( params [ 0 ] ) * logarithm;
 if ( style == 2 ) radial = - ( radial * radial );
 else if ( style == 3 ) {
 const bool negative = metal :: signbit ( float ( radial ) );
 radial = abs ( radial ) + params [ 22 ];
 if ( negative ) radial = - radial;
 radial = E ( 1 ) / radial;

}

}
 const E angle = ( extendedAtan2 ( x , y ) * params [ 21 ] ) * E ( int ( params [ 7 ] ) ) + radial;
 const E distance = sqrt ( abs ( round ( angle ) - angle ) * params [ 8 ] + z * z ) - abs ( E ( params [ 5 ] ) );
 return native . finish ( distance , de );

}
 FormulaVec3 logspIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_LOGSPIFS > ( p , params , de );

}

}}
