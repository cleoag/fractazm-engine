#include "core/dof.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>

// Independent exact focus invariant from mb3d/DOF.pas:
// raw depth 8388352 decodes to zero, sharp=sharp2=0 gives radius=0.
// Round(Abs(radius)+0.5)=0 in nearest-even mode. Each source contributes only
// to itself: plain weight 0.5, sorted weight 1; normalization restores RGB.
// This catches the Pascal 1-based -> C++ 0-based plain scatter bounds defect.
int main() {
    try {
        int cases = 0;
        for (int width : {1, 2, 4, 9}) {
            for (int height : {1, 2, 5}) {
                for (int padding : {0, 3}) {
                    const int stride = width + padding;
                    std::vector<uint8_t> expected(stride * height * 3 + 16, 0xa5);
                    for (int y = 0; y < height; ++y)
                        for (int x = 0; x < width; ++x)
                            for (int c = 0; c < 3; ++c)
                                expected[(y * stride + x) * 3 + c] =
                                    static_cast<uint8_t>((17 + x * 31 + y * 47 + c * 59) & 255);
                    std::vector<TsiLight5> depth(width * height);
                    for (auto& pixel : depth) {
                        const uint32_t packed = 8388352u << 8;
                        std::memcpy(&pixel.RoughZposFine, &packed, sizeof(packed));
                    }
                    const auto originalDepth = depth;
                    for (int plain : {0, 8}) {
                        // Direct header mode3 is a fourth pass in Pascal's
                        // for pass:=0 to z caller, despite the GUI radio range.
                        for (int passMode : {0, 1, 2, 3}) {
                            TMandHeader10 header{};
                            header.Width = width; header.Height = height;
                            header.dZoom = 1; header.dFOVy = 45;
                            header.dZstart = -2; header.dZend = 2;
                            header.bCalc3D = 1;
                            header.sDOFaperture = 1.25f;
                            header.sDOFclipR = 25;
                            header.sDOFZsharp = header.sDOFZsharp2 = 0;
                            header.bCalcDOFtype = 1 | plain | (passMode << 1);
                            auto actual = expected;
                            mb3d::CalcDepthOfField(actual.data(), depth.data(), width, height,
                                                  stride, nullptr, &header);
                            if (actual != expected)
                                throw std::runtime_error("DOF changed exact-focus RGB or row/tail canaries");
                            if (std::memcmp(depth.data(), originalDepth.data(), depth.size() * sizeof(TsiLight5)))
                                throw std::runtime_error("DOF changed its input depth buffer");
                            ++cases;
                        }
                    }
                }
            }
        }
        std::printf("DOF exact-focus invariance: %d scenarios passed; both drivers, 1/2/3/4 passes, edges and strides.\n", cases);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "DOF invariant failed: %s\n", error.what());
        return 1;
    }
}
