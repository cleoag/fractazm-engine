// Generated from 3Da/ABoxMapSFold.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10180 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXMAPSFOLD = 10180;
 inline FormulaVec3 aboxMapSFold ( FormulaVec3 p , FormulaParams params , thread float & de , bool map ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 const E fold ( params [ 3 ] );
 struct NativeClosure0 {
 thread decltype ( fold ) & fold;
 auto operator ( ) ( E v ) const {
 return abs ( v + fold ) - ( abs ( v - fold ) + v );

}

}
;
 auto box = NativeClosure0 {
 fold
}
;
 const E x = box ( E ( p . x ) ) , y = box ( E ( p . y ) ) , z = box ( E ( p . z ) );
 TVec3D sample = {
 static_cast < float > ( x ) , static_cast < float > ( y ) , static_cast < float > ( z )
}
;
 if ( map ) {
 native_formula_map ( params , & sample , & sample , int ( params [ 4 ] ) , false );

}
 else {
 const auto rgb = native_formula_map_sphere ( params , sample [ 0 ] , sample [ 1 ] , sample [ 2 ] , int ( params [ 4 ] ) );
 sample [ 0 ] = rgb . x;
 sample [ 1 ] = rgb . y;
 sample [ 2 ] = rgb . z;

}
 const E offset ( static_cast < float > ( params [ 5 ] ) );
 const E scale ( static_cast < float > ( params [ 6 ] ) );
 const E rx = ( E ( sample [ 0 ] ) + offset ) * scale + x;
 const E ry = ( E ( sample [ 1 ] ) + offset ) * scale + y;
 const E rz = ( E ( sample [ 2 ] ) + offset ) * scale + z;
 const E radius = ( rx * rx + ry * ry ) + rz * rz;
 const E m = ! ( radius >= E ( params [ 2 ] ) ) ? E ( params [ 1 ] ) : ( E ( 1 ) >= radius ? E ( params [ 0 ] ) / radius : E ( params [ 0 ] ) );
 de = static_cast < float > ( E ( de ) * m );
 return FormulaVec3 ( static_cast < float > ( x * m + E ( params [ 7 ] ) ) , static_cast < float > ( y * m + E ( params [ 8 ] ) ) , static_cast < float > ( z * m + E ( params [ 9 ] ) ) );

}
 template < > FormulaVec3 execute_formula < FORMULA_ABOXMAPSFOLD > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return aboxMapSFold ( p , params , de , false );

}
 template < > FormulaVec3 execute_formula_with_sphere_map < FORMULA_ABOXMAPSFOLD > ( FormulaVec3 p , FormulaParams params , thread float & de , bool map ) {
 return aboxMapSFold ( p , params , de , map );

}

}}
