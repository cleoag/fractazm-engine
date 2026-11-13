// Generated from 3Da/ATetraVS2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10061 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ATETRAVS2 = 10061;
 static void atetra_fold_pair ( thread float & a , thread float & b , float Fold45 ) {
 float twoF45 = 2.0f * Fold45;
 float sum = a + b + twoF45;
 if ( sum > 0.0f ) {
 float tmp = - b - twoF45;
 b = - a - twoF45;
 a = tmp;

}
 else {
 float sum2 = a + b - twoF45;
 if ( sum2 <= 0.0f ) {
 float tmp = twoF45 - b;
 b = twoF45 - a;
 a = tmp;

}

}

}
 template < > FormulaVec3 execute_formula < FORMULA_ATETRAVS2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float ScaleOverMinR2 = params [ 1 ];
 const float minR2 = params [ 2 ];
 const float Fold = params [ 3 ];
 const float Scale_vary = params [ 4 ];
 const float Radius_recip = params [ 5 ];
 float prevVaryScale = params [ DIFS_VARYSCALE_SLOT ];
 if ( native_formula_counter ( params ) <= 0.0f ) {
 prevVaryScale = Scale;
 native_increment_formula_counter ( params );

}
 float VaryScale = prevVaryScale + Scale_vary * ( abs ( prevVaryScale ) - 1.0f );
 params [ DIFS_VARYSCALE_SLOT ] = VaryScale;
 const float inv_sqrt2 = - 0.70710678118655f;
 float Fold45 = Fold * inv_sqrt2;
 float x = p . x;
 float y = p . y;
 float z = p . z;
 atetra_fold_pair ( y , x , Fold45 );
 atetra_fold_pair ( z , x , Fold45 );
 atetra_fold_pair ( z , y , Fold45 );
 float rr = ( x * x + y * y + z * z ) * Radius_recip;
 float m;
 if ( rr < minR2 ) {
 m = ScaleOverMinR2;

}
 else if ( rr <= 1.0f ) {
 m = VaryScale / rr;

}
 else {
 m = VaryScale;

}
 de = de * m;
 FormulaVec3 result;
 result . x = z * m;
 result . y = y * m;
 result . z = x * m;
 return result;

}
 FormulaVec3 ATetraVS2_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ATETRAVS2 > ( p , params , de );

}

}}
