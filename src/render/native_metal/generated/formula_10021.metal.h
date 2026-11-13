// Generated from Ads/_PartlyJuliaR.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10021 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__PARTLYJULIAR = 10021;
 template < > FormulaVec3 execute_formula < FORMULA__PARTLYJULIAR > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float dx = p . x - params [ 0 ] , dy = p . y - params [ 1 ] , dz = p . z - params [ 2 ];
 const float radius = params [ 3 ] , speed = params [ 4 ] , recenter = params [ 9 ];
 const int mode = int ( params [ 8 ] ) , shape = int ( params [ 10 ] );
 float r;
 if ( ( shape == - 2 || shape == - 4 ) && dz < 0 ) r = 1;
 else {
 if ( shape == 1 ) {
 const float ax = fabs ( dx ) , ay = fabs ( dy ) , az = fabs ( dz );
 const float yz = az > ay ? az : ay;
 r = yz > ax ? yz : ax;

}
 else if ( shape == 2 ) r = sqrt ( ( dx * dx + dy * dy ) / ( dz * dz ) );
 else if ( shape == - 2 ) r = sqrt ( dx * dx + dy * dy ) / dz;
 else if ( shape == 3 ) r = sqrt ( dx * dx + dy * dy );
 else if ( shape == 4 || shape == - 4 ) {
 const float ax = fabs ( dx ) , ay = fabs ( dy );
 r = ( ay > ax ? ay : ax ) / fabs ( dz );

}
 else r = native_formula_length3 ( dx , dy , dz );
 r /= radius;

}
 float cx = params [ 11 ] , cy = params [ 12 ] , cz = params [ 13 ];
 float offset = 0;
 bool move = false;
 if ( mode == 1 ) {
 if ( r < 1 ) {
 move = true;
 r -= speed;
 if ( r < 0 ) {
 cx = params [ 5 ];
 cy = params [ 6 ];
 cz = params [ 7 ];
 offset = recenter;

}
 else {
 r /= 1 - speed;
 cx = r * cx + ( 1 - r ) * params [ 5 ];
 cy = r * cy + ( 1 - r ) * params [ 6 ];
 cz = r * cz + ( 1 - r ) * params [ 7 ];
 offset = ( 1 - r ) * recenter;

}

}

}
 else if ( ! ( r < 1 ) ) {
 cx = params [ 5 ];
 cy = params [ 6 ];
 cz = params [ 7 ];

}
 else {
 move = true;
 r -= speed;
 if ( r < 0 ) {
 cx -= recenter * params [ 0 ];
 cy -= recenter * params [ 1 ];
 cz -= recenter * params [ 2 ];
 offset = recenter;

}
 else {
 r /= 1 - speed;
 cx = r * params [ 5 ] + ( 1 - r ) * ( cx - recenter * params [ 0 ] );
 cy = r * params [ 6 ] + ( 1 - r ) * ( cy - recenter * params [ 1 ] );
 cz = r * params [ 7 ] + ( 1 - r ) * ( cz - recenter * params [ 2 ] );
 offset = ( 1 - r ) * recenter;

}

}
 params [ WRITE_J1_SLOT ] = cx;
 params [ WRITE_J2_SLOT ] = cy;
 params [ WRITE_J3_SLOT ] = cz;
 if ( move ) return FormulaVec3 ( p . x - offset * params [ 0 ] , p . y - offset * params [ 1 ] , p . z - offset * params [ 2 ] );
 return p;

}

}}
