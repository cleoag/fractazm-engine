#pragma once
// Portable policy selection only. A host policy never replaces a device ABI,
// formula identity, finite-value or work-limit guard.
#include "pixel_state_contract.h"
#include "pixel_independence.h"
#include "native_ao_de_abi.h"
#include "postprocess_plan.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <sstream>

namespace mb3d::native_metal {
static_assert(alignof(mb3d_native::NativeFormulaSlot)==4);
static_assert(offsetof(mb3d_native::NativeFormulaSlot,constants)+32*sizeof(float)==sizeof(mb3d_native::NativeFormulaSlot));
static_assert(alignof(mb3d_native::NativeFormulaProgram)==4);
static_assert(offsetof(mb3d_native::NativeFormulaProgram,slots)==224);
static_assert(offsetof(mb3d_native::NativeFormulaProgram,slots)+6*sizeof(mb3d_native::NativeFormulaSlot)==sizeof(mb3d_native::NativeFormulaProgram));
static_assert(offsetof(mb3d_native_ao::DEParams,row_counts)==192);
static_assert(offsetof(mb3d_native_ao::DEParams,row_counts)+4*sizeof(uint32_t)==sizeof(mb3d_native_ao::DEParams));
static_assert(offsetof(mb3d_native_ao::Params,directions)==80 && sizeof(mb3d_native_ao::Params)==80+64*4);
static_assert(offsetof(mb3d_native_ao::Level,reserved2)==72 && sizeof(mb3d_native_ao::Level)==80);
static_assert(offsetof(mb3d_dof::Params,reserved_float)==60 && sizeof(mb3d_dof::Params)==64);
static_assert(offsetof(mb3d_zn::Params,reserved)==68 && sizeof(mb3d_zn::Params)==80);
enum class ExecutionEntry : uint32_t { Primary=1, SoftShadow, HardShadow, Reflection, DistanceAO };
enum class StateDomain : uint32_t { GeneralFormula47=1, RestrictedPrivateOrbit, ImageOnly };
enum class LogicalOwner : uint32_t { Pixel=1, InterleavedRows, ContiguousRows, WholeRow, OrderedPass, SerialTask, CollectiveTile };
enum class Initialization : uint32_t { NativeInitialState=1, ZeroThenPartialActive, DEInitialState, ImagePass, SeedPerLevel };
enum class SelectionProof : uint32_t { ConservativeLegacyOrder=1, LegacyQualifiedPixels, ClosedPrimaryEntry, ClosedSoftShadowEntry, QualifiedDepthEntry, LegacyQualified4DReflection, IndependentPrimaryState, PixelReflectionState };
enum class FaultCursor : uint32_t { NotApplicable=1, AdvanceAttemptedPixel, AdvanceCompletedPixelOnly };
enum class FaultRecords : uint32_t { AttemptRecord=1, EntrySpecificDiagnostic };
enum class FaultState : uint32_t { OwnerLocalCarry=1, PreserveMutatedContinuation };
enum class CommitScope : uint32_t { AdmittedInBoundsAttempt=1 };
enum class OwnerFaultFlow : uint32_t { SingleAttempt=1, StopContinuation, ContinueWholeRow };
enum class FaultOutput : uint32_t { DiagnosticSurface=1, PreserveIncomingRGB, EntrySpecificUnpublished };
enum class SkipTransition : uint32_t { PrimaryBranches=1, HardShadowRowCache, SoftShadowNoQuery, ReflectionPartialInitialization, DEAOBackgroundClear };
enum AuxiliaryState : uint64_t {
    RayControls=1ull<<0, InsideActivity=1ull<<1, NormalRecords=1ull<<2,
    HitOrbitPayload=1ull<<3, ShadowRowShortcut=1ull<<4, OrderedLights=1ull<<5,
    SoftVisibility=1ull<<6, BounceSubtree=1ull<<7, Radiance=1ull<<8,
    RngContinuation=1ull<<9
};
struct EntryExecutionContract {
    uint32_t version=1;
    ExecutionEntry entry=ExecutionEntry::Primary;
    StateDomain domain=StateDomain::GeneralFormula47;
    LogicalOwner owner=LogicalOwner::Pixel;
    Initialization initialization=Initialization::NativeInitialState;
    SelectionProof proof=SelectionProof::ConservativeLegacyOrder;
    FaultCursor fault_cursor=FaultCursor::NotApplicable;
    FaultRecords fault_records=FaultRecords::AttemptRecord;
    // Commit/cursor rules describe an admitted in-bounds attempt only. Invalid
    // descriptors, zero-count launches and tail lanes retain kernel-specific
    // diagnostics. NotApplicable refers to an external continuation cursor.
    CommitScope commit_scope=CommitScope::AdmittedInBoundsAttempt;
    FaultRecords nonattempt_records=FaultRecords::EntrySpecificDiagnostic;
    FaultState fault_state=FaultState::OwnerLocalCarry;
    OwnerFaultFlow fault_flow=OwnerFaultFlow::SingleAttempt;
    FaultOutput fault_output=FaultOutput::DiagnosticSurface;
    SkipTransition skip=SkipTransition::PrimaryBranches;
    uint64_t auxiliary=0;
    uint32_t tasks=0,rows_per_task=0,continuation_pixels=0,dispatch_batch=0,group_width=0;
    // A configured owner is retained for dormant stages, but tasks==0 means
    // no execution. group_width==0 retains the pipeline's SIMD-width choice.
    bool enabled=false;
    bool operator==(const EntryExecutionContract&) const = default;
};
inline std::string DescribeEntry(const EntryExecutionContract& e) {
    std::ostringstream s;
    s<<"entry="<<uint32_t(e.entry)<<",enabled="<<e.enabled<<",domain="<<uint32_t(e.domain)
     <<",owner="<<uint32_t(e.owner)<<",init="<<uint32_t(e.initialization)<<",proof="<<uint32_t(e.proof)
     <<",scope="<<uint32_t(e.commit_scope)<<",cursor="<<uint32_t(e.fault_cursor)
     <<",record="<<uint32_t(e.fault_records)<<",nonattempt="<<uint32_t(e.nonattempt_records)
     <<",state="<<uint32_t(e.fault_state)<<",flow="<<uint32_t(e.fault_flow)
     <<",output="<<uint32_t(e.fault_output)<<",skip="<<uint32_t(e.skip)<<",aux="<<e.auxiliary
     <<",tasks="<<e.tasks<<",rows="<<e.rows_per_task<<",quota="<<e.continuation_pixels
     <<",batch="<<e.dispatch_batch<<",group="<<e.group_width;
    return s.str();
}
template<class T> struct DescriptorBits {
    // These upload layouts contain only named 4-byte fields/arrays, with
    // explicit reserved words covering alignment. Do not admit arbitrary C++
    // objects: padding is not a stable binding identity.
    static_assert(std::is_same_v<T,mb3d_native::NativeFormulaProgram> ||
                  std::is_same_v<T,mb3d_native_ao::DEParams> ||
                  std::is_same_v<T,mb3d_native_ao::Params> ||
                  std::is_same_v<T,mb3d_native_ao::Level> ||
                  std::is_same_v<T,mb3d_dof::Params> ||
                  std::is_same_v<T,mb3d_zn::Params>);
    static_assert(std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>);
    std::array<std::byte,sizeof(T)> bytes{};
    explicit DescriptorBits(const T& value){std::memcpy(bytes.data(),&value,sizeof(T));}
    bool matches(const T& value) const {return std::memcmp(bytes.data(),&value,sizeof(T))==0;}
};
struct GeneralExecutionRequest {
    uint32_t width=0,height=0,workers=0;
    int32_t initial_inside=0;
    uint32_t in_and_out=0,light_mask=0;
    bool legacy_formula_persistent=false,shadows=false,soft_shadows=false,reflections=false,maps_empty=true;
    // Admit a conditional (dIFS) state contract; the renderer re-renders in
    // CPU order when any record counts a carried-state read.
    bool conditional_state=false;
    // Position lights march only in the multi-light hard-shadow entry.
    uint32_t position_mask=0;
    bool operator==(const GeneralExecutionRequest&) const = default;
};
// Pixels per primary or reflection pixel command. A command ends with its
// slowest pixels, so the GPU idles while they finish; 16384-pixel commands
// left reflections about a quarter busy. Each pixel owns a fresh lane, so the
// batch changes only the physical schedule.
constexpr uint32_t kPixelDispatchBatch=262144;
class GeneralExecutionPolicy {
    GeneralExecutionRequest request_;
    DescriptorBits<mb3d_native::NativeFormulaProgram> program_,second_;
    EntryExecutionContract primary_,shadow_,reflection_;
    mb3d_native::PixelStateContract primary_state_,soft_state_;
    uint32_t shadow_rows_=0,shadow_tasks_=0;
    bool multi_shadow_=false,conditional_state_=false;
public:
    // second: the DE combination's second part (p.formula_type), else null.
    GeneralExecutionPolicy(const mb3d_native::NativeFormulaProgram& p,const GeneralExecutionRequest& r,
        const mb3d_native::NativeFormulaProgram* second=nullptr)
        :request_(r),program_(p),second_(second?*second:mb3d_native::NativeFormulaProgram{}),
         primary_state_(mb3d_native::AnalyzePrimaryPixelState(p,second)),
         soft_state_(mb3d_native::AnalyzeSoftShadowPixelState(p,r.initial_inside,r.in_and_out)) {
        if(!r.width||r.width>4096||!r.height||r.height>4096||!r.workers||r.workers>r.height)
            throw std::invalid_argument("Invalid Native execution policy dimensions/workers");
        const uint32_t n=r.width*r.height;
        // An independent primary state contract replaces the legacy persistent
        // order. Exact captured descriptors keep their closed proof, and scenes
        // the host already runs as pixels keep their legacy qualification.
        const bool independentState=primary_state_.independent&&(!primary_state_.conditional||r.conditional_state);
        const bool statePixels=r.legacy_formula_persistent&&!primary_state_.production_chain&&independentState;
        conditional_state_=r.legacy_formula_persistent&&independentState&&primary_state_.conditional;
        const bool primaryPersistent=r.legacy_formula_persistent&&!primary_state_.production_chain&&!statePixels;
        const bool softPixels=r.shadows&&r.soft_shadows&&soft_state_.production_chain&&r.maps_empty;
        // The same contract frees shadow rows from the persistent task order:
        // each row starts a fresh lane, keeping only its ordered shortcut.
        const bool shadowPersistent=r.legacy_formula_persistent&&!softPixels&&!statePixels;
        // Soft-shadow rows carry no shortcut, only the lane, and each pixel's
        // start query sets inside from the policy. Wherever the rows already
        // start fresh lanes (state contract or legacy pixel qualification),
        // each pixel may too: host proof 5 or 6 for the soft pixel entry.
        const bool softHostPixels=r.shadows&&r.soft_shadows&&!softPixels&&!shadowPersistent;
        const bool softAnyPixels=softPixels||softHostPixels;
        if(r.position_mask&~r.light_mask)
            throw std::invalid_argument("Invalid Native position-light shadow request");
        multi_shadow_=r.shadows&&!r.soft_shadows&&(r.light_mask!=1||r.position_mask);
        shadow_rows_=r.workers==1?r.height:std::max(1u,r.height/(r.workers*4));
        shadow_tasks_=(r.height+shadow_rows_-1)/shadow_rows_;
        primary_.enabled=true;primary_.entry=ExecutionEntry::Primary;
        primary_.owner=primaryPersistent?LogicalOwner::InterleavedRows:LogicalOwner::Pixel;
        primary_.proof=primary_state_.production_chain?SelectionProof::ClosedPrimaryEntry:
            statePixels?SelectionProof::IndependentPrimaryState:
            primaryPersistent?SelectionProof::ConservativeLegacyOrder:SelectionProof::LegacyQualifiedPixels;
        primary_.fault_cursor=primaryPersistent?FaultCursor::AdvanceAttemptedPixel:FaultCursor::NotApplicable;
        primary_.fault_state=primaryPersistent?FaultState::PreserveMutatedContinuation:FaultState::OwnerLocalCarry;
        primary_.fault_flow=primaryPersistent?OwnerFaultFlow::StopContinuation:OwnerFaultFlow::SingleAttempt;
        primary_.auxiliary=RayControls|InsideActivity|NormalRecords|HitOrbitPayload;
        primary_.tasks=primaryPersistent?r.workers:n;primary_.continuation_pixels=primaryPersistent?32:0;
        primary_.dispatch_batch=primaryPersistent?r.workers:kPixelDispatchBatch;primary_.group_width=primaryPersistent?1:32;
        shadow_.enabled=r.shadows;shadow_.entry=r.soft_shadows?ExecutionEntry::SoftShadow:ExecutionEntry::HardShadow;
        shadow_.owner=softAnyPixels?LogicalOwner::Pixel:shadowPersistent?LogicalOwner::ContiguousRows:LogicalOwner::WholeRow;
        shadow_.proof=softPixels?SelectionProof::ClosedSoftShadowEntry:
            statePixels&&r.shadows?SelectionProof::IndependentPrimaryState:
            softHostPixels?SelectionProof::LegacyQualifiedPixels:SelectionProof::ConservativeLegacyOrder;
        shadow_.skip=r.soft_shadows?SkipTransition::SoftShadowNoQuery:SkipTransition::HardShadowRowCache;
        shadow_.auxiliary=RayControls|(r.soft_shadows?uint64_t(SoftVisibility):uint64_t(ShadowRowShortcut|OrderedLights));
        shadow_.fault_cursor=shadowPersistent?FaultCursor::AdvanceAttemptedPixel:FaultCursor::NotApplicable;
        shadow_.fault_state=shadowPersistent?FaultState::PreserveMutatedContinuation:FaultState::OwnerLocalCarry;
        shadow_.fault_flow=shadowPersistent?OwnerFaultFlow::StopContinuation:
            softAnyPixels?OwnerFaultFlow::SingleAttempt:OwnerFaultFlow::ContinueWholeRow;
        shadow_.tasks=!r.shadows?0:softAnyPixels?n:shadowPersistent?shadow_tasks_:r.height;
        shadow_.rows_per_task=shadowPersistent?shadow_rows_:softAnyPixels?0:1;
        shadow_.continuation_pixels=shadowPersistent?32:0;
        const bool wideRows=multi_shadow_||r.soft_shadows||statePixels||mb3d_native::IndependentDepthScenePixels(p);
        shadow_.dispatch_batch=softAnyPixels?kPixelDispatchBatch:shadowPersistent?shadow_tasks_:wideRows?512:64;
        shadow_.group_width=softAnyPixels?32:shadowPersistent?1:wideRows?8:1;
        // Reflection DE, normal and refinement queries enter the primary orbit
        // entry, and each pixel's partial initialization sets every caller
        // control. Where the primary pixel state is legacy-qualified or proven
        // independent, a fresh lane per pixel replaces the ordered tasks.
        const bool reflection4D=!r.legacy_formula_persistent&&
            (p.driver==mb3d_native::Hybrid4D||p.driver==mb3d_native::Hybrid4DAnalytic);
        const bool reflectionPixels=!r.legacy_formula_persistent||independentState;
        reflection_.enabled=r.reflections;reflection_.entry=ExecutionEntry::Reflection;
        reflection_.owner=reflectionPixels?LogicalOwner::Pixel:LogicalOwner::ContiguousRows;
        reflection_.initialization=Initialization::ZeroThenPartialActive;
        reflection_.proof=reflection4D?SelectionProof::LegacyQualified4DReflection:
            reflectionPixels?SelectionProof::PixelReflectionState:SelectionProof::ConservativeLegacyOrder;
        reflection_.fault_cursor=reflectionPixels?FaultCursor::NotApplicable:FaultCursor::AdvanceAttemptedPixel;
        reflection_.fault_state=reflectionPixels?FaultState::OwnerLocalCarry:FaultState::PreserveMutatedContinuation;
        reflection_.fault_flow=reflectionPixels?OwnerFaultFlow::SingleAttempt:OwnerFaultFlow::StopContinuation;
        reflection_.fault_output=FaultOutput::PreserveIncomingRGB;
        reflection_.skip=SkipTransition::ReflectionPartialInitialization;
        reflection_.auxiliary=BounceSubtree|RayControls|InsideActivity|NormalRecords|HitOrbitPayload|OrderedLights|Radiance;
        reflection_.tasks=!r.reflections?0:reflectionPixels?n:shadow_tasks_;
        reflection_.rows_per_task=reflectionPixels?0:shadow_rows_;
        reflection_.continuation_pixels=reflectionPixels?0:32;
        reflection_.dispatch_batch=reflectionPixels?kPixelDispatchBatch:shadow_tasks_;
        reflection_.group_width=reflectionPixels?32:1;
    }
    void validate(const mb3d_native::NativeFormulaProgram& p,const GeneralExecutionRequest& r,
        const mb3d_native::NativeFormulaProgram* second=nullptr) const {
        if(!(r==request_)||!program_.matches(p)||!second_.matches(second?*second:mb3d_native::NativeFormulaProgram{}))
            throw std::invalid_argument("Native execution policy input mismatch");
    }
    const EntryExecutionContract& primary() const{return primary_;}
    const EntryExecutionContract& shadow() const{return shadow_;}
    const EntryExecutionContract& reflection() const{return reflection_;}
    const auto& primary_state() const{return primary_state_;}
    const auto& soft_state() const{return soft_state_;}
    uint32_t shadow_rows() const{return shadow_rows_;}
    uint32_t shadow_tasks() const{return shadow_tasks_;}
    bool multi_shadow() const{return multi_shadow_;}
    bool primary_persistent() const{return primary_.owner==LogicalOwner::InterleavedRows;}
    bool independent_primary_state() const{return primary_.proof==SelectionProof::IndependentPrimaryState;}
    bool shadow_persistent() const{return shadow_.owner==LogicalOwner::ContiguousRows;}
    bool parallel_soft_shadows() const{return shadow_.enabled&&shadow_.entry==ExecutionEntry::SoftShadow&&shadow_.owner==LogicalOwner::Pixel;}
    // Soft pixel-entry proof word: 1 the device-checked chain, 5 the state
    // contract, 6 the legacy pixel qualification (host proofs).
    uint32_t soft_shadow_pixel_proof() const{
        if(!parallel_soft_shadows())return 0;
        return shadow_.proof==SelectionProof::ClosedSoftShadowEntry?1u:
            shadow_.proof==SelectionProof::IndependentPrimaryState?5u:6u;}
    bool parallel_reflections() const{return reflection_.owner==LogicalOwner::Pixel;}
    // WholeRow hard shadows: rows carry only the shortcut caches, which a
    // per-row scan rebuilds from fresh-entry pixel traces.
    bool speculative_shadows() const{
        return shadow_.enabled&&shadow_.entry==ExecutionEntry::HardShadow&&
            shadow_.owner==LogicalOwner::WholeRow;
    }
    // Pixel ownership rests on a conditional contract: valid only for a frame
    // whose geometry records count no difs_carried_reads.
    bool conditional_state() const{return conditional_state_;}
};
class DistanceAOExecutionPolicy {
    DescriptorBits<mb3d_native::NativeFormulaProgram> program_,second_;
    DescriptorBits<mb3d_native_ao::DEParams> parameters_;
    EntryExecutionContract entry_;
    bool conditional_=false,seeded_=false;
public:
    // second: the DE combination's second part (program.formula_type), else null.
    // ordered_carried_state: the ordered tasks, which a conditional contract
    // falls back to and the order controls compare against.
    DistanceAOExecutionPolicy(const mb3d_native::NativeFormulaProgram& program,const mb3d_native_ao::DEParams& p,
        const mb3d_native::NativeFormulaProgram* second=nullptr,bool ordered_carried_state=false)
        :program_(program),second_(second?*second:mb3d_native::NativeFormulaProgram{}),parameters_(p) {
        // This is only the bounded scheduling domain. Runtime must still call
        // valid_de before allocation/dispatch; byte binding is not admission.
        if(!p.width||p.width>4096||!p.height||p.height>4096)
            throw std::invalid_argument("Invalid Native DE AO policy dimensions");
        // Every DE AO query owns its caller controls (point, inside, limits,
        // smoothing) like a primary distance query, so the independent primary
        // state contract and the closed primary chains (every consumed lane
        // reset by each orbit entry) also free AO pixels. A random first step
        // keeps its per-task sequence: the host plans each pixel's entering
        // seed from the rays of the active pixels before it (seeded()). A
        // conditional contract is valid only for a pass whose records count no
        // difs_carried_reads; otherwise the host reruns the ordered tasks.
        const bool closed=mb3d_native::IndependentDepthScenePixels(program);
        const bool chain=!closed&&!second&&mb3d_native::IndependentPrimaryPixels(program);
        const auto state=mb3d_native::AnalyzePrimaryPixelState(program,second);
        const bool contract=!closed&&!chain&&state.independent&&!state.conditional;
        conditional_=!closed&&!chain&&!ordered_carried_state&&state.independent&&state.conditional;
        const bool independent=!ordered_carried_state&&(closed||chain||contract||conditional_);
        seeded_=independent&&p.first_step_random;
        entry_.enabled=true;entry_.entry=ExecutionEntry::DistanceAO;
        entry_.owner=independent?LogicalOwner::Pixel:LogicalOwner::ContiguousRows;
        entry_.initialization=Initialization::DEInitialState;
        entry_.proof=!independent?SelectionProof::ConservativeLegacyOrder:
            closed?SelectionProof::QualifiedDepthEntry:chain?SelectionProof::LegacyQualifiedPixels:
            SelectionProof::IndependentPrimaryState;
        entry_.fault_cursor=independent?FaultCursor::NotApplicable:FaultCursor::AdvanceCompletedPixelOnly;
        entry_.fault_state=independent?FaultState::OwnerLocalCarry:FaultState::PreserveMutatedContinuation;
        entry_.fault_flow=independent?OwnerFaultFlow::SingleAttempt:OwnerFaultFlow::StopContinuation;
        entry_.skip=SkipTransition::DEAOBackgroundClear;entry_.auxiliary=RayControls|InsideActivity|RngContinuation;
        entry_.tasks=independent?p.width*p.height:p.task_count;
        entry_.rows_per_task=independent?0:p.task_rows;
        entry_.continuation_pixels=independent?0:1;entry_.dispatch_batch=independent?mb3d_native_ao::kDEPixelBatch:p.task_count;
    }
    void validate(const mb3d_native::NativeFormulaProgram& program,const mb3d_native_ao::DEParams& p,
        const mb3d_native::NativeFormulaProgram* second=nullptr) const {
        if(!program_.matches(program)||!parameters_.matches(p)||
           !second_.matches(second?*second:mb3d_native::NativeFormulaProgram{}))
            throw std::invalid_argument("Native DE AO execution policy input mismatch");
    }
    const EntryExecutionContract& entry() const{return entry_;}
    bool independent() const{return entry_.owner==LogicalOwner::Pixel;}
    // Pixel ownership rests on the conditional dIFS contract.
    bool conditional_state() const{return conditional_;}
    // Pixels take host-planned entering seeds of their random first steps.
    bool seeded() const{return seeded_;}
    // Pixel-entry proof word the device checks: 1 closed depth chain, 5 state
    // contract, 6 closed primary chain, 7 conditional state contract.
    uint32_t pixel_proof() const{using namespace mb3d_native_ao;
        return entry_.proof==SelectionProof::QualifiedDepthEntry?kDEProofClosedDepth:
        entry_.proof==SelectionProof::LegacyQualifiedPixels?kDEProofClosedChain:
        entry_.proof!=SelectionProof::IndependentPrimaryState?0u:
        conditional_?kDEProofConditionalState:kDEProofStateContract;}
};
} // namespace mb3d::native_metal
