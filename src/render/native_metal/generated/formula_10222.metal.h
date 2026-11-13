// Generated from dIFS/gnarlyIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10222 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_GNARLYIFS = 10222;
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
 template < > FormulaVec3 execute_formula < FORMULA_GNARLYIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 if ( ( int ( params [ 8 ] ) & 1 ) == 0 ) {
 Arm64ApplyIFSState state ( p , params );
 using E = Arm64ApplyIFSState :: E;
 const E x ( state . transformed . x ) , y ( state . transformed . y ) , z ( state . transformed . z );
 const E threshold = ( abs ( E ( params [ 5 ] ) ) + E ( params [ 14 ] ) ) * E ( params [ 13 ] ) + E ( params [ DIFS_ROLD_SLOT ] );
 if ( ! ( threshold >= abs ( z ) ) ) return state . finish ( - z , de );
 const auto sine = numeric :: extendedSin;
 const E beta ( params [ 0 ] ) , alpha ( params [ 9 ] );
 const E stepX ( params [ 10 ] ) , stepY ( params [ 11 ] );
 E a = x , b = y;
 long remaining = static_cast < int > ( params [ 7 ] );
 FormulaWorkBudget work ( params , "gnarlyIFS" );
 do {
 if ( ! work . beforeStep ( ) ) return NAN;
 const E nextA = sine ( sine ( ( sine ( a * beta ) + a ) * alpha ) + a ) * stepX + b;
 const E nextB = sine ( sine ( ( sine ( b * beta ) + b ) * alpha ) + b ) * stepY + a;
 a = nextA;
 b = nextB;

}
 while ( -- remaining > 0 );
 const E difference = sqrt ( a * a + b * b ) + ( - sqrt ( y * y + x * x ) );
 E trap = difference;
 if ( E ( 0 ) >= E ( params [ 12 ] ) ) trap = sqrt ( abs ( ( trap + E ( 1 ) ) + E ( 1 ) ) );
 params [ MAX_FORMULA_PARAMS - 1 ] = static_cast < float > ( abs ( trap * E ( params [ 12 ] ) ) );
 return state . finish ( difference * E ( params [ 5 ] ) - z , de );

}
 const float Beta = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Amplitude = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const int Iters = int ( params [ 7 ] );
 const int Flip_XZ = int ( params [ 8 ] );
 const float Alpha = params [ 9 ];
 const float StepX = params [ 10 ];
 const float StepY = params [ 11 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 float state_x = p . x;
 float state_y = p . y;
 float state_z = p . z;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;
 state_x = x;
 state_y = y;
 state_z = z;

}
 float h_var = ( Flip_XZ & 1 ) != 0 ? x : z;
 float other_var = ( Flip_XZ & 1 ) != 0 ? z : x;
 float bound = ( fabs ( Amplitude ) + 0.1f ) * 1.5f + params [ MAX_FORMULA_PARAMS - 6 ];
 if ( fabs ( h_var ) > bound ) {
 float shape_de = - h_var;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( state_x , state_y , state_z );

}
 float a = other_var;
 float b = y;
 for ( int i = 0;
 i < ( Iters > 0 ? Iters : 1 );
 i ++ ) {
 float t1 = sin ( a * Beta );
 t1 = ( t1 + a ) * Alpha;
 t1 = sin ( t1 ) + a;
 t1 = sin ( t1 ) * StepX + b;
 float t2 = sin ( b * Beta );
 t2 = ( t2 + b ) * Alpha;
 t2 = sin ( t2 ) + b;
 t2 = sin ( t2 ) * StepY + a;
 a = t1;
 b = t2;

}
 float gnarl_r = sqrt ( a * a + b * b );
 float orig_r = sqrt ( y * y + other_var * other_var );
 float height_diff = gnarl_r - orig_r;
 float shape_de = height_diff * Amplitude - h_var;
 float OTrapClrMul = params [ 12 ];
 float otrap_val = height_diff;
 if ( OTrapClrMul <= 0.0f ) {
 otrap_val = sqrt ( fabs ( height_diff + 2.0f ) );

}
 params [ MAX_FORMULA_PARAMS - 1 ] = fabs ( otrap_val * OTrapClrMul );
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( state_x , state_y , state_z );

}

}}
