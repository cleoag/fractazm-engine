// Generated from dIFS/QuadricsIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10066 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_QUADRICSIFS = 10066;
 template < > FormulaVec3 execute_formula < FORMULA_QUADRICSIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 const int apply = int ( params [ 6 ] );
 const FormulaVec3 transformed ( p . x * params [ 1 ] + params [ 4 ] , p . y * params [ 1 ] + params [ 3 ] , p . z * params [ 1 ] + params [ 2 ] );
 if ( apply ) params [ DIFS_VARYSCALE_SLOT ] *= params [ 1 ];
 E x ( transformed . x ) , y ( transformed . y ) , z ( transformed . z );
 if ( int ( params [ 10 ] ) ) native_std :: swap ( x , z );
 const E quadric = sqrt ( abs ( ( z * z + ( y * y + ( x * x ) * E ( params [ 7 ] ) ) ) - E ( params [ 8 ] ) ) ) - E ( params [ 5 ] );
 E best = int ( params [ 9 ] ) ? x : abs ( x ) - E ( params [ 0 ] );
 const E yCut = abs ( y ) - E ( params [ 0 ] ) , zCut = abs ( z ) - E ( params [ 0 ] );
 if ( yCut >= best ) best = yCut;
 if ( zCut >= best ) best = zCut;
 E distance = best >= quadric ? best : quadric;
 if ( ! apply ) distance /= E ( params [ 1 ] );
 de = float ( distance );
 return apply ? transformed : p;

}
 FormulaVec3 QuadricsIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_QUADRICSIFS > ( p , params , de );

}

}}
