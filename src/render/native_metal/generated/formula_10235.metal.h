// Generated from dIFS/hextgrid4IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10235 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_HEXTGRID4IFS = 10235;
 template < > FormulaVec3 execute_formula < FORMULA_HEXTGRID4IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float RadiusMod = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Styling1 = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float Styling2 = params [ 16 ];
 const float Styling3 = params [ 17 ];
 const float c_half = params [ 18 ];
 const float c_2s3 = params [ 19 ];
 const float c_s32 = params [ 20 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;

}
 const FormulaVec3 transformed ( x , y , z );
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 E rotated [ 3 ];
 for ( int row = 0;
 row < 3;
 ++ row ) {
 thread const float * m = params + 7 + row * 3;
 rotated [ row ] = float ( ( E ( x ) * float ( m [ 0 ] ) + E ( y ) * float ( m [ 1 ] ) ) + E ( z ) * float ( m [ 2 ] ) );

}
 const E cell = E ( c_2s3 ) * Styling1 , h = float ( cell );
 const E adj1 = float ( cell * Styling3 ) , adj2 = float ( cell * Styling2 );
 const E rx = abs ( rotated [ 0 ] ) , ry = rotated [ 1 ] , rz = rotated [ 2 ];
 struct NativeClosure0 {
 thread decltype ( adj1 ) & adj1;
 thread decltype ( adj2 ) & adj2;
 thread decltype ( h ) & h;
 auto operator ( ) ( E along , E cross , bool subtract ) const {
 const int index = x87_fistp_int32 ( along );
 const E fraction = float ( abs ( E ( float ( along ) ) - E ( index ) ) );
 const E offset = abs ( subtract ? E ( index ) * adj2 - cross : E ( index ) * adj2 + cross );
 const E snapped = abs ( - ( round ( offset / h ) * h ) + E ( float ( offset ) ) ) + adj1;
 return E ( float ( fraction >= snapped ? fraction : snapped ) );

}

}
;
 const auto line = NativeClosure0 {
 adj1 , adj2 , h
}
;
 const E a = line ( rx , ry , true );
 const E b = line ( abs ( rx * c_half - ry * c_s32 ) , ry * c_half + rx * c_s32 , false );
 const E c = line ( abs ( rx * c_half + ry * c_s32 ) , ry * c_half + ( - ( rx * c_s32 ) ) , false );
 E nearest = c >= b ? b : c;
 if ( nearest >= a ) nearest = a;
 E distance = sqrt ( nearest * nearest + rz * rz ) - RadiusMod;
 if ( ! Apply_scale_add ) distance /= Scale;
 de = float ( distance );
 return Apply_scale_add != 0 ? transformed : p;

}
 FormulaVec3 hextgrid4IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_HEXTGRID4IFS > ( p , params , de );

}

}}
