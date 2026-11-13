// Generated from dIFSa/foldinghexIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10314 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_FOLDINGHEXIFS = 10314;
 template < > FormulaVec3 execute_formula < FORMULA_FOLDINGHEXIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float GridStyling = params [ 0 ];
 const float c_half = 0.5f;
 const float c_invrt3 = 0.57735f;
 const float c_neg866 = - 0.866025f;
 const float c_neghalf = - 0.5f;
 const float c_hex = 0.4330125f;
 float x = p . x;
 float y = p . y;
 x = fabs ( x - GridStyling );
 float t = round ( x / GridStyling * c_half - c_half );
 t = t * 2.0f + 1.0f;
 t = t * GridStyling;
 x = fabs ( x - t );
 float yscale = GridStyling * c_invrt3;
 y = fabs ( y - yscale );
 float t2 = round ( y / yscale * c_half - c_half );
 t2 = t2 * 2.0f + 1.0f;
 t2 = t2 * yscale;
 y = fabs ( y - t2 );
 for ( int iter = 0;
 iter < 2;
 iter ++ ) {
 float v = x * c_neg866 + y * c_neghalf + c_hex;
 float fold = v - fabs ( v );
 x = fabs ( x - fold * c_neg866 );
 y = fabs ( y - fold * c_neghalf );

}
 return FormulaVec3 ( x , y , p . z );

}
 FormulaVec3 foldinghexIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_FOLDINGHEXIFS > ( p , params , de );

}

}}
