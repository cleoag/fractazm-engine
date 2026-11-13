// Generated from 3D/ManowarBulb.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10262 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MANOWARBULB = 10262;
 template < > FormulaVec3 execute_formula < FORMULA_MANOWARBULB > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 if ( native_formula_counter ( params ) == 0.0f ) {
 native_formula_counter ( params ) = 1.0f;
 params [ FORMULA_STATE_X_SLOT ] = params [ FORMULA_STATE_Y_SLOT ] = 0.0f;
 params [ FORMULA_STATE_Z_SLOT ] = params [ FORMULA_STATE_W_SLOT ] = 0.0f;
 return p;

}
 float state [ 18 ] = {
 p . x , p . y , p . z , params [ FORMULA_ORBIT_W_SLOT ] , params [ FORMULA_STATE_X_SLOT ] , params [ FORMULA_STATE_Y_SLOT ] , params [ FORMULA_STATE_Z_SLOT ] , params [ FORMULA_STATE_W_SLOT ]
}
;
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 const E x ( p . x ) , y ( p . y ) , z ( p . z );
 const E radius = native_formula_length3 ( z , y , x );
 const float power = static_cast < float > ( numeric :: extendedExp2 ( E ( params [ 0 ] ) * numeric :: extendedLog2 ( radius ) , false ) );
 const auto phi = numeric :: extendedSinCos ( numeric :: extendedAtan2 ( y , x ) * E ( params [ 0 ] ) );
 const auto theta = numeric :: extendedSinCos ( numeric :: extendedAtan2 ( z , sqrt ( x * x + y * y ) ) * E ( params [ 0 ] ) );
 const float cp = static_cast < float > ( phi . cos ) , sp = static_cast < float > ( phi . sin );
 const float ct = static_cast < float > ( theta . cos ) , st = static_cast < float > ( theta . sin );
 const E a = E ( state [ 4 ] ) * E ( params [ 2 ] ) , b = E ( state [ 5 ] ) * E ( params [ 2 ] ) , c = E ( state [ 7 ] ) * E ( params [ 2 ] );
 const float rx = static_cast < float > ( a * E ( params [ 4 ] ) + b * E ( params [ 5 ] ) + c * E ( params [ 6 ] ) + E ( params [ 13 ] ) );
 const float ry = static_cast < float > ( a * E ( params [ 7 ] ) + b * E ( params [ 8 ] ) + c * E ( params [ 9 ] ) + E ( params [ 14 ] ) );
 const float rz = static_cast < float > ( a * E ( params [ 10 ] ) + b * E ( params [ 11 ] ) + c * E ( params [ 12 ] ) + E ( params [ 15 ] ) );
 const float bx = static_cast < float > ( x * E ( params [ 3 ] ) + E ( rx ) );
 const float by = static_cast < float > ( y * E ( params [ 3 ] ) + E ( ry ) );
 const float bz = static_cast < float > ( z * E ( params [ 3 ] ) + E ( rz ) );
 state [ 0 ] = static_cast < float > ( E ( cp ) * E ( ct ) * E ( power ) + E ( bx ) );
 state [ 1 ] = static_cast < float > ( E ( sp ) * E ( ct ) * E ( power ) + E ( by ) );
 state [ 2 ] = static_cast < float > ( E ( st ) * E ( power ) * E ( params [ 1 ] ) + E ( bz ) );
 state [ 4 ] = p . x;
 state [ 5 ] = p . y;
 state [ 6 ] = p . z;
 state [ 7 ] = state [ 3 ];
 params [ FORMULA_STATE_X_SLOT ] = state [ 4 ];
 params [ FORMULA_STATE_Y_SLOT ] = state [ 5 ];
 params [ FORMULA_STATE_Z_SLOT ] = state [ 6 ];
 params [ FORMULA_STATE_W_SLOT ] = state [ 7 ];
 return FormulaVec3 ( state [ 0 ] , state [ 1 ] , state [ 2 ] );

}

}}
