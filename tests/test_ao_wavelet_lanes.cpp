#include "core/ambient_occlusion.h"
#include "io/m3p_parser.h"
#include "fixtures/ao_wavelet_lanes_expected.h"
#include "fixtures/ao_threshold_expected.h"

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

using namespace mb3d;

static void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int main() {
    try {
        const auto fixtures = std::filesystem::path(__FILE__).parent_path() / "fixtures";
        const auto scene = LoadM3PFile((fixtures / "ao_wavelet_lanes_scene.m3p").string());
        check(scene.success, "independent AO fixture scene did not load");
        std::ifstream input(fixtures / "ao_wavelet_lanes_input.silight", std::ios::binary);
        char magic[8]{};
        uint32_t info[5]{};
        input.read(magic, 8);
        input.read(reinterpret_cast<char*>(info), sizeof(info));
        check(input && std::memcmp(magic, "SIL5DMP1", 8) == 0 && info[0] == 1 &&
              info[1] == 96 && info[2] == 72 && info[3] == sizeof(TsiLight5) && info[4] == 6912,
              "invalid independent AO fixture dimensions");
        std::vector<TsiLight5> initial(info[4]);
        input.read(reinterpret_cast<char*>(initial.data()), initial.size() * sizeof(TsiLight5));
        check(input.good(), "incomplete independent AO fixture input");
        std::ifstream thresholdInput(fixtures / "ao_threshold_input.silight", std::ios::binary);
        thresholdInput.seekg(28);
        std::vector<TsiLight5> thresholdInitial(initial.size());
        thresholdInput.read(reinterpret_cast<char*>(thresholdInitial.data()), thresholdInitial.size() * sizeof(TsiLight5));
        check(thresholdInput.good(), "incomplete independent threshold AO input");
        int scenarios = 0;
        std::vector<AOWaveletLaneFixture> cases(std::begin(kAOWaveletLaneFixtures), std::end(kAOWaveletLaneFixtures));
        cases.insert(cases.end(), std::begin(kAOThresholdFixtures), std::end(kAOThresholdFixtures));
        for (const auto& fixture : cases) {
            auto header = scene.header;
            header.bCalcAmbShadowAutomatic = static_cast<uint8_t>(1 + 2 * fixture.mode);
            header.SSAORcount = static_cast<uint8_t>(fixture.repeats);
            header.sAmbShadowThreshold = fixture.threshold;
            const auto& fixtureInput = fixture.mode < 2 ? thresholdInitial : initial;
            auto actual = fixtureInput;
            ReplayAmbientOcclusion(actual.data(), 96, 72, 96, nullptr, nullptr, 2, 0,
                                   &header, fixture.threads);
            uint64_t hash = UINT64_C(14695981039346656037);
            for (size_t i = 0; i < actual.size(); ++i) {
                const uint16_t amb = actual[i].AmbShadow;
                hash = (hash ^ (amb & 255)) * UINT64_C(1099511628211);
                hash = (hash ^ (amb >> 8)) * UINT64_C(1099511628211);
                actual[i].AmbShadow = fixtureInput[i].AmbShadow;
                check(std::memcmp(&actual[i], &fixtureInput[i], sizeof(TsiLight5)) == 0,
                      "AO modified unrelated packed fields");
            }
            if (hash != fixture.ambHash) {
                std::fprintf(stderr, "mode=%d repeats=%d threads=%d actual=%016llx expected=%016llx\n",
                             fixture.mode, fixture.repeats, fixture.threads,
                             static_cast<unsigned long long>(hash),
                             static_cast<unsigned long long>(fixture.ambHash));
                throw std::runtime_error("wavelet/lane AO state differs from independent Pascal");
            }
            ++scenarios;
        }
        auto tinyHeader = scene.header;
        tinyHeader.bCalcAmbShadowAutomatic = 1;
        auto tiny = initial.front();
        bool rejected = false;
        try {
            ReplayAmbientOcclusion(&tiny, 1, 1, 1, nullptr, nullptr, 2, 0, &tinyHeader, 1);
        } catch (const std::invalid_argument& error) {
            rejected = std::strstr(error.what(), "original wavelet buffer") != nullptr;
        }
        check(rejected, "undefined original scalar border must reject explicitly");
        // Re-run a valid case after the rejection to detect retained partial state.
        auto recovered = thresholdInitial;
        ReplayAmbientOcclusion(recovered.data(), 96, 72, 96, nullptr, nullptr, 2, 0, &tinyHeader, 1);
        uint64_t recoveryHash = UINT64_C(14695981039346656037);
        for (const auto& pixel : recovered) {
            recoveryHash = (recoveryHash ^ (pixel.AmbShadow & 255)) * UINT64_C(1099511628211);
            recoveryHash = (recoveryHash ^ (pixel.AmbShadow >> 8)) * UINT64_C(1099511628211);
        }
        check(recoveryHash == kAOThresholdFixtures[0].ambHash, "threshold AO did not recover after invalid buffer rejection");
        std::printf("AO wavelet/lane state: %d independent Pascal cases passed.\n", scenarios);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "AO wavelet/lane FAILED: %s\n", error.what());
        return 1;
    }
}
