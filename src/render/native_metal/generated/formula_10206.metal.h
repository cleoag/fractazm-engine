// Generated from dIFS/CommaIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10206 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_COMMAIFS = 10206;
 template < > FormulaVec3 execute_formula < FORMULA_COMMAIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int apply = int ( params [ 6 ] );
 const FormulaVec3 transformed ( p . x * params [ 1 ] + params [ 4 ] , p . y * params [ 1 ] + params [ 3 ] , p . z * params [ 1 ] + params [ 2 ] );
 if ( apply != 0 ) params [ DIFS_VARYSCALE_SLOT ] *= params [ 1 ];
 float x = transformed . x , y = transformed . y , z = transformed . z;
 if ( int ( params [ 9 ] ) != 0 ) {
 const float t = x;
 x = z;
 z = t;

}
 if ( int ( params [ 10 ] ) != 0 ) {
 const float t = y;
 y = z;
 z = t;

}
 const int signs = int ( params [ 11 ] );
 if ( ! ( signs & 1 ) ) x = - x;
 if ( ! ( signs & 2 ) ) y = - y;
 if ( ! ( signs & 4 ) ) z = - z;
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 const E sx = x , sy = y , sz = z;
 const E r = sqrt ( sx * sx + sy * sy );
 const E theta = numeric :: extendedAtan2 ( sy , sx ) * float ( .159155f );
 const E t2 = theta * theta;
 const float radius = float ( ( theta + ( - t2 * theta + t2 ) ) * params [ 0 ] + params [ 5 ] );
 const E a = r - params [ 7 ] , b = sx + params [ 7 ];
 const E torus = sqrt ( a * a + sz * sz ) - radius;
 const E sphere = native_formula_length3 ( b , sy , sz ) - params [ 8 ];
 E distance = ! ( sphere >= torus ) ? sphere : torus;
 if ( apply == 0 ) distance /= params [ 1 ];
 de = float ( distance );
 return apply != 0 ? transformed : p;

}

}}
