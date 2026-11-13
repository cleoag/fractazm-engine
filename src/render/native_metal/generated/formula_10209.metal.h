// Generated from dIFS/ducksIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10209 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_DUCKSIFS = 10209;
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
 template < > FormulaVec3 execute_formula < FORMULA_DUCKSIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 if ( ( int ( params [ 8 ] ) & 1 ) == 0 ) {
 Arm64ApplyIFSState state ( p , params );
 using E = Arm64ApplyIFSState :: E;
 const E x ( state . transformed . x ) , y ( state . transformed . y ) , z ( state . transformed . z );
 const E threshold = ( abs ( E ( params [ 5 ] ) ) + E ( params [ 16 ] ) ) * E ( params [ 15 ] ) + E ( params [ DIFS_ROLD_SLOT ] );
 if ( ! ( threshold >= abs ( z ) ) ) return state . finish ( - z , de );
 struct NativeClosure0 {
 auto operator ( ) ( E v ) const {
 const E infinity ( native_std :: numeric_limits < float > :: infinity ( ) );
 if ( v == E ( 0 ) ) return - infinity;
 if ( v == infinity ) return infinity;
 return numeric :: extendedLn2 ( ) * numeric :: extendedLog2 ( v );

}

}
;
 auto logarithm = NativeClosure0 {

}
;
 E duckP = x , duckQ = y , accum ( 0 );
 const E sine ( static_cast < float > ( params [ 12 ] ) );
 const E cosine ( static_cast < float > ( params [ 11 ] ) );
 const E heightDecay ( params [ 0 ] ) , seedX ( params [ 9 ] ) , seedY ( params [ 10 ] );
 const E logScale ( params [ 14 ] );
 E radiusSquared = duckQ * duckQ + duckP * duckP;
 const int iters = static_cast < int > ( params [ 7 ] );
 long remaining = iters;
 FormulaWorkBudget work ( params , "ducksIFS" );
 do {
 if ( ! work . beforeStep ( ) ) return NAN;
 const E angle = logarithm ( radiusSquared ) * logScale + seedX;
 const E angle2 = numeric :: extendedAtan2 ( abs ( duckQ ) , duckP ) + seedY;
 const E nextP = angle * sine + ( - ( angle2 * cosine ) );
 const E nextQ = angle * cosine + angle2 * sine;
 duckP = nextP;
 duckQ = nextQ;
 radiusSquared = duckQ * duckQ + duckP * duckP;
 accum = accum * heightDecay + sqrt ( radiusSquared );

}
 while ( -- remaining > 0 );
 const E result = logarithm ( accum + E ( 1 ) );
 E trap = result;
 if ( E ( 0 ) >= E ( params [ 13 ] ) ) trap *= trap;
 params [ MAX_FORMULA_PARAMS - 1 ] = static_cast < float > ( abs ( trap * E ( params [ 13 ] ) ) );
 return state . finish ( ( result / E ( iters ) ) * E ( params [ 5 ] ) - z , de );

}
 const float PerIterHeight_mul = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float FinalHeight_mul = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const int Iters = int ( params [ 7 ] );
 const int Flip_XZ = int ( params [ 8 ] );
 const float Seed_X = params [ 9 ];
 const float Seed_Y = params [ 10 ];
 const float rot_sin = params [ 12 ];
 const float rot_cos = params [ 11 ];
 const float OTrapClrMul = params [ 13 ];
 const float K_HALF = 0.5f;
 const float K_CONST2 = 0.03f;
 const float K_CONST3 = 0.1f;
 const float Rold = params [ DIFS_ROLD_SLOT ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;

}
 float h_axis = ( Flip_XZ & 1 ) != 0 ? x : z;
 float d_axis0 = ( Flip_XZ & 1 ) != 0 ? z : x;
 float threshold = ( fabs ( FinalHeight_mul ) + K_CONST3 ) * K_CONST2 + Rold;
 if ( fabs ( h_axis ) > threshold ) {
 float shape_de = - h_axis;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return Apply_scale_add != 0 ? FormulaVec3 ( x , y , z ) : p;

}
 float duck_p = d_axis0;
 float duck_q = y;
 float accum = 0.0f;
 int iters = metal :: max ( 1 , Iters );
 for ( int i = 0;
 i < iters;
 i ++ ) {
 float r2 = duck_q * duck_q + duck_p * duck_p;
 float angle = log ( r2 ) * K_HALF;
 angle += Seed_X;
 float angle2 = atan2 ( fabs ( duck_q ) , duck_p );
 angle2 += Seed_Y;
 float new_p = angle * rot_sin - angle2 * rot_cos;
 float new_q = angle * rot_cos + angle2 * rot_sin;
 duck_p = new_p;
 duck_q = new_q;
 float r = sqrt ( duck_p * duck_p + duck_q * duck_q );
 accum = accum * PerIterHeight_mul + r;

}
 float result = log ( 1.0f + accum );
 float otrap_result;
 if ( OTrapClrMul > 0.0f ) {
 otrap_result = result * OTrapClrMul;

}
 else {
 otrap_result = result * result * OTrapClrMul;

}
 params [ MAX_FORMULA_PARAMS - 1 ] = fabs ( otrap_result );
 result = result / float ( Iters );
 result *= FinalHeight_mul;
 float shape_de = result - h_axis;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return Apply_scale_add != 0 ? FormulaVec3 ( x , y , z ) : p;

}
 FormulaVec3 ducksIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_DUCKSIFS > ( p , params , de );

}

}}
