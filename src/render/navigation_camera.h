#pragma once
#include "core/types.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <stdexcept>

// Internal shared camera adapter. Preserves optical projection in the source header.
namespace mb3d::navigation {
using Vec = std::array<double, 3>;
inline Vec add(Vec a, Vec b, double s = 1) {
    for (int i=0;i<3;++i) a[i] += b[i]*s;
    return a;
}
inline Vec unit(Vec a) {
    double n=std::hypot(a[0],a[1],a[2]);
    if (!std::isfinite(n) || n<1e-100) throw std::runtime_error("Invalid camera basis");
    for (double &v:a) v/=n;
    return a;
}
struct Camera {
    Vec right, up, forward, center;
    double offset, farOffset, span;
    bool changed=false;
    static bool supports(const TMandHeader10& h){return h.bStereoMode==0 && h.bPlanarOptic<=2;}
    explicit Camera(const TMandHeader10& h) {
        if(!supports(h))throw std::runtime_error("Unsupported camera projection/stereo");
        double rows[3][3]; std::memcpy(rows,h.hVGrads,sizeof(rows));
        right=unit({rows[0][0],rows[0][1],rows[0][2]});
        up=unit({rows[1][0],rows[1][1],rows[1][2]});
        forward=unit({rows[2][0],rows[2][1],rows[2][2]});
        center={h.dXmid,h.dYmid,h.dZmid};
        offset=h.dZstart-h.dZmid; farOffset=h.dZend-h.dZmid;
        span=2.1345/std::max(1e-100,h.dZoom);
        if (!std::isfinite(span) || span<=0) throw std::runtime_error("Invalid camera scale");
    }
    Vec eye() const { return add(center,forward,offset); }
    void rotate(double yaw,double pitch,bool fly) {
        if(yaw==0&&pitch==0)return;changed=true;
        Vec fixedEye=eye();
        Vec r=right,f=forward,u=up;
        right=unit(add(add(Vec{},r,std::cos(yaw)),f,-std::sin(yaw)));
        forward=unit(add(add(Vec{},r,std::sin(yaw)),f,std::cos(yaw)));
        f=forward;
        up=unit(add(add(Vec{},u,std::cos(pitch)),f,std::sin(pitch)));
        forward=unit(add(add(Vec{},u,-std::sin(pitch)),f,std::cos(pitch)));
        if(fly) center=add(fixedEye,forward,-offset);
    }
    // About the view direction: `right` turns towards `up`.
    void roll(double angle) {
        if(angle==0)return;changed=true;
        const Vec r=right,u=up;
        right=unit(add(add(Vec{},r,std::cos(angle)),u,std::sin(angle)));
        up=unit(add(add(Vec{},r,-std::sin(angle)),u,std::cos(angle)));
    }
    void move(double x,double y,double z) {
        if(x==0&&y==0&&z==0)return;changed=true;
        center=add(add(add(center,right,x*span),up,y*span),forward,z*span);
    }
    // Towards (step > 0) or away from what the camera looks at, in scene units.
    // While the pivot is ahead the camera comes closer to it or backs away from
    // it and the far limit stays where it is; a camera that turns in place takes
    // the pivot along. Moving back never brings the far limit nearer.
    void dolly(double step) {
        if(step==0)return;changed=true;
        if(offset<0){
            const double along=step>0?std::min(step,-offset):step;
            offset+=along;step-=along;
        }
        if(step!=0){
            center=add(center,forward,step);
            if(step<0)farOffset-=step;
        }
    }
    // Same viewpoint, start plane moved along the view axis by `shift` scene
    // units (the caller scales the zoom to match). The pivot and the far limit
    // stay where they are; a camera that turns in place keeps doing so.
    void shiftPlane(double shift) {
        if(shift==0)return;changed=true;
        if(offset==0){center=add(center,forward,shift);farOffset-=shift;return;}
        offset+=shift;
        if(offset>0)repivot(0);
    }
    // Turn about the point `distance` ahead of the start plane from now on.
    void repivot(double distance) {
        const double shift=offset+distance;
        if(shift==0)return;changed=true;
        center=add(center,forward,shift);
        offset=-distance;farOffset-=shift;
    }
    // The far limit at least `depth` behind the start plane.
    void keepDepth(double depth) {
        if(farOffset-offset>=depth)return;changed=true;
        farOffset=offset+depth;
    }
    void write(TMandHeader10& h) const {
        if(!changed)return;
        h.dXmid=center[0];h.dYmid=center[1];h.dZmid=center[2];
        h.dZstart=center[2]+offset;h.dZend=center[2]+farOffset;
        double rows[3][3];
        for(int i=0;i<3;++i){ rows[0][i]=right[i];rows[1][i]=up[i];rows[2][i]=forward[i]; }
        std::memcpy(h.hVGrads,rows,sizeof(rows));
    }
};
}
