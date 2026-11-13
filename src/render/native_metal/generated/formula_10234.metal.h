// Generated from dIFS/NewFoldCutIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10234 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_NEWFOLDCUTIFS = 10234;
 template < > FormulaVec3 execute_formula < FORMULA_NEWFOLDCUTIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float IterScale = params [ 0 ];
 const float Scale = params [ 1 ];
 const float preZ_off = params [ 2 ];
 const float preY_off = params [ 3 ];
 const float preX_off = params [ 4 ];
 const int Apply_NoRestore = int ( params [ 7 ] );
 const float XOff = params [ 8 ];
 const float YOff = params [ 9 ];
 const float ZOff = params [ 10 ];
 const int MaxIter = int ( params [ 11 ] );
 const float c_initDE = params [ 21 ];
 float pre_x = p . x * Scale;
 float pre_y = p . y * Scale;
 float pre_z = p . z * Scale;
 float x = pre_x + preX_off;
 float y = pre_y + preY_off;
 float z = pre_z + preZ_off;
 if ( Apply_NoRestore >= 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;

}
 float running_scale = 1.0f;
 float running_de = c_initDE;
 int iters = MaxIter & 0x1F;
 if ( iters == 0 ) iters = 1;
 for ( int it = 0;
 it < iters;
 it ++ ) {
 float m00 = params [ 12 ] , m01 = params [ 13 ] , m02 = params [ 14 ];
 float m10 = params [ 15 ] , m11 = params [ 16 ] , m12 = params [ 17 ];
 float m20 = params [ 18 ] , m21 = params [ 19 ] , m22 = params [ 20 ];
 float rx = pre_x * m00 + pre_y * m01 + pre_z * m02;
 float ry = pre_x * m10 + pre_y * m11 + pre_z * m12;
 float rz = pre_x * m20 + pre_y * m21 + pre_z * m22;
 float ax = rx * IterScale;
 float abs_ax = fabs ( ax );
 float cut_x = abs_ax - preX_off;
 float ay = ry * IterScale;
 float abs_ay = fabs ( ay );
 float cur_max = ( abs_ax > abs_ay ) ? abs_ax : abs_ay;
 float cut_y = abs_ay - preY_off;
 float az = rz * IterScale;
 float abs_az = fabs ( az );
 cur_max = ( cur_max > abs_az ) ? cur_max : abs_az;
 float cut_z = abs_az - preZ_off;
 float sorted [ 3 ] = {
 cut_x , cut_y , cut_z
}
;
 if ( sorted [ 0 ] > sorted [ 1 ] ) {
 float t = sorted [ 0 ];
 sorted [ 0 ] = sorted [ 1 ];
 sorted [ 1 ] = t;

}
 if ( sorted [ 1 ] > sorted [ 2 ] ) {
 float t = sorted [ 1 ];
 sorted [ 1 ] = sorted [ 2 ];
 sorted [ 2 ] = t;

}
 if ( sorted [ 0 ] > sorted [ 1 ] ) {
 float t = sorted [ 0 ];
 sorted [ 0 ] = sorted [ 1 ];
 sorted [ 1 ] = t;

}
 float v0 = sorted [ 1 ] - YOff;
 float v1 = sorted [ 2 ] - XOff;
 float v2 = sorted [ 0 ] - ZOff;
 if ( v0 > v1 ) {
 float t = v0;
 v0 = v1;
 v1 = t;

}
 if ( v1 > v2 ) {
 float t = v1;
 v1 = v2;
 v2 = t;

}
 if ( v0 > v1 ) {
 float t = v0;
 v0 = v1;
 v1 = t;

}
 running_scale *= fabs ( IterScale );
 float numerator = cur_max - 1.0f;
 float sin_xy = params [ 5 ];
 float cos_xy = params [ 6 ];
 float denom_val = v2 * cos_xy + v1 * sin_xy;
 if ( IterScale < 0.0f ) {
 denom_val = - denom_val;

}
 float iter_de;
 if ( denom_val > numerator ) {
 iter_de = denom_val;

}
 else {
 iter_de = numerator;

}
 iter_de = iter_de / running_scale;
 if ( iter_de < running_de ) {
 running_de = iter_de;

}
 if ( Apply_NoRestore <= 0 ) {
 pre_x = ax;
 pre_y = ay;
 pre_z = az;

}
 else {
 pre_x = v2;
 pre_y = v1;
 pre_z = v0;

}

}
 if ( Apply_NoRestore >= 0 ) {
 running_de /= Scale;

}
 de = running_de;
 if ( Apply_NoRestore >= 0 ) {
 return FormulaVec3 ( x , y , z );

}
 return p;

}
 FormulaVec3 NewFoldCutIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_NEWFOLDCUTIFS > ( p , params , de );

}

}}
