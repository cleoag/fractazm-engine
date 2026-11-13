// Generated from dIFS/SphereHMVoid.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10301 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template < bool Void > inline FormulaVec3 execute_spherical_heightmap ( FormulaVec3 p , FormulaParams params , thread float & de , bool map ) {
 const numeric :: X87Scope extendedScope;
 float s [ ] = {
 p . x , p . y , p . z , de , params [ DIFS_ROLD_SLOT ] , params [ MAX_FORMULA_PARAMS - 1 ] , 0.f , 0.f , 0.f , 0.f , .5f
}
;
 const numeric :: X87Real x = ( numeric :: X87Real ) p . x - params [ 2 ] , y = ( numeric :: X87Real ) p . y - params [ 3 ] , z = ( numeric :: X87Real ) p . z - params [ 4 ];
 const numeric :: X87Real rx = ( ( x * params [ 8 ] + y * params [ 9 ] ) + z * params [ 10 ] ) * params [ 5 ];
 const numeric :: X87Real ry = ( ( x * params [ 11 ] + y * params [ 12 ] ) + z * params [ 13 ] ) * params [ 6 ];
 const numeric :: X87Real rz = ( x * params [ 14 ] + y * params [ 15 ] ) + z * params [ 16 ];
 const numeric :: X87Real distance = numeric :: x87Sqrt ( ( rz * rz + ry * ry ) + rx * rx ) - numeric :: X87Real ( 1.f );
 numeric :: X87Real threshold = numeric :: x87Abs ( ( numeric :: X87Real ) params [ 7 ] ) + s [ 4 ];
 if constexpr ( Void ) threshold += params [ 22 ];
 const bool sample = threshold >= numeric :: x87Abs ( distance );
 if ( sample ) {
 s [ 6 ] = float ( rx );
 s [ 7 ] = float ( ry );
 s [ 8 ] = float ( rz );
 s [ 9 ] = float ( distance );
 if constexpr ( Void ) {
 numeric :: X87Real m = numeric :: x87Abs ( ( numeric :: X87Real ) s [ 6 ] ) , v = numeric :: x87Abs ( ( numeric :: X87Real ) s [ 7 ] );
 if ( v >= m ) m = v;
 v = numeric :: x87Abs ( ( numeric :: X87Real ) s [ 8 ] );
 if ( v >= m ) m = v;
 s [ 9 ] = float ( ( m - numeric :: X87Real ( 1.f ) ) * params [ 20 ] + ( numeric :: X87Real ( 1.f ) - params [ 20 ] ) * s [ 9 ] );

}

}
 else s [ 3 ] = float ( Void ? numeric :: x87Abs ( distance ) : distance );
 if ( sample ) {
 const int number = int ( params [ 0 ] );
 if ( map ) {
 TVec3D xyz = {
 s [ 6 ] , s [ 7 ] , s [ 8 ]
}
 , rgb = {

}
;
 native_formula_map ( params , & rgb , & xyz , number , true );
 for ( int i = 0;
 i < 3;
 ++ i ) s [ 6 + i ] = rgb [ i ];

}
 else {
 auto rgb = native_formula_map_sphere ( params , s [ 6 ] , s [ 7 ] , s [ 8 ] , number );
 s [ 6 ] = rgb . x;
 s [ 7 ] = rgb . y;
 s [ 8 ] = rgb . z;

}
 const int channel = int ( params [ 1 ] ) & 3 , trap = int ( params [ 17 ] ) & 3;
 numeric :: X87Real d = ( numeric :: X87Real ) s [ 9 ] - ( ( numeric :: X87Real ) s [ 6 + channel ] - numeric :: X87Real ( .5f ) ) * params [ 7 ];
 if constexpr ( Void ) d = numeric :: x87Abs ( d ) - params [ 21 ];
 s [ 3 ] = float ( d );
 s [ 5 ] = float ( ( ( numeric :: X87Real ) s [ 6 + trap ] + params [ 18 ] ) * params [ 19 ] );

}
 de = s [ 3 ];
 params [ MAX_FORMULA_PARAMS - 1 ] = s [ 5 ];
 return p;

}
 constant constexpr unsigned int FORMULA_SPHEREHMVOID = 10301;
 template < > FormulaVec3 execute_formula_with_sphere_map < FORMULA_SPHEREHMVOID > ( FormulaVec3 p , FormulaParams params , thread float & de , bool map ) {
 return execute_spherical_heightmap < true > ( p , params , de , map );

}
 template < > FormulaVec3 execute_formula < FORMULA_SPHEREHMVOID > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula_with_sphere_map < FORMULA_SPHEREHMVOID > ( p , params , de , false );

}
 FormulaVec3 SphereHMVoid_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_SPHEREHMVOID > ( p , params , de );

}

}}
