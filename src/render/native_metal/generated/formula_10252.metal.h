// Generated from dIFSa/amazingIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10252 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_AMAZINGIFS = 10252;
 template < > FormulaVec3 execute_formula < FORMULA_AMAZINGIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Add_X = params [ 0 ];
 const float Add_Y = params [ 1 ];
 const float Add_Z = params [ 2 ];
 RotMatrix3 RM = rotMatrix3FromFloats ( params , 3 );
 const float Fold_X = params [ 12 ];
 const float Fold_Y = params [ 13 ];
 const float Fold_Z = params [ 14 ];
 const float Inv_MinR = params [ 15 ];
 float x = p . x;
 float y = p . y;
 float z = p . z;
 z = abs ( z + Fold_X ) - abs ( z - Fold_X ) - z;
 y = abs ( y + Fold_Y ) - abs ( y - Fold_Y ) - y;
 x = abs ( x + Fold_Z ) - abs ( x - Fold_Z ) - x;
 z += Add_Z;
 y += Add_Y;
 x += Add_X;
 FormulaVec3 rv = RM * FormulaVec3 ( x , y , z );
 float rx = rv . x;
 float ry = rv . y;
 float rz = rv . z;
 float r2 = rx * rx + ry * ry + rz * rz;
 float factor = 1.0f / r2;
 if ( factor > 1.0f ) {
 if ( factor > Inv_MinR ) {
 factor = Inv_MinR;

}
 rx *= factor;
 ry *= factor;
 rz *= factor;
 de = abs ( de * factor );

}
 return FormulaVec3 ( rx , ry , rz );

}
 FormulaVec3 amazingIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_AMAZINGIFS > ( p , params , de );

}

}}
