#pragma once
#include "scene_types.h"
#include "scene_catalog.h"
#include "formula_math.h"
#include "bulb_math.h"
#include "normal_abi.h"

namespace mb3d_metal_exact {
// Admit only implemented formula capabilities with bounded, finite render constants.
// Formula selection is uniform; all capabilities use the same ray marcher.
inline bool valid_scene(ExactScene s) {
    if(s.formula_kind!=selected_formula(s)||s.bulb.power!=selected_bulb_power(s)||
       s.capability_id!=selected_capability(s)||
       s.version!=4||s.fog_iteration||s.color_on_it||s.color_radius_stop||s.width==0||s.height==0||s.width>4096||s.height>4096||
       s.max_steps==0||s.max_steps>65536||!valid_formula_scene(s)||
       s.min_iterations!=1||s.refinement_steps!=8) return false;
    for(int i=0;i<9;++i) if(!isfinite(D(s.vgrads[i])))return false;
    for(int i=0;i<3;++i) if(!isfinite(D(s.y_start[i])))return false;
    if(!isfinite(D(s.max_ray_length))||D(s.max_ray_length)<=F64(0)||
       !isfinite(S(s.de_stop))||S(s.de_stop)<=F64(0)||
       !isfinite(S(s.de_stop_factor))||S(s.de_stop_factor)<F64(0)||
       !isfinite(S(s.z_step_div))||S(s.z_step_div)<=F64(0)||
       !isfinite(S(s.step_limiter))||S(s.step_limiter)<=F64(0)||
       !isfinite(S(s.de_scale))||S(s.de_scale)<=F64(0)||
       !isfinite(S(s.step_width_103))||S(s.step_width_103)<=F64(0)||
       !isfinite(S(s.de_sub)))return false;
    return true;
}
struct ExactNormalResult { Vec normal; F64 trap; };
inline ExactNormalResult raw_normal_with_trap(Vec p,F64 depth,F64 stop,ExactScene s) {
    F64 off=(S(s.de_stop)<F64(1)?S(s.de_stop):F64(1))*(F64(1)+depth*S(s.de_stop_factor))*D(0x3fc3333333333333ULL);
    F64 components[3],trap(0);
    // Production evaluates Z, X, Y, and leaves orbit color at the Y-minus
    // probe even though it restores C1/C2/C3 after packing the normal.
    for(int probe=0;probe<3;++probe) {
        int i=probe==0?2:probe==1?0:1;
        Vec d=column(s,i);Vec plus=add(p,mul(d,off));
        F64 a=distance(plus,stop,s).distance;
        Vec minus=add(plus,mul(d,F64(-2)*off));
        if(probe==2) {
            const auto last=distance_with_trap(minus,stop,s);
            components[i]=(a-last.distance)*D(0x3fe0000000000000ULL);
            trap=last.trap;
        } else components[i]=(a-distance(minus,stop,s).distance)*D(0x3fe0000000000000ULL);
    }
    return {{components[0],components[1],components[2]},trap};
}
inline Vec raw_normal(Vec p,F64 depth,F64 stop,ExactScene s) {
    return raw_normal_with_trap(p,depth,stop,s).normal;
}
// Implemented with 64-significand-bit Extended operations, retaining the
// original intermediate rounding and explicit Double/Single stores.
inline Vec refine_position(Vec p,Vec ray,F64 step) {
    return {(E80(ray.x)*E80(step)+E80(p.x)).to_f64(),
            (E80(ray.y)*E80(step)+E80(p.y)).to_f64(),
            (E80(ray.z)*E80(step)+E80(p.z)).to_f64()};
}
inline ExactU32 maximum_iteration_result(ExactScene s) {
    return s.max_iterations+(selected_formula(s)==3?0u:1u);
}
struct IterationRefinement {
    Vec position;
    F64 depth,smooth;
    ExactU32 iteration;
};
inline F64 iteration_boundary_step(F64 target,F64 last,F64 smooth,F64 multiplier) {
    const F64 difference=single(last-smooth);
    F64 step;
    if(abs(difference)<D(0x39b4484bfeebc2a0ULL))
        step=(last<smooth?F64(1):F64(0))-D(0x3fe0000000000000ULL);
    else {
        const F64 numerator=single(target-smooth);
        const F64 denominator=single(difference*(difference<F64(0)?F64(500):F64(1000)));
        step=single(numerator/denominator);
    }
    if(step>F64(4))step=sqrt(step)*F64(2);
    else if(step<F64(-9))step=sqrt(-step)*F64(-3);
    const F64 product=step*multiplier;
    return (E80(product)+E80::from_bits(0x3ff4,0x83126e978d4fdf3bULL)).to_f64();
}
// Production RMdoBinSearchIt. Its DE stop stays unchanged while maxIt is
// temporarily increased, and all ordinary vector additions remain binary64.
inline IterationRefinement refine_iteration_boundary(Vec p,Vec ray,F64 depth,F64 stop,ExactScene s) {
    const F64 target=single(F64(s.max_iterations)-D(0x3fefae147ae147aeULL));
    const ExactU32 temporary_max=s.max_iterations+1;
    F64 step(0),multiplier(1),last(0),last_difference(0),smooth(0);
    ExactU32 iteration=0;
    bool first=true;
    for(ExactU32 pass=0;pass<=s.refinement_steps;++pass) {
        depth=depth+step;
        p=add(p,mul(ray,step));
        auto value=bulb_distance(p,stop,s,true,temporary_max);
        smooth=value.state.smooth;iteration=value.iteration;
        if(!first&&last_difference<abs(single(target-smooth))) {
            depth=depth-step;
            p=add(p,mul(ray,-step));
            smooth=last;
            multiplier=multiplier*(step>F64(0)?D(0x3fe0000000000000ULL):D(0x3fe6666666666666ULL));
        }
        last_difference=abs(single(target-smooth));last=smooth;
        if(smooth>single(F64(temporary_max)-S(0x3dcccccdU)))step=F64(-3);
        else {
            depth=(E80(depth)-E80::from_bits(0x3ff5,0x83126e978d4fdf3bULL)).to_f64();
            p=add(p,mul(ray,D(0xbf50624dd2f1a9fcULL)));
            value=bulb_distance(p,stop,s,true,temporary_max);
            smooth=value.state.smooth;iteration=value.iteration;
            step=iteration_boundary_step(target,last,smooth,multiplier);
        }
        first=false;
    }
    return {p,depth,smooth,iteration};
}
template<bool InlineNormals>
inline ExactPixel render_primary_impl(ExactScene s,ExactTrig tx,ExactTrig ty,ExactU32 x,ExactU32 y) {
    ExactPixel out={};
    if(!valid_scene(s)||x>=s.width||y>=s.height||
       !valid_camera_trig(tx)||!valid_camera_trig(ty)) {out.fault=1;return out;}
    Vec ray=camera_ray(tx,ty,s);
    Vec start=add(add(mul(column(s,0),F64(x)),mul(column(s,1),F64(y))),{D(s.y_start[0]),D(s.y_start[1]),D(s.y_start[2])});
    out.ray[0]=ray.x.to_bits();out.ray[1]=ray.y.to_bits();out.ray[2]=ray.z.to_bits();
    out.start[0]=start.x.to_bits();out.start[1]=start.y.to_bits();out.start[2]=start.z.to_bits();
    if(!finite_vec(ray)||!finite_vec(start)) {out.fault=4;return out;}
    Vec p=add(start,mul(ray,F64(0)));Vec act=start;
    F64 length=dot(ray,ray),zzmul=length/sqrt(length*length);
    F64 z(0),stop=S(s.de_stop),count(0),rsf(1),finalDE(0),axial(0);
    DE result=distance(p,stop,s);F64 de=result.distance;
    if(!isfinite(de)||!isfinite(zzmul)) {out.fault=4;return out;}
    const ExactU32 maximum=maximum_iteration_result(s);
    if(result.iteration>=maximum||de<stop) {
        out.rm_result=de<stop?1:2;out.start_inside=1;act=p;
    }
    else {
        F64 lastStep=de*S(s.z_step_div),lastDE(0);bool ended=false;
        for(ExactU32 k=0;k<s.max_steps;++k) {
            if(result.iteration>=maximum) {
                const F64 step=lastStep*D(0xbfe0000000000000ULL);
                z=z+step;p=add(p,mul(ray,step));
                stop=single(S(s.de_stop)*(F64(1)+z*S(s.de_stop_factor)));
                result=distance(p,stop,s);de=result.distance;lastStep=-step;
                if(!isfinite(de)||!isfinite(stop)) {out.fault=4;return out;}
            }
            if(result.iteration<s.min_iterations||(result.iteration<maximum&&de>=stop)) {
                lastDE=de;
                de=pasmax(S(0x3de147aeU),single(((de-S(s.de_sub)*stop)*S(s.z_step_div))*rsf)); // s011f = 0.11f
                F64 limit=single(pasmax(stop,S(0x3ecccccdU))*S(s.step_limiter));
                count=single(count+(limit<de?single(limit/de):F64(1)));
                if(limit<de)de=limit;
                z=z+de;
                if(z>D(s.max_ray_length)) {ended=true;break;}
                lastStep=de;p=add(p,mul(ray,de));
                stop=single(S(s.de_stop)*(F64(1)+clamp0(z*zzmul)*S(s.de_stop_factor)));
                result=distance(p,stop,s);de=result.distance;
                if(!isfinite(de)||!isfinite(stop)) {out.fault=4;return out;}
                if(de>lastDE+lastStep)de=lastDE+lastStep;
                if(lastDE>de+D(0x39b4484bfeebc2a0ULL)) {F64 ratio=lastStep/(lastDE-de);rsf=ratio<F64(1)?pasmax(S(0x3f000000U),single(ratio)):F64(1);}
                else rsf=F64(1);
            } else {
                finalDE=de;
                const bool de_limited=result.iteration<maximum||de<stop;
                if(de_limited) {
                    F64 step=lastStep*D(0xbfe0000000000000ULL);
                    for(ExactU32 n=s.refinement_steps;n>0&&abs(de-stop)>D(0x3f50624dd2f1a9fcULL);--n) {
                        z=(E80(z)+E80(step)).to_f64();p=refine_position(p,ray,step);
                        stop=S(((E80(z)*E80(S(s.de_stop_factor))+E80(F64(1)))*E80(S(s.de_stop))).to_f32_bits());
                        if(n<=1)break;
                        result=distance(p,stop,s);de=result.distance;
                        if(!isfinite(de)||!isfinite(stop)) {out.fault=4;return out;}
                        step=abs(step)*(de<stop?-S(0x3f0ccccdU):S(0x3f0ccccdU));
                    }
                } else {
                    // dIFS returns at most max_iterations, so only internal
                    // formulas can reach the maximum-iteration boundary.
                    if(selected_formula(s)!=3){out.fault=8;return out;}
                    const auto refined=refine_iteration_boundary(p,ray,z,stop,s);
                    p=refined.position;z=refined.depth;result.iteration=refined.iteration;
                }
                axial=clamp0(z*zzmul);act=p;out.rm_result=de_limited?1:2;ended=true;break;
            }
        }
        if(!ended)out.fault=2;
        if(!out.rm_result)axial=clamp0(z*zzmul);
    }
    out.position[0]=act.x.to_bits();out.position[1]=act.y.to_bits();out.position[2]=act.z.to_bits();
    out.depth=z.to_bits();out.axial_depth=axial.to_bits();out.final_de=finalDE.to_bits();
    out.final_stop=stop.to_f32_bits();
    out.step_count=count.to_f32_bits();out.final_iterations=result.iteration;
    if(out.rm_result&&!out.start_inside) {
        out.smooth_iterations=selected_formula(s)==3?
            bulb_distance(act,stop,s,true).state.smooth.to_f32_bits():
            F64(distance(act,stop,s).iteration).to_f32_bits();
        if(InlineNormals) {
            const auto normal=raw_normal_with_trap(act,z,stop,s);
            const Vec n=normal.normal;out.orbit_trap=normal.trap.to_bits();
            out.raw_normal[0]=n.x.to_bits();out.raw_normal[1]=n.y.to_bits();out.raw_normal[2]=n.z.to_bits();
            E80 ex(n.x),ey(n.y),ez(n.z);
            E80 scale=E80(F64(32767))/sqrt(ex*ex+ey*ey+ez*ez+E80(D(0x2b2bff2ee48e0530ULL)));
            out.normal[0]=(ex*scale).to_i32_rne();out.normal[1]=(ey*scale).to_i32_rne();out.normal[2]=(ez*scale).to_i32_rne();
            // Original FISTP m16 uses the integer-indefinite word when the rounded
            // value is outside signed 16-bit range.
            for(int i=0;i<3;++i)if(out.normal[i]<-32768||out.normal[i]>32767)out.normal[i]=-32768;
            if(out.normal[0]==0&&out.normal[1]==0)out.normal[2]=out.normal[2]>0?32767:-32767;
        }
    }
    if(!isfinite(z)||!isfinite(axial)||!isfinite(finalDE)||!isfinite(stop)||!isfinite(de)||
       !finite_vec(act)||!isfinite(S(out.step_count))||!isfinite(S(out.smooth_iterations)))out.fault|=4;
    if(out.rm_result&&!out.start_inside&&!isfinite(D(out.orbit_trap)))out.fault|=4;
    for(int i=0;i<3;++i)if(!isfinite(D(out.raw_normal[i])))out.fault|=4;
    return out;
}
// Preserve the monolithic kernel as a same-binary scheduling/normal baseline.
inline ExactPixel render_primary(ExactScene s,ExactTrig tx,ExactTrig ty,ExactU32 x,ExactU32 y) {
    return render_primary_impl<true>(s,tx,ty,x,y);
}
inline ExactPixel render_primary_geometry(ExactScene s,ExactTrig tx,ExactTrig ty,ExactU32 x,ExactU32 y) {
    return render_primary_impl<false>(s,tx,ty,x,y);
}

struct ExactNormalProbeResult { F64 distance,trap; };
inline ExactNormalProbeResult normal_probe(ExactPixel pixel,ExactScene s,ExactU32 probe) {
    if(probe>=6||!pixel.rm_result||pixel.start_inside)return {F64(0),F64(0)};
    const Vec p={D(pixel.position[0]),D(pixel.position[1]),D(pixel.position[2])};
    const F64 depth=D(pixel.depth),stop=S(pixel.final_stop);
    const F64 off=(S(s.de_stop)<F64(1)?S(s.de_stop):F64(1))*
        (F64(1)+depth*S(s.de_stop_factor))*D(0x3fc3333333333333ULL);
    const ExactU32 axis=probe<2?2:probe<4?0:1;
    const Vec direction=column(s,int(axis));
    Vec position=add(p,mul(direction,off));
    // Minus is deliberately constructed from the rounded plus coordinate.
    // Computing p-direction*off would change double rounding and final RGB.
    if(probe&1u)position=add(position,mul(direction,F64(-2)*off));
    if(probe==5) {
        const auto last=distance_with_trap(position,stop,s);
        return {last.distance,last.trap};
    }
    return {distance(position,stop,s).distance,F64(0)};
}
inline ExactPixel finish_normal(ExactPixel pixel,ExactNormalSamples samples) {
    if(!pixel.rm_result||pixel.start_inside)return pixel;
    // Retain the monolithic path's arithmetic grouping and x/y/z packing.
    const F64 halfScale=D(0x3fe0000000000000ULL);
    const Vec n={(D(samples.distance[2])-D(samples.distance[3]))*halfScale,
                 (D(samples.distance[4])-D(samples.distance[5]))*halfScale,
                 (D(samples.distance[0])-D(samples.distance[1]))*halfScale};
    pixel.orbit_trap=samples.orbit_trap;
    pixel.raw_normal[0]=n.x.to_bits();pixel.raw_normal[1]=n.y.to_bits();pixel.raw_normal[2]=n.z.to_bits();
    E80 ex(n.x),ey(n.y),ez(n.z);
    const E80 scale=E80(F64(32767))/sqrt(ex*ex+ey*ey+ez*ez+E80(D(0x2b2bff2ee48e0530ULL)));
    pixel.normal[0]=(ex*scale).to_i32_rne();
    pixel.normal[1]=(ey*scale).to_i32_rne();
    pixel.normal[2]=(ez*scale).to_i32_rne();
    for(int i=0;i<3;++i)if(pixel.normal[i]<-32768||pixel.normal[i]>32767)pixel.normal[i]=-32768;
    if(pixel.normal[0]==0&&pixel.normal[1]==0)pixel.normal[2]=pixel.normal[2]>0?32767:-32767;
    if(!isfinite(D(pixel.orbit_trap))||!finite_vec(n))pixel.fault|=4;
    return pixel;
}
} // namespace
