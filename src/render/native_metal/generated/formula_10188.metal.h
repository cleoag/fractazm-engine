// Generated from 3Da/ABoxVSShapes.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10188 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXVSSHAPES = 10188;
 template < > FormulaVec3 execute_formula < FORMULA_ABOXVSSHAPES > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 E previous ( params [ DIFS_VARYSCALE_SLOT ] );
 if ( native_formula_counter ( params ) <= 0.0f ) {
 previous = E ( params [ 0 ] );
 native_increment_formula_counter ( params );

}
 const float vary = static_cast < float > ( previous + ( abs ( previous ) - E ( 1 ) ) * E ( params [ 4 ] ) );
 params [ DIFS_VARYSCALE_SLOT ] = vary;
 const E fold ( params [ 3 ] ) , twice = fold + fold;
 struct NativeClosure0 {
 thread decltype ( fold ) & fold;
 thread decltype ( twice ) & twice;
 auto operator ( ) ( E v ) const {
 return v >= fold ? - v + twice : ( ! ( v >= - fold ) ? - v - twice : v );

}

}
;
 auto box = NativeClosure0 {
 fold , twice
}
;
 const E x = box ( E ( p . x ) ) , y = box ( E ( p . y ) ) , z = box ( E ( p . z ) );
 const E xx = x * x , yy = y * y , zz = z * z;
 const E a ( params [ 6 ] ) , b ( params [ 7 ] );
 const int mode = int ( params [ 5 ] );
 E rr;
 switch ( mode ) {
 case 1 : {
 const E r = sqrt ( zz + yy ) - a;
 rr = ( r * r + xx ) * b;
 break;

}
 case 2 : case 3 : {
 const E axis = abs ( x );
 const E cap = a >= axis ? ( a - axis ) * ( a - axis ) : E ( 0 );
 const E radial = ( zz + yy ) * b;
 rr = mode == 2 ? ( radial >= cap ? radial : cap ) : ( radial >= cap ? cap : radial );
 break;

}
 case 4 : case 5 : case 6 : case 7 : {
 const E axis = mode <= 5 ? abs ( x ) : xx;
 const E cap = a >= axis ? a - axis : E ( 1 );
 const E radial = ( mode <= 5 ? zz + yy : ( yy >= zz ? yy : zz ) ) * b;
 const bool divide = ( mode == 4 || mode == 6 ) ? cap >= radial : ! ( cap >= radial );
 rr = divide ? radial / cap : radial;
 break;

}
 case 8 : rr = ( xx * ( xx - a ) + ( zz + yy ) ) * b;
 break;
 case 9 : {
 const E quartic ( static_cast < float > ( ( xx * xx + yy * yy ) + zz * zz ) );
 rr = ( ( xx + yy ) + zz ) * b + quartic + a;
 break;

}
 case 10 : rr = ( ( xx * xx + yy * yy ) + zz * zz ) * b - a;
 break;
 case 11 : {
 const E sx ( static_cast < float > ( xx ) ) , sy ( static_cast < float > ( yy ) );
 const E sz ( static_cast < float > ( zz ) );
 rr = ( ( ( ( ( x * y ) * z ) * E ( params [ 8 ] ) + ( sx * sy ) * sz ) + ( ( sx * sy + sx * sz ) + sz * sy ) * a ) + ( ( sx + sy ) + sz ) * a * a + b ) * E ( params [ 9 ] );
 break;

}
 case 12 : {
 E r = ( - x ) * a + sqrt ( zz + yy ) * b;
 if ( E ( 0 ) >= r ) r = E ( 0 );
 rr = r * r;
 break;

}
 case 13 : rr = ( ( E ( 1 ) >= - x ? - x : E ( 1 ) ) * a + ( zz + yy ) ) * b;
 break;
 case 14 : {
 const E u = xx * a + yy , v = xx * a + zz;
 rr = ( v >= u ? v : u ) * b;
 break;

}
 case 15 : {
 const E axis = abs ( x );
 const E cap = axis >= a ? ( axis - a ) * ( axis - a ) : E ( 0 );
 rr = ( zz + yy ) * b + cap;
 break;

}
 case 16 : {
 const E xm = x - a , ym = y - a , zm = z - a , xp = x + a , yp = y + a , zp = z + a;
 const E d1 ( static_cast < float > ( ( zz + ym * ym ) + xm * xm ) );
 const E d2 ( static_cast < float > ( ( zm * zm + yy ) + xm * xm ) );
 const E d3 ( static_cast < float > ( ( zm * zm + ym * ym ) + xx ) );
 const E d4 ( static_cast < float > ( ( zp * zp + yp * yp ) + xp * xp ) );
 rr = d2 >= d1 ? d2 : d1;
 rr = d3 >= rr ? d3 : rr;
 rr = d4 >= rr ? d4 : rr;
 rr *= b;
 break;

}
 case 17 : {
 E axis = abs ( x );
 axis += axis;
 axis += axis;
 const E ratio = ( ( ( xx + xx ) + yy ) + zz ) / axis;
 rr = ( ratio * ratio ) * b + a;
 break;

}
 default : rr = z;
 break;

}
 const E scale = ! ( rr >= E ( params [ 2 ] ) ) ? E ( params [ 1 ] ) : ( E ( 1 ) >= rr ? E ( vary ) / rr : E ( vary ) );
 de = static_cast < float > ( E ( de ) * scale );
 if ( mode < 1 || mode > 17 ) return FormulaVec3 ( static_cast < float > ( numeric :: extendedNaN ( ) ) , static_cast < float > ( x * scale + E ( params [ 12 ] ) ) , static_cast < float > ( y * scale + E ( params [ 13 ] ) ) );
 return FormulaVec3 ( static_cast < float > ( x * scale + E ( params [ 11 ] ) ) , static_cast < float > ( y * scale + E ( params [ 12 ] ) ) , static_cast < float > ( z * scale + E ( params [ 13 ] ) ) );

}
 FormulaVec3 ABoxVSShapes_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ABOXVSSHAPES > ( p , params , de );

}

}}
