#include "core/formulas.h"
#include "core/parity_trace.h"
#include <cstdlib>
#include <sstream>

namespace mb3d {
namespace {

thread_local int g_formulaTracePixelX = -1;
thread_local int g_formulaTracePixelY = -1;
thread_local std::vector<std::string>* g_formulaTraceLines = nullptr;

static bool DebugFormulaTraceEmuEnabled() {
    static const bool value = [] {
        const char* spec = std::getenv("MB3D_DEBUG_FORMULA_TRACE_EMU");
        return spec && *spec;
    }();
    return value;
}

} // namespace

void SetFormulaTraceContext(int pixelX, int pixelY, std::vector<std::string>* lines) {
    g_formulaTracePixelX = pixelX;
    g_formulaTracePixelY = pixelY;
    g_formulaTraceLines = lines;
}

ScopedFormulaTraceContext::ScopedFormulaTraceContext() noexcept
    : previous_x_(g_formulaTracePixelX), previous_y_(g_formulaTracePixelY),
      previous_lines_(g_formulaTraceLines) {
    SetFormulaTraceContext(-1, -1, nullptr);
}

ScopedFormulaTraceContext::~ScopedFormulaTraceContext() {
    SetFormulaTraceContext(previous_x_, previous_y_, previous_lines_);
}

void AppendFormulaTraceEvent(const char* stage, const std::string& payload) {
    if (!HasParityTraceContext() && !g_formulaTraceLines && !DebugFormulaTraceEmuEnabled()) return;
    AppendParityTrace(stage, payload);
    if (!g_formulaTraceLines) return;
    std::ostringstream ss;
    ss << "stage=" << stage
       << " x=" << g_formulaTracePixelX
       << " y=" << g_formulaTracePixelY;
    if (!payload.empty()) ss << " " << payload;
    g_formulaTraceLines->push_back(ss.str());
}

bool FormulaTraceActive() {
    return HasParityTraceContext() || g_formulaTraceLines != nullptr || DebugFormulaTraceEmuEnabled();
}

} // namespace mb3d
