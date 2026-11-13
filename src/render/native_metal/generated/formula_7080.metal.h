// Generated from 3D/IQ-bulb.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7080 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
enum class Arm64IQKind {
 Bulb , Normal , Pine
}
;
 inline FormulaVec3 arm64IQ ( FormulaVec3 p , FormulaParams params , Arm64IQKind kind , bool addYZ ) {
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 const E x = p . x , y = p . y , z = p . z;
 const E radius = native_formula_length3 ( x , y , z );
 const E power = float ( extendedExp2 ( E ( params [ 0 ] ) * extendedLog2 ( abs ( radius ) ) , false ) );
 const bool yPole = kind == Arm64IQKind :: Bulb;
 const auto polar = extendedSinCos ( extendedAtan2 ( sqrt ( ( yPole ? x * x : y * y ) + z * z ) , yPole ? y : x ) * params [ 0 ] );
 const auto azimuth = extendedSinCos ( extendedAtan2 ( yPole ? x : y , z ) * params [ 1 ] );
 const E ps = float ( polar . sin ) , pc = float ( polar . cos );
 const E azs = float ( azimuth . sin ) , azc = float ( azimuth . cos );
 E ox , oy , oz;
 if ( kind == Arm64IQKind :: Bulb ) {
 ox = ( ps * azs ) * power;
 oy = pc * power;
 oz = ( ( ps * azc ) * params [ 2 ] ) * power;

}
 else if ( kind == Arm64IQKind :: Normal ) {
 ox = ps * power;
 oy = ( pc * azs ) * power;
 oz = ( ( pc * azc ) * params [ 2 ] ) * power;

}
 else {
 ox = pc * power;
 oy = ( ps * azc ) * power;
 oz = ( ( ps * azs ) * params [ 2 ] ) * power;

}
 ox += params [ 6 ];
 if ( addYZ ) {
 oy += params [ 7 ];
 oz += params [ 8 ];

}
 return FormulaVec3 ( float ( ox ) , float ( oy ) , float ( oz ) );

}
 constant constexpr unsigned int FORMULA_IQBULB = 7080;
 template < > FormulaVec3 execute_formula < FORMULA_IQBULB > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return arm64IQ ( p , params , Arm64IQKind :: Bulb , true );

}

}}
