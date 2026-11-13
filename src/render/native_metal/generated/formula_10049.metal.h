// Generated from 3Da/Mandalex.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10049 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MANDALEX = 10049;
 static inline void linearPull ( thread float & x , thread float & y , thread float & z , float threshold , float effect , int fiddler ) {
 int count = 0;
 if ( fabs ( x ) < threshold ) count ++;
 if ( fabs ( y ) < threshold ) count ++;
 if ( fabs ( z ) < threshold ) count ++;
 if ( ( count ^ ( fiddler & 3 ) ) != 0 ) return;
 float pull = threshold * effect;
 x = ( x <= 0.0f ) ? ( x + pull ) : ( x - pull );
 y = ( y <= 0.0f ) ? ( y + pull ) : ( y - pull );
 z = ( z <= 0.0f ) ? ( z + pull ) : ( z - pull );

}
 template < > FormulaVec3 execute_formula < FORMULA_MANDALEX > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 struct NativeClosure0 {
 thread decltype ( p ) & p;
 thread decltype ( params ) & params;
 auto operator ( ) ( float threshold ) const {
 int count = ! ( abs ( E ( p . x ) ) >= threshold ) + int ( ! ( abs ( E ( p . y ) ) >= threshold ) ) + int ( ! ( abs ( E ( p . z ) ) >= threshold ) );
 if ( ( count ^ ( int ( params [ 16 ] ) & 3 ) ) != 0 ) return;
 const E amount = E ( threshold ) * params [ 15 ];
 struct NativeClosure1 {
 thread decltype ( amount ) & amount;
 auto operator ( ) ( float v ) const {
 return float ( E ( v ) - ( ( E ( 0 ) >= E ( v ) ) ? - amount : amount ) );

}

}
;
 auto axis = NativeClosure1 {
 amount
}
;
 p = FormulaVec3 ( axis ( p . x ) , axis ( p . y ) , axis ( p . z ) );

}

}
;
 auto pull = NativeClosure0 {
 p , params
}
;
 struct NativeClosure2 {
 thread decltype ( params ) & params;
 auto operator ( ) ( thread E & x , thread E & y , thread E & z ) const {
 const E nx = ( z * params [ 10 ] + y * params [ 11 ] ) + x * params [ 12 ];
 const E ny = ( z * params [ 7 ] + y * params [ 8 ] ) + x * params [ 9 ];
 const E nz = ( z * params [ 4 ] + y * params [ 5 ] ) + x * params [ 6 ];
 x = nx;
 y = ny;
 z = nz;

}

}
;
 auto rotate = NativeClosure2 {
 params
}
;
 pull ( params [ 13 ] );
 E x = p . x , y = p . y , z = p . z;
 rotate ( x , y , z );
 x = float ( x );
 y = float ( y );
 z = float ( z );
 const E fold = params [ 3 ] , twice = fold + fold;
 struct NativeClosure3 {
 thread decltype ( fold ) & fold;
 thread decltype ( twice ) & twice;
 auto operator ( ) ( E v ) const {
 if ( v >= fold ) return - v + twice;
 if ( ! ( v >= - fold ) ) return - v - twice;
 return v;

}

}
;
 auto box = NativeClosure3 {
 fold , twice
}
;
 x = box ( x );
 y = box ( y );
 z = box ( z );
 rotate ( x , y , z );
 const E rr = ( z * z + y * y ) + x * x;
 const E m = native_box_inner_unordered ( float ( rr ) , params ) ? E ( native_aboxmod1_inner_scale ( params ) ) : E ( 1 ) >= rr ? E ( params [ 0 ] ) / rr : E ( params [ 0 ] );
 de = float ( E ( de ) * m );
 p = FormulaVec3 ( float ( x * m + params [ 17 ] ) , float ( y * m + params [ 18 ] ) , float ( z * m + params [ 19 ] ) );
 pull ( params [ 14 ] );
 return p;

}

}}
