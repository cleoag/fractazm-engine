#include "core/header_trafos.h"
#include "core/formulas.h"
#include <cfenv>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "fixtures/integer_option_rounding.h"
#include "fixtures/option_capacity.h"

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" unsigned short GetX87ControlWord();
extern "C" void SetFixtureX87ControlWord(unsigned short);
#endif

static int CheckOptionCapacity() {
    int failed = 0;
    for (const auto& fixture : kOptionCapacityFixtures) {
        THeaderCustomAddon addon{};
        auto& f = addon.Formulas[0];
        f.iFnr = 0;
        f.iOptionCount = fixture.count;
        std::memcpy(f.byOptionType, fixture.types, 16);
        for (int i = 0; i < 16; ++i) f.dOptionValue[i] = i + 2;
        mb3d::BuildFormulaConstBuffer(0, &addon, 0);
        const auto* p = static_cast<const uint8_t*>(mb3d::GetFormulaConstBuffers().pConstPointer[0]);
        const bool pointerCase = std::strcmp(fixture.name, "folding_pointer_then_capacity") == 0;
        for (int i = 0; i < 256; ++i) {
            const int offset = i - 256;
            unsigned expected;
            std::sscanf(fixture.bytes + i * 2, "%2x", &expected);
            // Original FOLDING pointer occupies [-60,-56), native pointer
            // [-56-sizeof(pointer),-56). Compare its numeric neighbours only.
            const bool pointerByte = pointerCase && offset >= -60 && offset < -56;
            if (pointerByte) continue;
            const int nativeOffset = pointerCase && offset < -60
                ? offset - (static_cast<int>(sizeof(void*)) - 4) : offset;
            if (p[nativeOffset] != expected) {
                std::fprintf(stderr, "capacity %s offset=%d got=%u expected=%u\n",
                    fixture.name, offset, p[nativeOffset], expected);
                ++failed;
            }
        }
    }
    std::printf("Option capacity: 7 original Pascal buffers, failures=%d\n", failed);
    return failed;
}

int main() {
    const int originalMode = std::fegetround();
#if defined(_MSC_VER) && defined(_M_X64)
    const auto originalX87 = GetX87ControlWord();
#endif
    const int modes[] = {FE_TONEAREST, FE_DOWNWARD, FE_UPWARD, FE_TOWARDZERO};
    int failed = 0;
    for (const auto& fixture : kIntegerOptionFixtures) {
        std::fesetround(modes[fixture.mode]);
#if defined(_MSC_VER) && defined(_M_X64)
        // MSVC x64 fesetround updates SSE only; Pascal Round uses x87 FISTP.
        SetFixtureX87ControlWord(static_cast<unsigned short>(
            (originalX87 & ~0x0c00u) | (fixture.mode << 10)));
#endif
        THeaderCustomAddon addon{};
        auto& formula = addon.Formulas[0];
        formula.iFnr = 0;
        formula.iOptionCount = 2;
        formula.byOptionType[0] = 2;
        formula.byOptionType[1] = 20;
        std::memcpy(&formula.dOptionValue[0], &fixture.bits, 8);
        std::memcpy(&formula.dOptionValue[1], &fixture.bits, 8);
        mb3d::BuildFormulaConstBuffer(0, &addon, 0);
        const auto* p = static_cast<const uint8_t*>(mb3d::GetFormulaConstBuffers().pConstPointer[0]);
        for (int offset : {-12, -16, -20}) {
            int32_t actual;
            std::memcpy(&actual, p + offset, 4);
            if (actual != fixture.expected) {
                std::fprintf(stderr, "mode=%d bits=%016llx offset=%d got=%d expected=%d\n",
                    fixture.mode, static_cast<unsigned long long>(fixture.bits),
                    offset, actual, fixture.expected);
                ++failed;
            }
        }
        formula.iOptionCount = 3;
        formula.byOptionType[0] = 10; // NOVARIABLE: power selector
        formula.byOptionType[1] = 0;
        formula.byOptionType[2] = 8;  // FOLDING: four doubles + function pointer
        formula.dOptionValue[1] = 1.0;
        formula.dOptionValue[2] = 1.0;
        mb3d::BuildFormulaConstBuffer(0, &addon, 0);
        p = static_cast<const uint8_t*>(mb3d::GetFormulaConstBuffers().pConstPointer[0]);
        ThybridIteration function;
        std::memcpy(&function, p - 48 - sizeof(function), sizeof(function));
        if (function != mb3d::fHIntFunctions[fixture.expected_power - 2]) {
            std::fprintf(stderr, "FOLDING mode=%d bits=%016llx expected power=%d\n",
                fixture.mode, static_cast<unsigned long long>(fixture.bits), fixture.expected_power);
            ++failed;
        }
    }
    std::fesetround(originalMode);
#if defined(_MSC_VER) && defined(_M_X64)
    SetFixtureX87ControlWord(originalX87);
#endif
    std::printf("Integer options: 308 Pascal fixtures, 924 packed stores + 308 FOLDING selections, failures=%d\n", failed);
    failed += CheckOptionCapacity();
    return failed ? 1 : 0;
}
