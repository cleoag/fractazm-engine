// Generated from dIFS/KleinBotTIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10221 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_KLEINBOTTIFS = 10221;
 template < > FormulaVec3 execute_formula_dIFS < FORMULA_KLEINBOTTIFS > ( FormulaVec3 p , FormulaParams params , thread float & varyScale , thread float & rout ) {
 const float x = p . x * params [ 1 ] + params [ 4 ] , y = p . y * params [ 1 ] + params [ 3 ];
 const float z = p . z * params [ 1 ] + params [ 2 ];
 const int apply = int ( params [ 6 ] );
 const bool negativeOrUnordered = ! ( y >= 0.0f );
 const float shifted = float ( static_cast < float > ( x ) + ( negativeOrUnordered ? params [ 8 ] : - params [ 8 ] ) );
 float radial = sqrt ( static_cast < float > ( shifted ) * shifted + static_cast < float > ( y ) * y ) - params [ 8 ];
 float outer = float ( abs ( sqrt ( radial * radial + static_cast < float > ( z ) * z ) - params [ 0 ] ) - params [ 7 ] );
 float scratch = 0.0f;
 float current;
 if ( negativeOrUnordered ) {
 const float radius = float ( atan2 ( static_cast < float > ( y ) , static_cast < float > ( x ) ) * params [ 12 ] * ( static_cast < float > ( params [ 5 ] ) + params [ 5 ] ) + params [ 0 ] );
 radial = sqrt ( static_cast < float > ( x ) * x + static_cast < float > ( y ) * y ) - params [ 9 ];
 current = sqrt ( radial * radial + static_cast < float > ( z ) * z ) - radius;
 scratch = float ( current );
 current = abs ( current ) - params [ 7 ];

}
 else {
 const float shifted2 = float ( static_cast < float > ( x ) + params [ 9 ] );
 radial = ( sqrt ( static_cast < float > ( shifted2 ) * shifted2 + static_cast < float > ( z ) * z ) + params [ 5 ] ) - params [ 0 ];
 current = abs ( sqrt ( radial * radial + static_cast < float > ( y ) * y ) - params [ 5 ] ) - params [ 7 ];

}
 outer = float ( ( abs ( static_cast < float > ( scratch ) ) - scratch ) + outer );
 if ( current >= outer ) current = outer;
 if ( ! apply ) current /= params [ 1 ];
 rout = float ( current );
 params [ DIFS_COMMIT_XYZ_SLOT ] = apply != 0 ? 1.0f : 0.0f;
 if ( apply != 0 ) varyScale *= params [ 1 ];
 return FormulaVec3 ( x , y , z );

}
 template < > FormulaVec3 execute_formula < FORMULA_KLEINBOTTIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float varyScale = 1.0f;
 return execute_formula_dIFS < FORMULA_KLEINBOTTIFS > ( p , params , varyScale , de );

}
 FormulaVec3 KleinBotTIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_KLEINBOTTIFS > ( p , params , de );

}

}}
