// Generated from dIFS/waveIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5197 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_WAVEIFS = 5197;
 template < > FormulaVec3 execute_formula < FORMULA_WAVEIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Frequency = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Amplitude = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const int Circular = int ( params [ 7 ] );
 const int Flip_XZ = int ( params [ 8 ] );
 const int Circ_R_off = int ( params [ 9 ] );
 float state_x = p . x;
 float state_y = p . y;
 float state_z = p . z;
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;
 state_x = x;
 state_y = y;
 state_z = z;

}
 float h_var = ( Flip_XZ & 1 ) != 0 ? z : x;
 float other_var = ( Flip_XZ & 1 ) != 0 ? x : z;
 float amp_eff = Amplitude;
 float wave;
 if ( Circular != 0 ) {
 float r = sqrt ( h_var * h_var + y * y );
 float angle = r * Frequency;
 if ( Circ_R_off != 0 ) {
 amp_eff = Amplitude / ( 1.0f + r * Frequency );

}
 wave = cos ( angle );

}
 else {
 wave = cos ( h_var * Frequency ) + cos ( y * Frequency );

}
 float shape_de = wave * amp_eff - other_var;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 waveIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_WAVEIFS > ( p , params , de );

}

}}
