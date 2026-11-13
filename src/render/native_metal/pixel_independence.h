#pragma once
#include "formula_abi.h"
namespace mb3d_native {
// Closed depth-scene chains. Hybrid3DAnalytic resets xyz/w, j.xyz, c,
// radius/trap, first and iteration on every DE call. _PartlyJuliaRoff reads
// immutable params, j.xyz and current c; it writes only orbit-local j.xyz.
// Amazing Box reads xyz/w and j.xyz. koch_surf reads xyz/w and immutable
// typed parameters; _Rotate reads xyz and its matrix. None consumes retained
// SSE/dfree/derivative lanes, j.w, tmp, or mutates counts/original_j/c.
// Do not merge these sets with other whitelists without another state audit.
#ifdef __METAL_VERSION__
inline bool IndependentDepthScenePixels(constant NativeFormulaProgram& p) {
#else
inline bool IndependentDepthScenePixels(const NativeFormulaProgram& p) {
#endif
    // A DE combination's second part runs on the same lane; never audited.
    if(p.formula_type || p.driver!=Hybrid3DAnalytic || (p.de_option!=2 && p.de_option!=11))return false;
    bool found=false;
#ifdef __METAL_VERSION__
    for(constant auto& s:p.slots) {
#else
    for(const auto& s:p.slots) {
#endif
        if(!s.enabled)continue;
        found=true;
        if(s.version!=kFormulaAbiVersion || s.difs_style)return false;
        if(p.de_option==11) {
            if(s.namespace_id==InternalFormula) {
                if(s.id!=4 || s.de_option!=11 || s.param_count || s.traits || s.wrapper_flags!=2)return false;
            } else if(s.namespace_id==ExternalFormula) {
                if(s.id!=10005 || s.de_option!=-1 || s.param_count!=5 || s.traits!=4160 || s.wrapper_flags!=1)return false;
            } else return false;
        } else {
            if(s.namespace_id!=ExternalFormula || s.traits || s.wrapper_flags!=1)return false;
            if(s.id==7227) {if(s.de_option!=2 || s.param_count!=22)return false;}
            else if(s.id==7060) {if(s.de_option!=-1 || s.param_count!=9)return false;}
            else return false;
        }
    }
    return found;
}
#ifndef __METAL_VERSION__
// A deliberately closed, source-audited set. In these two 3D drivers each
// distance call overwrites c, xyz, j.xyz, first, iteration, rout,
// otrap and the analytic w derivative before use. Vary is preserved; the
// stateless bodies below must not mutate it. The listed bodies consume
// only those fields and immutable parameters/maps. They never access retained
// SSE/dfree/derivative lanes, mutate hybrid counts or update the Julia vector.
// State within a pixel (march, refinement, normals) is still preserved.
// New formulas, other 4D, interpolation, dIFS and XYZ-derivative drivers must keep
// logical CPU task order until separately audited. Traits alone are not proof.
inline bool IndependentPrimaryPixels(const NativeFormulaProgram& p) {
    if(p.formula_type)return false;
    if(IndependentDepthScenePixels(p))return true;
    // Closed Amazing Surf 2 singleton (Ellarien). Hybrid3DAnalytic resets xyz/w,
    // j.xyz, first, iteration and radius/trap for each distance call. The body
    // reads immutable parameters and j.xyz, writes only xyz/w, and never
    // consumes retained SSE/dfree/derivative lanes or changes original_j/counts.
    // Julia's original_j remains live immutable input, not discarded state.
    if(p.driver==Hybrid3DAnalytic && p.de_option==2 && p.slot_count==6 &&
       p.start_slot==0 && p.repeat_slot==0 && p.end_slot==0 && p.hybrid_counts[0]==1) {
        const auto& s=p.slots[0];
        bool match=s.version==kFormulaAbiVersion && s.namespace_id==ExternalFormula &&
            s.enabled==1 && s.id==10186 && s.param_count==19 && s.de_option==2 &&
            !s.traits && s.wrapper_flags==1 && !s.difs_style;
        for(unsigned i=1;i<6;++i)match&=!p.slots[i].enabled && p.hybrid_counts[i]==0;
        if(match)return true;
    }
    // Closed 4D _Rotate -> box chain. The former 3D AmazingSurf exception
    // cannot use first=0 as a Vary entry reset: preceding slots already run
    // a guard that consumes carried Vary. It keeps ordered fallback until a
    // separate path/finite-invariant proof exists. The 4D offset body consumes
    // xyz/w, j.xyzw and derivative[0], all reset by
    // Hybrid4DAnalytic. _Rotate consumes xyz and immutable matrix values.
    // This chain reads no retained SSE/dfree and changes no counts/original_j.
    if(p.slot_count==6 && p.start_slot==0 && p.end_slot==1 &&
       p.driver==Hybrid4DAnalytic && p.de_option==5 && p.repeat_slot==0) {
        const auto& a=p.slots[0];const auto& b=p.slots[1];
        bool match=a.version==kFormulaAbiVersion && a.namespace_id==ExternalFormula &&
            a.enabled==1 && a.id==7060 && a.param_count==9 && a.de_option==-1 &&
            !a.traits && a.wrapper_flags==1 && !a.difs_style &&
            b.version==kFormulaAbiVersion && b.namespace_id==ExternalFormula && b.enabled==1 &&
            b.id==10324 && b.param_count==9 &&
            b.de_option==5 && b.traits==2 &&
            b.wrapper_flags==1 && !b.difs_style &&
            (U32(p.hybrid_counts[0])&0x7fffffffu)==1 && p.hybrid_counts[1]==1;
        for(unsigned i=2;i<6;++i)match&=!p.slots[i].enabled;
        if(match)return true;
    }
    // Jost1's ordered chain: PolyFold/reciprocal read only xyz and immutable
    // parameters. MengerKochV2 additionally scales the analytic w derivative,
    // reset on entry to every Hybrid3DAnalytic orbit. No body consumes retained
    // SSE/dfree/j.w or writes counts, original_j, c or a map. Preserve every
    // sample within a pixel; only separate pixels may start with fresh state.
    if(p.driver==Hybrid3DAnalytic&&p.de_option==2&&p.slot_count==6&&
       p.start_slot==0&&p.repeat_slot==0&&p.end_slot==3){
        const U32 ids[]={7076,10037,10037,7007},counts[]={7,25,25,1};
        const I32 options[]={-1,2,2,-1};
        const U32 repetitions[]={0x80000002u,3,3,0x80000001u};
        bool match=!p.slots[4].enabled&&!p.slots[5].enabled;
        for(unsigned i=0;i<4;++i){const auto& s=p.slots[i];
            match&=s.version==kFormulaAbiVersion&&s.namespace_id==ExternalFormula&&
                s.enabled==1&&s.id==ids[i]&&s.param_count==counts[i]&&
                s.de_option==options[i]&&!s.traits&&s.wrapper_flags==1&&!s.difs_style&&
                U32(p.hybrid_counts[i])==repetitions[i];}
        if(match)return true;
    }
    // Closed stateless analytic-box chains. ABoxMod1 (8024) is no longer
    // admitted here: its conditional first-use Vary initialization is not an
    // unconditional orbit-entry reset. _updateC2 changes j.xyz and w
    // only inside that orbit: j.xyz is restored from original_j/c on entry.
    // None of these bodies consumes j.w, retained SSE/derivative/dfree, mutates
    // original_j or hybrid counts. Keep all samples within each pixel ordered.
    // Do not mix this exception with the older stateless whitelist below.
    if(p.driver==Hybrid3DAnalytic && p.de_option==2) {
        bool found=false, audited=true;
        for(const auto& s:p.slots) {
            if(!s.enabled)continue;
            found=true;
            if(s.version!=kFormulaAbiVersion || s.namespace_id!=ExternalFormula || s.difs_style) {
                audited=false;break;
            }
            if(s.id==8024) {
                audited=false; // Needs a separate first-write/guard-order proof.
            } else if(s.id==8900) {
                audited=s.de_option==2 && s.param_count==10 && s.traits==0 && s.wrapper_flags==0;
            } else if(s.id==10014) {
                audited=s.de_option==-1 && s.param_count==13 && s.traits==576 && s.wrapper_flags==1;
            } else audited=false;
            if(!audited)break;
        }
        if(found && audited)return true;
    }
    // Closed 4D bodies: no variable-scale, SSE, dfree, count or Julia-state
    // mutation. MixPinski4 reads/writes only derivative[0], initialized by
    // Hybrid4DAnalytic on every orbit. Retained derivative[1:2] are unused.
    if(p.driver==Hybrid4D || p.driver==Hybrid4DAnalytic) {
        if(p.de_option!=(p.driver==Hybrid4D?4u:6u))return false;
        bool found=false;
        for(const auto& s:p.slots) {
            if(!s.enabled)continue;
            if(s.namespace_id!=ExternalFormula || s.difs_style)return false;
            if(p.driver==Hybrid4D) {
                if(s.id!=9401 || s.traits || s.wrapper_flags)return false;
            } else {
                if(s.wrapper_flags!=1)return false;
                if(s.id==10343) {if(s.traits!=2)return false;}
                else if(s.id==10350 || s.id==5823) {if(s.traits)return false;}
                else return false;
            }
            found=true;
        }
        return found;
    }
    if(p.driver!=Hybrid3D && p.driver!=Hybrid3DAnalytic)return false;
    if(p.driver==Hybrid3DAnalytic && (p.de_option&0x38)==32)return false;
    bool external=false;
    for(const auto& s:p.slots) {
        if(!s.enabled)continue;
        if(s.namespace_id==InternalFormula) {
            if(s.id!=4 && s.id!=6)return false;
            continue;
        }
        if(s.namespace_id!=ExternalFormula || s.difs_style)return false;
        external=true;
        switch(s.id) {
        case 429: case 2153: case 3778: case 9526:
        case 10006: case 10030: case 10269:
            if(s.traits)return false;
            break;
        case 6030: // _SphereFolding1 reads the current orbit radius.
            if(s.traits!=256)return false;
            break;
        default:return false;
        }
    }
    return external;
}
#endif
}
