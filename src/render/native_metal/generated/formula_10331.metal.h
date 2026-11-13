// Generated from 4D/PG-bulb.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10331 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_PG_BULB = 10331;
 template < > FormulaVec3 execute_formula < FORMULA_PG_BULB > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 int power = int ( params [ 1 ] );
 if ( power < 1 || power >= 17 ) power = 2;
 const int old_angles = int ( params [ 2 ] );
 float state [ ] = {
 p . x , p . y , p . z , de , params [ 3 ] , params [ 4 ] , params [ 5 ] , params [ 6 ]
}
;
 const numeric :: X87Scope extendedScope;
 const numeric :: X87Real x = p . x , y = p . y , z = p . z , w = de;
 const numeric :: X87Real angle_x = old_angles ? state [ 4 ] : p . x;
 const numeric :: X87Real angle_y = old_angles ? state [ 5 ] : p . y;
 const numeric :: X87Real sum = angle_x * angle_x + angle_y * angle_y;
 const float ct = float ( ( angle_x * angle_x - angle_y * angle_y ) / sum );
 const float st = float ( ( angle_x * angle_y ) / sum );
 float a = p . x , b = p . y , c = p . z , d = float ( w + params [ 0 ] );
 const int iterations = power > 1 ? power - 1 : 1;
 for ( int i = 0;
 i < iterations;
 ++ i ) {
 const numeric :: X87Real aa = a , bb = b , cc = c , dd = d , cos_theta = ct , sin_theta = st;
 const numeric :: X87Real na = ( ( ( aa * x - bb * y ) - ( cos_theta * cc ) * z ) + ( cc * w + dd * z ) * sin_theta ) + ( cos_theta * dd ) * w;
 const numeric :: X87Real nb = ( ( ( aa * y + bb * x ) - ( sin_theta * cc ) * z ) - ( cc * w + dd * z ) * cos_theta ) + ( sin_theta * dd ) * w;
 const numeric :: X87Real nc = ( ( aa * z + cc * x ) - bb * w ) - dd * y;
 const numeric :: X87Real nd = ( ( aa * w + dd * x ) + bb * z ) + cc * y;
 if ( i + 1 == iterations ) {
 state [ 0 ] = float ( na + state [ 4 ] );
 state [ 1 ] = float ( nb + state [ 5 ] );
 state [ 2 ] = float ( nc + state [ 6 ] );
 state [ 3 ] = float ( nd + state [ 7 ] );

}
 else {
 a = float ( na );
 b = float ( nb );
 c = float ( nc );
 d = float ( nd );

}

}
 de = state [ 3 ];
 return FormulaVec3 ( state [ 0 ] , state [ 1 ] , state [ 2 ] );

}

}}
