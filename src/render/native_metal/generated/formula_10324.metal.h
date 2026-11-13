// Generated from 4Da/ABoxSphereOffset4d.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10324 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXSPHEREOFFSET4D = 10324;
 template < > FormulaVec3 execute_formula < FORMULA_ABOXSPHEREOFFSET4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float fold = params [ 3 ];
 struct NativeClosure0 {
 decltype ( fold ) fold;
 auto operator ( ) ( float v ) const {
 return fabs ( v + fold ) - ( fabs ( v - fold ) + v );

}

}
;
 auto box = NativeClosure0 {
 fold
}
;
 const float x = box ( p . x ) - params [ 4 ];
 const float y = box ( p . y ) - params [ 5 ];
 const float z = box ( p . z ) - params [ 6 ];
 const float w = box ( de ) - params [ 7 ];
 const float rr = ( ( z * z + y * y ) + x * x ) + w * w;
 const float m = native_box_inner_unordered ( rr , params ) ? native_aboxmod1_inner_scale ( params ) : ( rr > 1.f ? params [ 0 ] : params [ 0 ] / rr );
 params [ FORMULA_STATE_X_SLOT ] *= m;
 de = ( ( w * m + params [ 8 ] ) + params [ 7 ] ) + params [ 12 ];
 return FormulaVec3 ( ( x * m + params [ 9 ] ) + params [ 4 ] , ( y * m + params [ 10 ] ) + params [ 5 ] , ( z * m + params [ 11 ] ) + params [ 6 ] );

}

}}
