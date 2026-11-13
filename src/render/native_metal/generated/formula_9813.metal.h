// Generated from 3D/Quadrat3D.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9813 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_QUADRAT3D = 9813;
 template < > FormulaVec3 execute_formula < FORMULA_QUADRAT3D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float X_mul = params [ 0 ];
 const float Y_mul = params [ 1 ];
 const float Z_mul = params [ 2 ];
 const float HRadius = params [ 3 ];
 const float ZRadius = params [ 4 ];
 const float Cx = params [ 5 ];
 const float Cy = params [ 6 ];
 const float Cz = params [ 7 ];
 float x = p . x , y = p . y , z = p . z;
 float temp = x * x + y * y + HRadius;
 if ( temp > 0.0f ) {
 float factor = ZRadius - z * z / temp;
 FormulaVec3 result;
 result . x = X_mul * ( x * x - y * y ) * factor + Cx;
 result . y = Y_mul * x * y * factor + Cy;
 result . z = Z_mul * z * sqrt ( temp ) + Cz;
 return result;

}
 else if ( temp == 0.0f ) {
 return FormulaVec3 ( Cx , Cy , Cz );

}
 else {
 return FormulaVec3 ( 0.0f , 0.0f , 0.0f );

}

}
 FormulaVec3 Quadrat3D_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float X_mul = params [ 0 ];
 const float Y_mul = params [ 1 ];
 const float Z_mul = params [ 2 ];
 const float HRadius = params [ 3 ];
 const float ZRadius = params [ 4 ];
 const float Cx = params [ 5 ];
 const float Cy = params [ 6 ];
 const float Cz = params [ 7 ];
 float x = p . x , y = p . y , z = p . z;
 float temp = x * x + y * y + HRadius;
 if ( temp > 0.0f ) {
 float factor = ZRadius - z * z / temp;
 return FormulaVec3 ( X_mul * ( x * x - y * y ) * factor + Cx , Y_mul * x * y * factor + Cy , Z_mul * z * sqrt ( temp ) + Cz );

}
 else if ( temp == 0.0f ) {
 return FormulaVec3 ( Cx , Cy , Cz );

}
 else {
 return FormulaVec3 ( 0.0f );

}

}

}}
