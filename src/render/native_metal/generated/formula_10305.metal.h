// Generated from dIFS/foldcut12-IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10305 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_FOLDCUT12IFS = 10305;
 template < > FormulaVec3 execute_formula < FORMULA_FOLDCUT12IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Size = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const int Apply_scale_add = int ( params [ 5 ] );
 const float R = params [ 6 ];
 const float Normal1 = params [ 7 ];
 const float Normal2 = params [ 8 ];
 const float Normal3 = params [ 9 ];
 const float n1 = - 0.809017f;
 const float n2 = 0.309017f;
 const float n3 = 0.5f;
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 float state_x = p . x;
 float state_y = p . y;
 float state_z = p . z;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;
 state_x = x;
 state_y = y;
 state_z = z;

}
 for ( int iter = 0;
 iter < 2;
 iter ++ ) {
 x = fabs ( x );
 y = fabs ( y );
 z = fabs ( z );
 float dot1 = x * n1 + y * n2 + z * n3;
 float cut1 = - ( dot1 + fabs ( dot1 ) );
 x += cut1 * n1;
 y += cut1 * n2;
 z += cut1 * n3;
 float dot2 = x * n2 - y * n3 - z * n1;
 float cut2 = - ( dot2 + fabs ( dot2 ) );
 x += cut2 * n2;
 y += cut2 * ( - n3 );
 z += cut2 * ( - n1 );

}
 float plane_de = fabs ( x ) * Normal2 + fabs ( y ) * Normal1 + fabs ( z ) * Normal3 - Size;
 float r = native_formula_length3 ( x , y , z );
 float sphere_de = r - fabs ( R );
 float shape_de;
 if ( R >= 0.0f ) {
 shape_de = min ( sphere_de , plane_de );

}
 else {
 shape_de = max ( sphere_de , plane_de );

}
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 foldcut12IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_FOLDCUT12IFS > ( p , params , de );

}

}}
