// Generated from 3Da/ABoxMod1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8024 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXMOD1 = 8024;
 template < > FormulaVec3 execute_formula < FORMULA_ABOXMOD1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float Fold = params [ 3 ];
 const float Scale_vary = params [ 4 ];
 const float FoldXMod = params [ 5 ];
 const float FoldYMod = params [ 6 ];
 const float FoldZMod = params [ 7 ];
 float VaryScale = params [ DIFS_VARYSCALE_SLOT ];
 if ( native_formula_counter ( params ) <= 0.0f ) {
 VaryScale = Scale;
 native_increment_formula_counter ( params );

}
 VaryScale += Scale_vary * ( abs ( VaryScale ) - 1.0f );
 params [ DIFS_VARYSCALE_SLOT ] = VaryScale;
 float x = Fold - abs ( abs ( p . x + FoldXMod ) - Fold ) - abs ( FoldXMod );
 float y = Fold - abs ( abs ( p . y + FoldYMod ) - Fold ) - abs ( FoldYMod );
 float z = Fold - abs ( abs ( p . z + FoldZMod ) - Fold ) - abs ( FoldZMod );
 float rr = x * x + y * y + z * z;
 float m;
 if ( native_aboxmod1_inner ( rr , params ) ) {
 m = native_aboxmod1_inner_scale ( params );

}
 else if ( rr < 1.0f ) {
 m = VaryScale / rr;

}
 else {
 m = VaryScale;

}
 de = de * m;
 FormulaVec3 result;
 result . x = x * m + params [ 9 ];
 result . y = y * m + params [ 8 ];
 result . z = z * m + params [ 10 ];
 return result;

}
 FormulaVec3 ABoxMod1_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ABOXMOD1 > ( p , params , de );

}

}}
