#pragma once

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void greekIFSOriginal_x87(double*, const void*);
extern "C" void KleinBagelIFSOriginal_x87(double*, const void*);
extern "C" void schwartzIFSOriginal_x87(double*, const void*);
extern "C" void tritgridIFSOriginal_x87(double*, const void*);

// Reconstruct the numeric PVar layout from expanded parameters. This preserves
// original Single matrix coefficients and Integer flags without pointer state.
// The native routine owns both the SSE2 transform/commit and the x87 distance.
template<int Kind>
inline cpu_double3 execute_original_ifs_apply(cpu_double3 p, double* params, double& de) {
    alignas(16) unsigned char buffer[128]{};
    auto* var = buffer + 112;
    const auto real = [&](int offset, double value) { std::memcpy(var + offset, &value, 8); };
    const auto integer = [&](int offset, double value) {
        const int32_t flag = static_cast<int32_t>(value);
        std::memcpy(var + offset, &flag, 4);
    };
    real(-16, params[0]);
    real(-32, params[1]); real(-24, params[1]);
    real(-48, params[2]); real(-40, params[2]);
    real(-56, params[3]); real(-64, params[4]);
    if constexpr (Kind == 0) {
        integer(-68, params[5]); integer(-72, params[6]); integer(-76, params[7]);
    } else {
        real(-72, params[5]); integer(-76, params[6]);
    }
    if constexpr (Kind == 1) {
        integer(-80, params[7]); integer(-84, params[8]); real(-92, params[9]);
        real(0, params[10]);
    } else {
        constexpr int first = Kind == 0 ? 8 : 7;
        for (int i = 0; i < 9; ++i) {
            const float coefficient = static_cast<float>(params[first + i]);
            std::memcpy(var - 80 - 4*i, &coefficient, 4);
        }
        real(0, params[first + 9]);
        if constexpr (Kind == 3) real(8, params[first + 10]);
    }
    double state[] = {p.x, p.y, p.z, params[DIFS_VARYSCALE_SLOT], 0.};
    if constexpr (Kind == 0) greekIFSOriginal_x87(state, var);
    if constexpr (Kind == 1) KleinBagelIFSOriginal_x87(state, var);
    if constexpr (Kind == 2) schwartzIFSOriginal_x87(state, var);
    if constexpr (Kind == 3) tritgridIFSOriginal_x87(state, var);
    params[DIFS_VARYSCALE_SLOT] = state[3];
    de = state[4];
    return cpu_double3(state[0], state[1], state[2]);
}
#endif
