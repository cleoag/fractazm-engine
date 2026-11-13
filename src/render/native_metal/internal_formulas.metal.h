#pragma once
#include "formula_abi.h"
#include "formula_math.metal.h"
namespace mb3d_native {
using namespace metal;
inline float3 native_sample_direct(float3,int,thread const NativeResources&,thread NativeFormulaFault&);
inline int internal_i32(constant NativeFormulaSlot& s,int offset) {
    return as_type<int>(s.raw_words[(256+offset)/4]);
}
inline float internal_f32(constant NativeFormulaSlot& s,int offset) {
    return as_type<float>(s.raw_words[(256+offset)/4]);
}
inline float internal_fold(float x,float r) { return abs(x+r)-abs(x-r)-x; }
inline void native_conditional_write(thread NativeFormulaState& state,int slot,int count) {
    const uint index=uint(clamp(slot,1,6)-1);
    state.hybrid_counts[index]=as_type<int>((as_type<uint>(state.hybrid_counts[index])&0x80000000u)|uint(count));
}
inline bool native_conditional_step(thread NativeFormulaState& state,constant NativeFormulaSlot& slot) {
    if(!(native_formula_selector_selected(10353u)||native_formula_selector_selected(10354u)||
         native_formula_selector_selected(10355u)||native_formula_selector_selected(10356u)||
         native_formula_selector_selected(10357u)))return false;
    if(slot.id<10353 || slot.id>10357)return false;
    if(slot.id==10353) {
        const int offset=!(state.rout>slot.negative_params[1])?-32:
            !(state.rout>slot.negative_params[2])?-36:-40;
        native_conditional_write(state,internal_i32(slot,-28),clamp(internal_i32(slot,offset),0,99999));
    } else {
        const uint counter=as_type<uint>(state.first)+1u;state.first=as_type<int>(counter);
        if(slot.id==10354 || slot.id==10355) {
            for(int base=0;base<(slot.id==10355?40:20);base+=20) {
                const int threshold=clamp(internal_i32(slot,-12-base),1,100);
                const int offset=state.first<threshold?-20:state.first==threshold?-24:-28;
                native_conditional_write(state,internal_i32(slot,-16-base),clamp(internal_i32(slot,offset-base),0,100));
            }
        } else {
            const uint shift=uint(max(0,int(counter&15u)-1));
            const bool bit=(as_type<uint>(internal_i32(slot,-12))>>shift)&1u;
            const int a=internal_i32(slot,-16),b=internal_i32(slot,-20);
            const int low=slot.id==10357?clamp(internal_i32(slot,-24),0,100):0;
            const int high=slot.id==10357?clamp(internal_i32(slot,-28),1,100):1;
            native_conditional_write(state,bit?b:a,low);native_conditional_write(state,bit?a:b,high);
        }
    }
    return true;
}
inline float3 internal_box(float3 v,constant NativeFormulaSlot& s,thread float& w,bool derivative) {
    float fold=s.negative_params[4];
    v=float3(internal_fold(v.x,fold),internal_fold(v.y,fold),internal_fold(v.z,fold));
    float r=(v.x*v.x+v.y*v.y)+v.z*v.z;
    float scale=r<s.negative_params[3]?s.negative_params[2]:
        r<s.constants[4]?s.negative_params[1]/r:s.negative_params[1];
    if(derivative)w*=scale;
    return v*scale;
}
inline float3 internal_int_power(float3 v,uint power,float z_mul) {
    float x=v.x,y=v.y,z=v.z,sx=x*x,sy=y*y,sz=z*z,R=sx+sy,A;
    switch(power) {
    case 2:A=(R-sz)/R;return float3((sx-sy)*A,2.f*x*y*A,2.f*sqrt(R)*z_mul*z);
    case 3:A=1.f-3.f*sz/R;return float3(A*(sx-3.f*sy)*x,A*(3.f*sx-sy)*y,-z_mul*z*(sz-3.f*R));
    case 4:A=1.f+sz*(sz-6.f*R)/(R*R+1e-40f);return float3(A*(sx*(sx-6.f*sy)+sy*sy),4.f*x*y*A*(sx-sy),z_mul*4.f*sqrt(R)*z*(R-sz));
    case 5:A=1.f+5.f*(sz*sz-2.f*R*sz)/(R*R+1e-40f);return float3(A*x*(sx*(sx-10.f*sy)+5.f*sy*sy),A*y*(5.f*sx*sx-sy*(10.f*sx-sy)),z_mul*z*(sz*(sz-10.f*R)+5.f*R*R));
    case 6:A=1.f-sz*(sz*(sz-15.f*R)+15.f*R*R)/(R*R*R+1e-40f);return float3(A*(sx*sx*(sx-15.f*sy)+sy*sy*(15.f*sx-sy)),2.f*A*x*y*(sx*(3.f*sx-10.f*sy)+3.f*sy*sy),z_mul*2.f*z*sqrt(R)*(sz*(3.f*sz-10.f*R)+3.f*R*R));
    case 7:A=1.f-7.f*sz*(sz*(sz-5.f*R)+3.f*R*R)/(R*R*R+1e-40f);return float3(A*x*(sx*(sx*(sx-21.f*sy)+35.f*sy*sy)-7.f*sy*sy*sy),A*y*(sx*(sx*(7.f*sx-35.f*sy)+21.f*sy*sy)-sy*sy*sy),-z_mul*z*(sz*sz*sz-7.f*R*(sz*(3.f*sz-5.f*R)+R*R)));
    case 8:{float rr=R*R,zz=sz*sz,xx=sx*sx,yy=sy*sy;
        A=1.f+(zz*(70.f*rr+zz)-28.f*sz*R*(zz+rr))/(rr*rr+1e-40f);
        return float3(A*(xx*xx+yy*(70.f*xx+yy)-28.f*sx*sy*(yy+xx)),8.f*x*y*A*(yy*(7.f*sx-sy)+xx*(sx-7.f*sy)),-z_mul*8.f*z*sqrt(R)*(sz-R)*(zz-6.f*R*sz+rr));}
    default:return float3(NAN);
    }
}

inline bool native_internal_formula_step(thread NativeFormulaState& o,
    constant NativeFormulaSlot& s,thread const NativeResources& resources,
    thread NativeFormulaFault& fault) {
    (void)resources;
    float3 v(o.xyz[0],o.xyz[1],o.xyz[2]),j(o.j[0],o.j[1],o.j[2]);
    float w=o.xyz[3],x=v.x,y=v.y,z=v.z;
    switch(s.id) {
    case 0:
        if(!native_formula_selector_selected(kInternalFormulaSelector|0u)){fault.code=UnsupportedFormula;return false;}
        if(s.internal_power<2 || s.internal_power>8){fault.code=InvalidFormulaParameter;return false;}
        v=internal_int_power(v,s.internal_power,s.negative_params[1])+j;break;
    case 1:{if(!native_formula_selector_selected(kInternalFormulaSelector|1u)){fault.code=UnsupportedFormula;return false;}
        float power=s.negative_params[1],th=atan2(y,x)*power,ph=atan2(z,sqrt(x*x+y*y))*power;
        float r=pow(o.rout,.5f*power);v=float3(r*cos(ph)*cos(th),r*cos(ph)*sin(th),s.negative_params[2]*r*sin(ph))+j;break;}
    case 2:
        if(!native_formula_selector_selected(kInternalFormulaSelector|2u)){fault.code=UnsupportedFormula;return false;}
        v=float3(x*x-y*y-z*z-w*w,2.f*(y*x+z*w),2.f*(z*x+s.negative_params[1]*y*w))+j;
        w=2.f*(w*x+y*z)+s.negative_params[2]+o.j[3];break;
    case 3:if(!native_formula_selector_selected(kInternalFormulaSelector|3u)){fault.code=UnsupportedFormula;return false;}
        v=float3(x*x-y*y-z*z+j.x,2.f*x*y+j.y,s.negative_params[1]*x*z+s.negative_params[2]*j.z);break;
    case 4:if(!native_formula_selector_selected(kInternalFormulaSelector|4u)){fault.code=UnsupportedFormula;return false;}
        v=internal_box(v,s,w,(s.wrapper_flags&2u)!=0)+j;break;
    case 5:{if(!native_formula_selector_selected(kInternalFormulaSelector|5u)){fault.code=UnsupportedFormula;return false;}
        float outer=s.negative_params[9],inner=s.negative_params[10];
        if(o.rout<outer){
            float scale=s.negative_params[8];float3 q=v/scale;
            float R=q.x*q.x+q.y*q.y,A=(R-q.z*q.z)/R;
            float3 bulb=float3(scale*(q.x*q.x-q.y*q.y)*A,2.f*scale*q.x*q.y*A,-2.f*scale*q.z*sqrt(R))+j;
            if(o.rout<inner)v=bulb;
            else {float m=(o.rout-inner)/(outer-inner);float3 box=internal_box(v,s,w,false)+j;
                float target=length(box)*m+(1.f-m)*length(bulb);v=box*m+(1.f-m)*bulb;v*=target/sqrt(dot(v,v)+1e-40f);}
        }else v=internal_box(v,s,w,false)+j;break;}
    case 6:{if(!native_formula_selector_selected(kInternalFormulaSelector|6u)){fault.code=UnsupportedFormula;return false;}
        float fold=s.negative_params[2];
        if(s.internal_power<2 || s.internal_power>8){fault.code=InvalidFormulaParameter;return false;}
        v=float3(internal_fold(x,fold),internal_fold(y,fold),internal_fold(z,fold));
        v=internal_int_power(v,s.internal_power,s.negative_params[1])+j;break;}
    case 7:{if(!native_formula_selector_selected(kInternalFormulaSelector|7u)){fault.code=UnsupportedFormula;return false;}
        uint dst=uint(internal_i32(s,-12))&3u,src=uint(internal_i32(s,-16))&3u;
        float t=(o.xyz[src]-internal_f32(s,-20))*internal_f32(s,-24)*1.5707963267948966f;
        float r=t-rint(t-.5f);r+=r;bool negate=false;
        if(r>1.f){r=2.f-r;negate=!negate;}
        float a=r*3.141592653589793f;
        if(a>=1.5707963267948966f){a=3.141592653589793f-a;negate=!negate;}
        float q=a-.148148148148148f*a*a*a;if(negate)q=-q;
        o.xyz[dst]+=q*internal_f32(s,-28)+internal_f32(s,-32);
        return true;}
    case 8:{if(!native_formula_selector_selected(kInternalFormulaSelector|8u)){fault.code=UnsupportedFormula;return false;}
        float3 normal(s.negative_params[3],s.negative_params[2],s.negative_params[1]);
        float dist=dot(normal,v)-s.negative_params[4];if(!internal_i32(s,-68))dist=abs(dist);
        o.rout=dist;
        if(internal_i32(s,-52)!=0) {
            const uint channel=uint(internal_i32(s,-56))&3u;
            if(channel>=3){fault.code=InvalidFormulaParameter;return false;}
            const float magnitude=max(abs(normal.x),max(abs(normal.y),abs(normal.z)));
            if(magnitude>0.f){normal/=magnitude;normal/=sqrt(dot(normal,normal));}
            float3 ortho=abs(normal.x)>.1f?float3(normal.z,0.f,-normal.x):float3(0.f,-normal.z,normal.y);
            const float length_squared=dot(ortho,ortho);if(length_squared>0.f)ortho/=sqrt(length_squared);
            const float3 ortho2=cross(ortho,normal);
            const float scale=s.negative_params[5];
            const float3 uv(dot(ortho,v)*scale,dot(ortho2,v)*scale,0.f);
            const auto sample=native_sample_direct(uv,internal_i32(s,-52),resources,fault);
            if(fault.code)return false;
            o.dfree1=(sample[channel]+internal_f32(s,-60))*internal_f32(s,-64);
        }
        break;}
    case 9:{if(!native_formula_selector_selected(kInternalFormulaSelector|9u)){fault.code=UnsupportedFormula;return false;}
        float power=s.negative_params[1],r=y*y+(x*x+z*z);
        float th=atan2(sqrt(x*x+z*z),y)*power,ph=atan2(z,x)*power;
        r=pow(r,.5f*power);v=float3(cos(th)*cos(ph)*r,cos(th)*sin(ph)*r,sin(th)*r*s.negative_params[2])+j;
        if(internal_i32(s,-28)){
            float power_c=internal_i32(s,-52)?length(v-j):s.negative_params[4];
            r=length(j);int mode=internal_i32(s,-56);float3 q=(mode&16)?v-j:j,original=q;
            if(mode&8){float tmp=q.y;q.y=q.x;q.x=tmp;}
            float xz=sqrt(q.z*q.z+q.x*q.x);
            th=(mode&1)?atan2(q.y,xz):atan2(xz,q.y);ph=(mode&2)?atan2(q.x,q.z):atan2(q.z,q.x);
            if(mode&4){float tmp=th;th=ph;ph=tmp;}
            float cond=original.x;
            if(mode&4)cond=(mode&8)?original.y:original.z;else if(mode&2)cond=(mode&8)?original.z:original.y;
            if(internal_i32(s,-32) && !(as_type<uint>(cond)&0x80000000u))ph=-ph;
            ph*=power_c;th*=power_c;
            o.j[0]=cos(th)*cos(ph)*r;o.j[1]=cos(th)*sin(ph)*r;o.j[2]=sin(th)*r*s.negative_params[5];
        }break;}
    default:fault.code=UnsupportedFormula;return false;
    }
    o.xyz[0]=v.x;o.xyz[1]=v.y;o.xyz[2]=v.z;o.xyz[3]=w;
    return true;
}
} // namespace mb3d_native
