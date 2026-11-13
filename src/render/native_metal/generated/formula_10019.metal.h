// Generated from Ads/_RotateRnd.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10019 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
inline int incrementFormulaCounter ( FormulaParams params ) {
 return native_increment_formula_counter ( params );

}
 inline uint formulaCounterRandomWord ( int count ) {
 const uint low = ( static_cast < uint > ( count ) - 0x3D07u ) & 0xFFFFu;
 return ( low * low ) & 0xFFFFu;

}
 constant constexpr unsigned int FORMULA__ROTATERND = 10019;
 template < > FormulaVec3 execute_formula < FORMULA__ROTATERND > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const uint word = formulaCounterRandomWord ( incrementFormulaCounter ( params ) );
 float xyz [ ] = {
 p . x , p . y , p . z
}
;
 const int axes [ ] [ 2 ] = {
 {
 0 , 1
}
 , {
 0 , 2
}
 , {
 1 , 2
}

}
;
 for ( int plane = 0;
 plane < 3;
 ++ plane ) {
 const uint seed = static_cast < uint > ( native_int32_parameter ( params , 3 + plane ) );
 const float randomValue = ( ( seed & 0xFFFFu ) * word ) & 0x3FFFu;
 const int a = axes [ plane ] [ 0 ] , b = axes [ plane ] [ 1 ];
 if ( ! ( fabs ( params [ plane ] ) <= params [ 6 ] ) ) continue;
 float angle = ( static_cast < float > ( randomValue ) * params [ 7 ] - params [ 8 ] ) * params [ plane ];
 bool flip = false;
 if ( ! ( angle <= 1.f ) ) {
 angle = params [ 6 ] - angle;
 flip = true;

}
 else if ( ! ( angle > - 1.f ) ) {
 angle = - ( params [ 6 ] + angle );
 flip = true;

}
 float cosine = sqrt ( 1.f - angle * angle );
 if ( flip ) cosine = - cosine;
 const float x = float ( xyz [ a ] * cosine + xyz [ b ] * angle );
 xyz [ b ] = float ( xyz [ b ] * cosine - xyz [ a ] * angle );
 xyz [ a ] = x;

}
 return FormulaVec3 ( xyz [ 0 ] , xyz [ 1 ] , xyz [ 2 ] );

}

}}
