#include "core/zbuffer_normals.h"
#include "core/header_trafos.h"

#include <array>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>

using namespace mb3d;

static void check(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

static TMandHeader10 makeHeader(int width, int height, int projection) {
    TMandHeader10 header{};
    header.Width = width;
    header.Height = height;
    header.dZoom = 1;
    header.dFOVy = 45;
    header.dZstart = -2;
    header.dZend = 2;
    header.sDEstop = 0.001f;
    header.bPlanarOptic = static_cast<uint8_t>(projection);
    CalcStepWidth(&header);
    return header;
}

static TsiLight5 pixel(int index) {
    TsiLight5 value{};
    value.NormalX = 123;
    value.NormalY = -234;
    value.NormalZ = 345;
    value.Zpos = static_cast<uint16_t>(32766 - index);
    value.RoughZposFine = static_cast<uint16_t>(index * 13);
    value.SIgradient = 1234;
    value.Shadow = 4321;
    value.AmbShadow = 2222;
    value.OTrap = 3333;
    return value;
}

int main() {
    int scenarios = 0;
    try {
        for (int projection : {0, 1, 2}) {
            for (const auto dimensions : {std::array<int, 2>{1, 1}, {1, 4}, {4, 1}, {4, 3}}) {
                auto header = makeHeader(dimensions[0], dimensions[1], projection);
                std::vector<TsiLight5> values(dimensions[0] * dimensions[1]);
                for (size_t i = 0; i < values.size(); ++i) values[i] = pixel(static_cast<int>(i));
                if (values.size() > 2) {
                    values[0].Zpos = 32768;
                    values.back().SIgradient = 32768;
                }
                const auto original = values;
                RecalculateNormalsFromZBuffer(header, values);
                int changed = 0;
                for (size_t i = 0; i < values.size(); ++i) {
                    if (original[i].Zpos >= 32768 || original[i].SIgradient >= 32768) {
                        check(std::memcmp(&values[i], &original[i], sizeof(TsiLight5)) == 0,
                              "normals stage modified background/interior pixels");
                    } else {
                        changed += std::memcmp(&values[i], &original[i], 6) != 0;
                    }
                    check(std::memcmp(reinterpret_cast<const char*>(&values[i]) + 6,
                                      reinterpret_cast<const char*>(&original[i]) + 6, 12) == 0,
                          "normals stage changed depth/material/shadow fields");
                }
                check(changed > 0, "normals stage did not affect foreground normals");
                auto second = original;
                RecalculateNormalsFromZBuffer(header, second);
                check(std::memcmp(values.data(), second.data(), values.size()*sizeof(TsiLight5)) == 0,
                      "normals stage is not deterministic");
                ++scenarios;
            }
        }
        auto header = makeHeader(4, 3, 0);
        std::vector<TsiLight5> shortBuffer(11);
        try {
            RecalculateNormalsFromZBuffer(header, shortBuffer);
            throw std::runtime_error("normals stage accepted an incomplete surface");
        } catch (const std::invalid_argument&) {
            ++scenarios;
        }
        std::printf("Z-buffer normals: %d scenarios passed.\n", scenarios);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Z-buffer normals FAILED: %s\n", error.what());
        return 1;
    }
}
