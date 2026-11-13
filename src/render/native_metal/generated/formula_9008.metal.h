// Generated from dIFS/gear2IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9008 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_GEAR2IFS = 9008;
 template < > FormulaVec3 execute_formula_dIFS < FORMULA_GEAR2IFS > ( FormulaVec3 p , FormulaParams params , thread float & varyScale , thread float & rout ) {
 const float scale = params [ 1 ];
 const int apply = int ( params [ 6 ] );
 const float x = p . x * scale + params [ 4 ];
 const float y = p . y * scale + params [ 3 ];
 const float z = p . z * scale + params [ 2 ];
 params [ DIFS_COMMIT_XYZ_SLOT ] = apply != 0 ? 1.0f : 0.0f;
 if ( apply != 0 ) varyScale *= scale;
 const float radius = static_cast < float > ( sqrt ( static_cast < float > ( x ) * x + static_cast < float > ( y ) * y ) - params [ 5 ] );
 float angle = atan2 ( static_cast < float > ( y ) , static_cast < float > ( x ) ) * params [ 12 ];
 if ( ! ( radius >= 0.0f ) ) angle -= params [ 10 ];
 angle /= params [ 7 ];
 angle = abs ( ( angle - rint ( angle ) ) * params [ 7 ] );
 float distance = - ( ( angle / params [ 0 ] - 1.0f ) * params [ 0 ] * params [ 13 ] );
 const float shell1 = abs ( static_cast < float > ( radius ) ) - params [ 8 ];
 if ( shell1 >= distance ) distance = shell1;
 const float shell2 = abs ( static_cast < float > ( radius ) ) - params [ 11 ];
 if ( ! ( shell2 >= distance ) ) distance = shell2;
 const float slab = abs ( static_cast < float > ( z ) ) - params [ 9 ];
 if ( slab >= distance ) distance = slab;
 if ( apply == 0 ) distance /= scale;
 rout = static_cast < float > ( distance );
 return FormulaVec3 ( x , y , z );

}
 template < > FormulaVec3 execute_formula < FORMULA_GEAR2IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float varyScale = 1.0f;
 return execute_formula_dIFS < FORMULA_GEAR2IFS > ( p , params , varyScale , de );

}
 FormulaVec3 gear2IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_GEAR2IFS > ( p , params , de );

}

}}
