#pragma once
#include "native_ao_screen.metal.h"
#include "native_ao_threshold_math.h"
using namespace metal;

kernel void native_ao15_row_range(device const ExactSurface* input [[buffer(0)]],
    device uint2* ranges [[buffer(1)]],constant mb3d_native_ao::Params& params [[buffer(2)]],
    constant uint& allocated_pixels [[buffer(3)]],constant uint& allocated_rows [[buffer(4)]],
    uint row [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;Params p=params;
    if(row>=allocated_rows)return;
    if(!valid(p)||p.algorithm>1u||allocated_pixels!=p.output_count||allocated_rows!=p.height){ranges[row]=uint2(0xffffffffu);return;}
    uint low=32767,high=0;
    for(uint x=0;x<p.width;++x){uint z=input[row*p.width+x].depth_packed>>16;if(z<32768){low=min(low,z);high=max(high,z);}}
    ranges[row]=uint2(low,high);
}
kernel void native_ao15_range(device const uint2* ranges [[buffer(0)]],
    device mb3d_native_ao::Range15* result [[buffer(1)]],constant mb3d_native_ao::Params& params [[buffer(2)]],
    constant uint& allocated_rows [[buffer(3)]],uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;if(tid)return;Params p=params;Range15 r{};r.version=Version;
    if(!valid(p)||p.algorithm>1u||allocated_rows!=p.height){r.fault=Invalid;*result=r;return;}
    r.minimum=32767;r.maximum=0;
    for(uint row=0;row<p.height;++row){uint2 v=ranges[row];if(v.x>32767||v.y>32767){r.fault=Invalid;break;}r.minimum=min(r.minimum,v.x);r.maximum=max(r.maximum,v.y);}
    if(r.maximum<r.minimum){r.maximum=32768;r.minimum=0;}else ++r.maximum;
    r.correction=128.0f/float(r.maximum-r.minimum);r.subtract=I32(r.minimum<<8);
    r.angle_scale=p.depth_scale/(r.correction*256.0f);
    r.threshold=p.threshold*(p.algorithm==Threshold15T0?2.0f:1.0f)/r.angle_scale;
    if(p.algorithm==Threshold15T0)r.threshold=max(0.01f,r.threshold);
    if(!isfinite(r.correction)||r.correction<=0||!isfinite(r.angle_scale)||r.angle_scale<=0||!isfinite(r.threshold)||r.threshold<=0)r.fault|=NonFinite;
    *result=r;
}
kernel void native_ao15_encode(device const ExactSurface* input [[buffer(0)]],
    device ExactSurface* output [[buffer(1)]],device ushort* depth [[buffer(2)]],
    device mb3d_native_ao::Record* records [[buffer(3)]],device mb3d_native_ao::PreprocessRecord* prep [[buffer(4)]],
    constant mb3d_native_ao::Params& params [[buffer(5)]],device const mb3d_native_ao::Range15* range [[buffer(6)]],
    constant mb3d_native_ao::Launch& launch [[buffer(7)]],uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;Params p=params;Launch run=launch;U32 pixel;
    if(!launch_pixel(p,run,tid,pixel))return;
    Record r{};r.version=Version;r.algorithm=p.algorithm;r.initial_ambient=input[pixel].amb_shadow;r.final_ambient=r.initial_ambient;
    PreprocessRecord pr{};pr.version=Version;output[pixel]=input[pixel];
    if(!valid(p)||p.algorithm>1u||p.levels>8u||run.reserved||run.allocated_pixels!=p.output_count||range->fault||range->version!=Version||input[pixel].fault){r.fault=Invalid;records[pixel]=r;pr.fault=Invalid;prep[pixel]=pr;return;}
    if(pixel<p.width*p.height){
        r.active=(input[pixel].depth_packed&0x80000000u)?0:1;
        U32 encoded=0;
        if(!threshold15_encode(input[pixel].depth_packed,range->subtract,range->correction,encoded))r.fault=NonFinite;
        depth[pixel]=ushort(encoded);pr.normalizations=1;pr.reads=1;pr.writes=1;
    }
    records[pixel]=r;prep[pixel]=pr;
}
// Original median mutates its input in raster order. This bounded serial GPU
// continuation preserves dependencies on already modified upper/left pixels.
kernel void native_ao15_median(device ushort* depth [[buffer(0)]],
    device mb3d_native_ao::MedianState* state [[buffer(1)]],constant mb3d_native_ao::Params& params [[buffer(2)]],
    constant uint& allocated_depth [[buffer(3)]],constant uint& reset [[buffer(4)]],uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;if(tid)return;Params p=params;MedianState s=*state;
    if(reset)s={Version,0,0,0,0,0,0,0};
    if(!valid(p)||p.algorithm>1u||p.levels>8u||reset>1u||allocated_depth!=p.width*p.height||s.version!=Version||s.reserved){s.fault|=Invalid;*state=s;return;}
    if(s.finished||s.fault)return;
    if(s.cursor>allocated_depth){s.fault|=Bounds;*state=s;return;}
    for(uint visit=0;visit<256u&&s.cursor<allocated_depth;++visit,++s.cursor){
        const uint x=s.cursor%p.width,y=s.cursor/p.width;
        if(x==0||y==0||x+1>=p.width||y+1>=p.height)continue;
        if(s.processed>=p.work_limit){s.fault|=WorkExhausted;break;}++s.processed;
        const ushort center=depth[s.cursor];ushort maximum=0;bool lower=true;
        const int offsets[8]={-int(p.width)-1,-int(p.width),-int(p.width)+1,-1,1,int(p.width)-1,int(p.width),int(p.width)+1};
        for(uint n=0;n<8u;++n){const int at=int(s.cursor)+offsets[n];if(at<0||uint(at)>=allocated_depth){s.fault|=Bounds;lower=false;break;}
            const ushort v=depth[at];++s.reads;if(v>=center){lower=false;break;}maximum=max(maximum,v);}
        if(lower&&maximum>0){depth[s.cursor]=maximum+1;++s.writes;}
    }
    s.finished=s.cursor==allocated_depth;*state=s;
}
kernel void native_ao15_wavelet(device const ushort* input [[buffer(0)]],device ushort* output [[buffer(1)]],
    device mb3d_native_ao::PreprocessRecord* records [[buffer(2)]],constant mb3d_native_ao::Params& params [[buffer(3)]],
    constant uint& step [[buffer(4)]],constant uint& axis [[buffer(5)]],constant mb3d_native_ao::Launch& launch [[buffer(6)]],
    uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;Params p=params;Launch run=launch;U32 pixel;
    if(!launch_pixel(p,run,tid,pixel)||pixel>=p.width*p.height)return;
    PreprocessRecord r=records[pixel];
    if(!valid(p)||p.algorithm>1u||p.levels>8u||!step||step>128u||axis>1u||run.reserved||run.allocated_pixels!=p.output_count){r.fault|=Invalid;records[pixel]=r;return;}
    const uint x=pixel%p.width,y=pixel/p.width,count=p.width*p.height;
    const uint coordinate=axis?y:x,extent=axis?p.height:p.width,stride=axis?p.width:1u;
    const uint start=axis?x:y*p.width,last=start+(extent-1u)*stride;
    uint value,center=uint(input[pixel]);
    if(coordinate<min(step,extent)) {
        const uint lower=start+min(extent-1u,coordinate+step)*stride;
        const uint neighbor=input[lower],edge=uint(input[start]);
        if(axis&&x>=(p.width&~3u)) {
            // Exact legacy byte+1 word read; invalid narrow cases fault.
            const ulong actual=ulong(y+step)*p.width+x;
            if(actual+1u>=count){r.fault|=Bounds;records[pixel]=r;return;}
            value=threshold15_first(center,edge,neighbor,true,input[actual+1u]);r.reads+=4;
        }else {value=threshold15_first(center,edge,neighbor);r.reads+=3;}
    }else if(coordinate<extent-step) {
        value=threshold15_middle(center,input[pixel-step*stride],input[pixel+step*stride]);r.reads+=3;
    }else {
        const uint previous=start+(coordinate>step?coordinate-step:0u)*stride;
        value=threshold15_last(center,input[previous],input[last],axis!=0);r.reads+=3;
    }
    output[pixel]=ushort(value);++r.wavelet_passes;++r.writes;records[pixel]=r;
}

kernel void native_ao15_scan(device ExactSurface* surface [[buffer(0)]],
    device const ushort* levels [[buffer(1)]],device mb3d_native_ao::Record* records [[buffer(2)]],
    constant mb3d_native_ao::Params& params [[buffer(3)]],device const mb3d_native_ao::Range15* range [[buffer(4)]],
    constant mb3d_native_ao::Launch& launch [[buffer(5)]],constant uint& allocated_level_words [[buffer(6)]],
    uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;Params p=params;Launch run=launch;U32 pixel;
    if(!launch_pixel(p,run,tid,pixel)||pixel>=p.width*p.height)return;
    Record record=records[pixel];Range15 rg=*range;
    const uint count=p.width*p.height;
    if(!valid(p)||p.algorithm>1u||p.levels>8u||p.sum_multiplier<=0||
       allocated_level_words!=count*p.levels||run.reserved||run.allocated_pixels!=p.output_count||
       rg.version!=Version||rg.fault){record.fault|=Invalid;records[pixel]=record;return;}
    if(!isfinite(rg.correction)||rg.correction<=0||!isfinite(rg.angle_scale)||rg.angle_scale<=0||
       !isfinite(rg.threshold)||rg.threshold<=0){record.fault|=NonFinite;records[pixel]=record;return;}
    if(record.fault)return;
    const int width=int(p.width),height=int(p.height),x=int(pixel%p.width),y=int(pixel/p.width);
    if(!record.active) {
        // Original fixed four-pixel quartets clear background lanes whenever
        // any other lane is foreground; trailing columns are not quartets.
        const int start=x&~3;
        if(start+3<width){bool foreground=false;for(uint lane=0;lane<4;++lane)foreground|=(surface[uint(y)*p.width+uint(start)+lane].depth_packed&0x80000000u)==0;
            if(foreground)surface[pixel].amb_shadow=0;}
        record.final_ambient=surface[pixel].amb_shadow;record.changed=record.final_ambient!=record.initial_ambient;
        records[pixel]=record;return;
    }
    int zp;
    if(!rounded(float(int(surface[pixel].depth_packed>>8)-rg.subtract)*rg.correction,zp)){record.fault|=NonFinite;records[pixel]=record;return;}
    float maxima[32];for(uint i=0;i<32;++i)maxima[i]=-1e10f;
    int minimum=0,maximum=0;
    for(uint index=1;index<=p.levels&&!record.fault;++index){
        const int step=1<<(index-1);maximum+=4*step;
        const int minimum2=minimum*minimum,maximum2=maximum*maximum;
        const float spread=sqrt(float(step))*5.0f;
        int spread_test;
        if(!rounded(spread/float(minimum+1),spread_test)){record.fault|=NonFinite;break;}
        const bool multiple=spread_test>0;
        const float limit=rg.threshold*sqrt(sqrt(float(p.levels)/float(index)));
        if(!isfinite(limit)||limit<=0){record.fault|=NonFinite;break;}
        const int group=x&~3;
        const bool vector_path=group<width-3&&group>=maximum&&group<width-maximum-3;
        int first_y=-maximum,extra_y=0x7fffffff;
        if(first_y+y<0){extra_y=first_y;while(extra_y+y<0)extra_y+=step;
            if(extra_y+y>=height)extra_y=height-y-1;
            first_y=-y;if(first_y==extra_y)extra_y=0x7fffffff;}
        const int last_y=min(maximum,height-y-1);
        for(int dy=first_y;dy<=last_y&&!record.fault;){
            if(dy>extra_y){dy=extra_y;extra_y=0x7fffffff;}
            int first_x=-maximum,extra_x=0x7fffffff;
            if(first_x+x<0){extra_x=first_x;while(extra_x+x<0)extra_x+=step;
                if(extra_x+x>=width)extra_x=width-x-1;
                first_x=-x;if(first_x==extra_x)extra_x=0x7fffffff;}
            const int last_x=min(maximum,width-x-1);
            for(int dx=first_x;dx<=last_x&&!record.fault;){
                if(!spend(record,p.work_limit))break;
                if(dx>extra_x){dx=extra_x;extra_x=0x7fffffff;}
                const int radius2=dx*dx+dy*dy;
                if(radius2>minimum2&&radius2<=maximum2){
                    const int sx=x+dx,sy=y+dy;
                    if(sx<0||sx>=width||sy<0||sy>=height){record.fault|=Bounds;break;}
                    const uint at=(index-1u)*count+uint(sy)*p.width+uint(sx);
                    if(at>=allocated_level_words){record.fault|=Bounds;break;}
                    const float inverse=1.0f/sqrt(float(radius2));
                    float value=float(int(levels[at])-zp)*inverse;++record.depth_reads;
                    if(!isfinite(value)||!isfinite(inverse)){record.fault|=NonFinite;break;}
                    bool include=true;
                    if(p.algorithm==Threshold15T0){
                        include=vector_path||value<limit;
                        if(include){
                            if(vector_path){value=min(value,limit);const float t=(1.0f/limit)*value;const float cube=(t*t)*t;value=value-cube*value;}
                            else{const float t=value/limit;value=value*(1.0f-t*t*t);}
                        }
                    }else value=min(value,limit);
                    if(!isfinite(value)){record.fault|=NonFinite;break;}
                    if(include){
                        const int angle=angle15(dy,dx);int extent=0;
                        if(multiple&&!rounded(spread*inverse,extent)){record.fault|=NonFinite;break;}
                        if(angle<0||angle>31||extent<0||extent>64){record.fault|=Bounds;break;}
                        const int start=multiple?(angle-(extent>>1))&31:angle;
                        for(int n=0;n<=extent;++n)maxima[(start+n)&31]=max(maxima[(start+n)&31],value);
                    }
                }
                const int previous=dx;dx+=step;if(dx>last_x&&previous<last_x)dx=last_x;
            }
            const int previous=dy;dy+=step;if(dy>last_y&&previous<last_y)dy=last_y;
        }
        minimum=maximum;++record.levels;
    }
    if(!record.fault){
        float sum=0;
        for(uint i=0;i<32;++i)if(maxima[i]>-1e9f){const float term=maxima[i]*rg.angle_scale;if(!isfinite(term)){record.fault|=NonFinite;break;}sum+=atan(term);}
        const float value=sum*p.sum_multiplier;int ambient;
        if(!isfinite(sum)||!isfinite(value)||!rounded(clamp(value,0.0f,16383.0f),ambient))record.fault|=NonFinite;
        if(!record.fault){surface[pixel].amb_shadow=uint(ambient);record.final_ambient=uint(ambient);record.changed=record.final_ambient!=record.initial_ambient;record.repeats=1;}
    }
    records[pixel]=record;
}
