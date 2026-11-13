#pragma once
// Native FP32 depth-of-field kernels; compiled with contraction disabled.
#ifndef MB3D_METAL_NATIVE_FLOAT
#error Native DOF requires Native arithmetic profile
#endif
#include "dof_abi.h"
#include "dof_order.metal.h"
#include "dof_accumulate.metal.h"
#include "comparison/metal_exact/shadow_abi.h"
#include "comparison/metal_exact/color_abi.h"
namespace mb3d_dof {
inline bool valid(constant Params& p) {
    return p.version==Version&&p.width&&p.height&&p.width<=4096&&p.height<=4096&&
        p.plain<=1&&p.passes>=1&&p.passes<=4&&!p.reserved0&&!p.reserved1&&!p.reserved2&&p.reserved_float==0&&
        isfinite(p.inverse_z_mul)&&p.inverse_z_mul>0&&isfinite(p.inverse_z_corr)&&p.inverse_z_corr>0&&
        isfinite(p.fov_factor)&&p.fov_factor>=0&&isfinite(p.focus_low)&&isfinite(p.focus_high)&&p.focus_low<=p.focus_high&&
        isfinite(p.aperture)&&p.aperture>=.0001f&&p.aperture<=2&&isfinite(p.clip_radius)&&p.clip_radius>=.1f&&p.clip_radius<=1000;
}
inline void fail(device Global& g,uint code){atomic_fetch_or_explicit((device atomic_uint*)&g.fault,code,memory_order_relaxed);}
inline float depth(float difference,constant Params& p){const float base=difference*p.inverse_z_mul+1;return (base*base-1)*p.inverse_z_corr;}
inline bool finite_accum(thread const Accum& a){return isfinite(a.r)&&isfinite(a.g)&&isfinite(a.b)&&isfinite(a.transparency)&&a.transparency>=0;}
[[host_name("native_dof_abi")]] kernel void abi(device uint* o [[buffer(0)]],uint i [[thread_position_in_grid]]) {
    if(i)return;const uint v[]={Version,sizeof(Params),sizeof(Pass),sizeof(Source),sizeof(Global),sizeof(SortState),sizeof(Tile),sizeof(Accum),sizeof(Launch)};
    for(uint j=0;j<9;++j)o[j]=v[j];
}
// Global is initialized to {32768,0,0,0,0,0,0,0}; one entry per real pixel.
[[host_name("native_dof_depth")]] kernel void prepare_depth(constant Params& p [[buffer(0)]],
 device const ExactSurface* surface [[buffer(1)]],device Source* source [[buffer(2)]],device Item* order [[buffer(3)]],
 device Global& g [[buffer(4)]],constant uint& allocation [[buffer(5)]],uint i [[thread_position_in_grid]]) {
    if(i>=allocation)return;
    if(!valid(p)||p.width*p.height>allocation){fail(g,1);return;}if(i>=p.width*p.height)return;
    const auto s=surface[i];Source v{};v.index=i;v.raw_key=s.depth_packed>>8;v.key=int(v.raw_key);if(v.key>8388352)v.key=-1;
    if(s.fault||s.reserved[0]||s.reserved[1]||s.reserved[2]){v.fault=1;fail(g,2);}
    const uint z=s.depth_packed>>16;
    // Bit2 retains the plain background predicate, independent of sorted key.
    v.flags=(v.key==-1?2u:0u)|(z>32767?4u:0u);source[i]=v;order[i]={v.key,i};
    if(z<32768){atomic_fetch_min_explicit((device atomic_uint*)&g.nearest_z,z,memory_order_relaxed);
        atomic_fetch_max_explicit((device atomic_uint*)&g.far_z,z,memory_order_relaxed);}
}
[[host_name("native_dof_pass")]] kernel void prepare_pass(constant Params& p [[buffer(0)]],
 device Global& g [[buffer(1)]],device Pass& out [[buffer(2)]],constant uint& pass [[buffer(3)]],uint i [[thread_position_in_grid]]) {
    if(i)return;Pass q{};q.version=Version;q.index=pass;
    if(!valid(p)||pass>=p.passes||g.fault||g.nearest_z>32768||g.far_z>32767){q.fault=1;out=q;fail(g,4);return;}
    const uint mode=p.passes-1;
    if(mode==0)q.aperture=p.aperture*.5f;
    else if(mode==1)q.aperture=p.aperture*((p.plain?.3f:.25f)+float(pass)*(p.plain?.55f:.58f))*.5f;
    else {const float power=p.plain?float(1u<<pass):(pass==0?1.f:pass==1?3.f:pass==2?9.f:27.f);
        q.aperture=(p.aperture/(p.plain?12.f:22.f))*power;}
    const float lowden=1+p.focus_low*p.fov_factor,highden=1+p.focus_high*p.fov_factor;
    if(!isfinite(lowden)||!isfinite(highden)||lowden<=0||highden<=0){q.fault=1;out=q;fail(g,4);return;}
    const float ratio=lowden/highden,weighted=ratio*p.focus_high,numerator=weighted+p.focus_low,focusden=1+ratio;
    if(!isfinite(ratio)||!isfinite(weighted)||!isfinite(numerator)||!isfinite(focusden)||focusden<=0){q.fault=1;out=q;fail(g,4);return;}
    q.focus=numerator/focusden;
    const float difference=p.focus_low-q.focus,quotient=difference/lowden;
    q.rsub=abs(quotient*q.aperture);q.max_radius=p.clip_radius;
    if(!isfinite(difference)||!isfinite(quotient)){q.fault=1;out=q;fail(g,4);return;}
    if(!p.plain){const float near=depth(float((32768-g.nearest_z)*256),p),far=depth(float((32768-g.far_z)*256),p);
        const float nearDen=1+near*p.fov_factor,farDen=1+far*p.fov_factor;
        const float nearDiff=near-q.focus,farDiff=far-q.focus,nearNumerator=nearDiff*q.aperture;
        if(!isfinite(near)||!isfinite(far)||!isfinite(nearDen)||!isfinite(farDen)||nearDen<=0||farDen<=0||
           !isfinite(nearDiff)||!isfinite(farDiff)||!isfinite(nearNumerator)){q.fault=1;out=q;fail(g,4);return;}
        const float farQuotient=farDiff/farDen;
        const float rnear=abs(nearNumerator/nearDen),rfar=abs(farQuotient*q.aperture),maximum=max(rnear,rfar)*1.5f;
        if(!isfinite(farQuotient)||!isfinite(rnear)||!isfinite(rfar)||!isfinite(maximum)){q.fault=1;out=q;fail(g,4);return;}
        q.max_radius=min(p.clip_radius,maximum);}
    if(lowden<=0||highden<=0||!isfinite(q.aperture)||!isfinite(q.focus)||!isfinite(q.rsub)||!isfinite(q.max_radius)||q.max_radius<0){q.fault=1;fail(g,4);}
    g.max_radius_bits=0;g.max_bound=0;out=q;
}
[[host_name("native_dof_radii")]] kernel void prepare_radius(constant Params& p [[buffer(0)]],
 device const Pass& pass [[buffer(1)]],device Source* source [[buffer(2)]],device Global& g [[buffer(3)]],
 constant uint& allocation [[buffer(4)]],uint i [[thread_position_in_grid]]) {
    if(i>=allocation)return;if(!valid(p)||p.width*p.height>allocation){fail(g,1);return;}if(i>=p.width*p.height)return;
    auto s=source[i];if(s.index!=i||s.fault||s.raw_key>0xffffffu||s.reserved||(s.flags&~7u)||pass.version!=Version||pass.index>=p.passes||pass.fault||pass.reserved||
        !isfinite(pass.aperture)||!isfinite(pass.focus)||!isfinite(pass.rsub)||pass.rsub<0||!isfinite(pass.max_radius)||pass.max_radius<0||pass.max_radius>1000){fail(g,8);return;}
    const float delta=p.plain?((s.flags&4)?8388353.f:8388352.f-float(s.raw_key)):8388352.f-float(s.key);
    const float z=depth(delta,p),den=1+z*p.fov_factor;
    const float difference=z-pass.focus;
    const float intermediate=p.plain?difference/den:difference*pass.aperture;
    if(!isfinite(z)||!isfinite(den)||den<=0||!isfinite(difference)||!isfinite(intermediate)){s.fault=2;source[i]=s;fail(g,8);return;}
    const float raw=p.plain?intermediate*pass.aperture:intermediate/den;
    if(!isfinite(raw)){s.fault=2;source[i]=s;fail(g,8);return;}
    const float radius=max(0.f,abs(raw)-pass.rsub);
    s.radius=min(radius,pass.max_radius);s.flags=(s.flags&6u)|(raw<0?1u:0u);
    if(p.plain&&raw<0)s.radius=-s.radius;
    if(!isfinite(z)||den<=0||!isfinite(raw)||!isfinite(s.radius)||abs(s.radius)>1000){s.fault=2;source[i]=s;fail(g,8);return;}
    s.bound=uint(rint(abs(s.radius)+.5f));
    source[i]=s;atomic_fetch_max_explicit((device atomic_uint*)&g.max_radius_bits,as_type<uint>(abs(s.radius)),memory_order_relaxed);
    atomic_fetch_max_explicit((device atomic_uint*)&g.max_bound,s.bound,memory_order_relaxed);
}
inline bool valid_launch(constant Params& p,constant Launch& l){return valid(p)&&!l.reserved&&l.tile_count&&l.tile_count<=l.tile_allocation&&
    l.pixel_allocation>=p.width*p.height&&l.candidate_capacity&&ulong(l.tile_count)*l.candidate_capacity<=l.candidate_allocation&&
    l.source_begin<=l.source_end&&l.source_end-l.source_begin<=MaxSlice;}
inline bool valid_tile(thread const Tile& t,constant Params& p){return t.width&&t.height&&t.width<=TileSize&&t.height<=TileSize&&
    t.x<p.width&&t.y<p.height&&t.width<=p.width-t.x&&t.height<=p.height-t.y&&!t.reserved;}
// Independent invocation per tile. Append order is exactly the global permutation.
[[host_name("native_dof_candidates")]] kernel void candidates(constant Params& p [[buffer(0)]],constant Launch& l [[buffer(1)]],
 device const Source* sources [[buffer(2)]],device const Item* order [[buffer(3)]],device Tile* tiles [[buffer(4)]],
 device uint* list [[buffer(5)]],device Global& g [[buffer(6)]],uint t [[thread_position_in_grid]]) {
    if(t>=l.tile_allocation)return;if(!valid_launch(p,l)||l.source_end>p.width*p.height){fail(g,16);return;}if(t>=l.tile_count)return;
    auto tile=tiles[t];if(!valid_tile(tile,p)||tile.fault||tile.cursor!=l.source_begin||tile.count>l.candidate_capacity){fail(g,16);return;}
    for(uint j=l.source_begin;j<l.source_end;++j){const uint i=p.plain?j:order[j].source;
        if(i>=p.width*p.height){tile.fault=1;break;}const auto s=sources[i];
        if(s.fault||s.index!=i||s.bound>1000||s.raw_key>0xffffffu||s.reserved||(!p.plain&&order[j].key!=s.key)){tile.fault=1;break;}
        const int x=int(i%p.width),y=int(i/p.width),r=int(s.bound);
        if(x+r<int(tile.x)||y+r<int(tile.y)||x-r>int(tile.x+tile.width-1)||y-r>int(tile.y+tile.height-1))continue;
        if(tile.count==l.candidate_capacity){tile.fault=2;break;}
        list[ulong(t)*l.candidate_capacity+tile.count++]=i;
    }
    tile.cursor=l.source_end;tiles[t]=tile;if(tile.fault)fail(g,16);
}

[[host_name("native_dof_spatial_abi")]] kernel void spatial_abi(device uint* out [[buffer(0)]],uint i [[thread_position_in_grid]]) {
    if(i)return;out[0]=1;out[1]=sizeof(SpatialRecord);out[2]=4096;out[3]=256;
}
[[host_name("native_dof_inverse_rank")]] kernel void inverse_rank(constant Params& p [[buffer(0)]],
    device const Source* sources [[buffer(1)]],device const Item* order [[buffer(2)]],device atomic_uint* ranks [[buffer(3)]],
    device Global& g [[buffer(4)]],constant uint& allocation [[buffer(5)]],uint rank [[thread_position_in_grid]]) {
    if(rank>=allocation)return;if(!valid(p)||allocation!=p.width*p.height){fail(g,16);return;}
    const uint index=p.plain?rank:order[rank].source;
    if(index>=allocation){fail(g,16);return;}const auto s=sources[index];
    if(s.fault||s.index!=index||(!p.plain&&s.key!=order[rank].key)){fail(g,16);return;}
    if(atomic_exchange_explicit(ranks+index,rank,memory_order_relaxed)!=0xffffffffu)fail(g,16);
}
// Collect only sources whose support could intersect this tile, then sort their
// unique inverse ranks. Integer sorting restores the exact legacy permutation.
[[host_name("native_dof_spatial_candidates")]] kernel void spatial_candidates(
    constant Params& p [[buffer(0)]],constant Launch& l [[buffer(1)]],device const Source* sources [[buffer(2)]],
    device const Item* order [[buffer(3)]],device Tile* tiles [[buffer(4)]],device uint* list [[buffer(5)]],
    device Global& g [[buffer(6)]],device const uint* ranks [[buffer(7)]],device SpatialRecord* records [[buffer(8)]],
    uint group [[threadgroup_position_in_grid]],uint lane [[thread_index_in_threadgroup]]) {
    threadgroup uint sorted[4096];threadgroup uint kept;
    if(group>=l.tile_count)return;
    if(!valid_launch(p,l)||l.source_begin||l.source_end||g.max_bound>24){if(!lane)fail(g,16);return;}
    const auto tile=tiles[group];
    if(!valid_tile(tile,p)||tile.fault||tile.cursor||tile.count){if(!lane)fail(g,16);return;}
    const int radius=int(g.max_bound);const uint loX=uint(max(0,int(tile.x)-radius)),loY=uint(max(0,int(tile.y)-radius));
    const uint w=min(p.width,tile.x+tile.width+uint(radius))-loX,h=min(p.height,tile.y+tile.height+uint(radius))-loY;
    const uint count=w*h;uint size=1;while(size<count)size*=2;
    if(size>4096){if(!lane)fail(g,16);return;}
    for(uint j=lane;j<size;j+=256){uint rank=0xffffffffu;
        if(j<count){const uint i=(loY+j/w)*p.width+loX+j%w;const auto s=sources[i];
            const int x=int(i%p.width),y=int(i/p.width),r=int(s.bound);const uint q=ranks[i];
            if(s.fault||s.index!=i||s.bound>g.max_bound||s.raw_key>0xffffffu||s.reserved||q>=p.width*p.height||
                (!p.plain&&(order[q].source!=i||order[q].key!=s.key))){fail(g,16);}
            else if(!(x+r<int(tile.x)||y+r<int(tile.y)||x-r>int(tile.x+tile.width-1)||y-r>int(tile.y+tile.height-1)))rank=q;
        }sorted[j]=rank;
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);
    for(uint k=2;k<=size;k*=2)for(uint stride=k/2;stride;stride/=2){
        for(uint j=lane;j<size;j+=256){const uint other=j^stride;if(other>j){const uint a=sorted[j],b=sorted[other];
            const bool ascending=(j&k)==0;sorted[j]=ascending?min(a,b):max(a,b);sorted[other]=ascending?max(a,b):min(a,b);}}
        threadgroup_barrier(mem_flags::mem_threadgroup);
    }
    if(!lane){kept=0;while(kept<size&&sorted[kept]!=0xffffffffu)++kept;
        const uint fault=kept>l.candidate_capacity?1u:0u;
        records[group]={1,count,kept,fault};auto out=tile;out.count=kept;out.cursor=p.width*p.height;out.fault=fault;tiles[group]=out;
        if(fault)fail(g,16);
    }
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if(kept>l.candidate_capacity)return;
    for(uint j=lane;j<kept;j+=256){const uint rank=sorted[j];list[ulong(group)*l.candidate_capacity+j]=p.plain?rank:order[rank].source;}
}
// l.source_begin/end here describe candidate positions, not global source order.
// Input RGB is immutable for the entire pass; Accum is zeroed at batch start.
[[host_name("native_dof_gather")]] kernel void gather(constant Params& p [[buffer(0)]],constant Launch& l [[buffer(1)]],
 device const Source* sources [[buffer(2)]],device const Tile* tiles [[buffer(3)]],device const uint* list [[buffer(4)]],
 device const ExactColorPixel* rgb [[buffer(5)]],device Accum* output [[buffer(6)]],device Global& g [[buffer(7)]],uint lane [[thread_position_in_grid]]) {
    if(ulong(lane)>=ulong(l.tile_allocation)*256)return;
    if(!valid_launch(p,l)||l.source_end>l.candidate_capacity||g.max_bound>1000||g.max_radius_bits>as_type<uint>(1000.f)||g.reserved0||g.reserved1||g.reserved2){fail(g,32);return;}
    const uint t=lane/256,k=lane%256;if(t>=l.tile_count)return;const auto tile=tiles[t];
    if(!valid_tile(tile,p)||tile.fault||tile.cursor!=p.width*p.height||tile.count>l.candidate_capacity){fail(g,32);return;}
    if(k>=tile.width*tile.height)return;const uint x=tile.x+k%tile.width,y=tile.y+k/tile.width,target=y*p.width+x;
    Accum a=output[lane];const uint expected=min(l.source_begin,tile.count);
    if(a.fault||a.visited!=expected||!finite_accum(a)){fail(g,32);return;}
    const float sm=1/(1+as_type<float>(g.max_radius_bits));
    for(uint j=l.source_begin;j<min(l.source_end,tile.count);++j){++a.visited;const uint i=list[ulong(t)*l.candidate_capacity+j];
        if(i>=p.width*p.height){a.fault=1;break;}const auto s=sources[i];const auto color=rgb[i];
        if(s.fault||s.index!=i||s.bound>1000||s.raw_key>0xffffffu||s.reserved||(s.flags&~7u)||!isfinite(s.radius)||abs(s.radius)>1000||
           color.fault||color.reserved0||color.reserved1||color.rgb>0xffffffu){a.fault=1;break;}
        const int dx=int(x)-int(i%p.width),dy=int(y)-int(i/p.width);if(abs(dx)>int(s.bound)||abs(dy)>int(s.bound))continue;
        const float extent=abs(s.radius)+1;const int ds=dx*dx+dy*dy;
        const bool contributes=p.plain?extent*extent-float(dx*dx)-float(dy*dy)>0:ds<=int(extent*extent);
        if(!contributes)continue;++a.contributed;
        const float3 c=float3(color.rgb&255,(color.rgb>>8)&255,(color.rgb>>16)&255);
        if(p.plain)plain_contribution(a,c,s.radius,sources[target].radius,sm,dx,dy);
        else sorted_contribution(a,c,s.radius,(s.flags&1)!=0,(s.flags&2)!=0,dx,dy);
        if(!finite_accum(a)){a.fault=2;break;}
    }
    output[lane]=a;if(a.fault)fail(g,32);
}
[[host_name("native_dof_finish")]] kernel void finish(constant Params& p [[buffer(0)]],constant Launch& l [[buffer(1)]],
 device const Tile* tiles [[buffer(2)]],device Accum* accum [[buffer(3)]],device const ExactColorPixel* input [[buffer(4)]],
 device ExactColorPixel* output [[buffer(5)]],device Global& g [[buffer(6)]],uint lane [[thread_position_in_grid]]) {
    if(ulong(lane)>=ulong(l.tile_allocation)*256)return;
    if(!valid_launch(p,l)){fail(g,64);return;}const uint t=lane/256,k=lane%256;if(t>=l.tile_count)return;
    const auto tile=tiles[t];if(!valid_tile(tile,p)||tile.fault||tile.cursor!=p.width*p.height){fail(g,64);return;}if(k>=tile.width*tile.height)return;
    auto a=accum[lane];if(a.fault||a.visited!=tile.count||!finite_accum(a)){fail(g,64);return;}
    const uint target=(tile.y+k/tile.width)*p.width+tile.x+k%tile.width;
    ExactColorPixel color{};color.rgb=quantize(a);a.changed=color.rgb!=input[target].rgb;accum[lane]=a;output[target]=color;
}
} // namespace mb3d_dof
