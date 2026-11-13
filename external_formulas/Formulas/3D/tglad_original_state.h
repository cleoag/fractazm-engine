#pragma once

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void TgladTetraOriginal_x87(double*, const void*);
extern "C" void TgladTetra2Original_x87(double*, const void*);
extern "C" void TgladDihedOriginal_x87(double*, const void*);
extern "C" void TgladDihed2Original_x87(double*, const void*);

// The two integer selectors precede seven immutable Double constants in PVar.
// The native body adds Julia C only on the normal exit, before its final stores.
inline cpu_double3 execute_tglad_original(cpu_double3 p, const double* params,
                                         void (*body)(double*, const void*)) {
    alignas(16) unsigned char buffer[80]{};
    auto* var = buffer + 16;
    const int32_t outer = static_cast<int32_t>(params[0]);
    const int32_t inner = static_cast<int32_t>(params[1]);
    std::memcpy(var - 12, &outer, sizeof(outer));
    std::memcpy(var - 16, &inner, sizeof(inner));
    for (int i = 0; i < 7; ++i)
        std::memcpy(var + 8 * i, params + 2 + i, sizeof(double));
    double state[] = {p.x, p.y, p.z, params[9], params[10], params[11]};
    body(state, var);
    return cpu_double3(state[0], state[1], state[2]);
}
#endif
