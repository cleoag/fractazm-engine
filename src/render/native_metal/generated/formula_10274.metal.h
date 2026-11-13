// Generated from 3D/_e_3Dtess.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10274 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_E3DTESS = 10274;
 template < > FormulaVec3 execute_formula < FORMULA_E3DTESS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 FormulaWorkBudget work ( params , "_e_3Dtess" );
 float v [ 3 ] = {
 p . x , p . y , p . z
}
;
 for ( int axis = 0;
 axis < 3;
 ++ axis ) v [ axis ] = float ( static_cast < float > ( v [ axis ] ) * params [ 5 ] + params [ axis ] );
 const int selector = int ( params [ 6 ] ) , maxDepth = int ( params [ 7 ] );
 float axiom = selector == 1 ? params [ 8 ] : selector == 2 ? params [ 10 ] : selector == 3 ? params [ 11 ] : 1.f;
 float threshold = float ( ( static_cast < float > ( axiom ) + 1 ) * axiom );
 int count = 0;
 do {
 if ( ! work . beforeStep ( ) ) return NAN;
 ++ count;
 int dominant = 0;
 float maximum = fabs ( v [ 0 ] );
 for ( int axis = 1;
 axis < 3;
 ++ axis ) if ( ! ( maximum > fabs ( v [ axis ] ) ) ) {
 maximum = fabs ( v [ axis ] );
 dominant = axis;

}
 if ( threshold > maximum && ! ( axiom > maximum ) ) {
 const float range = static_cast < float > ( threshold ) - axiom;
 const float ratio = maximum / v [ dominant ];
 v [ dominant ] = float ( ( - ( static_cast < float > ( threshold ) + axiom ) * ratio * params [ 8 ] + v [ dominant ] ) * params [ 9 ] * params [ 4 ] / range );
 for ( int axis = 0;
 axis < 3;
 ++ axis ) if ( axis != dominant ) {
 v [ axis ] = float ( static_cast < float > ( v [ axis ] ) / range );
 const float rounded = rint ( static_cast < float > ( v [ axis ] ) + params [ 8 ] );
 v [ axis ] = float ( ( - rounded + params [ 8 ] + v [ axis ] ) * params [ 9 ] * params [ 4 ] );

}
 break;

}
 const float previous = axiom;
 axiom = threshold;
 threshold = float ( ( static_cast < float > ( threshold ) - previous ) * params [ 8 ] + threshold );
 if ( count >= maxDepth || params [ 12 ] > static_cast < float > ( threshold ) - axiom ) break;

}
 while ( true );
 for ( int axis = 0;
 axis < 3;
 ++ axis ) v [ axis ] = float ( static_cast < float > ( v [ axis ] ) * params [ 3 ] - params [ axis ] );
 return FormulaVec3 ( v [ 0 ] , v [ 1 ] , v [ 2 ] );

}

}}
