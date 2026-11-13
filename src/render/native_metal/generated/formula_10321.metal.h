// Generated from 4D/GeneralQuat.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10321 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_GENERALQUAT = 10321;
 template < > FormulaVec3 execute_formula < FORMULA_GENERALQUAT > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int IJ_yz = int ( params [ 0 ] );
 const int JI_yz = int ( params [ 1 ] );
 const int JJ_zz = int ( params [ 2 ] );
 const int IK_yw = int ( params [ 3 ] );
 const int KI_yw = int ( params [ 4 ] );
 const int JK_zw = int ( params [ 5 ] );
 const int KJ_zw = int ( params [ 6 ] );
 const int KK_ww = int ( params [ 7 ] );
 const int pvals [ 8 ] = {
 IJ_yz , JI_yz , JJ_zz , IK_yw , KI_yw , JK_zw , KJ_zw , KK_ww
}
;
 const numeric :: X87Scope extendedScope;
 const numeric :: X87Real x = p . x , y = p . y , z = p . z , w = de;
 float results [ 4 ] = {
 float ( x * x - y * y ) , float ( ( x * y ) + ( x * y ) ) , float ( ( x * z ) + ( x * z ) ) , float ( ( x * w ) + ( x * w ) )
}
;
 const numeric :: X87Real terms [ 8 ] = {
 y * z , y * z , z * z , y * w , y * w , z * w , z * w , w * w
}
;
 for ( int i = 0;
 i < 8;
 i ++ ) {
 unsigned int selector = static_cast < unsigned int > ( pvals [ i ] ) - 1u;
 numeric :: X87Real term = terms [ i ];
 if ( ( selector & 0x80000000u ) != 0u ) {
 selector = 0u - selector - 2u;
 term = - term;

}
 results [ selector & 3u ] = float ( term + results [ selector & 3u ] );

}
 results [ 0 ] += params [ 8 ];
 results [ 1 ] += params [ 9 ];
 results [ 2 ] += params [ 10 ];
 results [ 3 ] += params [ 11 ];
 de = results [ 3 ];
 return FormulaVec3 ( results [ 0 ] , results [ 1 ] , results [ 2 ] );

}

}}
