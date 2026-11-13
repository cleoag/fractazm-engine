// Generated from dIFS/NewMengerIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10119 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_NEWMENGERIFS = 10119;
 template < > FormulaVec3 execute_formula < FORMULA_NEWMENGERIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float MengerScale = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float MengerDiv = params [ 5 ];
 const int Apply_sa = int ( params [ 6 ] );
 const float XOff = params [ 7 ];
 const float YOff = params [ 8 ];
 const float ZOff = params [ 9 ];
 int MaxIter = int ( params [ 10 ] );
 const float MengerRC = params [ 20 ];
 const float C0 = params [ 21 ];
 const float C1 = params [ 22 ];
 const float C2 = params [ 23 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 float state_x = p . x;
 float state_y = p . y;
 float state_z = p . z;
 if ( Apply_sa != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;
 state_x = x;
 state_y = y;
 state_z = z;

}
 MaxIter = MaxIter & 0x1F;
 if ( MaxIter == 0 ) MaxIter = 1;
 x = x * C0 + C0;
 y = y * C0 + C0;
 z = z * C0 + C0;
 float dx = fabs ( x - C0 ) - C0;
 float dy = fabs ( y - C0 ) - C0;
 float dz = fabs ( z - C0 ) - C0;
 float max_de = dx;
 if ( dy > max_de ) max_de = dy;
 if ( dz > max_de ) max_de = dz;
 float accum_de = max_de;
 float scale_factor = 1.0f;
 for ( int iter = 0;
 iter < MaxIter;
 iter ++ ) {
 float zf = - fabs ( x87_fprem ( z * MengerDiv * scale_factor , MengerDiv ) - MengerRC ) + C0 + ZOff;
 float yf = - fabs ( x87_fprem ( y * MengerDiv * scale_factor , MengerDiv ) - MengerRC ) + C0 + YOff;
 float xf = - fabs ( x87_fprem ( x * MengerDiv * scale_factor , MengerDiv ) - MengerRC ) + C0 + XOff;
 float rx = xf * params [ 17 ] + yf * params [ 18 ] + zf * params [ 19 ];
 float ry = xf * params [ 11 ] + yf * params [ 12 ] + zf * params [ 13 ];
 float rz = xf * params [ 14 ] + yf * params [ 15 ] + zf * params [ 16 ];
 float mxy = max ( rx , ry );
 float mxz = max ( rx , rz );
 float myz = max ( ry , rz );
 float cross_de = min ( min ( mxy , mxz ) , myz );
 float iter_de = cross_de / scale_factor;
 if ( ! ( accum_de > iter_de ) ) accum_de = iter_de;
 scale_factor *= MengerScale;

}
 if ( Apply_sa == 0 ) {
 accum_de /= Scale;

}
 de = accum_de;
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 NewMengerIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_NEWMENGERIFS > ( p , params , de );

}

}}
