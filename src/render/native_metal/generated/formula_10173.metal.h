// Generated from 3Da/ASurfMod1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10173 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ASURFMOD1 = 10173;
 template < > FormulaVec3 execute_formula < FORMULA_ASURFMOD1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float Fold = params [ 3 ];
 const float Scale_vary = params [ 4 ];
 const float FoldXMod = params [ 5 ];
 const float FoldYMod = params [ 6 ];
 const float InvRadius = params [ 7 ];
 RotMatrix3 RM = rotMatrix3FromFloats ( params , 8 );
 const float R_00 = RM . col0 . x , R_01 = RM . col1 . x , R_02 = RM . col2 . x;
 const float R_10 = RM . col0 . y , R_11 = RM . col1 . y , R_12 = RM . col2 . y;
 const float R_20 = RM . col0 . z , R_21 = RM . col1 . z , R_22 = RM . col2 . z;
 const float J1 = params [ 17 ];
 const float J2 = params [ 18 ];
 const float J3 = params [ 19 ];
 float prevVaryScale = params [ DIFS_VARYSCALE_SLOT ];
 if ( native_formula_counter ( params ) <= 0.0f ) {
 prevVaryScale = Scale;
 native_increment_formula_counter ( params );

}
 float VaryScale = prevVaryScale + Scale_vary * ( abs ( prevVaryScale ) - 1.0f );
 params [ DIFS_VARYSCALE_SLOT ] = VaryScale;
 float x = Fold - abs ( abs ( p . x + FoldXMod ) - Fold ) - abs ( FoldXMod );
 float y = Fold - abs ( abs ( p . y + FoldYMod ) - Fold ) - abs ( FoldYMod );
 float z = p . z;
 float rr = ( x * x + y * y + z * z ) * InvRadius;
 float m;
 if ( native_aboxmod1_inner ( rr , params ) ) {
 m = native_aboxmod1_inner_scale ( params );

}
 else if ( rr >= 1.0f ) {
 m = VaryScale;

}
 else {
 m = VaryScale / rr;

}
 de = de * m;
 x *= m;
 y *= m;
 z *= m;
 float rx = x * R_00 + y * R_01 + z * R_02;
 float ry = x * R_10 + y * R_11 + z * R_12;
 float rz = x * R_20 + y * R_21 + z * R_22;
 rx += J2;
 ry += J1;
 rz += J3;
 return FormulaVec3 ( rx , ry , rz );

}

}}
