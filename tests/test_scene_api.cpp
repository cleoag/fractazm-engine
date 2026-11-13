// Application API contract: editing, stereo, clip volume, progressive passes
// and cancellation on one small real scene.
#include "mb3d/mb3d_scene.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#define CHECK(condition) do { if (!(condition)) { \
    std::fprintf(stderr, "FAILED %s:%d: %s (%s)\n", __FILE__, __LINE__, #condition, error); return 1; } } while (0)

struct Frames {
    std::vector<int> widths;
    std::vector<unsigned char> last;
    std::vector<unsigned short> depth;
    int passCount = 0;
    int visible = 0;
    int compiled = 0, loaded = 0;
    double seconds = 0;
    MB3DBackend backend = MB3D_BACKEND_CPU;
    bool fellBack = false;
};

static void collect(const MB3DFrame* frame, void* user) {
    auto* frames = static_cast<Frames*>(user);
    frames->widths.push_back(frame->width);
    frames->passCount = frame->pass_count;
    frames->last.assign(frame->rgba, frame->rgba + size_t(frame->width) * frame->height * 4);
    frames->depth.assign(frame->depth, frame->depth + size_t(frame->width) * frame->height);
    frames->compiled = frame->pipelines_compiled;
    frames->loaded = frame->pipelines_loaded;
    frames->seconds = frame->render_seconds;
    frames->backend = frame->backend;
    frames->fellBack = frame->cpu_fallback_reason != nullptr;
    frames->visible = 0;
    for (size_t i = 3; i < frames->last.size(); i += 4) frames->visible += frames->last[i] != 0;
}

int main(int argc, char** argv) {
    char error[256] = "";
    CHECK(argc == 2 || argc == 3);
    const std::string root = argv[1];
    MB3DScene* scene = mb3d_scene_load((root + "/comparison/scenes/test_01_intpow2.m3p").c_str(), error, sizeof(error));
    CHECK(scene);

    // Catalog and slot editing from an .m3f description.
    CHECK(mb3d_formula_count() > 400 && mb3d_formula_exists(mb3d_formula_name(0)));
    MB3DScene* edited = mb3d_scene_clone(scene);
    CHECK(mb3d_scene_set_formula_m3f(edited, 1, (root + "/mb3d/M3Formulas/_asdamTrans.m3f").c_str(), 1, error, sizeof(error)));
    MB3DFormulaSlot slot;
    CHECK(mb3d_scene_get_formula(edited, 1, &slot) && !std::strcmp(slot.name, "_asdamTrans") && slot.param_count == 4);
    CHECK(!std::strcmp(mb3d_scene_formula_param_name(edited, 1, 3), "Term Scale"));
    double value = 0;
    CHECK(mb3d_scene_set_formula_param(edited, 1, 3, 0.75) && mb3d_scene_get_formula_param(edited, 1, 3, &value) && value == 0.75);
    CHECK(!mb3d_scene_set_formula_m3f(edited, 1, "/nonexistent/NoSuchFormula.m3f", 1, error, sizeof(error)));
    mb3d_scene_destroy(edited);

    // Camera round trip and parallel-axis stereo eyes.
    MB3DCamera camera, left, right;
    CHECK(mb3d_scene_get_camera(scene, &camera) && mb3d_scene_set_camera(scene, &camera));
    MB3DScene* leftEye = mb3d_scene_create_eye(scene, MB3D_EYE_LEFT, 0.04);
    MB3DScene* rightEye = mb3d_scene_create_eye(scene, MB3D_EYE_RIGHT, 0.04);
    CHECK(leftEye && rightEye && mb3d_scene_get_camera(leftEye, &left) && mb3d_scene_get_camera(rightEye, &right));
    double shift = 0;
    for (int i = 0; i < 3; ++i) shift += (right.eye[i] - left.eye[i]) * camera.right[i];
    CHECK(shift > 0 && std::memcmp(left.forward, right.forward, sizeof(left.forward)) == 0);

    // Navigation keeps the start plane out of the fractal. Dollying in shows
    // the surface ever closer without a cut; a camera put into the fractal
    // without the helpers shows one.
    {
        const auto cutPixels = [](const Frames& frames) {
            int cut = 0;
            for (unsigned short value : frames.depth) cut += value == 32767;   // surface at the start plane
            return cut;
        };
        const auto along = [](const MB3DCamera& from, const MB3DCamera& to, const double axis[3]) {
            double sum = 0;
            for (int i = 0; i < 3; ++i) sum += (to.eye[i] - from.eye[i]) * axis[i];
            return sum;
        };
        MB3DRenderRequest look{};
        look.width = 96; look.height = 72;
        look.backend = MB3D_BACKEND_CPU;
        look.quality = MB3D_QUALITY_PREVIEW;
        look.precision = MB3D_PRECISION_NATIVE;
        look.preview_first_edge = 96;

        MB3DScene* flown = mb3d_scene_clone(scene);
        MB3DCamera start, close, turned, back;
        CHECK(mb3d_scene_get_camera(flown, &start) && start.depth > 0);
        // The test scene turns in place; give it a pivot ahead, as scenes that show an object have.
        start.pivot = 0.25 * start.depth;
        CHECK(mb3d_scene_set_camera(flown, &start) && mb3d_scene_get_camera(flown, &start));
        Frames authored, closeUp, cutThrough;
        CHECK(mb3d_scene_render(flown, &look, nullptr, collect, &authored, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
        for (int i = 0; i < 12; ++i) CHECK(mb3d_scene_dolly_camera(flown, std::log(2.0)));
        CHECK(mb3d_scene_get_camera(flown, &close));
        CHECK(close.zoom > start.zoom * 8 && along(start, close, start.forward) > 0);
        CHECK(mb3d_scene_render(flown, &look, nullptr, collect, &closeUp, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
        std::printf("scene API: 12 dolly steps, zoom %.3g to %.3g, surface on %d of %d pixels (was %d), %d cut\n",
                    start.zoom, close.zoom, closeUp.visible, 96 * 72, authored.visible, cutPixels(closeUp));
        CHECK(cutPixels(authored) == 0 && cutPixels(closeUp) == 0 && closeUp.visible > authored.visible);

        // Turning and panning there keep the plane free as well.
        CHECK(mb3d_scene_rotate_camera(flown, 0.2, 0.1, 0) && mb3d_scene_pan_camera(flown, 0.2, 0.1));
        // Close to the surface the camera turns about what it looks at.
        CHECK(mb3d_scene_get_camera(flown, &turned) && turned.pivot < close.pivot);
        Frames aside;
        CHECK(mb3d_scene_render(flown, &look, nullptr, collect, &aside, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
        CHECK(cutPixels(aside) == 0 && aside.visible > 0);

        // Backing away lets the plane out to where it started and ends at the
        // edge of the scene with the far limit no nearer than it was.
        int refused = 0;
        for (int i = 0; i < 40; ++i) refused += !mb3d_scene_dolly_camera(flown, -std::log(2.0));
        CHECK(mb3d_scene_get_camera(flown, &back));
        CHECK(refused > 0 && std::abs(back.zoom - start.zoom) < 1e-9 * start.zoom && back.depth >= start.depth);

        // The camera record restores a view completely.
        CHECK(mb3d_scene_set_camera(flown, &start) && mb3d_scene_get_camera(flown, &back));
        CHECK(std::abs(back.pivot - start.pivot) < 1e-9 * start.depth && std::abs(back.depth - start.depth) < 1e-9 * start.depth &&
              std::abs(along(start, back, start.forward)) < 1e-9 && back.zoom == start.zoom);
        Frames restored;
        CHECK(mb3d_scene_render(flown, &look, nullptr, collect, &restored, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
        CHECK(restored.last == authored.last);

        // Directions, pinned on rendered frames: `right` is the right edge of
        // the picture and `up` its bottom edge (MB3D's row direction).
        {
            const auto centre = [](const Frames& frames, double& x, double& y) {
                double sx = 0, sy = 0; int count = 0;
                for (int row = 0; row < 72; ++row) for (int column = 0; column < 96; ++column)
                    if (frames.depth[size_t(row) * 96 + column] <= 32767) { sx += column; sy += row; ++count; }
                x = count ? sx / count : 0; y = count ? sy / count : 0;
                return count;
            };
            double x0, y0, x, y;
            CHECK(centre(authored, x0, y0) > 100);
            MB3DCamera now;
            Frames shown;
            const auto show = [&] {
                return mb3d_scene_render(flown, &look, nullptr, collect, &shown, error, sizeof(error)) == MB3D_RENDER_SUCCESS &&
                       centre(shown, x, y) > 100 && mb3d_scene_get_camera(flown, &now);
            };
            // A camera moved to the right shows the picture further left; moved "up", further up.
            CHECK(mb3d_scene_pan_camera(flown, 0.1, 0) && show() && x < x0 - 1 && std::abs(y - y0) < 1);
            CHECK(mb3d_scene_set_camera(flown, &start) && mb3d_scene_pan_camera(flown, 0, 0.1) && show() && y < y0 - 1 && std::abs(x - x0) < 1);
            // Turned in place to the right and to the top, the picture goes left and down.
            CHECK(mb3d_scene_set_camera(flown, &start) && mb3d_scene_rotate_camera(flown, 0.1, 0, 1) && show() && x < x0 - 1);
            CHECK(mb3d_scene_set_camera(flown, &start) && mb3d_scene_rotate_camera(flown, 0, 0.1, 1) && show() && y > y0 + 1);
            // Round the pivot the camera itself goes the other way: left and down.
            CHECK(mb3d_scene_set_camera(flown, &start) && mb3d_scene_rotate_camera(flown, 0.1, 0, 0) && show() && along(start, now, start.right) < 0);
            CHECK(mb3d_scene_set_camera(flown, &start) && mb3d_scene_rotate_camera(flown, 0, 0.1, 0) && show() && along(start, now, start.up) > 0);
            // A roll turns `right` towards `up` and leaves the viewpoint and the view direction.
            CHECK(mb3d_scene_set_camera(flown, &start) && mb3d_scene_roll_camera(flown, 0.2) && mb3d_scene_get_camera(flown, &now));
            double towardsUp = 0, sameForward = 0;
            for (int i = 0; i < 3; ++i) { towardsUp += now.right[i] * start.up[i]; sameForward += now.forward[i] * start.forward[i]; }
            CHECK(std::abs(towardsUp - std::sin(0.2)) < 1e-9 && std::abs(sameForward - 1) < 1e-12 &&
                  std::abs(along(start, now, start.right)) < 1e-12 && std::abs(along(start, now, start.up)) < 1e-12);
            CHECK(!mb3d_scene_roll_camera(flown, NAN) && !mb3d_scene_roll_camera(nullptr, 0.1));
            CHECK(mb3d_scene_set_camera(flown, &start));
        }

        // Set directly, a camera in the middle of the bulb is cut by its start plane.
        MB3DCamera inside = start;
        for (int i = 0; i < 3; ++i) inside.eye[i] = 0;
        inside.pivot = 0;
        CHECK(mb3d_scene_set_camera(flown, &inside));
        CHECK(mb3d_scene_render(flown, &look, nullptr, collect, &cutThrough, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
        std::printf("scene API: camera set inside the fractal: %d cut pixels\n", cutPixels(cutThrough));
        CHECK(cutPixels(cutThrough) > 0);
        mb3d_scene_destroy(flown);
    }

    // Progressive preview: growing passes, the last at the requested size.
    MB3DRenderRequest request{};
    request.width = 96; request.height = 72;
    request.backend = MB3D_BACKEND_CPU;
    request.quality = MB3D_QUALITY_PREVIEW;
    request.preview_first_edge = 24;
    Frames preview;
    CHECK(mb3d_scene_render(scene, &request, nullptr, collect, &preview, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
    CHECK(preview.widths == std::vector<int>({24, 48, 96}) && preview.passCount == 3 && preview.visible > 0);

    // A clip volume around the near half of the view removes visible surface.
    MB3DVolume volume;
    for (int i = 0; i < 3; ++i) { volume.min[i] = camera.eye[i] - 1e3; volume.max[i] = camera.eye[i] + 1e3; }
    CHECK(mb3d_scene_set_clip_volume(scene, &volume));
    Frames whole;
    request.quality = MB3D_QUALITY_FINAL;
    request.precision = MB3D_PRECISION_NATIVE;
    CHECK(mb3d_scene_render(scene, &request, nullptr, collect, &whole, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
    CHECK(whole.widths.size() == 1 && whole.visible > 0);
    const double span = 2.1345 / camera.zoom;
    for (int i = 0; i < 3; ++i) { // an empty box far to the side of the view
        volume.min[i] = camera.eye[i] + camera.right[i] * span * 50 - span;
        volume.max[i] = volume.min[i] + 2 * span;
    }
    CHECK(mb3d_scene_set_clip_volume(scene, &volume));
    Frames clipped;
    CHECK(mb3d_scene_render(scene, &request, nullptr, collect, &clipped, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
    CHECK(clipped.last != whole.last);
    CHECK(mb3d_scene_set_clip_volume(scene, nullptr));

    // A cancelled token stops before any frame is delivered.
    MB3DCancel* cancel = mb3d_cancel_create();
    mb3d_cancel_request(cancel);
    Frames none;
    CHECK(mb3d_scene_render(scene, &request, cancel, collect, &none, error, sizeof(error)) == MB3D_RENDER_CANCELLED);
    CHECK(none.widths.empty());
    mb3d_cancel_destroy(cancel);

    // A persistent renderer gives the stateless image, on the CPU and on Metal,
    // and a second Metal frame needs no pipeline work.
    MB3DRenderRequest draft{};
    draft.width = 96; draft.height = 72;
    draft.quality = MB3D_QUALITY_PREVIEW;
    draft.preview_first_edge = 96;
    const char* library = argc == 3 ? argv[2] : nullptr;
    draft.backend = library ? MB3D_BACKEND_METAL : MB3D_BACKEND_CPU;
    draft.metal_library = library;
    if (library) CHECK(!mb3d_renderer_create("/nonexistent/none.metallib", error, sizeof(error)));
    MB3DRenderer* renderer = mb3d_renderer_create(library, error, sizeof(error));
    CHECK(renderer);
    for (int step = 0; step < 3; ++step) {
        Frames stateless, kept;
        CHECK(mb3d_scene_render(scene, &draft, nullptr, collect, &stateless, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
        CHECK(mb3d_renderer_render(renderer, scene, &draft, nullptr, collect, &kept, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
        CHECK(kept.widths == std::vector<int>({96}) && kept.visible > 0 && kept.last == stateless.last);
        CHECK(kept.seconds > 0 && kept.backend == draft.backend);
        // A pass that ran where it was requested names no fallback.
        CHECK(!kept.fellBack && !stateless.fellBack);
        if (step > 0) CHECK(kept.compiled == 0 && kept.loaded == 0);
        CHECK(mb3d_scene_rotate_camera(scene, 0.05, 0.02, 0));
    }
    CHECK(mb3d_renderer_render(nullptr, scene, &draft, nullptr, collect, &none, error, sizeof(error)) == MB3D_RENDER_FAILED);

    // Lattice frames: the order covers the frame ever more finely, the parts
    // assemble into exactly their pixels, and what is missing is interpolated.
    {
        int x = -1, y = -1;
        for (int stride = 2; stride <= 4; ++stride) {
            std::vector<int> seen(size_t(stride * stride), 0);
            for (int index = 0; index < stride * stride; ++index) {
                CHECK(mb3d_lattice_offset(stride, index, &x, &y) && x >= 0 && y >= 0 && x < stride && y < stride);
                CHECK(index > 0 || (x == 0 && y == 0));
                // The first four of stride 4 are the stride-2 lattice.
                if (stride == 4 && index < 4) CHECK(x % 2 == 0 && y % 2 == 0);
                ++seen[size_t(y * stride + x)];
            }
            for (int count : seen) CHECK(count == 1);
        }
        CHECK(!mb3d_lattice_offset(1, 0, &x, &y) && !mb3d_lattice_offset(5, 0, &x, &y) &&
              !mb3d_lattice_offset(3, 9, &x, &y) && !mb3d_lattice_offset(3, -1, &x, &y));

        // A ramp picture of a size the stride does not divide.
        const int width = 22, height = 15, stride = 3;
        std::vector<unsigned char> truth(size_t(width) * height * 4), picture(truth.size(), 0), known(size_t(width) * height, 0);
        std::vector<unsigned short> truthDepth(size_t(width) * height), pictureDepth(truthDepth.size(), 0);
        for (int py = 0; py < height; ++py) for (int px = 0; px < width; ++px) {
            const size_t i = size_t(py) * width + px;
            truth[i * 4] = static_cast<unsigned char>(5 * px); truth[i * 4 + 1] = static_cast<unsigned char>(9 * py);
            truth[i * 4 + 2] = static_cast<unsigned char>(3 * px + 4 * py); truth[i * 4 + 3] = 255;
            truthDepth[i] = static_cast<unsigned short>(100 + i);
        }
        for (int index = 0; index < stride * stride; ++index) {
            CHECK(mb3d_lattice_offset(stride, index, &x, &y));
            const int partWidth = (width - x + stride - 1) / stride, partHeight = (height - y + stride - 1) / stride;
            std::vector<unsigned char> part(size_t(partWidth) * partHeight * 4);
            std::vector<unsigned short> partDepth(size_t(partWidth) * partHeight);
            for (int j = 0; j < partHeight; ++j) for (int i = 0; i < partWidth; ++i) {
                const size_t from = size_t(y + j * stride) * width + size_t(x + i * stride), to = size_t(j) * partWidth + i;
                std::memcpy(&part[to * 4], &truth[from * 4], 4);
                partDepth[to] = truthDepth[from];
            }
            CHECK(mb3d_lattice_merge(picture.data(), pictureDepth.data(), known.data(), width, height,
                                     part.data(), partDepth.data(), partWidth + 1, partHeight, stride, x, y) == -1);
            CHECK(mb3d_lattice_merge(picture.data(), pictureDepth.data(), known.data(), width, height,
                                     part.data(), partDepth.data(), partWidth, partHeight, stride, x, y) == index + 1);
            int worst = 0;
            for (int py = 0; py < height; ++py) for (int px = 0; px < width; ++px) {
                const size_t i = size_t(py) * width + px;
                // Every pixel is filled: opaque, with the depth of some rendered pixel.
                CHECK(picture[i * 4 + 3] == 255 && pictureDepth[i] >= 100);
                if (known[i]) CHECK(std::memcmp(&picture[i * 4], &truth[i * 4], 4) == 0 && pictureDepth[i] == truthDepth[i]);
                // Between rendered pixels a ramp is interpolated exactly (to rounding).
                else if (px >= stride && py >= stride && px < width - stride && py < height - stride)
                    for (int c = 0; c < 3; ++c) worst = std::max(worst, std::abs(int(picture[i * 4 + c]) - int(truth[i * 4 + c])));
            }
            CHECK(worst <= 1);
        }
        CHECK(picture == truth && pictureDepth == truthDepth);

        // Requests: FINAL on Metal only, a valid lattice only.
        MB3DRenderRequest part{};
        part.width = 96; part.height = 72;
        part.quality = MB3D_QUALITY_FINAL;
        part.precision = MB3D_PRECISION_NATIVE;
        part.backend = MB3D_BACKEND_CPU;
        part.lattice_stride = 2;
        Frames refused;
        CHECK(mb3d_scene_render(scene, &part, nullptr, collect, &refused, error, sizeof(error)) == MB3D_RENDER_UNSUPPORTED);
        part.lattice_x = 2;
        CHECK(mb3d_scene_render(scene, &part, nullptr, collect, &refused, error, sizeof(error)) == MB3D_RENDER_FAILED);
        CHECK(refused.widths.empty());
        if (library) {
            part.backend = MB3D_BACKEND_METAL;
            part.metal_library = library;
            part.quality = MB3D_QUALITY_PREVIEW;
            part.lattice_x = 0;
            CHECK(mb3d_scene_render(scene, &part, nullptr, collect, &refused, error, sizeof(error)) == MB3D_RENDER_UNSUPPORTED);
            part.quality = MB3D_QUALITY_FINAL;
            // The four parts of a frame against the frame rendered whole.
            MB3DRenderRequest whole = part;
            whole.lattice_stride = 0;
            Frames complete;
            CHECK(mb3d_scene_render(scene, &whole, nullptr, collect, &complete, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
            // Quick pipelines: the same pictures from programs that compile sooner.
            MB3DRenderRequest quick = whole;
            quick.quick_pipelines = 1;
            Frames quickWhole, plainPart, quickPart;
            CHECK(mb3d_scene_render(scene, &quick, nullptr, collect, &quickWhole, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
            CHECK(quickWhole.backend == MB3D_BACKEND_METAL && quickWhole.last == complete.last && quickWhole.depth == complete.depth);
            quick = part;
            quick.quick_pipelines = 1;
            CHECK(mb3d_renderer_render(renderer, scene, &part, nullptr, collect, &plainPart, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
            CHECK(mb3d_renderer_render(renderer, scene, &quick, nullptr, collect, &quickPart, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
            CHECK(quickPart.widths == std::vector<int>({48}) && quickPart.last == plainPart.last && quickPart.depth == plainPart.depth);
            std::vector<unsigned char> assembled(size_t(96) * 72 * 4, 0), rendered(size_t(96) * 72, 0);
            std::vector<unsigned short> assembledDepth(size_t(96) * 72, 0);
            for (int index = 0; index < 4; ++index) {
                CHECK(mb3d_lattice_offset(2, index, &part.lattice_x, &part.lattice_y));
                Frames one;
                CHECK(mb3d_renderer_render(renderer, scene, &part, nullptr, collect, &one, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
                CHECK(one.widths == std::vector<int>({48}) && one.passCount == 1 && one.backend == MB3D_BACKEND_METAL);
                CHECK(mb3d_lattice_merge(assembled.data(), assembledDepth.data(), rendered.data(), 96, 72,
                                         one.last.data(), one.depth.data(), 48, 36, 2, part.lattice_x, part.lattice_y) == index + 1);
            }
            double difference = 0;
            int sameSurface = 0;
            for (size_t i = 0; i < assembledDepth.size(); ++i) {
                sameSurface += (assembledDepth[i] > 32767) == (complete.depth[i] > 32767);
                for (int c = 0; c < 3; ++c) difference += std::abs(int(assembled[i * 4 + c]) - int(complete.last[i * 4 + c]));
            }
            difference /= double(assembledDepth.size() * 3);
            std::printf("scene API: four lattice parts differ from the whole frame by %.2f of 255 on average, same surface on %d of %d pixels\n",
                        difference, sameSurface, 96 * 72);
            // The rays are the same, but the parts go through the general Metal
            // renderer (this scene's whole frame through the restricted one), and
            // ambient occlusion sees neighbours two pixels apart.
            CHECK(sameSurface > 96 * 72 * 0.995 && difference < 6);
        }
    }

    // Admission: a frame the device can render is admitted without rendering, and
    // one beyond the renderer's 4096 px is refused with a reason.
    if (library) {
        MB3DRenderRequest probe{};
        probe.metal_library = library;
        probe.width = 640; probe.height = 480;
        const auto probeStart = std::chrono::steady_clock::now();
        CHECK(mb3d_scene_admits(scene, &probe, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
        const double probeSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - probeStart).count();
        // The largest frame: admitted or not depends on the device's memory.
        probe.width = 4096; probe.height = 4096;
        const auto largest = mb3d_scene_admits(scene, &probe, error, sizeof(error));
        CHECK(largest == MB3D_RENDER_SUCCESS || (largest == MB3D_RENDER_UNSUPPORTED && error[0]));
        std::printf("scene API: admission of 640x480 took %.3f s; 4096x4096 %s%s\n", probeSeconds,
                    largest == MB3D_RENDER_SUCCESS ? "admitted" : "refused: ", largest == MB3D_RENDER_SUCCESS ? "" : error);
        error[0] = 0;
        probe.width = 640; probe.height = 480;
        probe.lattice_stride = 2; probe.lattice_x = 1;
        CHECK(mb3d_scene_admits(scene, &probe, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
        probe.lattice_stride = 0; probe.lattice_x = 0;
        probe.width = 8192; probe.height = 6144;
        CHECK(mb3d_scene_admits(scene, &probe, error, sizeof(error)) == MB3D_RENDER_UNSUPPORTED && error[0]);
        std::printf("scene API: 8192x6144 refused: %s\n", error);
        error[0] = 0;
    }
    MB3DRenderRequest missing{};
    CHECK(mb3d_scene_admits(scene, &missing, error, sizeof(error)) == MB3D_RENDER_FAILED);
    error[0] = 0;

    // Progress of FINAL: a quarter-size pass first, the same final image as
    // without progress, and a fraction that starts at 0, never falls and ends at 1.
    MB3DRenderRequest full{};
    full.width = 640; full.height = 120;
    full.quality = MB3D_QUALITY_FINAL;
    full.backend = library ? MB3D_BACKEND_AUTO : MB3D_BACKEND_CPU;
    full.precision = MB3D_PRECISION_NATIVE;
    full.metal_library = library;
    Frames plain, tracked;
    CHECK(mb3d_renderer_render(renderer, scene, &full, nullptr, collect, &plain, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
    std::vector<double> fractions;
    full.progress = [](double fraction, void* user) { static_cast<std::vector<double>*>(user)->push_back(fraction); };
    full.progress_user = &fractions;
    // Stages: the quarter-size pass goes through those of the full pass, and each
    // pass begins with preparation and reaches the surface.
    struct Stage { int stage, pass, count; };
    std::vector<Stage> stages;
    full.stage = [](MB3DRenderStage stage, int pass, int count, void* user) {
        static_cast<std::vector<Stage>*>(user)->push_back({int(stage), pass, count});
    };
    full.stage_user = &stages;
    CHECK(mb3d_renderer_render(renderer, scene, &full, nullptr, collect, &tracked, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
    full.stage = nullptr;
    {
        std::vector<int> seen[2];
        for (const Stage& s : stages) {
            CHECK(s.count == 2 && (s.pass == 0 || s.pass == 1) && s.stage >= 0 && s.stage <= MB3D_STAGE_DEPTH_OF_FIELD);
            if (s.pass == 0 || s.pass == 1) seen[s.pass].push_back(s.stage);
        }
        CHECK(!seen[0].empty() && seen[0] == seen[1] && seen[0].front() == MB3D_STAGE_PREPARATION);
        CHECK(std::find(seen[0].begin(), seen[0].end(), int(MB3D_STAGE_SURFACE)) != seen[0].end());
        for (size_t i = 1; i < stages.size(); ++i) CHECK(stages[i].pass >= stages[i - 1].pass);
        std::printf("scene API: %zu stage reports\n", stages.size());
    }
    CHECK(plain.widths == std::vector<int>({640}) && tracked.widths == std::vector<int>({160, 640}));
    CHECK(tracked.passCount == 2 && tracked.last == plain.last && tracked.backend == plain.backend);
    CHECK(fractions.size() >= 3 && fractions.front() == 0 && fractions.back() == 1);
    for (size_t i = 1; i < fractions.size(); ++i) CHECK(fractions[i] >= fractions[i - 1]);
    std::printf("scene API: %zu progress reports\n", fractions.size());
    // A small frame has no calibration pass and still reports its ends.
    full.width = 96; full.height = 72;
    fractions.clear();
    Frames small;
    CHECK(mb3d_renderer_render(renderer, scene, &full, nullptr, collect, &small, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
    CHECK(small.widths == std::vector<int>({96}) && fractions.front() == 0 && fractions.back() == 1);

    // Cancelling while a frame renders ends it at the next GPU command (or the
    // next CPU poll) without a frame, and the renderer renders the same image after.
    {
        MB3DRenderRequest large = full;
        large.width = 1024; large.height = 768;
        large.progress = nullptr;
        MB3DCancel* stop = mb3d_cancel_create();
        Frames interrupted, after, reference;
        MB3DRenderStatus status = MB3D_RENDER_FAILED;
        const auto started = std::chrono::steady_clock::now();
        std::thread worker([&] { status = mb3d_renderer_render(renderer, scene, &large, stop, collect, &interrupted, error, sizeof(error)); });
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        const auto asked = std::chrono::steady_clock::now();
        mb3d_cancel_request(stop);
        worker.join();
        const auto ended = std::chrono::steady_clock::now();
        std::printf("scene API: cancel answered in %.0f ms (frame ran %.0f ms)\n",
                    std::chrono::duration<double, std::milli>(ended - asked).count(),
                    std::chrono::duration<double, std::milli>(ended - started).count());
        CHECK(status == MB3D_RENDER_CANCELLED && interrupted.widths.empty());
        mb3d_cancel_reset(stop);
        full.progress = nullptr;
        CHECK(mb3d_renderer_render(renderer, scene, &full, stop, collect, &after, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
        CHECK(mb3d_renderer_render(renderer, scene, &full, nullptr, collect, &reference, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
        CHECK(after.last == reference.last);
        mb3d_cancel_destroy(stop);
    }

    // Colours, fog and light: writing back what was read changes nothing, every
    // control changes the picture, and input outside its range is clamped or refused.
    {
        MB3DScene* styled = mb3d_scene_clone(scene);
        MB3DRenderRequest look{};
        look.width = 96; look.height = 72;
        look.backend = MB3D_BACKEND_CPU;
        look.quality = MB3D_QUALITY_PREVIEW;
        look.preview_first_edge = 96;
        look.precision = MB3D_PRECISION_NATIVE;
        std::vector<unsigned short> surface; // depth of the last picture; the edits here keep the geometry
        const auto picture = [&](const MB3DScene* subject) {
            Frames frames;
            if (mb3d_scene_render(subject, &look, nullptr, collect, &frames, error, sizeof(error)) != MB3D_RENDER_SUCCESS)
                frames.last.clear();
            surface = frames.depth;
            return frames.last;
        };
        // Brightness `lit` gains over `unlit` on surface facing the image direction
        // (dx, dy), minus the gain on surface facing away. Surface faces the way
        // it recedes, and a nearer point has the larger depth value.
        const auto towards = [&](const std::vector<unsigned char>& lit, const std::vector<unsigned char>& unlit, int dx, int dy) {
            double sum[2] = {0, 0}, count[2] = {0, 0};
            for (int y = 1; y + 1 < look.height; ++y) for (int x = 1; x + 1 < look.width; ++x) {
                const int at = y * look.width + x, step = dx + dy * look.width;
                const int here = surface[at], ahead = surface[at + step], behind = surface[at - step];
                if (here > 32767 || ahead > 32767 || behind > 32767 || ahead == behind) continue;
                const int facing = ahead < behind;
                for (int channel = 0; channel < 3; ++channel) sum[facing] += int(lit[at * 4 + channel]) - int(unlit[at * 4 + channel]);
                count[facing] += 1;
            }
            return sum[1] / count[1] - sum[0] / count[0];
        };
        const std::vector<unsigned char> original = picture(styled);
        CHECK(!original.empty());
        MB3DColor palette[MB3D_PALETTE_COLORS];
        double positions[MB3D_PALETTE_COLORS], rangeStart = 0, rangeEnd = 0, ambient = 0;
        MB3DFog fog;
        MB3DLight light;
        CHECK(mb3d_scene_get_palette(styled, palette) && mb3d_scene_get_palette_positions(styled, positions));
        CHECK(mb3d_scene_get_palette_range(styled, &rangeStart, &rangeEnd) && mb3d_scene_get_fog(styled, &fog));
        CHECK(mb3d_scene_get_light(styled, &light) && mb3d_scene_get_ambient(styled, &ambient));
        CHECK(mb3d_scene_set_palette(styled, palette) && mb3d_scene_set_palette_positions(styled, positions));
        CHECK(mb3d_scene_set_palette_range(styled, rangeStart, rangeEnd) && mb3d_scene_set_fog(styled, &fog));
        CHECK(mb3d_scene_set_light(styled, &light) && mb3d_scene_set_ambient(styled, ambient));
        CHECK(picture(styled) == original);

        std::vector<unsigned char> before = original, after;
        const auto changed = [&] { after = picture(styled); const bool differs = !after.empty() && after != before; before = after; return differs; };
        MB3DColor inverted[MB3D_PALETTE_COLORS], readColors[MB3D_PALETTE_COLORS];
        for (int i = 0; i < MB3D_PALETTE_COLORS; ++i)
            inverted[i] = {static_cast<unsigned char>(255 - palette[i].r), static_cast<unsigned char>(255 - palette[i].g), palette[i].b};
        CHECK(mb3d_scene_set_palette(styled, inverted) && mb3d_scene_get_palette(styled, readColors));
        CHECK(!std::memcmp(inverted, readColors, sizeof(inverted)) && changed());
        double even[MB3D_PALETTE_COLORS], readPositions[MB3D_PALETTE_COLORS];
        for (int i = 0; i < MB3D_PALETTE_COLORS; ++i) even[i] = i / 10.0;
        CHECK(mb3d_scene_set_palette_positions(styled, even) && mb3d_scene_get_palette_positions(styled, readPositions));
        for (int i = 0; i < MB3D_PALETTE_COLORS; ++i) CHECK(std::abs(readPositions[i] - even[i]) < 1e-4);
        CHECK(changed());
        even[4] = 0.2; // not ascending
        CHECK(!mb3d_scene_set_palette_positions(styled, even));
        // A shifted and a stretched range come back within the stated precision.
        double start = 0, end = 0;
        const double ranges[3][2] = {{rangeStart + 0.021, rangeEnd + 0.021}, {0.2, 0.9}, {1.1, 0.4}};
        for (const auto& range : ranges) {
            CHECK(mb3d_scene_set_palette_range(styled, range[0], range[1]) && mb3d_scene_get_palette_range(styled, &start, &end));
            const double tolerance = 0.01 * std::abs(range[1] - range[0]) + 0.0003;
            CHECK(std::abs(start - range[0]) <= tolerance && std::abs(end - range[1]) <= tolerance && changed());
        }
        CHECK(mb3d_scene_set_palette_range(styled, -9, 9) && mb3d_scene_get_palette_range(styled, &start, &end));
        CHECK(std::abs(start - MB3D_PALETTE_RANGE_MIN) < 1e-9 && std::abs(end - MB3D_PALETTE_RANGE_MAX) < 1e-9);
        CHECK(!mb3d_scene_set_palette_range(styled, NAN, 1) && !mb3d_scene_set_palette_range(styled, 0, INFINITY));
        CHECK(mb3d_scene_set_palette_range(styled, rangeStart, rangeEnd) && changed());

        MB3DFog thick = fog, readFog;
        thick.amount = 0.15;
        CHECK(mb3d_scene_set_fog(styled, &thick) && changed());
        thick.color = thick.color_bottom = {200, 30, 30};
        CHECK(mb3d_scene_set_fog(styled, &thick) && mb3d_scene_get_fog(styled, &readFog) && changed());
        CHECK(readFog.color.r == 200 && readFog.color_bottom.b == 30 && std::abs(readFog.amount - 0.15) < 1.0 / 240);
        thick.amount = 7;
        CHECK(mb3d_scene_set_fog(styled, &thick) && mb3d_scene_get_fog(styled, &readFog) && readFog.amount == 1);
        thick.amount = NAN;
        CHECK(!mb3d_scene_set_fog(styled, &thick) && mb3d_scene_get_fog(styled, &readFog) && readFog.amount == 1);
        CHECK(mb3d_scene_set_fog(styled, &fog) && changed());

        // The pad follows the picture: +x lights surface facing right, +y surface
        // facing up (image rows run downwards).
        std::vector<unsigned char> lit[4];
        MB3DLight moved = light, readLight;
        for (int corner = 0; corner < 4; ++corner) {
            moved.x = corner == 0 ? -0.3 : corner == 1 ? 0.3 : 0;
            moved.y = corner == 2 ? -0.3 : corner == 3 ? 0.3 : 0;
            CHECK(mb3d_scene_set_light(styled, &moved) && changed());
            lit[corner] = after;
        }
        const double rightward = towards(lit[1], lit[0], 1, 0), upward = towards(lit[3], lit[2], 0, -1);
        std::printf("scene API: light pad +x adds %.0f to surface facing right, +y %.0f to surface facing up\n", rightward, upward);
        CHECK(rightward > 30 && upward > 30);
        CHECK(mb3d_scene_get_light(styled, &readLight) && readLight.x == 0 && std::abs(readLight.y - 0.3) < 1e-12);
        moved.brightness = 0.37;
        CHECK(mb3d_scene_set_light(styled, &moved) && mb3d_scene_get_light(styled, &readLight) && changed());
        CHECK(std::abs(readLight.brightness - 0.37) < 1e-6 && std::abs(readLight.y - 0.3) < 1e-12);
        moved.x = -5; moved.brightness = 1e9;
        CHECK(mb3d_scene_set_light(styled, &moved) && mb3d_scene_get_light(styled, &readLight));
        CHECK(std::abs(readLight.x + 1) < 1e-12 && readLight.brightness == 100);
        moved.y = NAN;
        CHECK(!mb3d_scene_set_light(styled, &moved) && mb3d_scene_get_light(styled, &readLight) && std::abs(readLight.y - 0.3) < 1e-12);
        CHECK(mb3d_scene_set_light(styled, &light) && changed());

        CHECK(mb3d_scene_set_ambient(styled, 2.5) && mb3d_scene_get_ambient(styled, &start) && std::abs(start - 2.5) < 1.0 / 90 && changed());
        CHECK(mb3d_scene_set_ambient(styled, 99) && mb3d_scene_get_ambient(styled, &start) && start == 3);
        CHECK(mb3d_scene_set_ambient(styled, -1) && mb3d_scene_get_ambient(styled, &start) && start == 0);
        CHECK(!mb3d_scene_set_ambient(styled, NAN) && !mb3d_scene_get_ambient(nullptr, &start) && !mb3d_scene_set_light(styled, nullptr));
        // Putting every value back restores the picture.
        CHECK(mb3d_scene_set_ambient(styled, ambient) && mb3d_scene_set_palette(styled, palette));
        CHECK(mb3d_scene_set_palette_positions(styled, positions) && picture(styled) == original);

        // Hard shadows need the final quality; the preview draws none.
        look.quality = MB3D_QUALITY_FINAL;
        MB3DLight shadowed = light;
        shadowed.x = 0.4; shadowed.hard_shadow = 0;
        CHECK(mb3d_scene_set_light(styled, &shadowed));
        before = picture(styled);
        shadowed.hard_shadow = 1;
        CHECK(mb3d_scene_set_light(styled, &shadowed) && mb3d_scene_get_light(styled, &readLight) && readLight.hard_shadow && changed());
        look.quality = MB3D_QUALITY_PREVIEW;
        mb3d_scene_destroy(styled);

        // A scene lit by a positional light only has no global light to read; the
        // setter switches one on.
        MB3DScene* lamp = mb3d_scene_load((root + "/mb3d/M3Parameter/Blood on the dancefloor.m3p").c_str(), error, sizeof(error));
        CHECK(lamp && !mb3d_scene_get_light(lamp, &readLight));
        moved = {0.2, -0.1, 2, 0};
        CHECK(mb3d_scene_set_light(lamp, &moved) && mb3d_scene_get_light(lamp, &readLight));
        CHECK(std::abs(readLight.x - 0.2) < 1e-12 && std::abs(readLight.y + 0.1) < 1e-12 && readLight.brightness == 2 && !readLight.hard_shadow);
        mb3d_scene_destroy(lamp);
    }

    // Reprojection: a frame redrawn for its own camera is itself, and redrawn
    // for a moved camera it is closer to the frame rendered there than it was.
    MB3DRenderRequest still = draft;
    still.width = 192; still.height = 144; still.preview_first_edge = 192;
    MB3DScene* moved = mb3d_scene_clone(scene);
    CHECK(mb3d_scene_rotate_camera(moved, 0.03, 0.02, 0) && mb3d_scene_move_camera(moved, 0.06, 0.04, 0.03));
    Frames here, there;
    CHECK(mb3d_renderer_render(renderer, scene, &still, nullptr, collect, &here, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
    CHECK(mb3d_renderer_render(renderer, moved, &still, nullptr, collect, &there, error, sizeof(error)) == MB3D_RENDER_SUCCESS);
    std::vector<unsigned char> warped(here.last.size());
    CHECK(mb3d_scene_reproject_frame(scene, scene, here.last.data(), here.depth.data(), 192, 144, warped.data()) == 1);
    CHECK(warped == here.last);
    const double coverage = mb3d_scene_reproject_frame(scene, moved, here.last.data(), here.depth.data(), 192, 144, warped.data());
    // Colour difference to the frame rendered there, and pixels whose surface/background state differs.
    const auto distance = [&](const std::vector<unsigned char>& a) {
        double sum = 0;
        for (size_t i = 0; i < a.size(); ++i) if (i % 4 != 3) sum += std::abs(int(a[i]) - int(there.last[i]));
        return sum / double(a.size() / 4 * 3);
    };
    const auto outline = [&](const std::vector<unsigned char>& a) {
        int count = 0;
        for (size_t i = 3; i < a.size(); i += 4) count += (a[i] != 0) != (there.last[i] != 0);
        return count;
    };
    std::printf("scene API: reprojection coverage %.2f, difference %.2f -> %.2f, outline %d -> %d\n",
                coverage, distance(here.last), distance(warped), outline(here.last), outline(warped));
    CHECK(coverage > 0.5 && coverage <= 1 && distance(warped) * 2 < distance(here.last) && outline(warped) * 2 < outline(here.last));
    CHECK(mb3d_scene_reproject_frame(scene, nullptr, here.last.data(), here.depth.data(), 192, 144, warped.data()) < 0);
    mb3d_scene_destroy(moved);
    mb3d_renderer_destroy(renderer);
    mb3d_renderer_destroy(nullptr);

    mb3d_scene_destroy(leftEye);
    mb3d_scene_destroy(rightEye);
    mb3d_scene_destroy(scene);
    std::puts("scene API: ok");
    return 0;
}
