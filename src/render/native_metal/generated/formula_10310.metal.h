// Generated from dIFS/WeirdCubeIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10310 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_WEIRDCUBEIFS = 10310;
 template < > FormulaVec3 execute_formula < FORMULA_WEIRDCUBEIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float MaxRadius = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Zadd = params [ 2 ];
 const float Yadd = params [ 3 ];
 const float Xadd = params [ 4 ];
 const float ShapeModder = params [ 5 ];
 const int ApplyScale = int ( params [ 6 ] );
 const float c25 = 25.0f;
 const float c50 = 50.0f;
 const float c_coeff1 = 0.520833333f;
 const float c_neg4 = - 4.0f;
 const float c_067 = 0.67f;
 float x = p . x * Scale + Xadd;
 float y = p . y * Scale + Yadd;
 float z = p . z * Scale + Zadd;
 const FormulaVec3 committed ( x , y , z );
 if ( ApplyScale != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;

}
 RotMatrix3 Rot = rotMatrix3FromFloats ( params , 7 );
 float rx = x * Rot . col0 . x + y * Rot . col1 . x + z * Rot . col2 . x;
 float ry = x * Rot . col0 . y + y * Rot . col1 . y + z * Rot . col2 . y;
 float rz = x * Rot . col0 . z + y * Rot . col1 . z + z * Rot . col2 . z;
 x = rx;
 y = ry;
 z = rz;
 float x2 = x * x;
 float y2 = y * y;
 float z2 = z * z;
 float xy = x * y;
 float yz = y * z;
 float xz = x * z;
 float xy2 = xy * xy;
 float yz2 = yz * yz;
 float xz2 = xz * xz;
 float pos_accum = c50 * ( xy2 + yz2 + xz2 );
 float neg_accum = 0.0f;
 float term1 = x * x2 * ( y + z ) * c25;
 if ( term1 < 0.0f ) neg_accum -= term1;
 else pos_accum += term1;
 float term2 = y * y2 * ( x + z ) * c25;
 if ( term2 < 0.0f ) neg_accum -= term2;
 else pos_accum += term2;
 float term3 = z * z2 * ( x + y ) * c25;
 if ( term3 < 0.0f ) neg_accum -= term3;
 else pos_accum += term3;
 float term4 = x * yz * ShapeModder;
 if ( term4 < 0.0f ) neg_accum -= term4;
 else pos_accum += term4;
 float coeff_mix = c_coeff1 * ShapeModder;
 float term5 = ( x2 * coeff_mix + 1.0f ) * yz * c_neg4;
 if ( term5 < 0.0f ) neg_accum -= term5;
 else pos_accum += term5;
 float term6 = ( y2 * coeff_mix + 1.0f ) * xz * c_neg4;
 if ( term6 < 0.0f ) neg_accum -= term6;
 else pos_accum += term6;
 float term7 = ( z2 * coeff_mix + 1.0f ) * xy * c_neg4;
 if ( term7 < 0.0f ) neg_accum -= term7;
 else pos_accum += term7;
 float shape_de = sqrt ( sqrt ( pos_accum ) ) - sqrt ( sqrt ( neg_accum ) );
 shape_de *= c_067;
 float r = native_formula_length3 ( x , y , z ) - MaxRadius;
 if ( r >= shape_de ) {
 shape_de = r;

}
 if ( ApplyScale == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return ApplyScale != 0 ? committed : p;

}
 FormulaVec3 WeirdCubeIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_WEIRDCUBEIFS > ( p , params , de );

}

}}
