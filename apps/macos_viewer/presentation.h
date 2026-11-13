#pragma once
#include <cstdint>
namespace viewer {
enum class Quality { Draft=0, Grid=1, Points=2, Full=3, FastGeometry=4 };
struct PreviewPass { Quality quality; unsigned phase; };
// Resolution changes start a fresh accumulation; samples from two pixel grids
// must never share a phase counter. Full remains at the settled resolution.
inline PreviewPass previewPass(Quality selected,bool refinement,unsigned nextPhase,
                               unsigned phaseCount,int previousEdge,int edge) {
    if(refinement&&previousEdge!=edge)
        return {selected==Quality::Full?Quality::Points:selected,0};
    const auto fast=selected==Quality::Full?Quality::Points:selected;
    if(!refinement)return {fast,0};
    if(phaseCount>1&&nextPhase<phaseCount)return {fast,nextPhase};
    return {Quality::Full,0};
}
inline PreviewPass nextPass(Quality selected,bool refinement,unsigned nextPhase,unsigned phaseCount) {
    return previewPass(selected,refinement,nextPhase,phaseCount,640,640);
}

struct MotionResolution {
    unsigned level=2,fastFrames=0;
    int edge()const {return level==0?160:level==1?240:level==2?320:480;}
    void observe(double seconds,bool cold) {
        if(cold||!(seconds>0))return; // Compilation is not a navigation sample.
        if(seconds>.10) {if(level)--level;fastFrames=0;}
        else if(seconds<.035) {if(++fastFrames>=4){if(level<3)++level;fastFrames=0;}}
        else fastFrames=0;
    }
};

// Let a moving-camera preview finish so continuous input cannot starve display.
// Only idle refinement is superseded; a service must have replied before signals
// are sent, because its startup handler may not yet be installed.
inline bool cancelIdleNavigation(bool full,bool refinement,bool ready) {
    return !full&&refinement&&ready;
}

inline bool shouldPresent(uint64_t frame,uint64_t requested,uint64_t shown,
                          uint64_t frameEpoch,uint64_t currentEpoch,bool refinement) {
    return frameEpoch==currentEpoch && (frame>shown || (refinement && frame==shown)) && frame<=requested &&
        (!refinement || frame==requested);
}
}
