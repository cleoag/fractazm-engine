// Generated from dIFS/pyramIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3700 {
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
 constant constexpr unsigned int FORMULA_PYRAMIFS = 3700;
 template < > FormulaVec3 execute_formula < FORMULA_PYRAMIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 Arm64ApplyIFSState native ( p , params );
 using E = Arm64ApplyIFSState :: E;
 const E x = native . transformed . x , y = native . transformed . y , z = native . transformed . z;
 const auto sc = extendedSinCos ( z * params [ 7 ] );
 const E sn = float ( sc . sin ) , cs = float ( sc . cos );
 const E a = float ( x * sn + ( - ( y * cs ) ) ) , b = float ( x * cs + y * sn );
 E distance = ( a + b ) + z;
 const E d1 = ( a - b ) - z , d2 = ( b - a ) - z , d3 = ( z - a ) - b;
 if ( d1 >= distance ) distance = d1;
 if ( d2 >= distance ) distance = d2;
 if ( d3 >= distance ) distance = d3;
 distance = ( distance - params [ 0 ] ) + native_formula_length3 ( a , b , z ) * params [ 5 ];
 return native . finish ( distance , de );

}
 FormulaVec3 pyramIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_PYRAMIFS > ( p , params , de );

}

}}
