// Generated from Ads/_FoldingCTetra3d.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10018 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__FOLDINGCTETRA3D = 10018;
 static inline void tetra3d_fold_1st ( thread float & a , thread float & b , thread float & c ) {
 if ( a + b <= 0.0f ) {
 float t = a;
 a = - b;
 b = - t;

}
 if ( a + c <= 0.0f ) {
 float t = a;
 a = - c;
 c = - t;

}
 if ( b + c <= 0.0f ) {
 float t = b;
 b = - c;
 c = - t;

}

}
 static inline void tetra3d_fold_2nd ( thread float & a , thread float & b , thread float & c ) {
 if ( a - b <= 0.0f ) {
 float t = a;
 a = b;
 b = t;

}
 if ( a - c <= 0.0f ) {
 float t = a;
 a = c;
 c = t;

}
 if ( b - c <= 0.0f ) {
 float t = b;
 b = c;
 c = t;

}

}
 template < > FormulaVec3 execute_formula < FORMULA__FOLDINGCTETRA3D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const bool first = params [ 0 ] != 0.0f;
 const bool second = ! first || params [ 1 ] != 0.0f;
 float cx = params [ 3 ] , cy = params [ 4 ] , cz = params [ 5 ];
 if ( first ) tetra3d_fold_1st ( cx , cy , cz );
 if ( second ) tetra3d_fold_2nd ( cx , cy , cz );
 params [ WRITE_J1_SLOT ] = cx;
 params [ WRITE_J2_SLOT ] = cy;
 params [ WRITE_J3_SLOT ] = cz;
 if ( params [ 2 ] != 0.0f ) {
 if ( first ) tetra3d_fold_1st ( p . x , p . y , p . z );
 if ( second ) tetra3d_fold_2nd ( p . x , p . y , p . z );

}
 return p;

}

}}
