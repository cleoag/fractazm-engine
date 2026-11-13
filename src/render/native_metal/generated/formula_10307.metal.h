// Generated from dIFS/cayleySurfIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10307 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_CAYLEYSURFIFS = 10307;
 template < > FormulaVec3 execute_formula < FORMULA_CAYLEYSURFIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float scale = params [ 1 ];
 const int apply = int ( params [ 6 ] );
 FormulaVec3 transformed ( p . x * scale + params [ 4 ] , p . y * scale + params [ 3 ] , p . z * scale + params [ 2 ] );
 const numeric :: X87Scope extendedScope;
 float v [ 3 ];
 for ( int row = 0;
 row < 3;
 ++ row ) {
 thread const float * m = params + 7 + row * 3;
 v [ row ] = float ( ( static_cast < numeric :: X87Real > ( transformed . x ) * m [ 0 ] + static_cast < numeric :: X87Real > ( transformed . y ) * m [ 1 ] ) + static_cast < numeric :: X87Real > ( transformed . z ) * m [ 2 ] );

}
 float positive = 0.0f , negative = 0.0f;
 struct NativeClosure0 {
 auto operator ( ) ( numeric :: X87Real term , thread float & pos , thread float & neg ) const {
 if ( term >= 0 ) pos = float ( term + pos );
 else neg = float ( neg - term );

}

}
;
 auto accumulate = NativeClosure0 {

}
;
 const int cubic [ 6 ] [ 2 ] = {
 {
 0 , 1
}
 , {
 0 , 2
}
 , {
 1 , 0
}
 , {
 1 , 2
}
 , {
 2 , 1
}
 , {
 2 , 0
}

}
;
 for ( thread const auto & axes : cubic ) accumulate ( ( static_cast < numeric :: X87Real > ( v [ axes [ 0 ] ] ) * v [ axes [ 0 ] ] ) * v [ axes [ 1 ] ] , positive , negative );
 positive = float ( numeric :: x87Abs ( static_cast < numeric :: X87Real > ( positive ) * params [ 5 ] ) );
 negative = float ( numeric :: x87Abs ( static_cast < numeric :: X87Real > ( negative ) * params [ 5 ] ) );
 const int quadratic [ 3 ] [ 2 ] = {
 {
 0 , 1
}
 , {
 0 , 2
}
 , {
 1 , 2
}

}
;
 for ( thread const auto & axes : quadratic ) {
 numeric :: X87Real term = static_cast < numeric :: X87Real > ( v [ axes [ 0 ] ] ) * v [ axes [ 1 ] ];
 accumulate ( term + term , negative , positive );

}
 numeric :: X87Real surface = ( numeric :: x87Sqrt ( numeric :: x87Sqrt ( static_cast < numeric :: X87Real > ( positive ) ) ) - numeric :: x87Sqrt ( numeric :: x87Sqrt ( static_cast < numeric :: X87Real > ( negative ) ) ) ) * params [ 16 ];
 numeric :: X87Real sphere = numeric :: x87Sqrt ( static_cast < numeric :: X87Real > ( v [ 0 ] ) * v [ 0 ] + ( static_cast < numeric :: X87Real > ( v [ 1 ] ) * v [ 1 ] + static_cast < numeric :: X87Real > ( v [ 2 ] ) * v [ 2 ] ) ) - params [ 0 ];
 numeric :: X87Real result = sphere >= surface ? sphere : surface;
 if ( ! apply ) result /= scale;
 de = float ( result );
 if ( ! apply ) return p;
 params [ DIFS_VARYSCALE_SLOT ] *= scale;
 return transformed;

}
 FormulaVec3 cayleySurfIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_CAYLEYSURFIFS > ( p , params , de );

}

}}
