#include "core/parity_trace.h"

#ifdef MB3D_ENABLE_PARITY_TRACE

#include "core/render_context.h"
#include "portable_math.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <utility>

namespace mb3d {

namespace {

struct ParityTraceSession {
    int pixel_x = -1;
    int pixel_y = -1;
    std::vector<std::string>* lines = nullptr;
    ParityTraceStageRangeStateHandle stage_range = std::make_shared<ParityTraceStageRangeState>();
};

thread_local ParityTraceSession g_parityTraceSession;

std::string FormatChecksum64(uint64_t h) {
    std::ostringstream ss;
    ss << "0x" << std::uppercase << std::setfill('0') << std::setw(16) << std::hex << h;
    return ss.str();
}

ParityTraceStageRangeStateHandle EnsureParityTraceStageRangeState() {
    if (!g_parityTraceSession.stage_range) {
        g_parityTraceSession.stage_range = std::make_shared<ParityTraceStageRangeState>();
    }
    return g_parityTraceSession.stage_range;
}

} // namespace

unsigned long long ParityFNV1a64(const char* data, std::size_t length) {
    constexpr uint64_t kOffsetBasis = 0xCBF29CE484222325ULL;
    constexpr uint64_t kPrime       = 0x00000100000001B3ULL;
    uint64_t h = kOffsetBasis;
    for (std::size_t i = 0; i < length; ++i) {
        h ^= static_cast<uint8_t>(data[i]);
        h *= kPrime;   // modulo 2^64 by natural wraparound
    }
    return h;
}

unsigned long long ParityFNV1a64(const std::string& input) {
    return ParityFNV1a64(input.data(), input.size());
}

// === Schema-compliant builder state (thread-local) ===
namespace {

struct ParityBuilderState {
    // Pixel context â€” set by ParityEnterPixel, used by ParityBeginCurrentStage
    int target_x = -1;
    int target_y = -1;
    bool pixel_active = false;

    // Current stage accumulator
    bool in_stage = false;
    std::string cur_stage;
    int cur_x = -1;
    int cur_y = -1;
    std::string cur_payload;      // textual "k1=v1 k2=v2 ..."
    std::string cur_hash_input;   // "k1=bits\nk2=bits\n..."
};

thread_local ParityBuilderState g_builder;

std::string DoubleBitsHex(double v) {
    // Canonicalize NaN payload so Pascal and C++ agree on NaN bits
    if (std::isnan(v)) {
        return "0x7FF8000000000000";
    }
    uint64_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    std::ostringstream ss;
    ss << "0x" << std::uppercase << std::setfill('0')
       << std::setw(16) << std::hex << bits;
    return ss.str();
}

std::string FormatDouble12Value(double v) {
    if (std::isnan(v)) return "nan";
    if (std::isinf(v)) return v < 0 ? "-inf" : "inf";
    // Match Pascal FloatToStrF(v, ffGeneral, 12, 0): 12 significant digits,
    // general (%.12g) format, '.' as decimal separator.
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::setprecision(12) << v;
    return ss.str();
}

std::string FormatDouble17Value(double v) {
    if (std::isnan(v)) return "nan";
    if (std::isinf(v)) return v < 0 ? "-inf" : "inf";
    std::ostringstream ss;
    ss.imbue(std::locale::classic());
    ss << std::setprecision(17) << v;
    return ss.str();
}

bool IsDetailStage(const std::string& stage) {
    const auto& config = CurrentRenderContext().parity_trace_config;
    if (config.level == ParityTraceLevel::DetailAll) return true;
    if (config.level == ParityTraceLevel::Checkpoint) return false;
    for (const auto& s : config.detail_stages) {
        if (s == stage) return true;
    }
    return false;
}

void AppendBuilderField(const std::string& name,
                        const std::string& text_value,
                        const std::string& hash_value) {
    if (!g_builder.in_stage) return;
    if (!g_builder.cur_payload.empty()) g_builder.cur_payload += ' ';
    g_builder.cur_payload += name + '=' + text_value;

    if (!g_builder.cur_hash_input.empty()) g_builder.cur_hash_input += '\n';
    g_builder.cur_hash_input += name + '=' + hash_value;
}

} // namespace

static thread_local bool g_tracePixelSuppressed = false;

struct ScopedParityTraceSession::State {
    ParityTraceSession session;
    ParityBuilderState builder;
    bool pixel_suppressed = false;
};

ScopedParityTraceSession::ScopedParityTraceSession()
    : previous_(std::make_unique<State>()) {
    // Allocate the fresh state before changing TLS so construction failure
    // preserves the caller's session. The swaps themselves do not allocate.
    std::swap(g_parityTraceSession, previous_->session);
    std::swap(g_builder, previous_->builder);
    std::swap(g_tracePixelSuppressed, previous_->pixel_suppressed);
}

ScopedParityTraceSession::~ScopedParityTraceSession() {
    std::swap(g_parityTraceSession, previous_->session);
    std::swap(g_builder, previous_->builder);
    std::swap(g_tracePixelSuppressed, previous_->pixel_suppressed);
}

ScopedParityTracePixel::ScopedParityTracePixel(int x, int y)
    : previous_(g_tracePixelSuppressed) {
    g_tracePixelSuppressed = x != GetParityTracePixelX() || y != GetParityTracePixelY();
    ParityEnterPixel(x, y);
}
ScopedParityTracePixel::~ScopedParityTracePixel() {
    ParityLeavePixel();
    g_tracePixelSuppressed = previous_;
}

void ParityEnterPixel(int x, int y) {
    if (!g_parityTraceSession.lines) return;
    // Reset any stale pixel state (in case previous pixel didn't LeavePixel)
    g_builder.pixel_active = false;
    g_builder.target_x = -1;
    g_builder.target_y = -1;
    if (x == g_parityTraceSession.pixel_x && y == g_parityTraceSession.pixel_y) {
        g_builder.target_x = x;
        g_builder.target_y = y;
        g_builder.pixel_active = true;
    }
}

void ParityLeavePixel() {
    g_builder.pixel_active = false;
    g_builder.target_x = -1;
    g_builder.target_y = -1;
}

bool ParityActive() {
    // Schema mode gates the new builder API; legacy path uses HasParityTraceContext.
    return GetParityTraceSchemaMode() && !g_tracePixelSuppressed
        && g_parityTraceSession.lines != nullptr
        && g_builder.pixel_active;
}

void ParityBeginStage(const char* stage, int x, int y) {
    if (!g_parityTraceSession.lines) return;
    g_builder.in_stage = true;
    g_builder.cur_stage = stage ? stage : "";
    g_builder.cur_x = x;
    g_builder.cur_y = y;
    g_builder.cur_payload.clear();
    g_builder.cur_hash_input.clear();
}

void ParityBeginCurrentStage(const char* stage) {
    ParityBeginStage(stage, g_builder.target_x, g_builder.target_y);
}

void ParityAddDouble12(const char* name, double v) {
    if (!g_builder.in_stage || !name) return;
    std::string bits_hex = DoubleBitsHex(v);
    AppendBuilderField(name, FormatDouble12Value(v), bits_hex);
    AppendBuilderField(std::string(name) + "_bits", bits_hex, bits_hex);
}

void ParityAddDouble17(const char* name, double v) {
    if (!g_builder.in_stage || !name) return;
    std::string bits_hex = DoubleBitsHex(v);
    AppendBuilderField(name, FormatDouble17Value(v), bits_hex);
    AppendBuilderField(std::string(name) + "_bits", bits_hex, bits_hex);
}

void ParityAddInt(const char* name, long long v) {
    if (!g_builder.in_stage || !name) return;
    std::string s = std::to_string(v);
    AppendBuilderField(name, s, s);
}

void ParityAddBoolNeg1(const char* name, bool v) {
    if (!g_builder.in_stage || !name) return;
    const char* s = v ? "-1" : "0";
    AppendBuilderField(name, s, s);
}

void ParityAddString(const char* name, const char* v) {
    if (!g_builder.in_stage || !name || !v) return;
    AppendBuilderField(name, v, v);
}

void ParityEndStage() {
    if (!g_parityTraceSession.lines || !g_builder.in_stage) {
        g_builder.in_stage = false;
        return;
    }
    uint64_t checksum = ParityFNV1a64(g_builder.cur_hash_input);
    std::string checksum_str = FormatChecksum64(checksum);
    bool detail = IsDetailStage(g_builder.cur_stage);

    std::string line = std::string("stage=") + g_builder.cur_stage
                     + " x=" + std::to_string(g_builder.cur_x)
                     + " y=" + std::to_string(g_builder.cur_y);
    if (detail && !g_builder.cur_payload.empty()) {
        line += ' ';
        line += g_builder.cur_payload;
    }
    line += " checksum=";
    line += checksum_str;

    g_parityTraceSession.lines->push_back(std::move(line));

    g_builder.in_stage = false;
    g_builder.cur_payload.clear();
    g_builder.cur_hash_input.clear();
}

std::string ParityCanaryLine() {
    // Hash input must match Pascal ParityTrace.EmitCanary byte-for-byte.
    // Computed math probes also detect runtime/FPU drift; a canary mismatch
    // is not necessarily a serialization error. See docs/PARITY_TRACE.md.
    const double testAngle = 1.23456789;
    const double testLogArg = 2.718281828;
    const double sin_val = pm_sin(testAngle);
    const double cos_val = pm_cos(testAngle);
    const double log_val = pm_log(testLogArg);
    uint64_t sinBits, cosBits, logBits;
    std::memcpy(&sinBits, &sin_val, sizeof(sinBits));
    std::memcpy(&cosBits, &cos_val, sizeof(cosBits));
    std::memcpy(&logBits, &log_val, sizeof(logBits));

    std::ostringstream hi;
    hi << "pi=0x400921FB54442D18\n"
       << "pi_bits=0x400921FB54442D18\n"
       << "one_third=0x3FD5555555555555\n"
       << "one_third_bits=0x3FD5555555555555\n"
       << "tenth=0x3FB999999999999A\n"
       << "tenth_bits=0x3FB999999999999A\n"
       << "neg_zero=0x8000000000000000\n"
       << "neg_zero_bits=0x8000000000000000\n"
       << "nan_sample=0x7FF8000000000000\n"
       << "nan_sample_bits=0x7FF8000000000000\n"
       << "neg_inf=0xFFF0000000000000\n"
       << "neg_inf_bits=0xFFF0000000000000\n"
       << "bool_true=-1\n"
       << "bool_false=0\n"
       << "pm_sin_test=0x" << std::uppercase << std::setfill('0')
                           << std::setw(16) << std::hex << sinBits << "\n"
       << "pm_sin_test_bits=0x" << std::setfill('0') << std::setw(16)
                                << std::hex << sinBits << "\n"
       << "pm_cos_test=0x" << std::setfill('0') << std::setw(16)
                           << std::hex << cosBits << "\n"
       << "pm_cos_test_bits=0x" << std::setfill('0') << std::setw(16)
                                << std::hex << cosBits << "\n"
       << "pm_log_test=0x" << std::setfill('0') << std::setw(16)
                           << std::hex << logBits << "\n"
       << "pm_log_test_bits=0x" << std::setfill('0') << std::setw(16)
                                << std::hex << logBits;
    std::string s = hi.str();
    uint64_t h = ParityFNV1a64(s.data(), s.size());
    return std::string("stage=_canary x=-1 y=-1 checksum=") + FormatChecksum64(h);
}

void SetParityTraceContext(int pixelX, int pixelY, std::vector<std::string>* lines) {
    g_parityTraceSession.pixel_x = pixelX;
    g_parityTraceSession.pixel_y = pixelY;
    g_parityTraceSession.lines = lines;
}

void ClearParityTraceContext() {
    g_parityTraceSession.pixel_x = -1;
    g_parityTraceSession.pixel_y = -1;
    g_parityTraceSession.lines = nullptr;
}

bool HasParityTraceContext() {
    // In schema mode, legacy AppendParityTrace emitters go dormant â€”
    // only the Parity* builder API emits.
    if (GetParityTraceSchemaMode() || g_tracePixelSuppressed) return false;
    return g_parityTraceSession.lines != nullptr;
}

void SetParityTraceSchemaMode(bool on) {
    CurrentRenderContext().parity_trace_config.schema_mode = on;
}

bool GetParityTraceSchemaMode() {
    return CurrentRenderContext().parity_trace_config.schema_mode;
}

void SetParityTraceLevel(ParityTraceLevel level) {
    CurrentRenderContext().parity_trace_config.level = level;
}

void SetParityTraceDetailStages(const std::string& csv) {
    auto& detailStages = CurrentRenderContext().parity_trace_config.detail_stages;
    detailStages.clear();
    std::string cur;
    for (char c : csv) {
        if (c == ',' || c == ';') {
            if (!cur.empty()) detailStages.push_back(cur);
            cur.clear();
        } else if (c != ' ' && c != '\t') {
            cur += c;
        }
    }
    if (!cur.empty()) detailStages.push_back(cur);
}


int GetParityTracePixelX() {
    return g_parityTraceSession.pixel_x;
}

int GetParityTracePixelY() {
    return g_parityTraceSession.pixel_y;
}

void ConfigureParityTraceStageRange(const std::string& startStage,
                                    const std::string& endStage) {
    auto state = EnsureParityTraceStageRangeState();
    state->start_stage = startStage;
    state->end_stage = endStage;
    state->pending_stage.clear();
    state->start_reached = startStage.empty();
    state->end_reached = false;
}

void ResetParityTraceStageRange() {
    g_parityTraceSession.stage_range = std::make_shared<ParityTraceStageRangeState>();
}

ParityTraceStageRangeStateHandle CaptureParityTraceStageRangeState() {
    return EnsureParityTraceStageRangeState();
}

void RestoreParityTraceStageRangeState(const ParityTraceStageRangeStateHandle& state) {
    g_parityTraceSession.stage_range = state ? state : std::make_shared<ParityTraceStageRangeState>();
}

bool ShouldLogParityStage(const char* stage) {
    if (!stage) return false;
    auto state = EnsureParityTraceStageRangeState();
    if (state->end_reached) return false;

    bool startReached = state->start_reached;
    if (!startReached) {
        if (state->start_stage == stage) {
            state->start_reached = true;
            startReached = true;
        } else {
            return false;
        }
    }

    if (startReached && !state->end_stage.empty() && state->end_stage == stage) {
        state->end_reached = true;
    }
    state->pending_stage = stage;
    return true;
}

void AppendParityTrace(const char* stage, const std::string& payload, int stepIndex) {
    if (!g_parityTraceSession.lines) return;
    // Schema mode suppresses legacy full-payload emission entirely â€”
    // only the Parity* builder API writes to the trace file.
    if (GetParityTraceSchemaMode()) return;
    auto state = EnsureParityTraceStageRangeState();
    if (!stage) return;
    if (state->pending_stage == stage) {
        state->pending_stage.clear();
    } else if (!ShouldLogParityStage(stage)) {
        return;
    } else {
        state->pending_stage.clear();
    }
    std::ostringstream ss;
    ss << "stage=" << stage;
    ss << " x=" << g_parityTraceSession.pixel_x
       << " y=" << g_parityTraceSession.pixel_y;
    (void)stepIndex;
    if (!payload.empty()) {
        ss << " " << payload;
    }
    g_parityTraceSession.lines->push_back(ss.str());
}

const char* ParityPointerState(const void* p) {
    return p ? "set" : "null";
}

} // namespace mb3d

#endif
