// Generated from dIFS/RandCubesIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10311 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_RANDCUBESIFS = 10311;
 template < > FormulaVec3 execute_formula < FORMULA_RANDCUBESIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float XVtn = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Zadd = params [ 2 ];
 const float Yadd = params [ 3 ];
 const float Xadd = params [ 4 ];
 const float YVtn = params [ 5 ];
 const int ApplyScale = int ( params [ 6 ] );
 const float ZMaxSz = params [ 7 ];
 const int RandX = int ( params [ 8 ] );
 const int RandY = int ( params [ 9 ] );
 const int ShapeType1 = int ( params [ 10 ] );
 const int ShapeType2 = int ( params [ 11 ] );
 const float ZFlight = params [ 12 ];
 const float c0 = - 6.86656e-6f;
 const float c1 = - 0.45f;
 const float c2 = 1.5e-5f;
 const float c3 = - 1.37331e-5f;
 const float c4 = - 10.3e-6f;
 const float c5 = - 3.1e-6f;
 float x = p . x * Scale + Xadd;
 float y = p . y * Scale + Yadd;
 float z = p . z * Scale + Zadd;
 float state_x = p . x;
 float state_y = p . y;
 float state_z = p . z;
 if ( ApplyScale != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;
 state_x = x;
 state_y = y;
 state_z = z;

}
 int ix = x87_fistp_int32 ( x );
 float fx = fabs ( x - float ( ix ) );
 int iy = x87_fistp_int32 ( y );
 float fy = fabs ( y - float ( iy ) );
 uint uix = uint ( ix ) ^ 0x9D81u;
 uint uiy = uint ( iy ) ^ 0xB641u;
 ulong product = ulong ( uiy ) * ulong ( uix );
 uint product_lo = uint ( product & 0xFFFFFFFFu );
 uint product_hi = uint ( ( product >> 32 ) & 0xFFFFFFFFu );
 uint rand_a = ~ product_lo;
 uint mix1 = ( rand_a & 0x5A5Au ) ^ ( product_hi | 0x5A5Au );
 uint mix2 = ( uiy | 0x5A5Au ) ^ ( uix & 0x5A5Au );
 uint sq1 = uint ( ( mix1 - uint ( RandY ) ) * mix1 );
 uint h1 = ( uint ( ( ( sq1 + rand_a ) >> 8 ) ^ ( ( sq1 + rand_a ) << 8 ) ) & 0xFFFFu );
 uint sq2 = uint ( ( mix2 - uint ( RandX ) ) * mix2 );
 uint h2 = ( uint ( ( ( sq2 + rand_a ) >> 8 ) ^ ( ( sq2 + rand_a ) << 8 ) ) & 0xFFFFu );
 uint h3 = ( ( h2 >> 6 ) ^ ( h1 << 6 ) ) & 0xFFFFu;
 uint h4 = ( h2 & 0xAAAAu ) ^ ( h1 & 0x5555u );
 float z_shifted = z - float ( h4 ) * c0 * ZFlight;
 float z_shape = fabs ( z_shifted ) + float ( h3 ) * c0 * ZMaxSz;
 params [ 63 ] = float ( h3 ) * c2;
 int shapeType = ( ( h2 & 0x40u ) != 0u ) ? ShapeType2 : ShapeType1;
 float shape_de;
 const float rx = float ( h2 ) * c0 * XVtn;
 const float ry = float ( h1 ) * c0 * YVtn;
 if ( shapeType == 1 ) {
 float candidate = sqrt ( fx * fx + fy * fy ) + rx;
 candidate += ( 1.0f - XVtn ) * c1;
 candidate -= fabs ( z_shifted ) * ry;
 shape_de = candidate > z_shape ? candidate : z_shape;

}
 else if ( shapeType == 2 ) {
 float candidate = sqrt ( fx * fx + fy * fy - z_shifted * z_shifted * ( float ( h2 ) * c3 * XVtn ) );
 candidate += ry;
 candidate += ( 1.0f - YVtn ) * c1;
 shape_de = candidate > z_shape ? candidate : z_shape;

}
 else if ( shapeType == 3 ) {
 float a = fx - rx * fabs ( z_shifted );
 float b = fy - ry * fabs ( z_shifted );
 float candidate = ( b > a ? b : a ) + c1;
 shape_de = candidate > z_shape ? candidate : z_shape;

}
 else if ( shapeType == 4 ) {
 float ax = h2 >= h1 ? fy : fx;
 float ay = h2 >= h1 ? fx : fy;
 float a = ax + rx + ( 1.0f - XVtn ) * c1;
 float t = z_shifted - float ( h3 ) * c0 * ZMaxSz;
 t = ( t - fabs ( t ) ) * c1;
 float b = sqrt ( t * t + ay * ay ) + ry + ( 1.0f - YVtn ) * c1;
 float candidate = b > a ? b : a;
 shape_de = z_shifted > candidate ? z_shifted : candidate;

}
 else if ( shapeType == 5 ) {
 float ax = h2 >= h1 ? fy : fx;
 float ay = h2 >= h1 ? fx : fy;
 uint hx = h2 >= h1 ? h1 : h2;
 uint hy = h2 >= h1 ? h2 : h1;
 float radius = float ( float ( fabs ( float ( hx ) * c5 * XVtn ) ) );
 float offset = float ( float ( float ( hy ) * c4 * YVtn ) );
 float t = z_shifted - float ( h3 ) * c0 * ZMaxSz;
 t = ( t - fabs ( t ) ) * c1;
 float t2 = t * t;
 float candidate = sqrt ( t2 + ay * ay + ax * ax + radius * ( offset * offset ) );
 candidate += sqrt ( sqrt ( t2 + ax * ax ) ) * offset;
 shape_de = candidate > z_shifted ? candidate : z_shifted;

}
 else if ( shapeType == 6 ) {
 float radius = float ( float ( float ( h1 ) * c4 * YVtn ) );
 float offset = rx + ( 1.0f - XVtn ) * c1 - radius;
 float narrowed_offset = float ( float ( offset ) );
 float dx = fx + offset;
 float dy = fy + narrowed_offset;
 float dz = z_shape + narrowed_offset;
 dx = ( dx + fabs ( dx ) ) * c1;
 dy = ( dy + fabs ( dy ) ) * c1;
 dz = ( dz + fabs ( dz ) ) * c1;
 shape_de = native_formula_length3 ( dx , dy , dz ) + radius;

}
 else {
 float dx = fx + rx + ( 1.0f - XVtn ) * c1;
 float dy = fy + ry + ( 1.0f - YVtn ) * c1;
 float candidate = dy > dx ? dy : dx;
 shape_de = candidate > z_shape ? candidate : z_shape;

}
 if ( ApplyScale == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 RandCubesIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_RANDCUBESIFS > ( p , params , de );

}

}}
