// Generated from dIFS/KnotsIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10199 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_KNOTSIFS = 10199;
 template < > FormulaVec3 execute_formula_dIFS < FORMULA_KNOTSIFS > ( FormulaVec3 p , FormulaParams params , thread float & varyScale , thread float & rout ) {
 const float x = p . x * params [ 1 ] + params [ 4 ] , y = p . y * params [ 1 ] + params [ 3 ];
 const float z = p . z * params [ 1 ] + params [ 2 ];
 const int apply = int ( params [ 6 ] );
 const float order = int ( params [ 7 ] );
 const float angle = atan2 ( static_cast < float > ( x ) , static_cast < float > ( y ) ) * ( int ( params [ 8 ] ) + static_cast < float > ( int ( params [ 9 ] ) ) / order );
 const float cs = float ( cos ( angle ) ) , sn = float ( sin ( angle ) );
 const float radial = float ( sqrt ( static_cast < float > ( x ) * x + static_cast < float > ( y ) * y ) - params [ 0 ] );
 const float v = float ( static_cast < float > ( radial ) * cs + static_cast < float > ( z ) * sn );
 const float u = float ( static_cast < float > ( radial ) * sn - static_cast < float > ( z ) * cs );
 const float snap = rint ( ( params [ 13 ] - atan2 ( static_cast < float > ( v ) , static_cast < float > ( u ) ) ) * params [ 11 ] * order ) / order * params [ 12 ];
 const float c = float ( cos ( snap ) ) , s = float ( sin ( snap ) );
 const float a = static_cast < float > ( v ) * c - static_cast < float > ( u ) * s + params [ 10 ];
 const float b = static_cast < float > ( v ) * s + static_cast < float > ( u ) * c;
 float distance = sqrt ( a * a + b * b ) - params [ 5 ];
 if ( ! apply ) distance /= params [ 1 ];
 rout = float ( distance );
 params [ DIFS_COMMIT_XYZ_SLOT ] = apply != 0 ? 1.0f : 0.0f;
 if ( apply != 0 ) varyScale *= params [ 1 ];
 return FormulaVec3 ( x , y , z );

}
 template < > FormulaVec3 execute_formula < FORMULA_KNOTSIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float varyScale = 1.0f;
 return execute_formula_dIFS < FORMULA_KNOTSIFS > ( p , params , varyScale , de );

}
 FormulaVec3 KnotsIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_KNOTSIFS > ( p , params , de );

}

}}
