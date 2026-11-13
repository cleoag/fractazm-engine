// Generated from 3Da/ATetraVS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10187 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ATETRAVS = 10187;
 static void rotated_fold ( thread float & x , thread float & y , thread float & z , float Fold , int tglad_mode , float M00 , float M01 , float M02 , float M10 , float M11 , float M12 , float M20 , float M21 , float M22 ) {
 float rx = x * M00 + y * M01 + z * M02;
 float ry = x * M10 + y * M11 + z * M12;
 float rz = x * M20 + y * M21 + z * M22;
 if ( tglad_mode == 1 ) {
 float new_rx = abs ( rx + Fold ) - abs ( rx - Fold ) - rx;
 rx = new_rx;

}
 else {
 rx = Fold - abs ( rx - Fold );

}
 x = rx * M00 + ry * M10 + rz * M20;
 y = rx * M01 + ry * M11 + rz * M21;
 z = rx * M02 + ry * M12 + rz * M22;

}
 template < > FormulaVec3 execute_formula < FORMULA_ATETRAVS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 float m [ 9 ];
 for ( int i = 0;
 i < 9;
 ++ i ) m [ i ] = static_cast < float > ( params [ 5 + i ] );
 const E fold ( params [ 3 ] );
 struct NativeClosure0 {
 thread decltype ( fold ) & fold;
 thread decltype ( m ) & m;
 thread decltype ( p ) & p;
 thread decltype ( params ) & params;
 auto operator ( ) ( ) const {
 const E x ( p . x ) , y ( p . y ) , z ( p . z );
 const E rz = x * E ( m [ 6 ] ) + y * E ( m [ 7 ] ) + z * E ( m [ 8 ] );
 E rx = x * E ( m [ 0 ] ) + y * E ( m [ 1 ] ) + z * E ( m [ 2 ] );
 const E ry = z * E ( m [ 5 ] ) + ( y * E ( m [ 4 ] ) + x * E ( m [ 3 ] ) );
 if ( int ( params [ 15 ] ) == 1 ) rx = - ( rx - ( abs ( rx + fold ) - abs ( rx - fold ) ) );
 else rx = - abs ( rx - fold ) + fold;
 const E nz = rx * E ( m [ 2 ] ) + ry * E ( m [ 5 ] ) + rz * E ( m [ 8 ] );
 const E nx = rx * E ( m [ 0 ] ) + ry * E ( m [ 3 ] ) + rz * E ( m [ 6 ] );
 const E ny = rz * E ( m [ 7 ] ) + ( ry * E ( m [ 4 ] ) + rx * E ( m [ 1 ] ) );
 p = FormulaVec3 ( static_cast < float > ( nx ) , static_cast < float > ( ny ) , static_cast < float > ( nz ) );

}

}
;
 auto applyFold = NativeClosure0 {
 fold , m , p , params
}
;
 struct NativeClosure1 {
 thread decltype ( applyFold ) & applyFold;
 thread decltype ( m ) & m;
 auto operator ( ) ( int a , int b , int c , int d ) const {
 m [ a ] = - m [ a ];
 m [ b ] = - m [ b ];
 m [ c ] = - m [ c ];
 m [ d ] = - m [ d ];
 applyFold ( );

}

}
;
 auto flip = NativeClosure1 {
 applyFold , m
}
;
 applyFold ( );
 const int mode = int ( params [ 14 ] );
 if ( mode == 0 ) {
 flip ( 2 , 5 , 6 , 7 );
 flip ( 1 , 5 , 3 , 7 );
 flip ( 2 , 5 , 6 , 7 );

}
 else if ( mode == 1 ) {
 flip ( 1 , 5 , 3 , 7 );
 flip ( 2 , 5 , 6 , 7 );
 flip ( 1 , 5 , 3 , 7 );

}
 else if ( mode == 2 ) {
 flip ( 2 , 1 , 6 , 3 );
 flip ( 1 , 5 , 3 , 7 );
 flip ( 2 , 1 , 6 , 3 );

}
 else {
 flip ( 1 , 5 , 3 , 7 );
 flip ( 2 , 1 , 6 , 3 );
 flip ( 1 , 5 , 3 , 7 );

}
 E previous ( params [ DIFS_VARYSCALE_SLOT ] );
 if ( native_formula_counter ( params ) <= 0 ) {
 previous = E ( params [ 0 ] );
 native_increment_formula_counter ( params );

}
 const float vary = static_cast < float > ( previous + ( abs ( previous ) - E ( 1 ) ) * E ( params [ 4 ] ) );
 params [ DIFS_VARYSCALE_SLOT ] = vary;
 const E x ( p . x ) , y ( p . y ) , z ( p . z ) , rr = z * z + y * y + x * x;
 const E scale = native_box_inner_unordered ( float ( rr ) , params ) ? E ( native_aboxmod1_inner_scale ( params ) ) : ( rr <= E ( 1 ) ? E ( vary ) / rr : E ( vary ) );
 de = static_cast < float > ( E ( de ) * scale );
 return FormulaVec3 ( static_cast < float > ( x * scale + E ( params [ 16 ] ) ) , static_cast < float > ( y * scale + E ( params [ 17 ] ) ) , static_cast < float > ( z * scale + E ( params [ 18 ] ) ) );

}
 FormulaVec3 ATetraVS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ATETRAVS > ( p , params , de );

}

}}
