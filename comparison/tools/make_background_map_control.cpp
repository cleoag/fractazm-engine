#include "io/m3p_parser.h"
#include <cstring>
#include <filesystem>
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3 || std::filesystem::exists(argv[2])) return 2;
    auto input = mb3d::LoadM3PFile(argv[1]);
    if (!input.success || !input.header.Light.BGbmp[0]) return 3;
    auto header = input.header;
    header.Light.BGbmp[0] = 0;
    if (!mb3d::SaveM3PFile(argv[2], header, input.addon)) return 4;
    auto check = mb3d::LoadM3PFile(argv[2]);
    if (!check.success || std::memcmp(&header, &check.header, sizeof(header)) ||
        std::memcmp(&input.addon, &check.addon, sizeof(input.addon))) return 5;
    std::cout << "Only BGbmp[0] disabled; decoded header/addon exact\n";
}
