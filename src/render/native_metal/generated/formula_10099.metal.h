// Generated from dIFS/CitrusIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10099 {
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
 constant constexpr unsigned int FORMULA_CITRUSIFS = 10099;
 template < > FormulaVec3 execute_formula < FORMULA_CITRUSIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 Arm64ApplyIFSState native ( p , params );
 using E = Arm64ApplyIFSState :: E;
 const auto rotated = native . rotate ( params + 7 );
 const E x = rotated . x , y = rotated . y , z = rotated . z , sub = params [ 5 ];
 float positive = float ( ( z * z + y * y ) * sub ) , negative = 0;
 const E shifted = x - params [ 0 ];
 const E term = ( ( ( ( shifted * shifted ) * shifted ) * x ) * x ) * x;
 if ( ! ( term >= E ( 0 ) ) ) negative = float ( E ( negative ) - term );
 else positive = float ( term + E ( positive ) );
 E distance = sqrt ( sqrt ( E ( positive ) ) ) - sqrt ( sqrt ( E ( negative ) ) );
 const E clipR = ( ( sqrt ( x * x + y * y ) - sub ) - sub ) - sub;
 if ( clipR >= distance ) distance = clipR;
 const E clipZ = ( ( abs ( z ) - sub ) - sub ) - sub;
 if ( clipZ >= distance ) distance = clipZ;
 return native . finish ( distance , de );

}
 FormulaVec3 CitrusIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_CITRUSIFS > ( p , params , de );

}

}}
