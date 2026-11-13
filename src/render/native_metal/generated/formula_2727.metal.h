// Generated from Ads/_gnarl3D.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_2727 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__GNARL3D = 2727;
 template < > FormulaVec3 execute_formula < FORMULA__GNARL3D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Strength = params [ 0 ];
 const float X_step = params [ 1 ];
 const float Y_step = params [ 2 ];
 const float Z_step = params [ 3 ];
 const float Alpha = params [ 4 ];
 const float Beta = params [ 5 ];
 float x = p . x , y = p . y , z = p . z;
 float sx = sin ( Beta * z );
 sx = sin ( Alpha * ( z + sx ) );
 sx = sin ( z + sx );
 float sy = sin ( Beta * x );
 sy = sin ( Alpha * ( x + sy ) );
 sy = sin ( x + sy );
 float sz = sin ( Beta * y );
 sz = sin ( Alpha * ( y + sz ) );
 sz = sin ( y + sz );
 FormulaVec3 result;
 result . x = ( x - X_step * sx ) * Strength;
 result . y = ( y - Y_step * sy ) * Strength;
 result . z = ( z - Z_step * sz ) * Strength;
 return result;

}
 FormulaVec3 _gnarl3D_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float S = params [ 0 ] , Xs = params [ 1 ] , Ys = params [ 2 ] , Zs = params [ 3 ];
 const float A = params [ 4 ] , B = params [ 5 ];
 float x = p . x , y = p . y , z = p . z;
 float sx = sin ( z + sin ( A * ( z + sin ( B * z ) ) ) );
 float sy = sin ( x + sin ( A * ( x + sin ( B * x ) ) ) );
 float sz = sin ( y + sin ( A * ( y + sin ( B * y ) ) ) );
 return FormulaVec3 ( ( x - Xs * sx ) * S , ( y - Ys * sy ) * S , ( z - Zs * sz ) * S );

}

}}
