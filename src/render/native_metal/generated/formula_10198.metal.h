// Generated from dIFS/HeightMap2IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10198 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_HEIGHTMAP2IFS = 10198;
 template < > FormulaVec3 execute_formula_with_direct_map < FORMULA_HEIGHTMAP2IFS > ( FormulaVec3 p , FormulaParams params , thread float & de , bool map ) {
 const numeric :: X87Scope extendedScope;
 float s [ ] = {
 p . x , p . y , p . z , de , params [ DIFS_ROLD_SLOT ] , params [ MAX_FORMULA_PARAMS - 1 ] , 0.f , 0.f , 0.f , 0.f , .5f
}
;
 numeric :: X87Real x = p . x , y = p . y , z = p . z;
 numeric :: X87Real inv = numeric :: X87Real ( 1.f ) / ( ( x * x + y * y ) + z * z );
 if ( ! ( inv > params [ 21 ] ) ) inv = params [ 21 + ( int ( params [ 20 ] ) & 1 ) ];
 x *= inv;
 y *= inv;
 z *= inv;
 s [ 3 ] = float ( inv );
 const numeric :: X87Real rx = ( x * params [ 7 ] + y * params [ 8 ] ) + z * params [ 9 ];
 const numeric :: X87Real ry = ( x * params [ 10 ] + y * params [ 11 ] ) + z * params [ 12 ];
 const numeric :: X87Real rz = ( x * params [ 13 ] + y * params [ 14 ] ) + z * params [ 15 ];
 const numeric :: X87Real center = ( numeric :: X87Real ) params [ 6 ] * numeric :: X87Real ( .5f );
 const bool sample = numeric :: x87Abs ( ( numeric :: X87Real ) params [ 6 ] ) + s [ 4 ] >= numeric :: x87Abs ( ( rz + params [ 3 ] ) - center );
 if ( sample ) {
 s [ 6 ] = float ( rx * params [ 4 ] - params [ 1 ] );
 s [ 7 ] = float ( ry * params [ 5 ] - params [ 2 ] );
 s [ 9 ] = float ( rz );

}
 else s [ 3 ] = float ( ( ( center - params [ 3 ] ) - rz ) / s [ 3 ] );
 if ( sample ) {
 const int number = int ( params [ 0 ] );
 if ( map ) {
 TVec3D uv = {
 s [ 6 ] , s [ 7 ] , 0.f
}
 , rgb = {

}
;
 native_formula_map ( params , & rgb , & uv , number , false );
 for ( int i = 0;
 i < 3;
 ++ i ) s [ 6 + i ] = rgb [ i ];

}
 else {
 auto rgb = native_formula_map_direct ( params , s [ 6 ] , s [ 7 ] , number );
 s [ 6 ] = rgb . x;
 s [ 7 ] = rgb . y;
 s [ 8 ] = rgb . z;

}
 const int channel = int ( params [ 16 ] ) & 3 , trap = int ( params [ 17 ] ) & 3;
 s [ 3 ] = float ( ( ( ( numeric :: X87Real ) s [ 6 + channel ] * params [ 6 ] - params [ 3 ] ) - s [ 9 ] ) / s [ 3 ] );
 s [ 5 ] = float ( ( ( numeric :: X87Real ) s [ 6 + trap ] + params [ 18 ] ) * params [ 19 ] );

}
 de = s [ 3 ];
 params [ MAX_FORMULA_PARAMS - 1 ] = s [ 5 ];
 return p;

}
 template < > FormulaVec3 execute_formula < FORMULA_HEIGHTMAP2IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula_with_direct_map < FORMULA_HEIGHTMAP2IFS > ( p , params , de , false );

}
 FormulaVec3 HeightMap2IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_HEIGHTMAP2IFS > ( p , params , de );

}

}}
