// Separate approximate preview library; production shader bytes are unchanged.
#define MB3D_METAL_NATIVE_FLOAT 1
#include <metal_stdlib>
using namespace metal;
#include "comparison/metal_exact/scene_math.h"
using namespace mb3d_metal_exact;
kernel void exact_arithmetic_profile(device uint* out [[buffer(0)]]) {out[0]=2;out[1]=4;out[2]=32;}
kernel void navigation_preview_abi(device uint* out [[buffer(0)]]) {out[0]=2;out[1]=9;out[2]=1;}
uint preview_hash(uint x){x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;return x^(x>>16);}
// One owner per disjoint screen cell. Sampling coordinates are full viewport
// coordinates, not rays through a smaller camera. No cross-pixel state or RNG.
kernel void navigation_points(device uint* rgb [[buffer(0)]],device uint4* records [[buffer(1)]],
    device const ExactTrig* columns [[buffer(2)]],device const ExactTrig* rows [[buffer(3)]],
    constant ExactScene& scene [[buffer(4)]],constant uint* cfg [[buffer(5)]],
    uint localIndex [[thread_position_in_grid]]) {
    const uint width=cfg[0],height=cfg[1],block=cfg[2],count=cfg[3];
    const uint i=cfg[7]+localIndex;
    if(i>=count)return;
    if(!width||!height||width>1600||height>1600||!block||block>4||cfg[4]>2||
       scene.width!=width||scene.height!=height||!valid_scene(scene)){
        records[i]=uint4(1,0,0,0);return;
    }
    const uint cellsX=(width+block-1)/block,ox=(i%cellsX)*block,oy=(i/cellsX)*block;
    if(ox>=width||oy>=height){records[i]=uint4(1,0,0,0);return;}
    const uint cw=min(block,width-ox),ch=min(block,height-oy),area=cw*ch;
    const uint start=cfg[4]==2?preview_hash(i^cfg[6])%area:0;
    const uint slot=(start+cfg[5]%area)%area,x=ox+slot%cw,y=oy+slot/cw;
    if(cfg[8] && cfg[5]>=area){records[i]=uint4(0,x,y,0);return;}
    ExactPixel p=render_primary(scene,columns[x],rows[y],x,y);
    uint color=0x231a13;
    if(p.rm_result) {
        // Neutral headlight surface: authored palette/effects are not claimed.
        float diffuse=p.start_inside?.12f:.15f+.85f*abs(float(p.normal[2])/32767.f);
        uint v=uint(clamp(diffuse,0.f,1.f)*255.f);
        color=v|(uint(float(v)*.82f)<<8)|(uint(float(v)*.6f)<<16);
    }
    records[i]=uint4(p.fault,x,y,1);
    // Explicit block reconstruction; does not calculate unsampled rays.
    if(!cfg[8] || cfg[5]==0)
        for(uint yy=oy;yy<oy+ch;++yy)for(uint xx=ox;xx<ox+cw;++xx)rgb[yy*width+xx]=color;
    else rgb[y*width+x]=color; // Preserve every previously measured point.
}
