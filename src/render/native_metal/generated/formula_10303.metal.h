// Generated from dIFS/SphCageIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10303 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_SPHCAGEIFS = 10303;
 template < > FormulaVec3 execute_formula_dIFS < FORMULA_SPHCAGEIFS > ( FormulaVec3 p , FormulaParams params , thread float & varyScale , thread float & rout ) {
 const float x = p . x * params [ 1 ] + params [ 4 ] , y = p . y * params [ 1 ] + params [ 3 ];
 const float z = p . z * params [ 1 ] + params [ 2 ];
 const int apply = int ( params [ 6 ] );
 const int rawParallels = int ( params [ 7 ] );
 const int parallels = rawParallels < 2 ? 2 : ( rawParallels > 1000 ? 1000 : rawParallels );
 const float radius = params [ 0 ];
 const float step = float ( radius / parallels * params [ 12 ] );
 float angle = ( params [ 11 ] - atan2 ( static_cast < float > ( x ) , static_cast < float > ( y ) ) ) * params [ 9 ];
 angle = rint ( angle * params [ 8 ] ) / params [ 8 ] * params [ 10 ];
 const float cs = float ( cos ( angle ) ) , sn = float ( sin ( angle ) );
 const float along = static_cast < float > ( x ) * cs + static_cast < float > ( y ) * sn;
 const float perp = static_cast < float > ( x ) * sn - static_cast < float > ( y ) * cs;
 const float ring = sqrt ( along * along + static_cast < float > ( z ) * z ) - radius;
 float distance = sqrt ( ring * ring + perp * perp ) - params [ 5 ];
 float latitude = - params [ 0 ] , dz = float ( abs ( static_cast < float > ( z ) ) - radius );
 for ( int i = 0;
 i < parallels / 2;
 ++ i ) {
 const float previous = float ( distance );
 dz = float ( static_cast < float > ( step ) + dz );
 latitude = float ( static_cast < float > ( step ) + latitude );
 const float radial = sqrt ( static_cast < float > ( x ) * x + static_cast < float > ( y ) * y ) - sqrt ( radius * radius - static_cast < float > ( latitude ) * latitude );
 const float candidate = sqrt ( radial * radial + static_cast < float > ( dz ) * dz ) - params [ 5 ];
 distance = previous >= candidate ? candidate : previous;

}
 if ( ! apply ) distance /= params [ 1 ];
 rout = float ( distance );
 params [ DIFS_COMMIT_XYZ_SLOT ] = apply != 0 ? 1.0f : 0.0f;
 if ( apply != 0 ) varyScale *= params [ 1 ];
 return FormulaVec3 ( x , y , z );

}
 template < > FormulaVec3 execute_formula < FORMULA_SPHCAGEIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float varyScale = 1.0f;
 return execute_formula_dIFS < FORMULA_SPHCAGEIFS > ( p , params , varyScale , de );

}
 FormulaVec3 SphCageIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_SPHCAGEIFS > ( p , params , de );

}

}}
