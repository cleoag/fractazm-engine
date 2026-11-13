// Generated from dIFS/CylinderHeightMap.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10193 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
inline numeric :: X87Real heightmapAngle ( numeric :: X87Real y , numeric :: X87Real x ) {
 return numeric :: X87Real ( atan2 ( float ( y ) , float ( x ) ) );

}
 template < int Kind > inline FormulaVec3 execute_cyl_torus_heightmap ( FormulaVec3 p , FormulaParams params , thread float & de , bool map ) {
 const numeric :: X87Scope extendedScope;
 ulong native_s8_bits = 0;
 float s [ 11 ] = {
 p . x , p . y , p . z , de , params [ MAX_FORMULA_PARAMS - 1 ]
}
;
 constexpr int constants = Kind == 1 ? 21 : 20;
 const numeric :: X87Real x = ( numeric :: X87Real ) p . x - params [ 2 ] , y = ( numeric :: X87Real ) p . y - params [ 3 ] , z = ( numeric :: X87Real ) p . z - params [ 4 ];
 numeric :: X87Real a = ( x * params [ 8 ] + y * params [ 9 ] ) + z * params [ 10 ];
 numeric :: X87Real b = ( x * params [ 11 ] + y * params [ 12 ] ) + z * params [ 13 ];
 numeric :: X87Real axis = ( x * params [ 14 ] + y * params [ 15 ] ) + z * params [ 16 ];
 numeric :: X87Real distance , ring = 0 , threshold = numeric :: x87Abs ( ( numeric :: X87Real ) params [ 7 ] );
 struct NativeClosure0 {
 thread decltype ( native_s8_bits ) & native_s8_bits;
 thread decltype ( s ) & s;
 auto operator ( ) ( int offset , numeric :: X87Real v ) const {
 const float f = static_cast < float > ( v );
 native_store_packed_single ( native_s8_bits , s [ 8 ] , offset , f );

}

}
;
 auto storeSingle = NativeClosure0 {
 native_s8_bits , s
}
;
 if constexpr ( Kind == 2 ) {
 a = float ( a );
 b = float ( b );
 axis = float ( axis );
 s [ 8 ] = float ( axis );
 native_s8_bits = mb3d_metal_exact :: native_encode_double ( s [ 8 ] );
 ring = numeric :: x87Sqrt ( a * a + b * b ) - params [ 5 ];
 distance = numeric :: x87Sqrt ( ring * ring + axis * axis ) - params [ 6 ];
 threshold += params [ constants + 1 ];
 threshold += params [ constants + 1 ];

}
 else {
 numeric :: X87Real radius = numeric :: x87Sqrt ( b * b + a * a );
 if constexpr ( Kind == 1 ) radius += axis * params [ 20 ];
 storeSingle ( 28 , radius );
 distance = radius - params [ 6 ];
 const numeric :: X87Real cap = numeric :: x87Abs ( axis ) - params [ 5 ];
 if ( cap >= distance ) distance = cap;

}
 const bool sample = threshold >= numeric :: x87Abs ( distance );
 if ( sample ) {
 storeSingle ( 24 , distance );
 s [ 5 ] = float ( heightmapAngle ( a , b ) * params [ constants ] );
 if constexpr ( Kind == 2 ) s [ 6 ] = float ( heightmapAngle ( ring , axis ) * params [ constants ] );
 else s [ 6 ] = float ( axis * params [ constants ] / params [ 6 ] );

}
 else s [ 3 ] = float ( Kind == 0 ? distance : numeric :: x87Abs ( distance ) );
 if ( sample ) {
 const int number = static_cast < int > ( params [ 0 ] );
 if ( map ) {
 TVec3D uv = {
 s [ 5 ] , s [ 6 ] , 0.f
}
 , rgb = {

}
;
 native_formula_map ( params , & rgb , & uv , number , false );
 for ( int i = 0;
 i < 3;
 ++ i ) s [ 5 + i ] = rgb [ i ];

}
 else {
 auto rgb = native_formula_map_direct ( params , s [ 5 ] , s [ 6 ] , number );
 s [ 5 ] = rgb . x;
 s [ 6 ] = rgb . y;
 s [ 7 ] = rgb . z;

}
 float storedDistance , storedRadius;
 storedDistance = as_type < float > ( uint ( native_s8_bits ) );
 storedRadius = as_type < float > ( uint ( native_s8_bits >> 32 ) );
 numeric :: X87Real displacement = ( ( numeric :: X87Real ) s [ 5 + ( int ( params [ 1 ] ) & 3 ) ] - numeric :: X87Real ( .5f ) ) * params [ 7 ];
 if constexpr ( Kind != 2 ) displacement *= storedRadius;
 numeric :: X87Real distance = ( numeric :: X87Real ) storedDistance - displacement;
 if constexpr ( Kind != 0 ) distance = numeric :: x87Abs ( distance ) - params [ constants + 1 ];
 s [ 3 ] = float ( distance );
 s [ 4 ] = float ( ( ( numeric :: X87Real ) s [ 5 + ( int ( params [ 17 ] ) & 3 ) ] + params [ 18 ] ) * params [ 19 ] );

}
 de = s [ 3 ];
 params [ MAX_FORMULA_PARAMS - 1 ] = s [ 4 ];
 return p;

}
 constant constexpr unsigned int FORMULA_CYLINDERHEIGHTMAP = 10193;
 template < > FormulaVec3 execute_formula_with_direct_map < FORMULA_CYLINDERHEIGHTMAP > ( FormulaVec3 p , FormulaParams params , thread float & de , bool map ) {
 return execute_cyl_torus_heightmap < 0 > ( p , params , de , map );

}
 template < > FormulaVec3 execute_formula < FORMULA_CYLINDERHEIGHTMAP > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula_with_direct_map < FORMULA_CYLINDERHEIGHTMAP > ( p , params , de , false );

}
 FormulaVec3 CylinderHeightMap_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_CYLINDERHEIGHTMAP > ( p , params , de );

}

}}
