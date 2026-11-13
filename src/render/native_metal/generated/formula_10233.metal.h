// Generated from dIFS/hexgridIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10233 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_HEXGRIDIFS = 10233;
 template < > FormulaVec3 execute_formula < FORMULA_HEXGRIDIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Spacing = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Grid_styling = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float Rho_sin = params [ 16 ];
 const float Rho_cos = params [ 17 ];
 const float Final_add = params [ 18 ];
 const float zThick = params [ 19 ];
 const float c_half = params [ 20 ];
 const float c_invs3 = params [ 21 ];
 const float c_ns32 = params [ 22 ];
 const float c_nhalf = params [ 23 ];
 const float c_s34 = params [ 24 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;

}
 const FormulaVec3 transformed ( x , y , z );
 float m00 = params [ 7 ] , m01 = params [ 8 ] , m02 = params [ 9 ];
 float m10 = params [ 10 ] , m11 = params [ 11 ] , m12 = params [ 12 ];
 float m20 = params [ 13 ] , m21 = params [ 14 ] , m22 = params [ 15 ];
 float rx = x * m00 + y * m01 + z * m02;
 float ry = x * m10 + y * m11 + z * m12;
 float rz = x * m20 + y * m21 + z * m22;
 x = rx;
 y = ry;
 z = rz;
 float ax = fabs ( x - Grid_styling );
 float fx = x87_frndint ( ax / Grid_styling * c_half - c_half );
 fx = fabs ( ax - ( fx * 2.0f + 1.0f ) * Grid_styling );
 float hy = Grid_styling * c_invs3;
 float ay = fabs ( y - hy );
 float fy = x87_frndint ( ay / hy * c_half - c_half );
 fy = fabs ( ay - ( fy * 2.0f + 1.0f ) * hy );
 float d1 = fx * c_ns32 + fy * c_nhalf + c_s34;
 float s1 = d1 - fabs ( d1 );
 float nfx = fabs ( fx - s1 * c_ns32 );
 float nfy = fabs ( fy - s1 * c_nhalf );
 float d2 = nfx * c_ns32 + nfy * c_nhalf + c_s34;
 float s2 = d2 - fabs ( d2 );
 nfx = nfx - s2 * c_ns32;
 nfy = fabs ( nfy - s2 * c_nhalf );
 float hex_dist = fabs ( nfx * Rho_sin + nfy * Rho_cos + Final_add );
 float z_dist = fabs ( z ) - zThick;
 float shape_de = max ( hex_dist , z_dist ) - Spacing;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return Apply_scale_add != 0 ? transformed : p;

}
 FormulaVec3 hexgridIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_HEXGRIDIFS > ( p , params , de );

}

}}
