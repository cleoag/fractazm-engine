// Generated from dIFS/RoomDoorsIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10309 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ROOMDOORSIFS = 10309;
 template < > FormulaVec3 execute_formula < FORMULA_ROOMDOORSIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 const int order = int ( params [ 7 ] ) , period = int ( params [ 13 ] );
 const float ys = p . y * params [ 3 ] , zs = p . z * params [ 3 ] , xs = p . x * params [ 3 ];
 const E angle = numeric :: extendedAtan2 ( E ( xs ) , E ( ys ) );
 const int sector = x87_fistp_int32 ( ( ( E ( params [ 21 ] ) - angle ) * params [ 19 ] * E ( order ) ) );
 const auto sc = numeric :: extendedSinCos ( ( E ( sector ) / E ( order ) ) * params [ 20 ] );
 const float c = float ( sc . cos ) , s = float ( sc . sin );
 const float tangent = float ( ( E ( xs ) * c + ( - ( E ( ys ) * s ) ) ) + params [ 6 ] );
 const float normal = float ( ( E ( xs ) * s + E ( ys ) * c ) + params [ 5 ] );
 const float height = float ( E ( zs ) + params [ 4 ] );
 if ( order >= 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= params [ 3 ];
 p = FormulaVec3 ( tangent , normal , height );

}
 const float wall = float ( E ( height ) * params [ 9 ] + E ( tangent ) * params [ 10 ] );
 E distance = - ( abs ( E ( wall ) ) - abs ( E ( params [ 0 ] ) ) );
 const uint wallIndex = uint ( sector ) + uint ( 256 );
 if ( period <= 0 || wallIndex % uint ( period ) == 1 ) {
 const E sides = ( abs ( E ( normal ) ) * params [ 12 ] + ( - ( E ( height ) * params [ 11 ] ) ) ) - params [ 8 ];
 const E top = - E ( params [ 2 ] ) - height;
 float center = params [ 14 ] , offset = 0;
 if ( E ( 0 ) >= E ( params [ 14 ] ) ) {
 center = float ( E ( params [ 8 ] ) - params [ 14 ] );
 offset = params [ 14 ];

}
 const E dx = abs ( E ( normal ) ) - offset;
 const E arc = sqrt ( top * top + dx * dx ) - center;
 E door = ! ( arc >= top ) ? arc : top;
 door = door >= sides ? door : sides;
 distance = ! ( door >= distance ) ? door : distance;

}
 distance = - distance;
 bool roofTrap = false;
 if ( metal :: signbit ( params [ 0 ] ) ) {
 const E heightRoof = abs ( E ( height ) + params [ 1 ] ) + params [ 0 ];
 const E wallRoof = E ( wall ) + params [ 0 ];
 const E roof = wallRoof >= heightRoof ? wallRoof : heightRoof;
 if ( E ( 0 ) >= roof + E ( params [ 22 ] ) * params [ DIFS_ROLD_SLOT ] && ! ( E ( 0 ) >= E ( params [ 17 ] ) ) ) {
 roofTrap = true;
 params [ MAX_FORMULA_PARAMS - 1 ] = params [ 17 ];

}
 distance = ! ( roof >= distance ) ? roof : distance;

}
 const E formula_half = E ( params [ 1 ] ) * params [ 18 ];
 const E heightDistance = abs ( E ( height ) + formula_half ) - float ( formula_half );
 distance = heightDistance >= distance ? heightDistance : distance;
 if ( ! roofTrap ) params [ MAX_FORMULA_PARAMS - 1 ] = float ( E ( int ( params [ ITER_COUNT_SLOT ] ) ) * params [ 15 ] + params [ 16 ] );
 if ( order < 0 ) distance /= params [ 3 ];
 de = float ( distance );
 return p;

}
 FormulaVec3 RoomDoorsIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ROOMDOORSIFS > ( p , params , de );

}

}}
