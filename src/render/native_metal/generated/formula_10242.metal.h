// Generated from dIFSa/TilingIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10242 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TILINGIFS = 10242;
 template < > FormulaVec3 execute_formula < FORMULA_TILINGIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int first = int ( params [ 7 ] ) & 1;
 const int third = 2 - 2 * first;
 float axes [ ] = {
 p . x , p . y , p . z
}
;
 axes [ third ] -= params [ 0 ];
 for ( int lane = 0;
 lane < 2;
 ++ lane ) {
 float cell = ( axes [ first + lane ] - params [ 2 - lane ] ) / params [ 4 - lane ];
 const float nearest = metal :: rint ( cell );
 const int index = nearest >= - 2147483648.0f && nearest <= 2147483647.0f ? static_cast < int > ( nearest ) : INT32_MIN;
 axes [ first + lane ] = ( cell - float ( index ) ) * params [ 4 - lane ] + params [ 6 - lane ];

}
 return FormulaVec3 ( axes [ 0 ] , axes [ 1 ] , axes [ 2 ] );

}

}}
