// Generated from 3D/TetraFolding_Pow2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10319 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TETRAFOLDINGPOW2 = 10319;
 inline float tetra_fold ( float val , float fold , int tglad ) {
 if ( tglad != 1 ) {
 return fold - fabs ( val - fold );

}
 else {
 return fabs ( val + fold ) - fabs ( val - fold ) - val;

}

}
 inline FormulaVec3 forward_rotate ( float x , float y , float z , float a , float b , float c , float d , float e , float f , float g , float h , float i ) {
 float R0 = a * x + b * y + c * z;
 float R1 = d * x + e * y + f * z;
 float R2 = g * x + h * y + i * z;
 return FormulaVec3 ( R0 , R1 , R2 );

}
 inline FormulaVec3 inverse_rotate ( float R0 , float R1 , float R2 , float a , float b , float c , float d , float e , float f , float g , float h , float i ) {
 float xp = a * R0 + d * R1 + g * R2;
 float yp = b * R0 + e * R1 + h * R2;
 float zp = c * R0 + f * R1 + i * R2;
 return FormulaVec3 ( xp , yp , zp );

}
 inline FormulaVec3 fold_rotate ( float x , float y , float z , float a , float b , float c , float d , float e , float f , float g , float h , float i , float fold_val , int tglad ) {
 FormulaVec3 R = forward_rotate ( x , y , z , a , b , c , d , e , f , g , h , i );
 R . x = tetra_fold ( R . x , fold_val , tglad );
 return inverse_rotate ( R . x , R . y , R . z , a , b , c , d , e , f , g , h , i );

}
 inline FormulaVec3 fold_rotate_scale ( float x , float y , float z , float a , float b , float c , float d , float e , float f , float g , float h , float i , float fold_val , int tglad , float zmul2 ) {
 FormulaVec3 R = forward_rotate ( x , y , z , a , b , c , d , e , f , g , h , i );
 R . x = tetra_fold ( R . x , fold_val , tglad );
 R . x *= zmul2;
 return inverse_rotate ( R . x , R . y , R . z , a , b , c , d , e , f , g , h , i );

}
 template < > FormulaVec3 execute_formula < FORMULA_TETRAFOLDINGPOW2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 float matrix [ 9 ];
 for ( int i = 0;
 i < 9;
 ++ i ) matrix [ i ] = static_cast < float > ( params [ 6 + i ] );
 const E fold ( params [ 5 ] );
 struct NativeClosure0 {
 thread decltype ( fold ) & fold;
 thread decltype ( matrix ) & matrix;
 thread decltype ( p ) & p;
 thread decltype ( params ) & params;
 auto operator ( ) ( bool scaleFold ) const {
 const E x ( p . x ) , y ( p . y ) , z ( p . z );
 const E rz = x * E ( matrix [ 6 ] ) + y * E ( matrix [ 7 ] ) + z * E ( matrix [ 8 ] );
 E rx = x * E ( matrix [ 0 ] ) + y * E ( matrix [ 1 ] ) + z * E ( matrix [ 2 ] );
 const E ry = z * E ( matrix [ 5 ] ) + ( y * E ( matrix [ 4 ] ) + x * E ( matrix [ 3 ] ) );
 if ( int ( params [ 16 ] ) == 1 ) rx = - ( rx - ( abs ( rx + fold ) - abs ( rx - fold ) ) );
 else rx = - abs ( rx - fold ) + fold;
 if ( scaleFold ) rx *= E ( params [ 17 ] );
 const E nz = rx * E ( matrix [ 2 ] ) + ry * E ( matrix [ 5 ] ) + rz * E ( matrix [ 8 ] );
 const E nx = rx * E ( matrix [ 0 ] ) + ry * E ( matrix [ 3 ] ) + rz * E ( matrix [ 6 ] );
 const E ny = rz * E ( matrix [ 7 ] ) + ( ry * E ( matrix [ 4 ] ) + rx * E ( matrix [ 1 ] ) );
 p = FormulaVec3 ( static_cast < float > ( nx ) , static_cast < float > ( ny ) , static_cast < float > ( nz ) );

}

}
;
 auto applyFold = NativeClosure0 {
 fold , matrix , p , params
}
;
 struct NativeClosure1 {
 thread decltype ( applyFold ) & applyFold;
 thread decltype ( matrix ) & matrix;
 auto operator ( ) ( int a , int b , int c , int d ) const {
 matrix [ a ] = - matrix [ a ];
 matrix [ b ] = - matrix [ b ];
 matrix [ c ] = - matrix [ c ];
 matrix [ d ] = - matrix [ d ];
 applyFold ( false );

}

}
;
 auto flip = NativeClosure1 {
 applyFold , matrix
}
;
 applyFold ( true );
 const int mode = int ( params [ 15 ] );
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
 const E x ( p . x ) , y ( p . y ) , z ( p . z );
 const E radius = x * x + ( y * y + E ( params [ 3 ] ) );
 if ( radius < E ( 0 ) ) return FormulaVec3 ( 0 , 0 , 0 );
 if ( ! ( radius > E ( 0 ) ) ) return FormulaVec3 ( params [ 18 ] , params [ 19 ] , params [ 20 ] );
 const E factor = E ( params [ 4 ] ) + - ( z * z / radius );
 const E nx = ( x * x + - ( y * y ) ) * E ( params [ 0 ] ) * factor + E ( params [ 18 ] );
 const E ny = x * y * E ( params [ 1 ] ) * factor + E ( params [ 19 ] );
 const E nz = z * sqrt ( radius ) * E ( params [ 2 ] ) + E ( params [ 20 ] );
 return FormulaVec3 ( static_cast < float > ( nx ) , static_cast < float > ( ny ) , static_cast < float > ( nz ) );

}
 FormulaVec3 TetraFoldingPow2_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_TETRAFOLDINGPOW2 > ( p , params , de );

}

}}
