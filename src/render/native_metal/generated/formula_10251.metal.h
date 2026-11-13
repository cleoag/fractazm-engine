// Generated from dIFSa/FernTransform.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10251 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_FERNTRANSFORM = 10251;
 template < > FormulaVec3 execute_formula < FORMULA_FERNTRANSFORM > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float x = p . x , y = p . y , z = p . z;
 const float test = - ( abs ( y ) * params [ 14 ] + x );
 if ( test >= params [ 0 ] ) {
 x = ( x + params [ 0 ] ) * params [ 1 ];
 y *= params [ 1 ];
 z *= params [ 1 ];
 params [ DIFS_VARYSCALE_SLOT ] *= params [ 1 ];
 const float rx = x * params [ 3 ] + y * params [ 2 ];
 const float ry = y * params [ 3 ] - x * params [ 2 ];
 x = rx * params [ 5 ] + z * params [ 4 ];
 z = z * params [ 5 ] - rx * params [ 4 ];
 y = ry;

}
 else if ( y >= 0.0f ) {
 x += params [ 13 ];
 const float rx = x * params [ 11 ] + y * params [ 10 ];
 const float ry = y * params [ 11 ] - x * params [ 10 ];
 x = rx * params [ 9 ];
 y = ry * params [ 9 ];
 z *= params [ 9 ];
 params [ DIFS_VARYSCALE_SLOT ] *= params [ 9 ];

}
 else {
 y = - y;
 if ( y >= 0.0f ) {
 x += params [ 12 ];
 const float rx = x * params [ 8 ] + y * params [ 7 ];
 const float ry = y * params [ 8 ] - x * params [ 7 ];
 x = rx * params [ 6 ];
 y = ry * params [ 6 ];
 z *= params [ 6 ];
 params [ DIFS_VARYSCALE_SLOT ] *= params [ 6 ];

}
 else {
 const float t = x;
 x = y;
 y = t;

}

}
 return FormulaVec3 ( x , y , z );

}
 FormulaVec3 FernTransform_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_FERNTRANSFORM > ( p , params , de );

}

}}
