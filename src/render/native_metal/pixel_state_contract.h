#pragma once
// Host state contract for primary queries. An independent contract lets the
// execution policy replace the legacy persistent order with pixel ownership.
// This is not a general licence to reorder normal/gradient/AO/shadow samples
// or dependent reflection bounces. Reflection pixels reuse it only to start
// each pixel from a fresh lane; samples and bounces keep their CPU order.
#include "formula_abi.h"
#include "soft_shadow_state_contract.h"
#include "generated/formula_state_effects.h"
#include <cstdint>
#include <string_view>
namespace mb3d_native {
using StateMask=std::uint64_t;
namespace pixel_state {
constexpr StateMask bit(unsigned n){return StateMask(1)<<n;}
constexpr StateMask XYZ=7, W=bit(3), C=7ull<<4, J=7ull<<8, J4=bit(11), SSE=15ull<<20;
constexpr StateMask OriginalJ=15ull<<12, Rout=bit(24), Rold=bit(25);
constexpr StateMask Trap=bit(28), Vary=bit(29), Iteration=bit(33), First=bit(34);
constexpr StateMask RstopD=bit(27), Dfree1=bit(30);
constexpr StateMask Counts=63ull<<37, Julia=bit(43);
constexpr StateMask CallerControls=C|bit(26)|bit(27)|bit(36)|bit(44)|bit(45);
// Orbit-driver writes only; caller control/restoration writes are separate.
constexpr StateMask DriverMayWrite=XYZ|W|J|Rout|Rold|Trap|Iteration|First|bit(32)|bit(35)|bit(46);
// Only fields definitely initialized before a consuming formula/guard in the
// admitted primary distance/direct-normal entry points. Immutable live inputs
// are represented separately: they must never be reset merely for parallelism.
constexpr StateMask OrbitInitialized=XYZ|W|C|J|Rout|Rold|Trap|Iteration|First;
// HybridDifs initializes Vary and clears dfree1 at orbit entry; every dIFS query enters through
// native_distance_controlled, which sets rstop_d and rold=rstop_d first.
constexpr StateMask DifsInitialized=OrbitInitialized|Vary|RstopD|Dfree1;
// Hybrid4D rotates c into xyz/w and sets j including j.w on every entry, but
// never resets vary_scale (see the live-input rule in AnalyzePrimaryPixelState).
constexpr StateMask FourInitialized=OrbitInitialized|J4;
constexpr StateMask LiveInputs=OriginalJ|Counts|Julia;
// native_step's own reads after every body: its FP32 range guard tests xyz/w,
// rout and vary_scale, and the orbit loop reads iteration.
constexpr StateMask StepReads=XYZ|W|Rout|Vary|Iteration;
constexpr StateMask Derivative=7ull<<16;
struct Effects {
    bool known=false;
    // contained: reads whose values reach only the same lane's own write.
    StateMask reads_before_write=0, must_write=0, may_write=0, contained=0;
};
inline Effects formula(const NativeFormulaSlot& s){
    if(s.version!=kFormulaAbiVersion||s.enabled!=1||s.difs_style||s.traits)return {};
    if(s.namespace_id==InternalFormula){
        if(s.id!=4||s.de_option!=11||s.param_count||
           (s.wrapper_flags!=0&&s.wrapper_flags!=2))return {};
        return {true,XYZ|W|J,XYZ,XYZ|W};
    }
    if(s.namespace_id!=ExternalFormula)return {};
    bool known=false;
    switch(s.id){
    case 9452:known=s.de_option==2&&s.param_count==1&&s.wrapper_flags==1;break;
    case 7046:known=s.de_option==0&&s.param_count==5&&s.wrapper_flags==0;break;
    case 3676:known=s.de_option==-1&&s.param_count==7&&s.wrapper_flags==1;break;
    case 10010:known=s.de_option==-1&&s.param_count==15&&s.wrapper_flags==1;break;
    case 7076:known=s.de_option==-1&&s.param_count==7&&s.wrapper_flags==1;break;
    case 10171:known=s.de_option==2&&s.param_count==22&&s.wrapper_flags==1;break;
    case 1509:known=s.de_option==11&&s.param_count==15&&s.wrapper_flags==1;break;
    }
    if(!known)return {};
    // Conservative semantic reads include the wrapper's finite guards and
    // first/vary pass-through, not only the generated formula body's accesses.
    // J.xyz is live for Ides/add-J, helispiral and AmazingSurf; over-approximating
    // the other listed bodies is safe. No contract here consumes J.w/SSE/dfree.
    // Refine only the already hand-admitted descriptors, using their exact
    // compiler mask. Stateless wrappers pass First/Vary through; a load/store
    // of the same value is not a semantic mutation. Do not claim a must-write
    // from this may-write analysis or treat a later slot as entry initialization.
    for(const auto& e:generated_effects::kEntries)
        if(e.namespace_id==s.namespace_id&&e.id==s.id&&e.exact&&
           e.de_option==s.de_option&&e.param_count==s.param_count&&
           e.traits==s.traits&&e.wrapper_flags==s.wrapper_flags)
            return {true,XYZ|W|J|Rout|Vary|First|Iteration,0,e.writes};
    return {};
}
// Compiler-derived masks for a slot the hand audit above does not know. External
// bodies bind the exact metadata the analysis specialized; internal bodies were
// analyzed with runtime slot metadata. Reads include the wrapper's guards and
// over-approximate reads before write; no must-write is claimed. Configuration
// fails when an analyzed body or the wrapper differs from the recorded source.
// The analysis kernel reads slot.difs_style at run time, so masks cover both styles.
inline Effects generated_formula(const NativeFormulaSlot& s){
    if(s.version!=kFormulaAbiVersion||s.enabled!=1)return {};
    for(const auto& e:generated_effects::kEntries){
        if(e.namespace_id!=s.namespace_id||e.id!=s.id)continue;
        if(e.exact&&(e.de_option!=s.de_option||e.param_count!=s.param_count||
                     e.traits!=s.traits||e.wrapper_flags!=s.wrapper_flags))return {};
        return {true,e.reads,0,e.writes,e.contained};
    }
    return {};
}
}
struct PixelStateContract {
    uint32_t version=1, entry=1; // ordered primary: distance + direct normals
    uint32_t scope=1; // formula/wrapper dependencies + separate caller ownership
    // Exact descriptors with captured device proofs (1 Loopy, 2 Spirals). Any
    // other independent contract is admitted by the execution policy.
    uint32_t production_chain=0;
    // generated: at least one slot uses compiler-derived masks.
    // conditional: the HybridDifs driver itself reads carried tmp/dfree2 when
    // no iteration improves its 65535 entry minimum. Pixel ownership then
    // holds only for a frame whose records count no difs_carried_reads.
    bool known=false, independent=false, generated=false, conditional=false;
    StateMask initialized=0,live_inputs=0,formula_reads_before_write=0,body_must_write_union=0,formula_may_write=0;
    StateMask caller_owned_controls=0,driver_may_write=0;
    // Carried lanes that cannot reach this pixel: no driver or caller reads
    // them, and every body reading them only rescales them in place.
    StateMask contained=0;
    // Version2: a source-proven first callback erases an incoming difference.
    // A zero-pass retry may retain it, but no admitted downstream consumer
    // observes it before the next qualifying entry. Not entry initialization.
    StateMask entry_equivalent=0;
};
namespace pixel_state {
inline bool admitted_driver(const NativeFormulaProgram& p){
    return (p.driver==Hybrid3D&&p.de_option==0)||
        (p.driver==Hybrid3DAnalytic&&(p.de_option==2||p.de_option==11))||
        (p.driver==HybridDifs&&p.de_option==20)||
        (p.driver==Hybrid4D&&p.de_option==4&&!p.formula_type);
}
// Deriv1..3 feed the analytic DE only in 4D, interpolated and XYZ-derivative
// (de_option&0x38==32) modes, and a DE 5/6 combination divides by Deriv1.
// Outside those, no driver or caller reads them; formula bodies alone do.
inline StateMask driver_unread(const NativeFormulaProgram& p){
    const bool threeD=p.driver==Hybrid3D||p.driver==Hybrid3DAnalytic||
        p.driver==HybridDifs||p.driver==HybridDifsNoInitialization;
    return threeD&&(p.de_option&0x38)!=32&&p.de_option!=5&&p.de_option!=6?Derivative:0;
}
// Fields a part's own orbit entry initializes. HybridDifsNoInitialization keeps
// the incoming xyz/w and j.xyz, so it is admitted only as a second part: its
// entry state is then the first part's orbit of the same query.
inline StateMask part_initialized(const NativeFormulaProgram& p,bool generated){
    if(p.driver==HybridDifsNoInitialization)return DifsInitialized&~(XYZ|W|J);
    if(p.driver==HybridDifs)return DifsInitialized;
    if(p.driver==Hybrid4D)return FourInitialized;
    // Ordinary3D preserves incoming J.w for both Julia and Mandelbrot.
    // It is an immutable live input only if no part/body writes it.
    (void)generated;
    return OrbitInitialized;
}
// Driver ownership is a union of actual possible writes across both parts.
// Return SSE writes happen after callbacks; they are not entry initialization.
inline StateMask part_driver_may_write(const NativeFormulaProgram& p){
    const bool four=p.driver==Hybrid4D||p.driver==Hybrid4DAnalytic||
        p.driver==Interpolated4D||p.driver==Interpolated4DAnalytic;
    const bool interpolated=p.driver>=Interpolated3D&&p.driver<=Interpolated4DAnalytic;
    const bool difs=p.driver==HybridDifs||p.driver==HybridDifsNoInitialization;
    StateMask result=DriverMayWrite|(four?J4:0)|(difs?Vary|Dfree1|bit(31):SSE);
    if(interpolated)result&=~bit(35); // nHybrid carries weights, not remaining work.
    return result;
}
inline StateMask first_callback_equivalence(const NativeFormulaProgram& p){
    // CMake checks this manifest against actual generated bodies, wrapper and
    // their dependencies. Regenerating the effects after a source change does
    // not silently renew this stronger conditional-effect proof.
    if(std::string_view(generated_effects::kSourcesSha256)!=
        "9d04fb63072496222defd88f51bd22865a58804bc77814b3682106e834fe8097")return 0;
    if(p.formula_type||!((p.driver==Hybrid3D&&p.de_option==0)||
        (p.driver==Hybrid3DAnalytic&&(p.de_option==2||p.de_option==11)))||
        p.start_slot>p.end_slot||p.end_slot>=6||p.repeat_slot>p.end_slot||
        p.hybrid_counts[p.start_slot]<=0)return 0;
    const auto& s=p.slots[p.start_slot];
    // Conditional effect summary for the original Surf body, not a chain
    // allowlist: First=0 selects Scale before any old-Vary-dependent operation.
    // Wrapper copies are inert; the first finiteness guard follows the body.
    // Counts must remain immutable (checked by the enclosing contract).
    return s.version==kFormulaAbiVersion&&s.namespace_id==ExternalFormula&&
        s.id==1509&&s.de_option==11&&s.param_count==15&&s.traits==0&&
        s.wrapper_flags==1&&s.difs_style==0&&s.internal_power==0&&
        s.enabled==1&&s.reserved[0]==0&&s.reserved[1]==0?Vary:0;
}
// Slots a part can run: native_advance starts at start_slot, steps up to
// end_slot and wraps to repeat_slot. The interval over-approximates that set.
inline bool part_slot(const NativeFormulaProgram& p,unsigned i){
    const uint32_t low=p.repeat_slot<p.start_slot?p.repeat_slot:p.start_slot;
    uint32_t high=p.end_slot>p.start_slot?p.end_slot:p.start_slot;
    if(p.repeat_slot>high)high=p.repeat_slot;
    return i>=low&&i<=high;
}
}
// A DE combination (p.formula_type) passes its second part. Both parts share
// the slots analyzed below; each admitted driver re-initializes its orbit lanes
// on entry. The second part runs after the first within one query, so a field
// the first part's entry initialized is query-local there too; a slot only the
// second part reaches may read it. Every other slot keeps the first part's
// initialization: the next query's first part sees the second part's writes.
// A dIFS part in either position makes the contract conditional: its driver
// counts each carried tmp/dfree2 result, whichever part it runs.
inline PixelStateContract AnalyzePrimaryPixelState(const NativeFormulaProgram& p,
    const NativeFormulaProgram* second=nullptr){
    using namespace pixel_state;
    PixelStateContract out;
    if(p.version!=kFormulaAbiVersion||p.slot_count!=6||!admitted_driver(p))return out;
    if(p.formula_type&&(!second||second->formula_type!=p.formula_type||second->version!=kFormulaAbiVersion||
        second->slot_count!=6||!(admitted_driver(*second)||
            (second->driver==HybridDifsNoInitialization&&second->de_option==20))))return out;
    if(!p.formula_type&&second)return out;
    out.conditional=p.driver==HybridDifs||(second&&second->driver>=HybridDifs);
    out.live_inputs=LiveInputs;
    // These masks are distinct from formula consumption. Ordered primary owns
    // inside/calculation flags, limits and stop values for the entire pixel.
    // Distance and direct-normal entry points prepare different subsets; the
    // latter must not reset all incoming fields to make this proof appear true.
    out.caller_owned_controls=CallerControls;
    out.driver_may_write=part_driver_may_write(p)|(second?part_driver_may_write(*second):0);
    Effects effects[6];
    bool found=false;
    for(unsigned i=0;i<6;++i){
        const auto& s=p.slots[i];
        if(s.version!=kFormulaAbiVersion||s.enabled>1||s.reserved[0]||s.reserved[1])return out;
        if(!s.enabled)continue;
        auto e=formula(s);
        if(!e.known){e=generated_formula(s);out.generated|=e.known;}
        if(!e.known)return out;
        found=true;effects[i]=e;out.formula_reads_before_write|=e.reads_before_write;
        // This union describes individual successful body returns, not a
        // query must-write set: a short orbit may never reach a later slot.
        out.body_must_write_union|=e.must_write;out.formula_may_write|=e.may_write;
    }
    out.known=found;
    // Ordinary entry does not reset Vary/J.w. A lane keeps its original
    // value across queries only when no body OR either driver can change it.
    // In particular a later dIFS part must not masquerade as an immutable
    // Vary input to the next query's ordinary first part.
    out.live_inputs|=(Vary|J4)&~(out.formula_may_write|out.driver_may_write);
    const StateMask first=part_initialized(p,out.generated);
    const StateMask later=second?first|part_initialized(*second,out.generated):first;
    out.initialized=later;
    // A lane no driver or caller reads, which every enabled body reading it
    // only rescales in place, carries the previous pixel's value only into
    // itself: that read is not a dependency of this pixel's result.
    StateMask contained=driver_unread(p)&(second?driver_unread(*second):~StateMask(0));
    for(unsigned i=0;i<6;++i)
        if(effects[i].known)contained&=~(effects[i].reads_before_write&~effects[i].contained);
    out.contained=contained&out.formula_reads_before_write;
    // This relation is separate from both unconditional initialization and
    // dead/contained lanes. No prefix search or may-write union proves it.
    if(!second&&!(out.formula_may_write&Counts))
        out.entry_equivalent=first_callback_equivalence(p);
    if(out.entry_equivalent)out.version=2;
    StateMask uncovered=0;
    for(unsigned i=0;i<6;++i){
        if(!effects[i].known)continue;
        const bool laterOnly=second&&part_slot(*second,i)&&!part_slot(p,i);
        uncovered|=(effects[i].reads_before_write|StepReads)&~((laterOnly?later:first)|out.live_inputs|out.contained|out.entry_equivalent);
    }
    out.independent=found&&!uncovered&&!(out.formula_may_write&(out.live_inputs|C));
    // These two exact descriptors keep their separate captured device proofs
    // and require the hand-audited slot metadata; the soft-shadow entry admits
    // only the second. Other independent contracts are admitted by policy.
    if(!out.independent||out.generated||out.entry_equivalent||p.start_slot!=0||p.formula_type)return out;
    if(p.driver==Hybrid3D&&p.de_option==0&&p.repeat_slot==0&&p.end_slot==2){
        const uint32_t ids[]={4,9452,7046},counts[]={12,1,2};
        bool match=p.slots[0].namespace_id==InternalFormula&&p.slots[0].wrapper_flags==0;
        for(unsigned i=0;i<3;++i)match&=p.slots[i].enabled==1&&p.slots[i].id==ids[i]&&
            uint32_t(p.hybrid_counts[i])==counts[i]&&(i==0||p.slots[i].namespace_id==ExternalFormula);
        for(unsigned i=3;i<6;++i)match&=!p.slots[i].enabled&&p.hybrid_counts[i]==0;
        if(match)out.production_chain=1;
    }
    if(p.driver==Hybrid3DAnalytic&&p.de_option==2&&p.repeat_slot==1&&p.end_slot==5){
        const uint32_t ids[]={3676,4,10010,7076,10171,1509};
        const uint32_t counts[]={0x80000001u,1,0x80000001u,0x80000001u,8,102};
        bool match=p.slots[1].wrapper_flags==2;
        for(unsigned i=0;i<6;++i)match&=p.slots[i].enabled==1&&p.slots[i].id==ids[i]&&
            uint32_t(p.hybrid_counts[i])==counts[i]&&
            p.slots[i].namespace_id==(i==1?InternalFormula:ExternalFormula);
        if(match)out.production_chain=2;
    }
    return out;
}
inline PixelStateContract AnalyzeSoftShadowPixelState(const NativeFormulaProgram& p,
    uint32_t initial_inside,uint32_t in_and_out){
    using namespace pixel_state;
    PixelStateContract out;out.entry=2;
    if(p.formula_type||!SoftShadowPixelChain(p)||initial_inside||in_and_out)return out;
    // The shadow bridge owns inside; each ordered DE/refinement query owns
    // C.xyz and limits. No direct-normal entry or neighboring row state is used.
    out.initialized=OrbitInitialized;out.live_inputs=LiveInputs;
    out.caller_owned_controls=CallerControls;out.driver_may_write=part_driver_may_write(p);
    for(const auto& s:p.slots){const auto e=formula(s);if(!e.known)return out;
        out.formula_reads_before_write|=e.reads_before_write;
        out.body_must_write_union|=e.must_write;out.formula_may_write|=e.may_write;}
    out.live_inputs|=(Vary|J4)&~(out.formula_may_write|out.driver_may_write);
    out.known=true;
    out.independent=!(out.formula_reads_before_write&~(out.initialized|out.live_inputs))&&
        !(out.formula_may_write&(out.live_inputs|C));
    if(out.independent)out.production_chain=2;
    return out;
}
} // namespace mb3d_native
