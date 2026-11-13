// Generated from 4D/Barnsley4Dc.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10337 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BARNSLEY4DC = 10337;
 template < > FormulaVec3 execute_formula < FORMULA_BARNSLEY4DC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 const numeric :: ExtendedScope scope;
 E x ( p . x ) , y ( p . y ) , z ( p . z ) , w ( de );
 const E dx ( params [ 0 ] ) , dy ( params [ 1 ] ) , dz ( params [ 2 ] ) , dw ( params [ 3 ] ) , addW ( params [ 4 ] );
 bool adjustC;
 if ( int ( params [ 8 ] ) != 0 ) {
 const E threshold = sqrt ( ( ( dx * dx + dy * dy ) + dz * dz ) + dw * dw ) * E ( params [ 9 ] );
 adjustC = ! ( x < threshold );
 const E scale ( params [ 10 ] );
 if ( adjustC ) {
 x = ( dx - x ) * scale;
 y = ( dy - y ) * scale;
 z = ( dz - z ) * scale;
 w = ( ( dw - w ) + addW ) * scale;

}
 else {
 x *= scale;
 y *= scale;
 z *= scale;
 w = w * scale + addW;

}

}
 else {
 adjustC = ! ( x < E ( params [ 5 ] ) );
 if ( adjustC ) {
 x -= dx;
 y -= dy;
 z -= dz;
 w = ( w - dw ) + addW;

}
 else {
 x += dx;
 y += dy;
 z += dz;
 w = ( w + dw ) + addW;

}

}
 x = E ( float ( x ) );
 y = E ( float ( y ) );
 z = E ( float ( z ) );
 w = E ( float ( w ) );
 E cx ( params [ 11 ] ) , cy ( params [ 12 ] );
 const E cz ( params [ 13 ] ) , cw ( params [ 14 ] );
 if ( adjustC && int ( params [ 6 ] ) != 0 ) cy = - cy;
 if ( adjustC && int ( params [ 7 ] ) != 0 ) {
 const E old = cx;
 cx = cy;
 cy = old;

}
 const E nx = ( ( x * cx + - ( y * cy ) ) + - ( z * cz ) ) + w * cw;
 const E ny = ( ( y * cx + x * cy ) + - ( w * cz ) ) + - ( z * cw );
 const E nz = ( ( z * cx + - ( w * cy ) ) + x * cz ) + - ( y * cw );
 const E nw = ( ( w * cx + z * cy ) + y * cz ) + x * cw;
 de = float ( nw );
 return FormulaVec3 ( float ( nx ) , float ( ny ) , float ( nz ) );

}

}}
