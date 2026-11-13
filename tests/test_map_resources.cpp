#include "core/render_context.h"
#include "core/thread_pool.h"
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <vector>

namespace {

void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

void TestResourceOwners() {
    const auto caller = mb3d::CaptureRenderContext();
    std::weak_ptr<mb3d::RenderContext> retired[2];
    {
        mb3d::ThreadPool pool(4);
        std::vector<std::future<void>> tasks;
        for (int job = 0; job < 2; ++job) {
            const auto root = std::filesystem::path("native-resource-test") / std::to_string(job);
            auto context = std::make_shared<mb3d::RenderContext>(root.string());
            retired[job] = context;
            check(context->maps.resource_root == std::filesystem::absolute(root).lexically_normal().string(),
                  "resource root was not captured as an absolute path");
            check(context->maps.frame_number == 1 && context->maps.missing == 0 &&
                  context->maps.sequences.Count() == 0 && context->maps.banned_numbers.empty(),
                  "fresh map owner inherited another job's state");
            mb3d::ScopedRenderContext binding(context);
            for (int slot = 0; slot < 3; ++slot) {
                auto& map = mb3d::GetCalcMaps()[slot];
                map.LMa = new uint32_t[4]{static_cast<uint32_t>(job * 10 + slot), 0, 0, 0};
                map.LMnumber = job * 10 + slot + 1;
                map.LMWidth = map.LMHeight = 1;
            }
            // Exercise the real destructor's borrowed view: freeing both records
            // would double-delete this allocation under an ordinary or ASan run.
            context->lighting_maps.background.LMa = new uint32_t[4]{1, 2, 3, 4};
            context->lighting_maps.small_background = context->lighting_maps.background;
            for (auto& light : context->lighting_maps.lights) light.LMa = new uint32_t[4]{};
            context->lighting_maps.diffuse.LMa = new uint32_t[4]{};
            mb3d::SetCurrMapFrameNumber(job + 7);
            mb3d::GetMapMissingState().store(job);
            context->maps.banned_numbers.insert(job + 91);
            mb3d::MapSequence sequence;
            sequence.destChannel = job + 11;
            sequence.imageFilename = "texture001.bmp";
            sequence.firstImage = 1;
            sequence.lastImage = 9;
            sequence.increment = 2;
            mb3d::GetMapSequenceList().AddSequence(sequence);
            for (int task = 0; task < 16; ++task) {
                tasks.push_back(pool.submit([job] {
                    check(mb3d::GetCurrMapFrameNumber() == job + 7, "map frame escaped its owner");
                    check(mb3d::GetMapMissingState().load() == job, "map miss escaped its owner");
                    check(mb3d::CurrentRenderContext().maps.banned_numbers.count(job + 91) == 1,
                          "negative map cache escaped its owner");
                    for (int slot = 0; slot < 3; ++slot)
                        check(mb3d::GetCalcMaps()[slot].LMa[0] == static_cast<uint32_t>(job * 10 + slot),
                              "map pixels escaped their owner");
                    const auto* sequence = mb3d::GetMapSequenceList().GetSequence(job + 11);
                    check(sequence && sequence->GetFilename(4) == "texture007.bmp",
                          "map sequence escaped its owner or changed frame arithmetic");
                }));
            }
        }
        for (auto& task : tasks) task.get();
        tasks.clear();
        pool.wait_all();
    }
    check(retired[0].expired() && retired[1].expired(), "finished tasks retained map allocations");
    check(mb3d::CaptureRenderContext() == caller, "resource task changed caller binding");
}

void TestMapRelease() {
    TLightMap map{};
    map.LMa = new uint32_t[4]{};
    map.LMnumber = 17;
    map.LMWidth = map.LMHeight = 1;
    map.iLMstart = 1;
    mb3d::FreeLightMap(&map);
    check(!map.LMa && map.LMnumber == 0 && map.LMWidth == 0 &&
          map.LMHeight == 0 && map.iLMstart == 0, "released map retained a live record");
    mb3d::FreeLightMap(&map);
}

} // namespace

int main() {
    try {
        TestResourceOwners();
        TestMapRelease();
        std::puts("Map resources: independent owners, three slots, sequences, borrowed alias and release OK");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
