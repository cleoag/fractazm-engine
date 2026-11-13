// Application API: editable scene, formula catalog, camera, stereo, clip
// volume and progressive rendering over the existing CPU/Metal entry point.
#include "mb3d/mb3d_scene.h"
#include "core/external_formulas.h"
#include "core/math3d.h"
#include "core/mb3d_render_control_api.h"
#include "io/m3p_parser.h"
#include "render/backend.h"
#include "render/frame_reprojection.h"
#include "render/navigation_camera.h"
#include "render/navigation_probe.h"
#include "comparison/metal_exact/work_meter.h"
#ifdef MB3D_ENABLE_METAL_RENDERER
#include "render/native_metal/general_renderer.h"
#endif
#include <chrono>
#include <algorithm>
#include <atomic>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct MB3DScene {
    TMandHeader10 header{};
    THeaderCustomAddon addon{};
    std::string resource_root;
    std::string param_names[MB3D_FORMULA_SLOTS][MB3D_FORMULA_PARAMS];
    bool has_volume = false;
    MB3DVolume volume{};
    // Navigation (see keepPlaneClear): the camera the gestures started from.
    double nav_near = 0;    // its viewpoint-to-plane distance; 0: not captured yet
    double nav_ratio = 0;   // plane radius allowed per free distance; < 0: gestures move the camera unchecked
    double nav_aspect = 0;  // shape of the rendered frames; 0: the scene's own
    double nav_home[3] = {0, 0, 0};  // its viewpoint
    double nav_range = 0;   // how far from there the viewpoint may go
    bool nav_open = false;  // its viewpoint was in free space: the viewpoint stays out of the fractal
};

struct MB3DCancel {
    std::atomic<bool> requested{false};
    std::mutex mutex;
    mb3d::RenderControlHandle active;
};

struct MB3DRenderer {
    std::string library;                                   // empty: CPU renderer
    mb3d::native_metal::RenderSession* session = nullptr;  // null without Metal
};

namespace {
using mb3d::navigation::Camera;

void setError(char* error, size_t size, const std::string& text) {
    if (error && size) std::snprintf(error, size, "%s", text.c_str());
}

bool validSlot(const MB3DScene* scene, int slot) {
    return scene && slot >= 0 && slot < MB3D_FORMULA_SLOTS;
}

std::string slotName(const THAformula& formula) {
    const char* text = reinterpret_cast<const char*>(formula.CustomFname);
    return std::string(text, strnlen(text, sizeof(formula.CustomFname)));
}

std::string trim(std::string text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) text.pop_back();
    size_t first = 0;
    while (first < text.size() && std::isspace(static_cast<unsigned char>(text[first]))) ++first;
    return text.substr(first);
}

std::string lower(std::string text) {
    for (char& c : text) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return text;
}

// Box faces the eye looks at from outside become MB3D cut planes. The engine
// keeps the half-space beyond a plane along the view direction and drops every
// ray travelling against it, so an axis is cut only when the eye is outside the
// box on the side the view comes from. The far side is the plane through the
// farthest corner.
void applyVolume(const MB3DScene& scene, TMandHeader10& header) {
    if (!scene.has_volume || !Camera::supports(header)) return;
    const Camera camera(header);
    const MB3DVolume& box = scene.volume;
    const auto eye = camera.eye();
    double planes[3] = {header.dCutX, header.dCutY, header.dCutZ}; // packed header fields
    header.bCutOption = 0;
    for (int axis = 0; axis < 3; ++axis) {
        const bool fromBelow = camera.forward[axis] > 0 && eye[axis] < box.min[axis];
        const bool fromAbove = camera.forward[axis] < 0 && eye[axis] > box.max[axis];
        if (!fromBelow && !fromAbove) continue;
        header.bCutOption |= static_cast<uint8_t>(1 << axis);
        planes[axis] = fromBelow ? box.min[axis] : box.max[axis];
    }
    header.dCutX = planes[0];
    header.dCutY = planes[1];
    header.dCutZ = planes[2];
    double far = -1e300;
    for (int corner = 0; corner < 8; ++corner) {
        double depth = 0;
        for (int axis = 0; axis < 3; ++axis)
            depth += ((corner >> axis & 1 ? box.max[axis] : box.min[axis]) - camera.center[axis]) *
                     camera.forward[axis];
        far = std::max(far, depth);
    }
    if (far > camera.offset && far < camera.farOffset) header.dZend = header.dZmid + far;
}

struct Pass { int width, height; };

std::vector<Pass> passes(int width, int height, const MB3DRenderRequest& request) {
    std::vector<Pass> out;
    if (request.quality == MB3D_QUALITY_PREVIEW) {
        const int longEdge = std::max(width, height);
        for (int edge = request.preview_first_edge > 0 ? request.preview_first_edge : 160;
             edge < longEdge; edge *= 2) {
            const double scale = double(edge) / longEdge;
            out.push_back({std::max(2, int(width * scale)), std::max(2, int(height * scale))});
        }
    }
    out.push_back({width, height});
    return out;
}
} // namespace

extern "C" {

MB3DScene* mb3d_scene_load(const char* m3p_path, char* error, size_t error_size) {
    if (!m3p_path) { setError(error, error_size, "m3p_path is NULL"); return nullptr; }
    try {
        auto loaded = mb3d::LoadM3PFile(m3p_path);
        if (!loaded.success) { setError(error, error_size, loaded.error); return nullptr; }
        auto* scene = new MB3DScene;
        scene->header = loaded.header;
        scene->addon = loaded.addon;
        return scene;
    } catch (const std::exception& e) {
        setError(error, error_size, e.what());
        return nullptr;
    }
}

MB3DScene* mb3d_scene_clone(const MB3DScene* scene) {
    try { return scene ? new MB3DScene(*scene) : nullptr; } catch (...) { return nullptr; }
}

void mb3d_scene_destroy(MB3DScene* scene) { delete scene; }

int mb3d_scene_save(const MB3DScene* scene, const char* m3p_path) {
    try { return scene && m3p_path && mb3d::SaveM3PFile(m3p_path, scene->header, scene->addon); }
    catch (...) { return 0; }
}

void mb3d_scene_set_resource_root(MB3DScene* scene, const char* directory) {
    if (scene) scene->resource_root = directory ? directory : "";
}

void mb3d_scene_get_size(const MB3DScene* scene, int* width, int* height) {
    if (width) *width = scene ? scene->header.Width : 0;
    if (height) *height = scene ? scene->header.Height : 0;
}

int mb3d_scene_get_iterations(const MB3DScene* scene) { return scene ? scene->header.Iterations : 0; }

int mb3d_scene_set_iterations(MB3DScene* scene, int iterations) {
    if (!scene || iterations < 1) return 0;
    scene->header.Iterations = iterations;
    return 1;
}

/* ---- Formulas ---------------------------------------------------------- */

int mb3d_formula_count(void) { return static_cast<int>(mb3d::ExternalFormulaCount()); }

const char* mb3d_formula_name(int index) {
    const auto* info = index < 0 ? nullptr : mb3d::ExternalFormulaAt(static_cast<size_t>(index));
    return info ? info->name : nullptr;
}

int mb3d_formula_exists(const char* name) { return name && mb3d::FindExternalFormula(name) != nullptr; }

int mb3d_scene_get_formula(const MB3DScene* scene, int slot, MB3DFormulaSlot* out) {
    if (!validSlot(scene, slot) || !out) return 0;
    const auto& formula = scene->addon.Formulas[slot];
    std::memset(out, 0, sizeof(*out));
    std::snprintf(out->name, sizeof(out->name), "%s", slotName(formula).c_str());
    out->iterations = formula.iItCount;
    out->param_count = std::clamp(formula.iOptionCount, 0, MB3D_FORMULA_PARAMS);
    out->builtin = formula.iFnr >= 0 && formula.iFnr <= 9;
    return 1;
}

int mb3d_scene_set_formula_m3f(MB3DScene* scene, int slot, const char* m3f_path,
                               int iterations, char* error, size_t error_size) {
    if (!validSlot(scene, slot) || !m3f_path || iterations < 1) {
        setError(error, error_size, "Invalid scene, slot, path or iteration count");
        return 0;
    }
    try {
        std::string name = m3f_path;
        name = name.substr(name.find_last_of("/\\") + 1);
        if (name.size() > 4 && lower(name.substr(name.size() - 4)) == ".m3f") name.resize(name.size() - 4);
        if (name.empty() || name.size() > 31 || !mb3d::FindExternalFormula(name.c_str())) {
            setError(error, error_size, "Formula is not compiled into the library: " + name);
            return 0;
        }
        std::ifstream file(m3f_path);
        if (!file) { setError(error, error_size, "Cannot read " + std::string(m3f_path)); return 0; }
        // Value slots follow Pascal CustomFormulas.pas: angle triples and sextets
        // expand one option line into several slots sharing its default.
        std::vector<std::pair<std::string, double>> values;
        bool options = false;
        for (std::string line; std::getline(file, line);) {
            line = trim(line);
            if (line.empty()) continue;
            if (line[0] == '[') {
                if (options) break;
                options = lower(line) == "[options]";
                continue;
            }
            const size_t equals = line.rfind('=');
            const size_t space = line.find(' ');
            if (!options || line[0] != '.' || equals == std::string::npos || space > equals) continue;
            const std::string type = lower(line.substr(1, space - 1));
            const std::string label = trim(line.substr(space, equals - space));
            if (label.empty()) continue; // .Version = 5 and other formula properties
            std::string number = trim(line.substr(equals + 1));
            std::replace(number.begin(), number.end(), ',', '.');
            const double value = std::strtod(number.c_str(), nullptr);
            if (type == "3doubleangles" || type == "3singleangles") {
                for (const char* axis : {" X", " Y", " Z"}) values.push_back({label + axis, value});
            } else if (type == "6singleangles") {
                for (int i = 1; i <= 6; ++i) values.push_back({label + " " + std::to_string(i), value});
            } else if (type == "3scalesangles") {
                values.push_back({label, value});
                for (const char* axis : {"Rotation A", "Rotation B", "Rotation C"}) values.push_back({axis, 0.0});
            } else if (type == "scalesrot") {
                setError(error, error_size, "Option type ScalesRot is not supported");
                return 0;
            } else {
                values.push_back({label, value});
            }
        }
        if (values.size() > MB3D_FORMULA_PARAMS) {
            setError(error, error_size, "Formula declares more than 16 parameter values");
            return 0;
        }
        auto& formula = scene->addon.Formulas[slot];
        std::memset(&formula, 0, sizeof(formula));
        formula.iItCount = iterations;
        formula.iFnr = 20; // resolved external formula, as the parser marks it
        formula.iOptionCount = static_cast<int32_t>(values.size());
        std::memcpy(formula.CustomFname, name.data(), name.size());
        for (size_t i = 0; i < MB3D_FORMULA_PARAMS; ++i) {
            formula.dOptionValue[i] = i < values.size() ? values[i].second : 0;
            scene->param_names[slot][i] = i < values.size() ? values[i].first : "";
        }
        mb3d::ApplyExternalFormulaOptionTypes(formula);
        scene->addon.iFCount = static_cast<uint8_t>(std::max<int>(scene->addon.iFCount, slot + 1));
        return 1;
    } catch (const std::exception& e) {
        setError(error, error_size, e.what());
        return 0;
    }
}

int mb3d_scene_clear_formula(MB3DScene* scene, int slot) {
    if (!validSlot(scene, slot)) return 0;
    auto& formula = scene->addon.Formulas[slot];
    std::memset(&formula, 0, sizeof(formula));
    formula.iFnr = -1; // Pascal's empty slot
    for (auto& name : scene->param_names[slot]) name.clear();
    return 1;
}

int mb3d_scene_set_formula_iterations(MB3DScene* scene, int slot, int iterations) {
    if (!validSlot(scene, slot) || iterations < 0) return 0;
    scene->addon.Formulas[slot].iItCount = iterations;
    return 1;
}

int mb3d_scene_get_formula_param(const MB3DScene* scene, int slot, int index, double* value) {
    if (!validSlot(scene, slot) || !value || index < 0 ||
        index >= std::clamp(scene->addon.Formulas[slot].iOptionCount, 0, MB3D_FORMULA_PARAMS)) return 0;
    *value = scene->addon.Formulas[slot].dOptionValue[index];
    return 1;
}

int mb3d_scene_set_formula_param(MB3DScene* scene, int slot, int index, double value) {
    if (!validSlot(scene, slot) || !std::isfinite(value) || index < 0 ||
        index >= std::clamp(scene->addon.Formulas[slot].iOptionCount, 0, MB3D_FORMULA_PARAMS)) return 0;
    scene->addon.Formulas[slot].dOptionValue[index] = value;
    return 1;
}

const char* mb3d_scene_formula_param_name(const MB3DScene* scene, int slot, int index) {
    if (!validSlot(scene, slot) || index < 0 || index >= MB3D_FORMULA_PARAMS) return "";
    return scene->param_names[slot][index].c_str();
}

MB3DHybridMode mb3d_scene_get_hybrid_mode(const MB3DScene* scene) {
    const int mode = scene ? scene->addon.bOptions1 & 3 : 0;
    return mode == 1 ? MB3D_HYBRID_INTERPOLATE : mode == 2 ? MB3D_HYBRID_DE_COMBINATE : MB3D_HYBRID_ALTERNATE;
}

int mb3d_scene_set_hybrid_mode(MB3DScene* scene, MB3DHybridMode mode) {
    if (!scene || mode < MB3D_HYBRID_ALTERNATE || mode > MB3D_HYBRID_DE_COMBINATE) return 0;
    scene->addon.bOptions1 = static_cast<uint8_t>((scene->addon.bOptions1 & ~3) | mode);
    return 1;
}

/* ---- Camera ------------------------------------------------------------ */

extern "C++" {
namespace {
using mb3d::navigation::StartPlane;
using mb3d::navigation::SurfaceProbe;
using mb3d::navigation::Vec;

// The MB3D navigator sizes the start plane to its local distance estimate:
// half the frame diagonal is 0.53 of it.
constexpr double kPlanePerClearance = 0.53;
// A start plane smaller than this share of the scene coordinates leaves the
// binary32 range of the Metal renderer.
constexpr double kSmallestPlane = 2e-5;

bool navigable(const MB3DScene* scene) {
    return scene->header.bPlanarOptic != 2 && scene->nav_ratio >= 0;
}

// Distance from `start` along `direction` to the fractal, walked with the
// distance estimate in the steps the scene's ray marcher takes; negative when
// nothing is met within `far` or the walk does not arrive.
double distanceAlong(const Vec& start, const Vec& direction, double far, SurfaceProbe& probe) {
    double along = 0;
    for (int i = 0; i < 160; ++i) {
        const double free = probe.clearance(mb3d::navigation::add(start, direction, along));
        if (free <= std::max(1e-12, along * 1e-3)) return i == 0 ? -1 : along;
        along += free * probe.stride();
        if (along > far) return -1;
    }
    return -1;
}

// From the centre of the start plane to the surface the camera looks at.
double distanceAhead(const Camera& camera, SurfaceProbe& probe) {
    return distanceAlong(camera.eye(), camera.forward, camera.farOffset - camera.offset, probe);
}

Vec viewpointOf(const TMandHeader10& header) {
    const Camera camera(header);
    return mb3d::navigation::add(camera.eye(), camera.forward, -StartPlane::of(header, camera.span, 0).near);
}

// Remember the camera the gestures start from. A scene whose start plane
// already cuts the fractal (some are authored so) is left to move unchecked.
void captureNavigation(MB3DScene* scene, SurfaceProbe& probe) {
    if (scene->nav_near > 0 || !navigable(scene)) return;
    const Camera camera(scene->header);
    const auto plane = StartPlane::of(scene->header, camera.span, scene->nav_aspect);
    const double free = probe.valid() ? probe.clearance(camera.eye()) : 0;
    if (!(free > 0) || !(plane.near > 0) || !(plane.radius > 0)) {
        scene->nav_ratio = -1;
        return;
    }
    scene->nav_near = plane.near;
    // An authored view may have a larger plane for its free space than the
    // navigator would give it, with scenery in front of it clipped away on
    // purpose: the gestures keep that proportion, so the first one shows the
    // picture as it was.
    scene->nav_ratio = std::max(kPlanePerClearance, plane.radius / (free * probe.stride()) * 1.001);
    const Vec home = mb3d::navigation::add(camera.eye(), camera.forward, -plane.near);
    for (int i = 0; i < 3; ++i) scene->nav_home[i] = home[i];
    const double ahead = distanceAhead(camera, probe);
    scene->nav_range = std::max(1.2 * (camera.farOffset - camera.offset), 3 * (plane.near + std::max(ahead, free)));
    scene->nav_open = probe.clearance(home) > 0;
}

// Same viewpoint and view, start plane `near` in front of the viewpoint.
void placePlane(TMandHeader10& header, double near) {
    Camera camera(header);
    const auto plane = StartPlane::of(header, camera.span, 0);
    if (!(plane.near > 0) || !(near > 0) || std::abs(near - plane.near) <= 1e-9 * plane.near) return;
    camera.shiftPlane(near - plane.near);
    camera.write(header);
    header.dZoom *= plane.near / near;
}

// Keep the camera's start plane out of the fractal. MB3D starts every ray on
// a rectangle in front of the viewpoint; where the rectangle is inside the
// fractal the picture shows a flat cut. The viewpoint and the view stay as
// they are and the rectangle is pulled towards the viewpoint (the zoom grows
// by the same factor) until the space around it is free. With free space
// ahead it is let out again, not past the surface the camera looks at and not
// past where the gestures started. False: no plane of this viewpoint is free.
bool keepPlaneClear(MB3DScene* scene, SurfaceProbe& probe) {
    if (!navigable(scene) || !(scene->nav_near > 0) || !probe.valid()) return true;
    auto& header = scene->header;
    const Camera camera(header);
    const auto plane = StartPlane::of(header, camera.span, scene->nav_aspect);
    if (!(plane.near > 0) || !(plane.radius > 0)) return true;
    const Vec viewpoint = mb3d::navigation::add(camera.eye(), camera.forward, -plane.near);
    const double radiusPerNear = plane.radius / plane.near;
    const double reach = std::max({1.0, std::abs(viewpoint[0]), std::abs(viewpoint[1]), std::abs(viewpoint[2])});
    const double smallest = kSmallestPlane * reach / std::max(1e-12, radiusPerNear);
    // The estimate may overshoot by as much as the ray marcher distrusts it.
    const auto clear = [&](double near) {
        const double free = probe.clearance(mb3d::navigation::add(viewpoint, camera.forward, near)) * probe.stride();
        return free > 0 && near * radiusPerNear <= scene->nav_ratio * free;
    };
    double near = plane.near;
    if (near < scene->nav_near && probe.clearance(camera.eye()) > 0) {
        const double ahead = distanceAhead(camera, probe);
        near = ahead < 0 ? scene->nav_near : std::min(scene->nav_near, near + 0.9 * ahead);
    }
    double rejected = 0;
    while (!clear(near)) {
        rejected = near;
        near *= 0.7;
        if (near < smallest) return false;
    }
    for (int i = 0; i < 4 && rejected > 0; ++i) {
        const double middle = std::sqrt(near * rejected);
        if (clear(middle)) near = middle; else rejected = middle;
    }
    placePlane(header, near);
    return true;
}

// One gesture step: `change` moves the camera and may return where the start
// plane goes (its distance from the viewpoint, 0: where it is); then the plane
// is kept clear. A step that would put the camera into the fractal is taken back.
template <class Change>
int navigate(MB3DScene* scene, Change change) {
    if (!scene || !Camera::supports(scene->header)) return 0;
    try {
        SurfaceProbe probe(scene->header, scene->addon, scene->resource_root);
        captureNavigation(scene, probe);
        const TMandHeader10 before = scene->header;
        Camera camera(scene->header);
        // A camera already inside (a slider moved the fractal over it) must be able to leave.
        const bool wasFree = !navigable(scene) || !probe.valid() || probe.clearance(camera.eye()) > 0;
        const double near = change(camera, probe);
        camera.write(scene->header);
        if (near > 0) placePlane(scene->header, near);
        if (navigable(scene) && scene->nav_near > 0 && probe.valid() && wasFree) {
            const Vec from = viewpointOf(before), to = viewpointOf(scene->header);
            const Vec home{scene->nav_home[0], scene->nav_home[1], scene->nav_home[2]};
            const auto distance = [](const Vec& a, const Vec& b) { return std::hypot(a[0] - b[0], a[1] - b[1], a[2] - b[2]); };
            // Not out of sight of the scene, and not through a wall with the viewpoint.
            const bool leaves = distance(to, home) > scene->nav_range && distance(to, home) > distance(from, home);
            const bool enters = scene->nav_open && probe.clearance(from) > 0 && !(probe.clearance(to) > 0);
            if (leaves || enters) {
                scene->header = before;
                return 0;
            }
        }
        if (!keepPlaneClear(scene, probe) && wasFree) {
            scene->header = before;
            return 0;
        }
        // The far limit stays ahead of the start plane wherever the camera went.
        Camera moved(scene->header);
        moved.keepDepth(4 * moved.span);
        moved.write(scene->header);
        return 1;
    } catch (...) { return 0; }
}
}  // namespace
}  // extern "C++"

int mb3d_scene_get_camera(const MB3DScene* scene, MB3DCamera* out) {
    if (!scene || !out || !Camera::supports(scene->header)) return 0;
    try {
        const Camera camera(scene->header);
        const auto eye = camera.eye();
        for (int i = 0; i < 3; ++i) {
            out->eye[i] = eye[i];
            out->right[i] = camera.right[i];
            out->up[i] = camera.up[i];
            out->forward[i] = camera.forward[i];
        }
        out->fov_y_degrees = scene->header.dFOVy;
        out->zoom = scene->header.dZoom;
        out->pivot = -camera.offset;
        out->depth = camera.farOffset - camera.offset;
        return 1;
    } catch (...) { return 0; }
}

int mb3d_scene_set_camera(MB3DScene* scene, const MB3DCamera* in) {
    if (!scene || !in || !Camera::supports(scene->header) || !(in->zoom > 0) ||
        !(in->fov_y_degrees > 0) || !std::isfinite(in->zoom) || !std::isfinite(in->fov_y_degrees) ||
        !std::isfinite(in->pivot) || !(in->depth > 0) || !std::isfinite(in->depth)) return 0;
    try {
        Camera camera(scene->header);
        for (int i = 0; i < 3; ++i) {
            camera.right[i] = in->right[i];
            camera.up[i] = in->up[i];
            camera.forward[i] = in->forward[i];
        }
        camera.right = mb3d::navigation::unit(camera.right);
        camera.up = mb3d::navigation::unit(camera.up);
        camera.forward = mb3d::navigation::unit(camera.forward);
        camera.offset = -in->pivot;
        camera.farOffset = camera.offset + in->depth;
        camera.center = mb3d::navigation::add({in->eye[0], in->eye[1], in->eye[2]}, camera.forward, -camera.offset);
        camera.changed = true;
        camera.write(scene->header);
        scene->header.dZoom = in->zoom;
        scene->header.dFOVy = in->fov_y_degrees;
        // The gestures start again from this camera.
        scene->nav_near = 0;
        scene->nav_ratio = 0;
        return 1;
    } catch (...) { return 0; }
}

int mb3d_scene_rotate_camera(MB3DScene* scene, double yaw, double pitch, int fly) {
    if (!std::isfinite(yaw) || !std::isfinite(pitch)) return 0;
    return navigate(scene, [&](Camera& camera, SurfaceProbe& probe) {
        // Close to a surface the camera turns about what it looks at: a turn
        // about a pivot far behind it would sweep the view away.
        if (!fly && camera.offset < 0 && navigable(scene) && probe.valid()) {
            const double ahead = distanceAhead(camera, probe);
            if (ahead > 0 && 3 * ahead < -camera.offset) camera.repivot(3 * ahead);
        }
        camera.rotate(yaw, pitch, fly != 0);
        return 0.0;
    });
}

int mb3d_scene_roll_camera(MB3DScene* scene, double angle) {
    if (!std::isfinite(angle)) return 0;
    return navigate(scene, [&](Camera& camera, SurfaceProbe&) { camera.roll(angle); return 0.0; });
}

int mb3d_scene_move_camera(MB3DScene* scene, double right, double up, double forward) {
    if (!std::isfinite(right) || !std::isfinite(up) || !std::isfinite(forward)) return 0;
    return navigate(scene, [&](Camera& camera, SurfaceProbe&) { camera.move(right, up, forward); return 0.0; });
}

int mb3d_scene_pan_camera(MB3DScene* scene, double right, double up) {
    if (!std::isfinite(right) || !std::isfinite(up)) return 0;
    return navigate(scene, [&](Camera& camera, SurfaceProbe& probe) {
        // The rays fan out from the start plane: at the surface ahead the view
        // is wider than the plane by (near + distance) / near.
        double wider = 1;
        if (navigable(scene) && probe.valid()) {
            const auto plane = StartPlane::of(scene->header, camera.span, 0);
            // Looking past a near surface at a far one, the near one sets the pace.
            const double free = probe.clearance(camera.eye());
            double ahead = distanceAhead(camera, probe);
            ahead = ahead > 0 ? std::min(ahead, 4 * free) : free;
            if (ahead > 0 && plane.near > 0) wider = (plane.near + ahead) / plane.near;
        }
        camera.move(right * wider, up * wider, 0);
        return 0.0;
    });
}

int mb3d_scene_dolly_camera(MB3DScene* scene, double amount) {
    if (!std::isfinite(amount)) return 0;
    if (!scene || !Camera::supports(scene->header)) return 0;
    amount = std::clamp(amount, -1.5, 1.5);
    // A step that ends in the fractal beside the central ray is tried shorter.
    for (double share = 1; share > 0.1; share *= 0.5) {
        const double part = amount * share;
        const int moved = navigate(scene, [&](Camera& camera, SurfaceProbe& probe) {
            const auto plane = StartPlane::of(scene->header, camera.span, 0);
            // Where nothing is ahead, and where distances are unknown: fractions
            // of the view span the gestures started with.
            const double limit = std::abs(part) * camera.span *
                (scene->nav_near > 0 && plane.near > 0 ? scene->nav_near / plane.near : 1.0);
            double step = part < 0 ? -limit : limit, near = 0;
            if (navigable(scene) && probe.valid()) {
                const double ahead = distanceAhead(camera, probe);
                if (ahead > 0) {
                    // The distance from the viewpoint to that surface shrinks or grows
                    // by e^amount: the camera never arrives. The start plane stays in
                    // front of the surface.
                    step = (plane.near + ahead) * -std::expm1(-part);
                    if (step > 0.9 * ahead) near = plane.near + 0.9 * ahead - step;
                    if (step < 0 && scene->nav_open) {
                        // Backing away stops in front of a wall behind the viewpoint.
                        const Vec viewpoint = mb3d::navigation::add(camera.eye(), camera.forward, -plane.near);
                        const Vec back{-camera.forward[0], -camera.forward[1], -camera.forward[2]};
                        const double behind = distanceAlong(viewpoint, back, -step, probe);
                        if (behind >= 0) step = -0.8 * behind;
                    }
                } else {
                    // Nothing in the middle of the picture: the nearest surface sets the pace.
                    const double free = probe.clearance(camera.eye());
                    if (free > 0) step = std::copysign(std::min(limit, free * std::abs(std::expm1(-part))), part);
                }
            }
            camera.dolly(step);
            return near;
        });
        if (moved) return 1;
    }
    return 0;
}

int mb3d_scene_set_navigation_aspect(MB3DScene* scene, double aspect) {
    if (!scene || !std::isfinite(aspect) || aspect < 0) return 0;
    scene->nav_aspect = aspect;
    return 1;
}

MB3DScene* mb3d_scene_create_eye(const MB3DScene* scene, MB3DEye eye, double separation) {
    if (!scene || !std::isfinite(separation) || (eye != MB3D_EYE_LEFT && eye != MB3D_EYE_RIGHT)) return nullptr;
    MB3DScene* out = mb3d_scene_clone(scene);
    try {
        if (!out || !Camera::supports(out->header)) throw std::runtime_error("camera");
        // Both eyes keep the start plane of the scene: no plane fit here.
        Camera camera(out->header);
        camera.move((eye == MB3D_EYE_LEFT ? -0.5 : 0.5) * separation, 0, 0);
        camera.write(out->header);
    } catch (...) {
        delete out;
        out = nullptr;
    }
    return out;
}

/* ---- Colours, fog and light -------------------------------------------- */
// Field meanings follow LightAdjust.pas (slider to header) and
// MakeLightValsFromHeaderLight / CalcSCstartAndSCmul (header to renderer).
// TBpos[n] here is Pascal's TBpos[n + 3]. Every setter leaves a field alone
// when the request equals what its getter reads from that field.

static MB3DColor colorOf(const TRGB& rgb) { return {rgb[0], rgb[1], rgb[2]}; }

static void storeColor(TRGB& rgb, const MB3DColor& color) {
    rgb[0] = color.r; rgb[1] = color.g; rgb[2] = color.b;
}

int mb3d_scene_get_palette(const MB3DScene* scene, MB3DColor* colors) {
    if (!scene || !colors) return 0;
    for (int i = 0; i < MB3D_PALETTE_COLORS; ++i) {
        const uint32_t diffuse = scene->header.Light.LCols[i].ColorDif; // Pascal TColor: red is the low byte
        colors[i] = {static_cast<unsigned char>(diffuse), static_cast<unsigned char>(diffuse >> 8),
                     static_cast<unsigned char>(diffuse >> 16)};
    }
    return 1;
}

int mb3d_scene_set_palette(MB3DScene* scene, const MB3DColor* colors) {
    if (!scene || !colors) return 0;
    for (int i = 0; i < MB3D_PALETTE_COLORS; ++i) {
        const uint32_t kept = scene->header.Light.LCols[i].ColorDif & 0xFF000000u;
        scene->header.Light.LCols[i].ColorDif =
            kept | colors[i].r | uint32_t(colors[i].g) << 8 | uint32_t(colors[i].b) << 16;
    }
    return 1;
}

int mb3d_scene_get_palette_positions(const MB3DScene* scene, double* positions) {
    if (!scene || !positions) return 0;
    for (int i = 0; i < MB3D_PALETTE_COLORS; ++i) positions[i] = scene->header.Light.LCols[i].Position / 32767.0;
    return 1;
}

int mb3d_scene_set_palette_positions(MB3DScene* scene, const double* positions) {
    if (!scene || !positions) return 0;
    uint16_t stored[MB3D_PALETTE_COLORS];
    for (int i = 0; i < MB3D_PALETTE_COLORS; ++i) {
        if (!std::isfinite(positions[i])) return 0;
        const uint16_t current = scene->header.Light.LCols[i].Position;
        // The renderer blends from the first colour only at and above its position.
        stored[i] = positions[i] == current / 32767.0 ? current
            : i == 0 ? 0 : static_cast<uint16_t>(std::lround(std::clamp(positions[i], 0.0, 1.0) * 32767.0));
        if (i > 0 && stored[i] < stored[i - 1]) return 0;
    }
    for (int i = 0; i < MB3D_PALETTE_COLORS; ++i) scene->header.Light.LCols[i].Position = stored[i];
    return 1;
}

// Colouring value, 0..1, at a "Color start/end" slider position (-30..90).
static double paletteValue(int position) {
    const double p = (position + 30) / 90.0;
    return p * p - 10900.0 / 32767.0;
}

// The slider position MB3D's histogram fit picks for a colouring value.
static int palettePosition(double value) {
    return static_cast<int>(std::lround(
        std::clamp(std::sqrt(std::max(0.0, value + 10900.0 / 32767.0)) * 90.0, 0.0, 120.0))) - 30;
}

static void paletteRange(const TLightingParas9& light, double& start, double& end) {
    start = paletteValue(light.TBpos[6]);
    end = paletteValue(light.TBpos[7]);
    if (light.TBoptions & 0x10000u) { // fine adjustment: 30 and 90 are the ends of the coarse range
        const double span = end - start;
        end = start + span * (int(light.FineColAdj2) - 30) / 60.0;
        start += span * (int(light.FineColAdj1) - 30) / 60.0;
    }
}

int mb3d_scene_get_palette_range(const MB3DScene* scene, double* start, double* end) {
    if (!scene || !start || !end) return 0;
    paletteRange(scene->header.Light, *start, *end);
    return 1;
}

int mb3d_scene_set_palette_range(MB3DScene* scene, double start, double end) {
    if (!scene || !std::isfinite(start) || !std::isfinite(end)) return 0;
    TLightingParas9& light = scene->header.Light;
    double currentStart, currentEnd;
    paletteRange(light, currentStart, currentEnd);
    if (start == currentStart && end == currentEnd) return 1;
    start = std::clamp(start, MB3D_PALETTE_RANGE_MIN, MB3D_PALETTE_RANGE_MAX);
    end = std::clamp(end, MB3D_PALETTE_RANGE_MIN, MB3D_PALETTE_RANGE_MAX);
    // Coarse sliders at the nearest positions, at least one apart; the fine
    // sliders divide that range into 60 steps and reach half a range beyond it.
    int first = palettePosition(start), last = palettePosition(end);
    if (first == last) {
        const int step = end < start ? -1 : 1;
        if (last + step < -30 || last + step > 90) first -= step; else last += step;
    }
    const double low = paletteValue(first), span = paletteValue(last) - low;
    const auto fine = [&](double value) {
        return static_cast<int>(std::clamp(std::lround((value - low) / span * 60.0), -30L, 90L)) + 30;
    };
    const int fineStart = fine(start), fineEnd = fine(end);
    const bool adjusted = fineStart != 30 || fineEnd != 90;
    light.TBpos[6] = first;
    light.TBpos[7] = last;
    light.FineColAdj1 = static_cast<uint8_t>(adjusted ? fineStart : 0); // Pascal stores 0 with fine adjustment off
    light.FineColAdj2 = static_cast<uint8_t>(adjusted ? fineEnd : 0);
    light.TBoptions = adjusted ? light.TBoptions | 0x10000u : light.TBoptions & ~0x10000u;
    return 1;
}

int mb3d_scene_get_fog(const MB3DScene* scene, MB3DFog* out) {
    if (!scene || !out) return 0;
    const TLightingParas9& light = scene->header.Light;
    out->color = colorOf(light.DepthCol);
    out->color_bottom = colorOf(light.DepthCol2);
    out->amount = light.TBpos[1] / 240.0; // "Depth" slider, 0..240
    return 1;
}

int mb3d_scene_set_fog(MB3DScene* scene, const MB3DFog* fog) {
    if (!scene || !fog || !std::isfinite(fog->amount)) return 0;
    TLightingParas9& light = scene->header.Light;
    storeColor(light.DepthCol, fog->color);
    storeColor(light.DepthCol2, fog->color_bottom);
    if (fog->amount != light.TBpos[1] / 240.0)
        light.TBpos[1] = static_cast<int32_t>(std::lround(std::clamp(fog->amount, 0.0, 1.0) * 240.0));
    return 1;
}

// Unit vector from the surface to a global light with the angles ax = -LXpos
// and ay = LYpos, in view axes: x right, y down, z away from the camera.
static void lightVector(double ax, double ay, double vector[3]) {
    vector[0] = std::sin(ax);
    vector[1] = -std::sin(ay);
    vector[2] = -std::cos(ax) * std::cos(ay);
    const double length = std::sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
    for (int i = 0; i < 3; ++i) vector[i] /= length; // length is at least 1
}

// Inverse of lightVector for a unit vector. A light behind the scene has one
// angle beyond 90 degrees: the one of the larger sideways component.
static void lightAngles(const double vector[3], double& ax, double& ay) {
    const double product = vector[0] * vector[1];
    // Squared length before normalising: 1 + sin(ax)^2 sin(ay)^2.
    const double scale = std::sqrt(2.0 / (1.0 + std::sqrt(std::max(0.0, 1.0 - 4.0 * product * product))));
    ax = std::asin(std::clamp(vector[0] * scale, -1.0, 1.0));
    ay = std::asin(std::clamp(-vector[1] * scale, -1.0, 1.0));
    if (vector[2] <= 0) return;
    if (std::fabs(vector[1]) >= std::fabs(vector[0])) ay = (ay < 0 ? -mb3d::Pi : mb3d::Pi) - ay;
    else ax = (ax < 0 ? -mb3d::Pi : mb3d::Pi) - ax;
}

// A light with "Rel. to object" keeps its angles in object axes; the renderer
// turns its vector by the camera rows. toObject undoes that turn.
static void turnLight(const TMandHeader10& header, bool toObject, double vector[3]) {
    double rows[3][3];
    std::memcpy(rows, header.hVGrads, sizeof(rows));
    double turned[3] = {0, 0, 0};
    for (int i = 0; i < 3; ++i) {
        const double length = std::sqrt(rows[i][0] * rows[i][0] + rows[i][1] * rows[i][1] + rows[i][2] * rows[i][2]) + 1e-100;
        for (int j = 0; j < 3; ++j) {
            if (toObject) turned[i] += rows[i][j] / length * vector[j];
            else turned[j] += rows[i][j] / length * vector[i];
        }
    }
    const double length = std::sqrt(turned[0] * turned[0] + turned[1] * turned[1] + turned[2] * turned[2]);
    for (int i = 0; i < 3; ++i) vector[i] = length > 1e-20 ? turned[i] / length : vector[i];
}

// Index of the first light that is on and neither positional nor a light map.
static int globalLight(const TLightingParas9& lighting) {
    for (int i = 0; i < 6; ++i)
        if ((lighting.Lights[i].Loption & 7) == 0) return i;
    return -1;
}

static bool castsHardShadow(const TMandHeader10& header, int index) {
    // Bit 0 schedules the shadow pass, bit index + 2 selects the light, and
    // Loption bit 6 hides a calculated shadow of this light.
    return (header.bCalculateHardShadow & 1) && (header.bCalculateHardShadow & (4 << index)) &&
           !(header.Light.Lights[index].Loption & 0x40);
}

static void readLight(const TMandHeader10& header, int index, MB3DLight& out) {
    const TLight8& light = header.Light.Lights[index];
    double ax = -mb3d::D7BtoDouble(light.LXpos), ay = mb3d::D7BtoDouble(light.LYpos);
    if (light.Loption & 0x20) {
        double vector[3];
        lightVector(ax, ay, vector);
        turnLight(header, false, vector);
        lightAngles(vector, ax, ay);
    }
    // The vector has the period of a full turn in each angle.
    out.x = std::remainder(ax, 2 * mb3d::Pi) / mb3d::Pi;
    out.y = std::remainder(ay, 2 * mb3d::Pi) / mb3d::Pi;
    ShortFloat lamp; // mantissa and decimal exponent, as ShortFloatToSingle reads them
    std::memcpy(&lamp, &light.Lamp, sizeof(lamp));
    const int exponent = std::clamp<int>(lamp[1], -25, 25) - 1;
    out.brightness = exponent < 0 ? lamp[0] / std::pow(10.0, -exponent) : lamp[0] * std::pow(10.0, exponent);
    out.hard_shadow = castsHardShadow(header, index);
}

int mb3d_scene_get_light(const MB3DScene* scene, MB3DLight* out) {
    if (!scene || !out) return 0;
    *out = MB3DLight{};
    const int index = globalLight(scene->header.Light);
    if (index < 0) return 0;
    readLight(scene->header, index, *out);
    return 1;
}

int mb3d_scene_set_light(MB3DScene* scene, const MB3DLight* in) {
    if (!scene || !in || !std::isfinite(in->x) || !std::isfinite(in->y) || !std::isfinite(in->brightness)) return 0;
    TMandHeader10 header = scene->header;
    int index = globalLight(header.Light);
    if (index < 0) {
        for (int i = 0; i < 6 && index < 0; ++i) {
            const TLight8& light = header.Light.Lights[i];
            const int option = light.Loption & 3; // the renderer's on/off/map state
            if (option == 1 || option == 3 || (option == 2 && light.LightMapNr == 0)) index = i;
        }
        if (index < 0) return 0;
        // Pascal's defaultLight8, switched on. AdditionalByteEx and FreeByte
        // of a light hold unrelated scene options and stay.
        TLight8& light = header.Light.Lights[index];
        light.Loption = 0;
        light.LFunction = 4;
        light.Lamp = 10; // ShortFloat 1.0
        light.Lcolor[0] = light.Lcolor[1] = light.Lcolor[2] = 255;
        light.LightMapNr = 0;
        std::memset(light.LXpos, 0, sizeof(light.LXpos));
        std::memset(light.LYpos, 0, sizeof(light.LYpos));
        std::memset(light.LZpos, 0, sizeof(light.LZpos));
    }
    TLight8& light = header.Light.Lights[index];
    MB3DLight current;
    readLight(header, index, current);
    if (in->x != current.x || in->y != current.y) {
        double ax = std::clamp(in->x, -1.0, 1.0) * mb3d::Pi, ay = std::clamp(in->y, -1.0, 1.0) * mb3d::Pi;
        if (light.Loption & 0x20) {
            double vector[3];
            lightVector(ax, ay, vector);
            turnLight(header, true, vector);
            lightAngles(vector, ax, ay);
        }
        mb3d::DoubleToD7B(-ax, light.LXpos);
        mb3d::DoubleToD7B(ay, light.LYpos);
    }
    if (in->brightness != current.brightness) {
        ShortFloat lamp;
        mb3d::SingleToShortFloat(static_cast<float>(std::clamp(in->brightness, 0.0, 100.0)), lamp);
        std::memcpy(&light.Lamp, &lamp, sizeof(lamp));
    }
    if ((in->hard_shadow != 0) != (current.hard_shadow != 0)) {
        const uint8_t bit = static_cast<uint8_t>(4 << index);
        if (in->hard_shadow) {
            // With the pass off, the other lights' selections were not in effect.
            if (!(header.bCalculateHardShadow & 1)) header.bCalculateHardShadow &= 2;
            header.bCalculateHardShadow |= 1 | bit;
            light.Loption &= static_cast<uint8_t>(~0x40);
        } else {
            header.bCalculateHardShadow &= static_cast<uint8_t>(~bit);
            header.bHScalculated &= static_cast<uint8_t>(~bit); // not a shadow kept from the file either
        }
        // MB3D offers "Softer H.S." for exactly one shadow light.
        int shadowLights = 0;
        for (int i = 0; i < 6; ++i) shadowLights += (header.bCalculateHardShadow >> (i + 2)) & 1;
        if (shadowLights != 1) header.bCalc1HSsoft &= static_cast<uint8_t>(~1);
    }
    scene->header = header;
    return 1;
}

int mb3d_scene_get_ambient(const MB3DScene* scene, double* amount) {
    if (!scene || !amount) return 0;
    *amount = (scene->header.Light.TBpos[5] & 0xFFF) / 90.0; // "Ambient" slider, 0..270
    return 1;
}

int mb3d_scene_set_ambient(MB3DScene* scene, double amount) {
    if (!scene || !std::isfinite(amount)) return 0;
    const int32_t stored = scene->header.Light.TBpos[5];
    if (amount == (stored & 0xFFF) / 90.0) return 1;
    // The bits above the slider hold the diffuse colour map rotation.
    scene->header.Light.TBpos[5] = (stored & ~0xFFF) | static_cast<int32_t>(std::lround(std::clamp(amount, 0.0, 3.0) * 90.0));
    return 1;
}

/* ---- Clip volume ------------------------------------------------------- */

int mb3d_scene_set_clip_volume(MB3DScene* scene, const MB3DVolume* volume) {
    if (!scene) return 0;
    if (!volume) { scene->has_volume = false; return 1; }
    for (int i = 0; i < 3; ++i)
        if (!std::isfinite(volume->min[i]) || !std::isfinite(volume->max[i]) || !(volume->min[i] < volume->max[i]))
            return 0;
    scene->volume = *volume;
    scene->has_volume = true;
    return 1;
}

int mb3d_scene_get_clip_volume(const MB3DScene* scene, MB3DVolume* out) {
    if (!scene || !out || !scene->has_volume) return 0;
    *out = scene->volume;
    return 1;
}

/* ---- Rendering --------------------------------------------------------- */

MB3DCancel* mb3d_cancel_create(void) { try { return new MB3DCancel; } catch (...) { return nullptr; } }

void mb3d_cancel_request(MB3DCancel* cancel) {
    if (!cancel) return;
    std::lock_guard<std::mutex> lock(cancel->mutex);
    cancel->requested = true;
    if (cancel->active) cancel->active->cancel();
}

void mb3d_cancel_reset(MB3DCancel* cancel) { if (cancel) cancel->requested = false; }
void mb3d_cancel_destroy(MB3DCancel* cancel) { delete cancel; }

// Shared by the stateless call and the persistent renderer. `library` is the
// effective shader library path (null or empty: CPU), `session` the renderer's
// device state or null.
static MB3DRenderStatus renderScene(const MB3DScene* scene, const MB3DRenderRequest* request,
                                    MB3DCancel* cancel, MB3DFrameCallback callback, void* user,
                                    char* error, size_t error_size,
                                    const char* library, mb3d::native_metal::RenderSession* session) {
    if (!scene || !request || !callback) {
        setError(error, error_size, "scene, request and callback are required");
        return MB3D_RENDER_FAILED;
    }
    const bool preview = request->quality == MB3D_QUALITY_PREVIEW;
    const bool hasLibrary = library && *library;
    if (request->backend == MB3D_BACKEND_METAL && !hasLibrary) {
        setError(error, error_size, "The Metal backend needs metal_library");
        return MB3D_RENDER_UNSUPPORTED;
    }
    const bool cpu = request->backend == MB3D_BACKEND_CPU || !hasLibrary;
    const int width = request->width > 0 ? request->width : scene->header.Width;
    const int height = request->height > 0 ? request->height : scene->header.Height;
    if (width < 2 || height < 2) {
        setError(error, error_size, "Invalid render size");
        return MB3D_RENDER_FAILED;
    }
    const bool lattice = request->lattice_stride > 1 || request->lattice_x || request->lattice_y;
    if (lattice) {
        const int stride = request->lattice_stride;
        if (stride < 2 || stride > 4 || request->lattice_x < 0 || request->lattice_y < 0 ||
            request->lattice_x >= stride || request->lattice_y >= stride ||
            request->lattice_x >= width || request->lattice_y >= height) {
            setError(error, error_size, "Invalid lattice: stride 2 to 4, x and y below it and inside the frame");
            return MB3D_RENDER_FAILED;
        }
        if (cpu || preview) {
            setError(error, error_size, "A lattice frame needs FINAL quality on the Metal backend");
            return MB3D_RENDER_UNSUPPORTED;
        }
    }
    struct ActiveControl { // the pass control is reachable from mb3d_cancel_request
        MB3DCancel* cancel;
        ActiveControl(MB3DCancel* c, const mb3d::RenderControlHandle& control) : cancel(c) {
            if (!cancel) return;
            std::lock_guard<std::mutex> lock(cancel->mutex);
            cancel->active = control;
            if (cancel->requested && control) control->cancel();
        }
        ~ActiveControl() {
            if (!cancel) return;
            std::lock_guard<std::mutex> lock(cancel->mutex);
            cancel->active.reset();
        }
    };
    try {
        auto plan = passes(width, height, *request);
        // Progress of FINAL: a quarter-size pass records how the dispatched
        // work grows with time; the full pass maps its own work through that
        // curve. Observation only: both passes render exactly as without it.
        const bool wantsProgress = !preview && request->progress;
        const bool calibrated = wantsProgress && !lattice && std::max(width, height) >= 640;
        if (calibrated) plan.insert(plan.begin(), {std::max(2, width / 4), std::max(2, height / 4)});
        struct WorkPoint { double work, seconds; };
        std::vector<WorkPoint> curve; // work dispatched before `seconds`, then the total
        double calibrationSeconds = 0;
        double reported = 0;
        const auto report = [&](double fraction) {
            if (!wantsProgress) return;
            reported = std::max(reported, std::min(1.0, fraction));
            request->progress(reported, request->progress_user);
        };
        report(0);
        std::vector<unsigned char> rgba;
        std::vector<unsigned short> depth;
        for (size_t index = 0; index < plan.size(); ++index) {
            if (cancel && cancel->requested) return MB3D_RENDER_CANCELLED;
            TMandHeader10 header = scene->header;
            THeaderCustomAddon addon = scene->addon;
            applyVolume(*scene, header);
            mb3d::RenderOptions options;
            options.width_override = plan[index].width;
            options.height_override = plan[index].height;
            options.thread_count = request->thread_count;
            options.resource_root = scene->resource_root;
            options.native_metal_outline_formulas = request->quick_pipelines != 0;
            mb3d::BackendOptions backend;
            backend.requested = cpu ? mb3d::RenderBackend::CPU
                : request->backend == MB3D_BACKEND_METAL ? mb3d::RenderBackend::Metal : mb3d::RenderBackend::Auto;
            backend.allow_cpu_fallback = request->backend != MB3D_BACKEND_METAL && !lattice;
            if (lattice) { // the general renderer is the one that takes a lattice
                options.lattice_stride = request->lattice_stride;
                options.lattice_x = request->lattice_x;
                options.lattice_y = request->lattice_y;
                backend.prefer_general_metal = true;
            }
            if (hasLibrary) backend.metal_library = library;
            backend.metal_session = session;
            if (preview) { // the viewer's draft policy
                options.no_ao = options.no_dof = options.no_reflections = true;
                header.bCalculateHardShadow &= ~1u;
                header.bHScalculated = 0;
                backend.prefer_general_metal = true;
            }
            if (!cpu || preview || request->precision == MB3D_PRECISION_NATIVE)
                options.precision = mb3d::PrecisionMode::Native;
            else if (request->precision == MB3D_PRECISION_COMPATIBLE)
                options.precision = mb3d::PrecisionMode::Compatible;
            // A CPU pass stops cooperatively through its control. A Metal pass
            // stops before its next GPU command; if it falls back to the CPU,
            // that frame gets a control of its own.
            if (cpu && (cancel || wantsProgress)) options.control = std::make_shared<mb3d::RenderControl>();
            if (!cpu && cancel && backend.allow_cpu_fallback)
                backend.cpu_fallback_control = std::make_shared<mb3d::RenderControl>();
            const bool calibration = calibrated && index == 0;
            const bool lastPass = index + 1 == plan.size();
            mb3d::BackendRenderResult result;
            double seconds = 0;
            // A calibration pass that compiled pipelines is timed again warm.
            for (int attempt = 0; attempt < 2; ++attempt) {
                if (attempt) { // FINAL changes nothing else in the scene copy
                    header = scene->header;
                    addon = scene->addon;
                    applyVolume(*scene, header);
                }
                std::atomic<std::uint64_t> dispatched{0};
                const auto work = [&] {
                    const auto& control = options.control ? options.control : backend.cpu_fallback_control;
                    return double(std::max<std::uint64_t>(dispatched.load(std::memory_order_relaxed),
                                                          control ? control->work_used() : 0));
                };
                std::vector<WorkPoint> samples;
                std::atomic<bool> running{true};
                const auto started = std::chrono::steady_clock::now();
                const auto elapsed = [&] { return std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count(); };
                std::thread watcher;
                if (calibration || (calibrated && lastPass)) watcher = std::thread([&] {
                    // Time on the calibration curve at which `scaled` work had been dispatched.
                    const auto timeAt = [&](double scaled) {
                        const double at = std::min(curve.back().work, scaled);
                        const auto upper = std::lower_bound(curve.begin(), curve.end(), at,
                            [](const WorkPoint& p, double v) { return p.work < v; });
                        if (upper == curve.end()) return curve.back().seconds;
                        if (upper == curve.begin()) return upper->seconds * (upper->work > 0 ? at / upper->work : 1);
                        const auto lower = upper - 1;
                        const double span = upper->work - lower->work;
                        return lower->seconds + (span > 0 ? (upper->seconds - lower->seconds) * (at - lower->work) / span : 0);
                    };
                    const double scale = double(width) * height / (double(plan[0].width) * plan[0].height);
                    // The command in flight covers the work between `done` and `current`.
                    double done = 0, current = 0, changed = 0;
                    while (running.load(std::memory_order_relaxed)) {
                        if (calibration) samples.push_back({work(), elapsed()});
                        else {
                            const double now = elapsed(), seen = work() / scale;
                            if (curve.empty() || seen <= 0) {
                                // No comparable work is visible (the pass runs on another
                                // backend than the calibration): approach 1 by time alone.
                                const double expected = std::max(0.05, calibrationSeconds * scale);
                                report(std::min(0.995, now / (now + expected)));
                                std::this_thread::sleep_for(std::chrono::milliseconds(30));
                                continue;
                            }
                            if (seen > current) { done = current; current = seen; changed = now; }
                            const double total = curve.back().seconds, from = timeAt(done), to = timeAt(current);
                            // Seconds of this pass per calibration second, from the finished part.
                            const double pace = from > 0.02 * total && changed > 0 ? changed / from : scale;
                            const double time = from + std::min(to - from, (now - changed) / std::max(1.0, pace));
                            report(std::min(0.995, time / total));
                        }
                        std::this_thread::sleep_for(calibration ? std::chrono::microseconds(500) : std::chrono::milliseconds(30));
                    }
                });
                struct Join {
                    std::atomic<bool>& running; std::thread& thread;
                    ~Join() { running.store(false); if (thread.joinable()) thread.join(); }
                } join{running, watcher};
                {
                    const mb3d::RenderStageObserver stages = [&](mb3d::RenderProfileStage stage) {
                        request->stage(static_cast<MB3DRenderStage>(stage), int(index), int(plan.size()), request->stage_user);
                    };
                    mb3d::RenderStageObserverScope observed(request->stage ? &stages : nullptr);
                    mb3d::RenderWorkMeterScope meter(&dispatched);
                    mb3d::RenderCancelScope stop(!cpu && cancel ? &cancel->requested : nullptr);
                    ActiveControl active(cancel, options.control ? options.control : backend.cpu_fallback_control);
                    result = mb3d::RenderWithBackend(header, addon, options, backend);
                }
                seconds = elapsed();
                running.store(false);
                if (watcher.joinable()) watcher.join();
                if (!calibration) break;
                const bool cold = result.backend.execution.pipeline_archive_misses + result.backend.execution.pipeline_archive_hits > 0;
                if (cold && attempt == 0 && result.render.success) {
                    if (options.control) options.control = std::make_shared<mb3d::RenderControl>();
                    if (backend.cpu_fallback_control) backend.cpu_fallback_control = std::make_shared<mb3d::RenderControl>();
                    continue;
                }
                calibrationSeconds = seconds;
                // Keep the work reached before each increase, at the time of the increase.
                curve.clear();
                const double total = work();
                for (size_t i = 1; i < samples.size(); ++i)
                    if (samples[i].work > samples[i - 1].work) curve.push_back({samples[i - 1].work, samples[i].seconds});
                if (total > 0 && seconds > 0) curve.push_back({total, seconds}); else curve.clear();
                break;
            }
            if (!result.render.success) {
                if (result.render.status == mb3d::RenderStatus::Cancelled) return MB3D_RENDER_CANCELLED;
                setError(error, error_size, result.render.error);
                return result.render.status == mb3d::RenderStatus::Unsupported
                    ? MB3D_RENDER_UNSUPPORTED : MB3D_RENDER_FAILED;
            }
            const size_t pixels = size_t(result.render.width) * result.render.height;
            if (result.render.image.size() < pixels * 3) {
                setError(error, error_size, "Renderer returned an incomplete image");
                return MB3D_RENDER_FAILED;
            }
            const bool hasDepth = result.render.silight.size() >= pixels;
            rgba.resize(pixels * 4);
            depth.assign(pixels, 0);
            for (size_t i = 0; i < pixels; ++i) {
                std::memcpy(&rgba[i * 4], &result.render.image[i * 3], 3);
                if (hasDepth) depth[i] = result.render.silight[i].Zpos;
                rgba[i * 4 + 3] = depth[i] > 32767 ? 0 : 255;
            }
            MB3DFrame frame{};
            frame.rgba = rgba.data();
            frame.depth = hasDepth ? depth.data() : nullptr;
            frame.width = result.render.width;
            frame.height = result.render.height;
            frame.pass = static_cast<int>(index);
            frame.pass_count = static_cast<int>(plan.size());
            frame.backend = result.backend.effective == mb3d::RenderBackend::Metal
                ? MB3D_BACKEND_METAL : MB3D_BACKEND_CPU;
            frame.render_seconds = seconds;
            frame.pipelines_compiled = static_cast<int>(result.backend.execution.pipeline_archive_misses);
            frame.pipelines_loaded = static_cast<int>(result.backend.execution.pipeline_archive_hits);
            frame.gpu_seconds = result.backend.execution.gpu_seconds;
            frame.gpu_commands = static_cast<int>(result.backend.execution.gpu_commands);
            const std::string fallback = result.backend.fallback_code.empty() ? result.backend.fallback_reason
                : result.backend.fallback_code + ": " + result.backend.fallback_reason;
            frame.cpu_fallback_reason = result.backend.fallback ? fallback.c_str() : nullptr;
            callback(&frame, user);
        }
        report(1);
        return MB3D_RENDER_SUCCESS;
    } catch (const mb3d::RenderStopped& stopped) {
        if (stopped.status() == mb3d::RenderStatus::Cancelled) return MB3D_RENDER_CANCELLED;
        setError(error, error_size, stopped.what());
    } catch (const std::exception& e) {
        setError(error, error_size, e.what());
    } catch (...) {
        setError(error, error_size, "Unknown render failure");
    }
    return MB3D_RENDER_FAILED;
}

namespace {
// Lattices of a stride in the order of mb3d_lattice_offset: each next one as far
// as possible (on the repeating stride x stride cell) from those before it.
std::vector<int> latticeOrder(int stride) {
    const int count = stride * stride;
    const auto distance = [stride](int a, int b) {
        int dx = std::abs(a % stride - b % stride), dy = std::abs(a / stride - b / stride);
        dx = std::min(dx, stride - dx); dy = std::min(dy, stride - dy);
        return dx * dx + dy * dy;
    };
    std::vector<int> order{0};
    while (int(order.size()) < count) {
        int best = -1, bestDistance = -1;
        for (int candidate = 0; candidate < count; ++candidate) {
            int nearest = 1 << 30;
            for (int chosen : order) nearest = std::min(nearest, distance(candidate, chosen));
            if (nearest > bestDistance) { best = candidate; bestDistance = nearest; }
        }
        order.push_back(best);
    }
    return order;
}
}  // namespace

int mb3d_lattice_offset(int stride, int index, int* x, int* y) {
    if (stride < 2 || stride > 4 || index < 0 || index >= stride * stride || !x || !y) return 0;
    const int cell = latticeOrder(stride)[size_t(index)];
    *x = cell % stride;
    *y = cell / stride;
    return 1;
}

int mb3d_lattice_merge(unsigned char* rgba, unsigned short* depth, unsigned char* known,
                       int width, int height,
                       const unsigned char* part_rgba, const unsigned short* part_depth,
                       int part_width, int part_height, int stride, int x, int y) {
    if (!rgba || !depth || !known || !part_rgba || !part_depth || stride < 2 || stride > 4 ||
        x < 0 || y < 0 || x >= stride || y >= stride || width < stride || height < stride ||
        part_width != (width - x + stride - 1) / stride || part_height != (height - y + stride - 1) / stride) return -1;
    for (int j = 0; j < part_height; ++j) {
        const size_t row = size_t(y + j * stride) * width;
        for (int i = 0; i < part_width; ++i) {
            const size_t to = row + size_t(x + i * stride), from = size_t(j) * part_width + i;
            std::memcpy(rgba + to * 4, part_rgba + from * 4, 4);
            depth[to] = part_depth[from];
            known[to] = 1;
        }
    }
    // Which lattices are in the frame: the first stride x stride pixels tell.
    bool has[16] = {};
    int count = 0;
    for (int cy = 0; cy < stride; ++cy) for (int cx = 0; cx < stride; ++cx)
        count += has[cy * stride + cx] = known[size_t(cy) * width + cx] != 0;
    if (count == stride * stride) return count;
    // A missing pixel is filled from the rendered pixels around it, weighted by
    // a tent as wide as the rendered pixels are apart: with one lattice that is
    // bilinear interpolation between its pixels. The tent is the stride wide,
    // and two pixels once the first four lattices of stride 4 (every second
    // pixel) are in.
    int reach = stride;
    if (stride == 4 && has[0] && has[2] && has[8] && has[10]) reach = 2;
    struct Tap { int dx, dy, weight; };
    std::vector<Tap> taps[16];
    Tap nearest[16] = {};
    for (int cy = 0; cy < stride; ++cy) for (int cx = 0; cx < stride; ++cx) {
        if (has[cy * stride + cx]) continue;
        const auto rendered = [&](int dx, int dy) {
            return has[((cy + dy) % stride + stride) % stride * stride + ((cx + dx) % stride + stride) % stride];
        };
        int best = 1 << 30;
        for (int dy = 1 - reach; dy < reach; ++dy) for (int dx = 1 - reach; dx < reach; ++dx) {
            if (!rendered(dx, dy)) continue;
            taps[cy * stride + cx].push_back({dx, dy, (reach - std::abs(dx)) * (reach - std::abs(dy))});
            if (dx * dx + dy * dy < best) { best = dx * dx + dy * dy; nearest[cy * stride + cx] = {dx, dy, 1}; }
        }
    }
    for (int py = 0; py < height; ++py) for (int px = 0; px < width; ++px) {
        const int cell = (py % stride) * stride + px % stride;
        if (has[cell]) continue;
        const size_t to = size_t(py) * width + px;
        unsigned sum[3] = {0, 0, 0}, total = 0;
        for (const Tap& tap : taps[cell]) {
            const int sx = px + tap.dx, sy = py + tap.dy;
            if (sx < 0 || sy < 0 || sx >= width || sy >= height) continue;
            const unsigned char* source = rgba + (size_t(sy) * width + sx) * 4;
            for (int c = 0; c < 3; ++c) sum[c] += unsigned(source[c]) * unsigned(tap.weight);
            total += unsigned(tap.weight);
        }
        // Depth and alpha are not averaged: they come from the nearest rendered pixel.
        const int nx = std::clamp(px + nearest[cell].dx, 0, width - 1), ny = std::clamp(py + nearest[cell].dy, 0, height - 1);
        size_t from = size_t(ny) * width + nx;
        if (!known[from]) { // clamped at an edge onto a pixel of a missing lattice
            const int ax = px - px % stride, ay = py - py % stride; // a rendered pixel of this cell or the one before
            int bx = -1, by = -1, bestDistance = 1 << 30;
            for (int sy = std::max(0, ay - stride); sy < std::min(height, ay + 2 * stride); ++sy)
                for (int sx = std::max(0, ax - stride); sx < std::min(width, ax + 2 * stride); ++sx)
                    if (known[size_t(sy) * width + sx]) {
                        const int d = (sx - px) * (sx - px) + (sy - py) * (sy - py);
                        if (d < bestDistance) { bestDistance = d; bx = sx; by = sy; }
                    }
            if (bx < 0) continue;
            from = size_t(by) * width + bx;
        }
        if (total) for (int c = 0; c < 3; ++c) rgba[to * 4 + c] = static_cast<unsigned char>((sum[c] + total / 2) / total);
        else std::memcpy(rgba + to * 4, rgba + from * 4, 3);
        rgba[to * 4 + 3] = rgba[from * 4 + 3];
        depth[to] = depth[from];
    }
    return count;
}

MB3DRenderStatus mb3d_scene_admits(const MB3DScene* scene, const MB3DRenderRequest* request,
                                   char* error, size_t error_size) {
    if (!scene || !request || !request->metal_library || !*request->metal_library) {
        setError(error, error_size, "scene, request and metal_library are required");
        return MB3D_RENDER_FAILED;
    }
    const int width = request->width > 0 ? request->width : scene->header.Width;
    const int height = request->height > 0 ? request->height : scene->header.Height;
    try {
        TMandHeader10 header = scene->header;
        THeaderCustomAddon addon = scene->addon;
        applyVolume(*scene, header);
        mb3d::RenderOptions options;
        options.width_override = width;
        options.height_override = height;
        options.thread_count = request->thread_count;
        options.resource_root = scene->resource_root;
        options.precision = mb3d::PrecisionMode::Native;
        options.admission_only = true;
        options.native_metal_outline_formulas = request->quick_pipelines != 0;
        mb3d::BackendOptions backend;
        backend.requested = mb3d::RenderBackend::Metal;
        backend.allow_cpu_fallback = false;
        // The engines are asked in the order a render asks them.
        if (request->lattice_stride > 1 || request->lattice_x || request->lattice_y) {
            options.lattice_stride = request->lattice_stride;
            options.lattice_x = request->lattice_x;
            options.lattice_y = request->lattice_y;
            backend.prefer_general_metal = true;
        }
        backend.metal_library = request->metal_library;
        const auto result = mb3d::RenderWithBackend(header, addon, options, backend);
        if (result.render.success) return MB3D_RENDER_SUCCESS;
        setError(error, error_size, result.render.error);
        return result.render.status == mb3d::RenderStatus::Unsupported ? MB3D_RENDER_UNSUPPORTED : MB3D_RENDER_FAILED;
    } catch (const std::exception& e) {
        setError(error, error_size, e.what());
    } catch (...) {
        setError(error, error_size, "Unknown failure");
    }
    return MB3D_RENDER_FAILED;
}

MB3DRenderStatus mb3d_scene_render(const MB3DScene* scene, const MB3DRenderRequest* request,
                                   MB3DCancel* cancel, MB3DFrameCallback callback, void* user,
                                   char* error, size_t error_size) {
    return renderScene(scene, request, cancel, callback, user, error, error_size,
                       request ? request->metal_library : nullptr, nullptr);
}

MB3DRenderer* mb3d_renderer_create(const char* metal_library, char* error, size_t error_size) {
    auto* renderer = new MB3DRenderer;
    if (metal_library && *metal_library) {
        renderer->library = metal_library;
#ifdef MB3D_ENABLE_METAL_RENDERER
        std::string reason;
        renderer->session = mb3d::native_metal::CreateRenderSession(renderer->library, reason);
        if (!renderer->session) {
            setError(error, error_size, reason);
            delete renderer;
            return nullptr;
        }
#endif
    }
    return renderer;
}

void mb3d_renderer_destroy(MB3DRenderer* renderer) {
    if (!renderer) return;
#ifdef MB3D_ENABLE_METAL_RENDERER
    if (renderer->session) mb3d::native_metal::DestroyRenderSession(renderer->session);
#endif
    delete renderer;
}

MB3DRenderStatus mb3d_renderer_render(MB3DRenderer* renderer, const MB3DScene* scene,
                                      const MB3DRenderRequest* request, MB3DCancel* cancel,
                                      MB3DFrameCallback callback, void* user,
                                      char* error, size_t error_size) {
    if (!renderer) {
        setError(error, error_size, "renderer is required");
        return MB3D_RENDER_FAILED;
    }
    return renderScene(scene, request, cancel, callback, user, error, error_size,
                       renderer->library.empty() ? nullptr : renderer->library.c_str(), renderer->session);
}

double mb3d_scene_reproject_frame(const MB3DScene* from, const MB3DScene* to,
                                  const unsigned char* rgba, const unsigned short* depth,
                                  int width, int height, unsigned char* out_rgba) {
    if (!from || !to || !rgba || !depth || !out_rgba || width < 1 || height < 1) return -1;
    try {
        // Depth was packed for the header the frame was rendered with.
        TMandHeader10 source = from->header, target = to->header;
        applyVolume(*from, source);
        applyVolume(*to, target);
        return mb3d::reprojection::warp(source, target, rgba, depth, width, height, out_rgba);
    } catch (...) { return -1; }
}

} // extern "C"
