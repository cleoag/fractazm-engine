#pragma once

inline int32_t incrementFormulaCounter(double* params) {
    // INC [ESI+D0] owns bFirstIt; it does not modify ItResultI.
    const uint32_t bits = static_cast<uint32_t>(static_cast<int32_t>(params[BFIRSTIT_SLOT])) + 1u;
    int32_t count;
    std::memcpy(&count, &bits, sizeof(count));
    params[BFIRSTIT_SLOT] = static_cast<double>(count);
    return count;
}

inline uint32_t formulaCounterRandomWord(int32_t count) {
    // Only the low word of the original SUB AX / MUL EAX is consumed.
    const uint32_t low = (static_cast<uint32_t>(count) - 0x3D07u) & 0xFFFFu;
    return (low * low) & 0xFFFFu;
}
