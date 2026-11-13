// Diagnostic binary: expose production loader records for independent oracles.
#include "io/m3p_parser.h"
#include <cstdio>
#include <cstring>
#include <fstream>

int main(int argc, char** argv) {
    if (argc != 3) return 2;
    const auto result = mb3d::LoadM3PFile(argv[1]);
    if (!result.success) {
        std::fprintf(stderr, "%s\n", result.error.c_str());
        return 1;
    }
    std::ofstream out(argv[2], std::ios::binary);
    out.write(reinterpret_cast<const char*>(&result.header), sizeof(result.header));
    out.write(reinterpret_cast<const char*>(&result.addon), sizeof(result.addon));
    return out ? 0 : 3;
}
