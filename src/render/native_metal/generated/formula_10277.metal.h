// Generated from 3D/Ikenagabulb.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10277 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_IKENAGABULB = 10277;
 static FormulaVec3 tpx_mul ( FormulaVec3 u , FormulaVec3 v ) {
 float rxy_u = sqrt ( u . x * u . x + u . y * u . y );
 float rxy_v = sqrt ( v . x * v . x + v . y * v . y );
 float prod = rxy_u * rxy_v;
 float rx , ry;
 if ( prod == 0.0f ) {
 rx = - v . z * u . z;
 ry = v . z * u . z;

}
 else {
 float alpha = v . z * u . z / prod;
 float factor = 1.0f - alpha;
 rx = ( u . x * v . x - u . y * v . y ) * factor;
 ry = ( v . x * u . y + u . x * v . y ) * factor;

}
 float rz = rxy_u * v . z + rxy_v * u . z;
 return FormulaVec3 ( rx , ry , rz );

}
 static FormulaVec3 tpx_mul_middle ( FormulaVec3 sa , FormulaVec3 c ) {
 float rxy_sa = sqrt ( sa . x * sa . x + sa . y * sa . y );
 float rxy_c = sqrt ( c . x * c . x + c . y * c . y );
 float prod = rxy_sa * rxy_c;
 float rx , ry;
 if ( prod == 0.0f ) {
 rx = - c . z * sa . z;
 ry = c . z * sa . z;

}
 else {
 float alpha = c . z * sa . z / prod;
 float factor = 1.0f - alpha;
 rx = ( sa . x * c . x - sa . y * c . y ) * factor;
 ry = ( c . x * sa . y + sa . z * c . y ) * factor;

}
 float rz = rxy_sa * c . z + rxy_c * sa . z;
 return FormulaVec3 ( rx , ry , rz );

}
 template < > FormulaVec3 execute_formula < FORMULA_IKENAGABULB > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 struct NativeClosure0 {
 auto operator ( ) ( FormulaVec3 u , FormulaVec3 v , bool middle ) const {
 const E ux = u . x , uy = u . y , uz = u . z , vx = v . x , vy = v . y , vz = v . z;
 const E ru = float ( sqrt ( ux * ux + uy * uy ) ) , rv = float ( sqrt ( vx * vx + vy * vy ) );
 const E product = ru * rv;
 E x , y;
 if ( ! ( product > E ( 0 ) || product < E ( 0 ) ) ) {
 x = ( - uz ) * vz;
 y = uz * vz;

}
 else {
 const E factor = E ( 1 ) - ( uz * vz ) / product;
 x = ( ux * vx - uy * vy ) * factor;
 y = ( vx * uy + ( middle ? uz : ux ) * vy ) * factor;

}
 return FormulaVec3 ( float ( x ) , float ( y ) , float ( ru * vz + rv * uz ) );

}

}
;
 const auto multiply = NativeClosure0 {

}
;
 const FormulaVec3 a ( params [ 0 ] , params [ 1 ] , params [ 2 ] ) , b ( params [ 3 ] , params [ 4 ] , params [ 5 ] );
 const FormulaVec3 c ( params [ 7 ] , params [ 8 ] , params [ 9 ] );
 const auto sa = multiply ( p , a , false ) , sac = multiply ( sa , c , true ) , sb = multiply ( p , b , false );
 const E linearX = float ( ( E ( sac . x ) - sb . x ) - c . x ) , linearY = float ( ( E ( sac . y ) - sb . y ) - c . y );
 const E x = p . x , y = p . y , z = p . z , k = params [ 6 ] , square = float ( x * x + y * y );
 E cubeX = 0 , cubeY = 0;
 if ( square != E ( 0 ) ) {
 const E factor = float ( E ( 1 ) - ( ( z * k ) * z ) / square );
 cubeX = float ( ( x * x - ( y * k ) * y ) * ( factor * x ) );
 cubeY = float ( ( ( x * k ) * x - y * y ) * ( factor * y ) );

}
 const E cubeZ = float ( ( square * k - z * z ) * z );
 return FormulaVec3 ( float ( linearX + cubeX ) , float ( linearY + cubeY ) , float ( cubeZ + sac . z ) );

}

}}
