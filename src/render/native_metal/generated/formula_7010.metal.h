// Generated from Ads/_SinY.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7010 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SINY = 7010;
 template < > FormulaVec3 execute_formula < FORMULA__SINY > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int Index = native_int32_parameter ( params , 0 );
 const float Offset1 = params [ 1 ];
 const float Scale1 = params [ 2 ];
 const float Scale2 = params [ 3 ];
 const float Offset2 = params [ 4 ];
 float coord;
 switch ( Index & 3 ) {
 case 0 : coord = p . x;
 break;
 case 1 : coord = p . y;
 break;
 case 2 : coord = p . z;
 break;
 default : coord = params [ FORMULA_STATE_W_SLOT ];
 break;

}
 float result = sin ( ( coord - Offset1 ) * Scale1 ) * Scale2 + Offset2;
 switch ( Index & 3 ) {
 case 0 : return FormulaVec3 ( result , p . y , p . z );
 case 1 : return FormulaVec3 ( p . x , result , p . z );
 case 2 : return FormulaVec3 ( p . x , p . y , result );
 default : params [ FORMULA_STATE_W_SLOT ] = result;
 return p;

}

}
 FormulaVec3 _SinY_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA__SINY > ( p , params , de );

}

}}
