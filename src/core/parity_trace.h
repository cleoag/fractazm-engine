#ifndef MB3D_CORE_PARITY_TRACE_H
#define MB3D_CORE_PARITY_TRACE_H

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace mb3d {

struct ParityTraceStageRangeState {
    std::string start_stage;
    std::string end_stage;
    std::string pending_stage;
    bool start_reached = true;
    bool end_reached = false;
};

using ParityTraceStageRangeStateHandle = std::shared_ptr<ParityTraceStageRangeState>;

enum class ParityTraceLevel { Checkpoint, DetailSelected, DetailAll };

// Immutable while a render's workers run. Each render context owns its own
// configuration so concurrent jobs can select independent trace formats.
struct ParityTraceConfig {
    bool schema_mode = false;
    ParityTraceLevel level = ParityTraceLevel::Checkpoint;
    std::vector<std::string> detail_stages;
};

#ifdef MB3D_ENABLE_PARITY_TRACE

// FNV-1a-64 hash â€” identical implementation between Pascal ParityTrace.pas and this file.
// Used to produce schema-compliant per-stage checksums that Pascal and C++ can compare
// byte-for-byte. See docs/PARITY_TRACE.md.
unsigned long long ParityFNV1a64(const char* data, std::size_t length);
unsigned long long ParityFNV1a64(const std::string& input);

// Returns a complete trace line ready to be written to the trace file as the first stage.
// Fixed values and computed math probes must match Pascal's EmitCanary byte-for-byte.
// If checksums disagree, inspect serialization, math functions and floating-point state
// before trusting real-data stages. See docs/PARITY_TRACE.md.
std::string ParityCanaryLine();

// Schema-compliant builder API â€” mirrors Pascal ParityTrace.pas.
// Use for Level-0 (checkpoint) emission with FNV1a64 checksum matching Pascal byte-for-byte.
// Usage:
//   if (ParityActive()) {
//       ParityBeginCurrentStage("ray_ready");
//       ParityAddDouble12("mZZ", mZZ);
//       ParityAddDouble12("CAFX", cafx);
//       ParityEndStage();
//   }

// Schema mode: when enabled, the legacy AppendParityTrace emitters become dormant
// and only the Parity* builder API emits. Keeps the 2 formats from interleaving
// in the same output file.
void SetParityTraceSchemaMode(bool on);
bool GetParityTraceSchemaMode();

// Emission verbosity â€” mirrors Pascal's --parity-trace-level flag.
// checkpoint (default): only checksum=... per stage
// detail_selected: full payload + checksum for stages listed in detail_stages
// detail_all: full payload + checksum for every stage
void SetParityTraceLevel(ParityTraceLevel level);
void SetParityTraceDetailStages(const std::string& csv);

void ParityEnterPixel(int x, int y);
void ParityLeavePixel();
bool ParityActive();
void ParityBeginCurrentStage(const char* stage);
void ParityBeginStage(const char* stage, int x, int y);
void ParityAddDouble12(const char* name, double v);
void ParityAddDouble17(const char* name, double v);
void ParityAddInt(const char* name, long long v);
void ParityAddBoolNeg1(const char* name, bool v);
void ParityAddString(const char* name, const char* v);
void ParityEndStage();

class ScopedParityTracePixel {
    bool previous_;
public:
    ScopedParityTracePixel(int x, int y);
    ~ScopedParityTracePixel();
};

// Isolate a public render or worker task from its caller's TLS output pointer,
// stage-range cursor, pixel scope and partially constructed schema stage.
// Explicit stage-range continuation can be restored after creating the scope.
class ScopedParityTraceSession {
public:
    ScopedParityTraceSession();
    ~ScopedParityTraceSession();
    ScopedParityTraceSession(const ScopedParityTraceSession&) = delete;
    ScopedParityTraceSession& operator=(const ScopedParityTraceSession&) = delete;
private:
    struct State;
    std::unique_ptr<State> previous_;
};

void SetParityTraceContext(int pixelX, int pixelY, std::vector<std::string>* lines);
void ClearParityTraceContext();
bool HasParityTraceContext();
int GetParityTracePixelX();
int GetParityTracePixelY();
void ConfigureParityTraceStageRange(const std::string& startStage,
                                    const std::string& endStage);
void ResetParityTraceStageRange();
ParityTraceStageRangeStateHandle CaptureParityTraceStageRangeState();
void RestoreParityTraceStageRangeState(const ParityTraceStageRangeStateHandle& state);
bool ShouldLogParityStage(const char* stage);
void AppendParityTrace(const char* stage, const std::string& payload = std::string(),
                       int stepIndex = -1);
const char* ParityCalcDEName(void* fn);
const char* ParityMandFunctionName(void* fn);
const char* ParityMandFunctionDEName(void* fn);
const char* ParityHybridName(void* fn);
const char* ParityPointerState(const void* p);

#else

inline unsigned long long ParityFNV1a64(const char*, std::size_t) { return 0; }
inline unsigned long long ParityFNV1a64(const std::string&) { return 0; }
inline std::string ParityCanaryLine() { return {}; }

inline void SetParityTraceSchemaMode(bool) {}
inline bool GetParityTraceSchemaMode() { return false; }
inline void SetParityTraceLevel(ParityTraceLevel) {}
inline void SetParityTraceDetailStages(const std::string&) {}
inline void ParityEnterPixel(int, int) {}
inline void ParityLeavePixel() {}
inline bool ParityActive() { return false; }
inline void ParityBeginCurrentStage(const char*) {}
inline void ParityBeginStage(const char*, int, int) {}
inline void ParityAddDouble12(const char*, double) {}
inline void ParityAddDouble17(const char*, double) {}
inline void ParityAddInt(const char*, long long) {}
inline void ParityAddBoolNeg1(const char*, bool) {}
inline void ParityAddString(const char*, const char*) {}
inline void ParityEndStage() {}

class ScopedParityTracePixel { public: ScopedParityTracePixel(int, int) {} };
class ScopedParityTraceSession {
public:
    ScopedParityTraceSession() = default;
    ScopedParityTraceSession(const ScopedParityTraceSession&) = delete;
    ScopedParityTraceSession& operator=(const ScopedParityTraceSession&) = delete;
};
inline void SetParityTraceContext(int, int, std::vector<std::string>*) {}
inline void ClearParityTraceContext() {}
inline bool HasParityTraceContext() { return false; }
inline int GetParityTracePixelX() { return -1; }
inline int GetParityTracePixelY() { return -1; }
inline void ConfigureParityTraceStageRange(const std::string&, const std::string&) {}
inline void ResetParityTraceStageRange() {}
inline ParityTraceStageRangeStateHandle CaptureParityTraceStageRangeState() { return {}; }
inline void RestoreParityTraceStageRangeState(const ParityTraceStageRangeStateHandle&) {}
inline bool ShouldLogParityStage(const char*) { return false; }
inline void AppendParityTrace(const char*, const std::string& = std::string(), int = -1) {}
inline const char* ParityCalcDEName(void*) { return "trace_disabled"; }
inline const char* ParityMandFunctionName(void*) { return "trace_disabled"; }
inline const char* ParityMandFunctionDEName(void*) { return "trace_disabled"; }
inline const char* ParityHybridName(void*) { return "trace_disabled"; }
inline const char* ParityPointerState(const void*) { return "trace_disabled"; }

#endif

} // namespace mb3d

#endif // MB3D_CORE_PARITY_TRACE_H
