// Generated from dIFS/hextgridIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10232 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_HEXTGRIDIFS = 10232;
 template < > FormulaVec3 execute_formula_dIFS < FORMULA_HEXTGRIDIFS > ( FormulaVec3 p , FormulaParams params , thread float & varyScale , thread float & rout ) {
 const float scale = params [ 1 ];
 const int apply = int ( params [ 6 ] );
 FormulaVec3 q ( p . x * scale + params [ 4 ] , p . y * scale + params [ 3 ] , p . z * scale + params [ 2 ] );
 params [ DIFS_COMMIT_XYZ_SLOT ] = apply != 0 ? 1.0f : 0.0f;
 if ( apply != 0 ) varyScale *= scale;
 float v [ 3 ];
 for ( int row = 0;
 row < 3;
 ++ row ) {
 thread const float * m = params + 7 + row * 3;
 v [ row ] = float ( ( static_cast < float > ( q . x ) * m [ 0 ] + static_cast < float > ( q . y ) * m [ 1 ] ) + static_cast < float > ( q . z ) * m [ 2 ] );

}
 const float x = v [ 0 ] , y = v [ 1 ] , z = v [ 2 ];
 float positive = float ( abs ( z * z * params [ 16 ] + params [ 0 ] ) ) , negative = 0.0f;
 struct NativeClosure0 {
 thread decltype ( negative ) & negative;
 thread decltype ( positive ) & positive;
 auto operator ( ) ( float term ) const {
 if ( term >= 0.0f ) positive = float ( term + positive );
 else negative = float ( negative - term );

}

}
;
 auto accumulate = NativeClosure0 {
 negative , positive
}
;
 accumulate ( cos ( y * params [ 18 ] ) );
 accumulate ( cos ( x * params [ 20 ] - y * params [ 19 ] ) );
 accumulate ( cos ( x * params [ 20 ] + y * params [ 19 ] ) );
 accumulate ( cos ( x * params [ 21 ] ) * params [ 5 ] );
 accumulate ( cos ( y * params [ 17 ] - x * params [ 20 ] ) * params [ 5 ] );
 accumulate ( cos ( x * params [ 20 ] + y * params [ 17 ] ) * params [ 5 ] );
 float distance = sqrt ( static_cast < float > ( positive ) ) - sqrt ( static_cast < float > ( negative ) );
 if ( ! apply ) distance /= scale;
 rout = float ( distance );
 return q;

}
 template < > FormulaVec3 execute_formula < FORMULA_HEXTGRIDIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float varyScale = 1.0f;
 return execute_formula_dIFS < FORMULA_HEXTGRIDIFS > ( p , params , varyScale , de );

}
 FormulaVec3 hextgridIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_HEXTGRIDIFS > ( p , params , de );

}

}}
