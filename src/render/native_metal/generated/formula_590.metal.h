// Generated from 3Da/SierpHilbert.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_590 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_SIERPHILBERT = 590;
 template < > FormulaVec3 execute_formula < FORMULA_SIERPHILBERT > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 const float Edge1 = params [ 22 ];
 const float Edge2 = params [ 23 ];
 RotMatrix3 R1 = rotMatrix3FromFloats ( params , 4 );
 RotMatrix3 R2 = rotMatrix3FromFloats ( params , 13 );
 FormulaVec3 r1 = R1 * p;
 float rx = r1 . x;
 float ry = r1 . y;
 float rz = r1 . z;
 float sa = - rx;
 float sb = ry;
 float sc = rz;
 if ( sa >= sb ) {
 float t = sa;
 sa = sb;
 sb = t;

}
 if ( sa >= sc ) {
 float t = sa;
 sa = sc;
 sc = t;

}
 sa = - sa;
 {
 float t = sa;
 sa = sb;
 sb = t;

}
 sa = - sa;
 if ( sa >= sc ) {
 float t = sa;
 sa = sc;
 sc = t;

}
 sa = - sa;
 {
 float t = sa;
 sa = sb;
 sb = t;

}
 if ( Edge1 > 0.0f ) {
 sb = Edge1 - abs ( Edge1 - sb );

}
 if ( Edge2 > 0.0f ) {
 sa = Edge2 - abs ( Edge2 - sa );

}
 FormulaVec3 r2 = R2 * FormulaVec3 ( sa , sb , sc );
 float r2x = r2 . x;
 float r2y = r2 . y;
 float r2z = r2 . z;
 de = de * Scale;
 float k = Scale - 1.0f;
 FormulaVec3 result;
 result . x = r2x * Scale - k * CScaleX;
 result . y = r2y * Scale - k * CScaleY;
 result . z = r2z * Scale - k * CScaleZ;
 return result;

}
 FormulaVec3 SierpHilbert_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 const float Edge1 = params [ 22 ];
 const float Edge2 = params [ 23 ];
 RotMatrix3 R1 = rotMatrix3FromFloats ( params , 4 );
 RotMatrix3 R2 = rotMatrix3FromFloats ( params , 13 );
 FormulaVec3 r1 = R1 * p;
 float rx = r1 . x;
 float ry = r1 . y;
 float rz = r1 . z;
 float sa = - rx;
 float sb = ry;
 float sc = rz;
 if ( sa >= sb ) {
 float t = sa;
 sa = sb;
 sb = t;

}
 if ( sa >= sc ) {
 float t = sa;
 sa = sc;
 sc = t;

}
 sa = - sa;
 {
 float t = sa;
 sa = sb;
 sb = t;

}
 sa = - sa;
 if ( sa >= sc ) {
 float t = sa;
 sa = sc;
 sc = t;

}
 sa = - sa;
 {
 float t = sa;
 sa = sb;
 sb = t;

}
 if ( Edge1 > 0.0f ) {
 sb = Edge1 - abs ( Edge1 - sb );

}
 if ( Edge2 > 0.0f ) {
 sa = Edge2 - abs ( Edge2 - sa );

}
 FormulaVec3 r2 = R2 * FormulaVec3 ( sa , sb , sc );
 float r2x = r2 . x;
 float r2y = r2 . y;
 float r2z = r2 . z;
 de = de * Scale;
 float k = Scale - 1.0f;
 FormulaVec3 result;
 result . x = r2x * Scale - k * CScaleX;
 result . y = r2y * Scale - k * CScaleY;
 result . z = r2z * Scale - k * CScaleZ;
 return result;

}

}}
