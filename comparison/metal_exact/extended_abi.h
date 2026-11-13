#pragma once
#include "soft_extended.h"
namespace mb3d_metal_exact {
struct ExtendedInput { U64 a_sig,b_sig,c_f64; U32 a_exp,b_exp,op,reserved; };
struct ExtendedOutput { U64 significand; U32 sign_exponent,single_bits; U64 double_bits; int integer_rne,integer_rtz; };
#ifndef __METAL_VERSION__
static_assert(sizeof(ExtendedInput)==40);
static_assert(sizeof(ExtendedOutput)==32);
#endif
}
