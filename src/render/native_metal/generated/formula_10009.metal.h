// Generated from Ads/_Fold45single.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10009 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__FOLD45SINGLE = 10009;
 template < > FormulaVec3 execute_formula < FORMULA__FOLD45SINGLE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float k = params [ 0 ];
 const int Mode = int ( params [ 1 ] );
 const int Opp_x = int ( params [ 2 ] );
 const int Opp_y = int ( params [ 3 ] );
 const int Opp_z = int ( params [ 4 ] );
 const int PreRot = int ( params [ 5 ] );
 const float h = 0.66666666666666667f;
 float x = p . x , y = p . y , z = p . z;
 float xsig = ( Opp_x == 0 ) ? 1.0f : - 1.0f;
 float ysig = ( Opp_y == 0 ) ? 1.0f : - 1.0f;
 float zsig = ( Opp_z == 0 ) ? 1.0f : - 1.0f;
 if ( PreRot != 0 ) {
 float sx = ( Opp_x != 0 ) ? x : - x;
 float sy = ( Opp_y != 0 ) ? y : - y;
 float sz = ( Opp_z != 0 ) ? z : - z;
 float dd_pre = ( sx + sy + sz ) * h;
 x = ( Opp_x == 0 ) ? - ( x + dd_pre ) : - ( x - dd_pre );
 y = ( Opp_y == 0 ) ? - ( y + dd_pre ) : - ( y - dd_pre );
 z = ( Opp_z == 0 ) ? - ( z + dd_pre ) : - ( z - dd_pre );

}
 float dd = x * xsig + y * ysig + z * zsig - k;
 bool do_fold = ( Mode == 0 ) ? ( dd >= 0.0f ) : ( dd <= 0.0f );
 if ( do_fold ) {
 dd = dd * h;
 x = x - dd * xsig;
 y = y - dd * ysig;
 z = z - dd * zsig;

}
 return FormulaVec3 ( x , y , z );

}

}}
