// Generated from 3Da/JCube3.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10174 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_JCUBE3 = 10174;
 template < > FormulaVec3 execute_formula < FORMULA_JCUBE3 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Alpha = params [ 0 ];
 const float InvAlpha = params [ 1 ];
 const float Cx1 = params [ 2 ];
 const float Cy1 = params [ 3 ];
 const float Cz1 = params [ 4 ];
 const float Cx2 = params [ 5 ];
 const float Cy2 = params [ 6 ];
 const float Cz2 = params [ 7 ];
 const float GScale = params [ 8 ];
 float ScaleA = GScale - 1.0f + Alpha;
 float ScaleB = ScaleA * InvAlpha;
 float InvScaleB = 1.0f / ScaleB;
 float threshold = 1.0f + ( - GScale ) * InvScaleB;
 float x = abs ( p . x );
 float y = abs ( p . y );
 float z = abs ( p . z );
 float a = x , b = y , c = z;
 if ( a >= b ) {
 float t = a;
 a = b;
 b = t;

}
 if ( a < c ) {
 float t = a;
 a = c;
 c = t;

}
 {
 float t = a;
 a = b;
 b = t;

}
 if ( a >= c ) {
 float t = a;
 a = c;
 c = t;

}
 float min_v = a;
 float mid_v = b;
 float max_v = c;
 if ( InvScaleB > min_v && mid_v > min_v + threshold ) {
 min_v -= Cx1;
 mid_v -= Cy1;
 max_v -= Cz1;
 de = de * ScaleB;
 min_v *= ScaleB;
 mid_v *= ScaleB;
 max_v *= ScaleB;
 min_v += Cx1;
 mid_v += Cy1;
 max_v += Cz1;

}
 else {
 min_v -= Cx2;
 mid_v -= Cy2;
 max_v -= Cz2;
 de = de * ScaleA;
 min_v *= ScaleA;
 mid_v *= ScaleA;
 max_v *= ScaleA;
 min_v += Cx2;
 mid_v += Cy2;
 max_v += Cz2;

}
 return FormulaVec3 ( min_v , mid_v , max_v );

}

}}
