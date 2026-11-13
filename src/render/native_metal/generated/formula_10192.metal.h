// Generated from dIFS/DecoCubeIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10192 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
struct Arm64ApplyIFSState {
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 FormulaVec3 original , transformed;
 FormulaParams params;
 bool apply;
 Arm64ApplyIFSState ( FormulaVec3 p , FormulaParams values , int applyIndex = 6 ) : original ( p ) , transformed ( p . x * values [ 1 ] + values [ 4 ] , p . y * values [ 1 ] + values [ 3 ] , p . z * values [ 1 ] + values [ 2 ] ) , params ( values ) , apply ( int ( values [ applyIndex ] ) != 0 ) {
 if ( apply ) {
 params [ DIFS_VARYSCALE_SLOT ] *= params [ 1 ];
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.f;

}

}
 FormulaVec3 rotate ( thread const float * m ) const {
 const E x = transformed . x , y = transformed . y , z = transformed . z;
 return FormulaVec3 ( float ( ( x * float ( m [ 0 ] ) + y * float ( m [ 1 ] ) ) + z * float ( m [ 2 ] ) ) , float ( ( x * float ( m [ 3 ] ) + y * float ( m [ 4 ] ) ) + z * float ( m [ 5 ] ) ) , float ( ( x * float ( m [ 6 ] ) + y * float ( m [ 7 ] ) ) + z * float ( m [ 8 ] ) ) );

}
 FormulaVec3 finish ( E distance , thread float & de ) const {
 if ( ! apply ) distance /= params [ 1 ];
 de = float ( distance );
 return apply ? transformed : original;

}

}
;
 constant constexpr unsigned int FORMULA_DECOCUBEIFS = 10192;
 template < > FormulaVec3 execute_formula < FORMULA_DECOCUBEIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Param_1 = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Param_2 = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float Surface_closeness = params [ 7 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;

}
 float x2 = x * x;
 float y2 = y * y;
 float z2 = z * z;
 float a1 = x2 + y2 - Param_1;
 float a2 = z2 - Param_2;
 float T1 = a1 * a1 + a2 * a2;
 float b1 = x2 + z2 - Param_1;
 float b2 = y2 - Param_2;
 float T2 = b1 * b1 + b2 * b2;
 float c1 = z2 + y2 - Param_1;
 float c2 = x2 - Param_2;
 float T3 = c1 * c1 + c2 * c2;
 float product = T1 * T2 * T3;
 float shape_de = sqrt ( sqrt ( product ) ) - Surface_closeness;
 if ( shape_de > 1.0f ) {
 shape_de = sqrt ( x2 + y2 + z2 );

}
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return Apply_scale_add != 0 ? FormulaVec3 ( x , y , z ) : p;

}
 FormulaVec3 DecoCubeIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_DECOCUBEIFS > ( p , params , de );

}

}}
