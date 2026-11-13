// Generated from dIFSa/transLogSpIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10253 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TRANSLOGSPIFS = 10253;
 template < > FormulaVec3 execute_formula < FORMULA_TRANSLOGSPIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float input [ ] = {
 static_cast < float > ( p . x ) + params [ 0 ] , static_cast < float > ( p . y ) + params [ 1 ] , static_cast < float > ( p . z ) + params [ 2 ]
}
;
 float xyz [ 3 ];
 for ( int row = 0;
 row < 3;
 ++ row ) {
 thread const float * m = params + 3 + row * 3;
 xyz [ row ] = float ( ( input [ 0 ] * m [ 0 ] + input [ 1 ] * m [ 1 ] ) + input [ 2 ] * m [ 2 ] );

}
 float radius = sqrt ( static_cast < float > ( xyz [ 0 ] ) * xyz [ 0 ] + static_cast < float > ( xyz [ 1 ] ) * xyz [ 1 ] ) * params [ 15 ];
 const float stored_radius = float ( radius );
 const float logarithm = float ( params [ 14 ] * log2 ( radius ) * params [ 16 ] );
 const float angle = float ( atan2 ( static_cast < float > ( xyz [ 1 ] ) , static_cast < float > ( xyz [ 0 ] ) ) * params [ 16 ] );
 const float bend = params [ 12 ] , arms = params [ 13 ];
 const float a = bend * logarithm + arms * angle;
 const float b = arms * logarithm - bend * angle;
 xyz [ 0 ] = float ( ( rint ( a ) - a ) * params [ 17 ] );
 xyz [ 1 ] = float ( ( rint ( b ) - b ) * params [ 17 ] );
 de = float ( ( ( sqrt ( static_cast < float > ( stored_radius ) ) + params [ 18 ] ) / de ) * sqrt ( arms * arms + bend * bend + static_cast < float > ( params [ 14 ] ) * params [ 14 ] ) * params [ 19 ] );
 return FormulaVec3 ( xyz [ 0 ] , xyz [ 1 ] , xyz [ 2 ] );

}

}}
