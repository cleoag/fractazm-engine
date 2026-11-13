// Generated from 3Da/HalfOctIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10026 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_HALFOCTIFS = 10026;
 template < > FormulaVec3 execute_formula < FORMULA_HALFOCTIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 RotMatrix3 R1 = rotMatrix3FromFloats ( params , 4 );
 RotMatrix3 R2 = rotMatrix3FromFloats ( params , 13 );
 FormulaVec3 r1v = R1 * p;
 float sB = r1v . x;
 float sC = r1v . y;
 float sA = r1v . z;
 if ( sB < sC ) {
 float t = sB;
 sB = sC;
 sC = t;

}
 float neg_sB = - sB;
 if ( neg_sB >= sC ) {
 float t = neg_sB;
 neg_sB = sC;
 sC = t;

}
 sB = - neg_sB;
 float fa , fb , fc;
 if ( sB >= sA ) {
 fa = sC;
 fb = - sA;
 fc = sB;

}
 else {
 fa = sC;
 fb = - sB;
 fc = sA;

}
 FormulaVec3 r2v = R2 * FormulaVec3 ( fb , fa , fc );
 de *= Scale;
 float k = Scale - 1.0f;
 return r2v * Scale - k * FormulaVec3 ( CScaleX , CScaleY , CScaleZ );

}
 FormulaVec3 HalfOctIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_HALFOCTIFS > ( p , params , de );

}

}}
