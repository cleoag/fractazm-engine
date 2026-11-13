// Generated from 4D/Magnet4D.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10340 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MAGNET4D = 10340;
 template < > FormulaVec3 execute_formula < FORMULA_MAGNET4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 int iteration = int ( native_formula_counter ( params ) );
 const numeric :: X87Scope extendedScope;
 const numeric :: X87Real zm = params [ 0 ] , up = params [ 1 ] , dn = params [ 2 ];
 const numeric :: X87Real add = params [ 3 ] , two = params [ 5 ];
 const numeric :: X87Real cx = params [ 6 ] , cy = params [ 7 ] , cz = params [ 8 ];
 float x = p . x , y = p . y , z = p . z , x1 , y1 , z1;
 if ( int ( params [ 4 ] ) != 0 && iteration == 0 ) {
 x = y = x1 = y1 = 0.f;
 z1 = z;
 ++ iteration;

}
 else {
 const numeric :: X87Real xx = x , yy = y , zz = z;
 const float radius = float ( xx * xx + yy * yy );
 const float factor = float ( - ( zz * zz / radius ) + numeric :: X87Real ( 1.f ) );
 x1 = float ( ( xx * xx - yy * yy ) * factor );
 y1 = float ( xx * yy * two * factor );
 z1 = float ( numeric :: x87Sqrt ( radius ) * zz * two * zm );

}
 x1 = float ( ( static_cast < numeric :: X87Real > ( x1 ) + cx * up ) + add );
 y1 = float ( static_cast < numeric :: X87Real > ( y1 ) + cy * up );
 z1 = float ( static_cast < numeric :: X87Real > ( z1 ) + cz * up );
 const float x2 = float ( ( static_cast < numeric :: X87Real > ( x ) * two + ( add - numeric :: X87Real ( 1.f ) ) ) + cx * dn );
 const float y2 = float ( static_cast < numeric :: X87Real > ( y ) * two + cy * dn );
 const float z2 = float ( static_cast < numeric :: X87Real > ( z ) * two * zm + cz * dn );
 const numeric :: X87Real ax = x1 , ay = y1 , az = z1 , bx = x2 , by = y2 , bz = z2;
 float radius = float ( bx * bx + ( by * by + bz * bz ) );
 const float r1 = float ( numeric :: x87Sqrt ( ax * ax + ay * ay ) );
 const float r2 = float ( numeric :: x87Sqrt ( bx * bx + by * by ) );
 z = float ( ( static_cast < numeric :: X87Real > ( r2 ) * az - static_cast < numeric :: X87Real > ( r1 ) * bz ) / radius * zm );
 radius = float ( ( ( az * bz / r1 ) / r2 + numeric :: X87Real ( 1.f ) ) / radius );
 x = float ( ( ax * bx + ay * by ) * radius );
 y = float ( ( ay * bx - ax * by ) * radius );
 const numeric :: X87Real xx = x , yy = y , zz = z;
 const float r = float ( xx * xx + yy * yy );
 const float f = float ( - ( zz * zz / r ) + numeric :: X87Real ( 1.f ) );
 x = float ( ( xx * xx - yy * yy ) * f );
 y = float ( xx * yy * two * f );
 z = float ( numeric :: x87Sqrt ( r ) * zz * two * zm );
 const numeric :: X87Real dx = static_cast < numeric :: X87Real > ( x ) - numeric :: X87Real ( 1.f );
 de = float ( numeric :: X87Real ( 1.f ) / ( dx * dx + ( static_cast < numeric :: X87Real > ( y ) * y + static_cast < numeric :: X87Real > ( z ) * z ) ) );
 native_formula_counter ( params ) = iteration;
 return FormulaVec3 ( x , y , z );

}

}}
