// Generated ordinary Benesi transform; Native FP32, unqualified.
#pragma once
namespace mb3d_native { namespace formula_95003 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
static void benesiTransform ( thread float & x , thread float & y , thread float & z , int mode , float scale , float offset ) {
 constexpr float a = 0.816496580927726f , b = 0.5773502691896258f , c = 0.7071067811865475f;
 float tx = ( x * a - z * b ) * c , yp = y * c , zp = x * b + z * a;
 float u = tx - yp , v = tx + yp;
 if ( mode == 1 ) {
 x = metal :: abs ( u );
 y = metal :: abs ( v );
 z = metal :: abs ( zp );
 tx = ( x + y ) * c;
 y = ( - x + y ) * c;
 x = ( tx * a + z * b ) * scale - offset;
 z = ( - tx * b + z * a ) * scale;
 y *= scale;

}
 else {
 u = mode == 2 ? u * u : metal :: abs ( u );
 v = mode == 2 ? v * v : metal :: abs ( v );
 zp = mode == 2 ? zp * zp : metal :: abs ( zp );
 struct NativeClosure0 {
 thread decltype ( mode ) & mode;
 thread decltype ( offset ) & offset;
 thread decltype ( scale ) & scale;
 auto operator ( ) ( float value ) const {
 return metal :: abs ( ( mode == 2 ? metal :: sqrt ( value ) : value ) - offset ) * scale;

}

}
;
 auto fold = NativeClosure0 {
 mode , offset , scale
}
;
 x = fold ( v + zp ) * c;
 y = fold ( u + zp ) * c;
 z = fold ( u + v );
 tx = x + y;
 y = y - x;
 x = tx * a + z * b;
 z = - tx * b + z * a;

}

}
 template < > FormulaVec3 execute_formula < 95003 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 benesiTransform ( p . x , p . y , p . z , 3 , params [ 0 ] , params [ 1 ] );
 return p;

}

}}
