// Generated from dIFS/HeightMapIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10189 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_HEIGHTMAPIFS = 10189;
 template < > FormulaVec3 execute_formula_with_direct_map < FORMULA_HEIGHTMAPIFS > ( FormulaVec3 p , FormulaParams params , thread float & de , bool map ) {
 const numeric :: X87Scope extendedScope;
 float state [ ] = {
 p . x , p . y , p . z , de , params [ DIFS_ROLD_SLOT ] , params [ MAX_FORMULA_PARAMS - 1 ] , 0.0f , 0.0f , 0.0f , 0.0f
}
;
 const numeric :: X87Real x = p . x , y = p . y , z = p . z;
 const numeric :: X87Real rx = ( x * params [ 7 ] + y * params [ 8 ] ) + z * params [ 9 ];
 const numeric :: X87Real ry = ( x * params [ 10 ] + y * params [ 11 ] ) + z * params [ 12 ];
 const numeric :: X87Real rz = ( x * params [ 13 ] + y * params [ 14 ] ) + z * params [ 15 ];
 const numeric :: X87Real center = static_cast < numeric :: X87Real > ( params [ 6 ] ) * numeric :: X87Real ( 0.5f );
 const bool sample = numeric :: x87Abs ( static_cast < numeric :: X87Real > ( params [ 6 ] ) ) + state [ 4 ] >= numeric :: x87Abs ( ( rz + params [ 3 ] ) - center );
 if ( sample ) {
 state [ 6 ] = static_cast < float > ( rx * params [ 4 ] - params [ 1 ] );
 state [ 7 ] = static_cast < float > ( ry * params [ 5 ] - params [ 2 ] );
 state [ 9 ] = static_cast < float > ( rz );

}
 else {
 state [ 3 ] = static_cast < float > ( ( center - params [ 3 ] ) - rz );

}
 if ( sample ) {
 const int mapNumber = static_cast < int > ( params [ 0 ] );
 if ( map ) {
 TVec3D uv = {
 state [ 6 ] , state [ 7 ] , 0.0f
}
 , rgb = {

}
;
 native_formula_map ( params , & rgb , & uv , mapNumber , false );
 for ( int i = 0;
 i < 3;
 ++ i ) state [ 6 + i ] = rgb [ i ];

}
 else {
 const auto rgb = native_formula_map_direct ( params , state [ 6 ] , state [ 7 ] , mapNumber );
 state [ 6 ] = rgb . x;
 state [ 7 ] = rgb . y;
 state [ 8 ] = rgb . z;

}
 const int heightChannel = static_cast < int > ( params [ 16 ] ) & 3;
 const int trapChannel = static_cast < int > ( params [ 17 ] ) & 3;
 state [ 3 ] = static_cast < float > ( ( static_cast < numeric :: X87Real > ( state [ 6 + heightChannel ] ) * params [ 6 ] - params [ 3 ] ) - state [ 9 ] );
 state [ 5 ] = static_cast < float > ( ( static_cast < numeric :: X87Real > ( state [ 6 + trapChannel ] ) + params [ 18 ] ) * params [ 19 ] );

}
 de = state [ 3 ];
 params [ MAX_FORMULA_PARAMS - 1 ] = state [ 5 ];
 return p;

}
 template < > FormulaVec3 execute_formula < FORMULA_HEIGHTMAPIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula_with_direct_map < FORMULA_HEIGHTMAPIFS > ( p , params , de , false );

}
 FormulaVec3 HeightMapIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_HEIGHTMAPIFS > ( p , params , de );

}

}}
