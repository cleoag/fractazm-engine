#pragma once
// Native camera-aware reflection material (CalcPixelColorSvec). Without a
// reflection material descriptor only the single directional-light subset is
// admitted (the bounded witness). The descriptor adds every enabled light,
// positional lights, squared color, CalcTotalLight2 and material maps. Its
// single-light arithmetic is the witness arithmetic, operation for operation.
constant uint kInsideMaterialFlags=kNativeLightGeometryFlag|kNativeTotalLight2Flag|kNativeMaterialMapFlag;
inline bool valid_reflection_material(constant NativeReflectionMaterial& m,constant NativeInteriorColorParams& p) {
    if(m.version!=kNativeReflectionMaterialVersion||m.mid[3])return false;
    for(uint i=0;i<3;++i)if(!metal::isfinite(f(m.mid[i])))return false;
    const float scalars[]={f(m.step_width),f(m.z_c_mul),f(m.z_corr)};
    for(float v:scalars)if(!metal::isfinite(v)||v<=0)return false;
    if((p.flags&kNativeLightGeometryFlag)&&!valid_light_geometry(m.geometry,p))return false;
    return !(p.flags&kNativeMaterialMapFlag)||valid_material_maps(m.maps,p);
}
// CalcPixelColorSvec's visible light shapes for SR's ray (CamPos relative to
// the scene center, AbsViewVec): the radiance terms (Shade) and the
// transparency descale. A position light's depth follows SR_CalcLightZPos
// for this ray. Inside an absorbing material (bDivOptions) the CPU takes a
// separate branch, which faults here (8) and the host rejects. Outlined, as
// material_pixel, to bound the shade pipeline's compile time.
template<bool Shade> __attribute__((noinline)) void inside_visible_lights(constant ExactColorScene& s,
    constant NativeInteriorColorParams& interior,constant NativeReflectionMaterial& m,ExactSurface p,
    float3 dep,float3 view,float3 cam,float3 absolute,float3 object,bool absorbing,
    thread float3& result,thread float& transparency,thread NativeReflectionRecord& record) {
    const uint zpos=p.depth_packed>>16;
    const float inverseDepth=1.0f/float(int(zpos<32768?32768-zpos:zpos));
    const bool squared=interior.squared!=0,farFog=(interior.flags&4u)!=0;
    for(uint i=0;i<6;++i) {
        constant auto& l=interior.lights[i];constant auto& q=m.geometry.lights[i];
        if(!l.enabled||!(q.options&14u))continue;
        const bool positional=(q.options&1u)!=0;
        const float3 ln=words(l.direction);
        float flux;
        if(positional) {
            // SqrDistSV: squared cross product of the camera-light offset and view.
            const float3 d=cam-ln;
            const float cx=d.y*absolute.z-d.z*absolute.y,cy=d.z*absolute.x-d.x*absolute.z,cz=d.x*absolute.y-d.y*absolute.x;
            flux=(cx*cx+cy*cy)+cz*cz;
        } else {
            if(zpos<32768)continue;
            flux=metal::max(0.0f,1.0f-metal::dot(ln,view));
        }
        float transp=f(q.max_length2)*1e-8f;
        if(!(flux<transp))continue;
        float depthPosition=f(q.depth_position),zPosition=f(q.z_position);
        if(positional) {
            if(metal::dot(ln-cam,absolute)<0.0f)continue;
            if(zpos<32768) {
                // The light is hidden when the surface lies before its sphere.
                const float entry=-metal::sqrt(transp-flux);
                float surface;
                if(metal::fabs(absolute.x)>0.5f)surface=(object.x-ln.x)/absolute.x;
                else if(metal::fabs(absolute.y)>0.5f)surface=(object.y-ln.y)/absolute.y;
                else surface=(object.z-ln.z)/absolute.z;
                if(entry>surface)continue;
            }
            float along;
            if(metal::fabs(absolute.x)>0.5f)along=(ln.x-cam.x)/absolute.x;
            else if(metal::fabs(absolute.y)>0.5f)along=(ln.y-cam.y)/absolute.y;
            else along=(ln.z-cam.z)/absolute.z;
            zPosition=metal::max(0.0f,along);
            const float raw=8388352.0f-f(m.z_c_mul)*(metal::sqrt(zPosition/f(m.step_width)*f(m.z_corr)+1.0f)-1.0f);
            depthPosition=metal::clamp(raw/256.0f,0.0f,32767.0f);
        }
        if(absorbing){record.fault|=8;return;}
        F32 shapeFlux=F32::from_native(flux),shapeTransp=F32::from_native(transp);
        native_light_shape(shapeFlux,shapeTransp,q.options,squared);
        flux=shapeFlux.value;transp=shapeTransp.value;
        float depthFog=1.0f-metal::max(0.0f,1.0f+(depthPosition-28000.0f)*f(s.depth));
        if(depthFog>0.0f){if(squared)depthFog*=depthFog;if(farFog)depthFog*=depthFog;}
        flux*=1.0f-depthFog*0.9f;
        if(Shade) {
            const float3 light=words(l.color);
            const uint shape=q.options&14u;
            if(shape==2||shape==8)result+=light*flux;
            else {
                depthFog=metal::max(0.0f,depthFog);
                const float steps=float(native_fog_steps(p.shadow,interior.flags))*(32768.0f-depthPosition)*inverseDepth;
                const float fog=(((steps-f(s.shadow_start))-f(s.shadow_z_mul)*(zPosition+f(m.geometry.zz_mid_difference)))*
                    f(s.shadow_gradient))*(1.0f-transp)*(1.0f-depthFog);
                const float fogB=metal::min(1.0f,steps*f(s.dynamic_fog_mul))*fog;
                result=((result*transp+dep*(depthFog*(1.0f-transp)))+
                    (words(s.fog_colors[0])*(fog-fogB)+words(s.fog_colors[1])*fogB))+light*flux;
            }
        }
        transparency*=metal::min(1.0f,transp);
    }
    if(!check(transparency,record)||(Shade&&!check(result,record)))return;
}
inline bool valid_inside_material(constant ExactColorScene& s,
                                  constant NativeInteriorColorParams& p,
                                  constant NativeReflectionMaterial* m=nullptr) {
    if(!valid_native_color_base(s)||!valid_interior_color(p,m?kInsideMaterialFlags:0u))return false;
    if(m) {
        if(!valid_reflection_material(*m,p))return false;
        // One hard-shadow bit per light, or a version 7 soft light whose
        // intensity CalcHSsoftPoint stores in the six shadow bits.
        for(uint i=0;i<6;++i) {
            constant auto& l=p.lights[i];
            if(l.enabled&&!(p.version==7&&l.hs_mask==0xffffffffu)&&
               (l.hs_mask<1024||l.hs_mask>32768||(l.hs_mask&(l.hs_mask-1))))return false;
        }
        return true;
    }
    if(p.squared||p.gamma_mode||s.light_pow<1||s.light_pow>1024||
       (s.light_pow&(s.light_pow-1))||s.light_function>3||s.hs_mask!=1024||
       s.hs_calculated>1||s.hs_enabled>1||!p.lights[0].enabled)return false;
    for(uint i=1;i<6;++i)if(p.lights[i].enabled)return false;
    constant auto& light=p.lights[0];
    if(light.power!=s.light_pow||light.function!=s.light_function||
       light.hs_mask!=s.hs_mask||light.hs_calculated!=s.hs_calculated||
       light.hs_enabled!=s.hs_enabled)return false;
    for(uint i=0;i<4;++i)if(light.direction[i]!=s.light_direction[i]||
        light.color[i]!=s.light_color[i])return false;
    return true;
}
inline ColorVec inside_color(float3 v){return {F32::from_native(v.x),F32::from_native(v.y),F32::from_native(v.z)};}
inline float3 inside_float(ColorVec v){return float3(v.x.value,v.y.value,v.z.value);}
inline float inside_luminance(float3 c){return (c.x*0.3f+c.y*0.59f)+c.z*0.11f;}
// specular_w is SDsvecs[0] w, the palette transparency SR transmission reads.
struct InsideMaterial { float3 radiance,specular,diffuse;float transparency,specular_w; };
// Opaque CalcPixelColorSvec/CalcPixelColorSvecTrans share this admitted branch.
// Keep radiance before RGB quantization and carry palette specular separately.
// position is the absolute hit point; it is read only with a descriptor.
// Shade=false is the trace's control subset: palette and transparency,
// without lighting, maps or radiance (left zero). Its caller validates the
// constant descriptors once per pixel; repeating those checks at every trace
// call site overflows the backend compiler on large formula programs.
template<bool Shade=true> __attribute__((noinline)) InsideMaterial inside_material(constant ExactColorScene& s,constant NativeInteriorColorParams& interior,ExactSurface p,float3 view,
                         float y,float zpos,float fogZ,float world_view_y,
                         thread NativeReflectionRecord& record,bool suppress_fog,
                         constant NativeReflectionMaterial* material=nullptr,device const float* samples=nullptr,
                         float3 position=float3(0),float3 cam=float3(0),float3 absolute=float3(0),
                         float scale_amount=0.f) {
    InsideMaterial out={float3(0),float3(0),float3(0),1,0};
    if((Shade&&!valid_inside_material(s,interior,material))||p.fault||
       p.si_gradient>65535||p.amb_shadow>65535||p.shadow>65535||p.o_trap>65535||
       p.reserved[0]||p.reserved[1]||p.reserved[2]){record.fault|=1;return out;}
    if(!check(view,record)||!check(y,record)||!check(zpos,record)||!check(fogZ,record))return out;
    if(interior.flags&64u) {
        if(!check(world_view_y,record))return out;
        y=native_material_object_depth(F32::from_native(world_view_y)).value;
    } else y=native_material_depth(F32::from_native(y),ColorVec{},interior).value; // PreCalcDepthCol / SR's iDfunc row
    const bool squared=interior.squared!=0;
    const bool mapped=material&&(interior.flags&kNativeMaterialMapFlag);
    const bool background=mapped&&(material->maps.background&kNativeBackgroundMap);
    // CalcPixelColorSvecTrans (an absorbing medium) is not this background branch.
    if(background&&suppress_fog&&(p.depth_packed>>16)>=32768){record.fault|=8;return out;}
    const float3 dep=words(s.depth_colors[0])+y*(words(s.depth_colors[1])-words(s.depth_colors[0]));
    float3 result;float depthFactor;
    const uint depth=p.depth_packed>>16;
    if(depth<32768) {
        const float rough=float(p.depth_packed&255)*f(s.roughness);
        float amb=color_ambient(s,p).value;
        if(squared)amb*=amb;
        const float fog=f(s.diffuse_shadowing)*(amb-1.0f)+1.0f;
        const float3 normal=packed_normal(p),mirrored=view-normal*(2.0f*metal::dot(normal,view));
        // SR_CalcRay's ObjPos: the hit point relative to the scene center.
        float3 object(0);
        if(Shade&&material) {
            object=position-float3(f(material->mid[0]),f(material->mid[1]),f(material->mid[2]));
            if(!check(object,record))return out;
        }
        float3 unshadowed(0),lit(0),spe(0);
        for(uint i=0;i<6&&Shade;++i) {
            constant auto& l=interior.lights[i];
            if(!l.enabled) {
                // Light-map light (iLightOption 2): the roughness-blended sphere
                // sample, unshadowed and ambient-weighted.
                if(mapped&&((material->maps.light_mask>>i)&1u)) {
                    constant auto& m=material->maps.lights[i];
                    const ColorVec sampled=color_scale(material_sphere(m,samples,inside_color(normal),squared),F32(255));
                    const float3 value=inside_float(color_mix(color_vec(squared?m.average_squared:m.average),sampled,F32::from_native(rough)));
                    unshadowed+=value;lit+=value*amb;
                }
                continue;
            }
            float3 light=words(l.direction);const float3 lightcol=words(l.color);
            // Positional light: LN holds the position relative to the scene
            // center. Out-of-range lights contribute nothing; the inverse
            // square scales every term.
            const bool positional=material&&(interior.flags&kNativeLightGeometryFlag)&&
                (material->geometry.lights[i].options&1u);
            float distance=1.0f;
            if(positional) {
                constant auto& g=material->geometry;
                float3 toLight=light-object;
                const float length2=metal::dot(toLight,toLight);
                if(!check(length2,record))return out;
                if(length2>f(g.lights[i].max_length2))continue;
                toLight*=1.0f/metal::sqrt(length2);
                light=float3(metal::dot(toLight,words(g.matrix[0])),metal::dot(toLight,words(g.matrix[1])),
                             metal::dot(toLight,words(g.matrix[2])));
                distance=1.0f/(length2+1.0e-30f);
            }
            float weight=native_light_cos(s,l.function,F32::from_native(metal::dot(normal,light)),F32::from_native(rough)).value;
            if(squared)weight*=weight; // GetCosTabValSqr
            if(positional)weight*=distance;
            unshadowed+=lightcol*weight;
            // A soft light is never masked; its terms scale by the intensity.
            const bool soft=interior.version==7&&l.hs_mask==0xffffffffu;
            const bool noShadow=soft||!(p.shadow&l.hs_mask)||!l.hs_calculated;
            const bool subtractAmbient=(l.hs_calculated^l.hs_enabled)!=0;
            const float softScale=float(p.shadow>>10)*(1.0f/63.0f);
            if(noShadow){float c=weight*(subtractAmbient?amb:fog);if(soft)c*=softScale;lit+=lightcol*c;}
            const float reflected=metal::dot(light,mirrored);
            if(reflected>0) {
                const float attenuation=(distance+metal::min(1.0f,rough*2.0f)*(1.0f/float(l.power)-distance))*f(s.specular);
                if(attenuation>0) {
                    unshadowed+=lightcol*(attenuation/float(l.power));
                    if(noShadow) {
                        float power=reflected;for(uint n=l.power>>1;n;n>>=1)power*=power;
                        float c=(attenuation*power)*(subtractAmbient?amb:fog);if(soft)c*=softScale;
                        spe+=lightcol*c;
                    }
                }
            }
        }
        if(Shade&&(!check(unshadowed,record)||!check(lit,record)||!check(spe,record)))return out;
        // The reflection adapter retains its legacy interior ABI and fault
        // reporting. Palette arithmetic itself is shared with primary hits.
        const bool interiorHit=p.si_gradient>=32768;
        if(interiorHit&&interior.version!=6){record.fault|=mb3d_inside::Invalid;return out;}
        const auto palette=native_material_palette(s,interior,
            {p.si_gradient,p.o_trap,F32::from_native(zpos)});
        if(palette.fault){
            if(interiorHit)record.fault|=palette.fault==2?mb3d_inside::NonFinite:mb3d_inside::Invalid;
            else {record.fault|=4;record.nonfinite=1;}
            return out;
        }
        if(interiorHit&&(!isfinite(palette.diffuse.x)||!isfinite(palette.diffuse.y)||!isfinite(palette.diffuse.z)||
            !isfinite(palette.specular.x)||!isfinite(palette.specular.y)||!isfinite(palette.specular.z))){
            record.fault|=mb3d_inside::NonFinite;return out;
        }
        float3 diffuse=float3(palette.diffuse.x.value,palette.diffuse.y.value,palette.diffuse.z.value);
        out.specular=float3(palette.specular.x.value,palette.specular.y.value,palette.specular.z.value);
        out.specular_w=palette.specular_w.value;
        const ColorVec localNormal=inside_color(normal);
        if(Shade&&mapped&&material->maps.diffuse_mode) {
            diffuse=inside_float(native_diffuse_map(material->maps,samples,&material->geometry,inside_color(object),p,
                localNormal,F32::from_native(rough),squared,inside_color(diffuse)));
            if(!check(diffuse,record))return out;
        }
        out.diffuse=diffuse;
        result=float3(0);
        if(Shade) {
        // bScaleAmbDiffDown: light continuing through a transmitting surface
        // leaves less ambient and diffuse light; zero keeps both unscaled.
        const float down=1.0f-out.specular_w*scale_amount;
        if(!check(down,record))return out;
        amb*=down;
        float3 ambient;
        if(mapped&&(material->maps.background&kNativeBackgroundAmbient)) {
            ambient=inside_float(native_background_ambient(material->maps,samples,material->geometry,localNormal,
                inside_color(dep),F32::from_native(amb),color_s(s.depth),squared,(interior.flags&4u)!=0))*diffuse;
        } else {
            const float ambientY=native_ambient_y(mapped?&material->maps:nullptr,&material->geometry,localNormal,interior).value;
            const float ambB=(ambientY*0.5f+0.5f)*amb,ambA=amb-ambB;
            ambient=(words(s.ambient_colors[0])*ambA+words(s.ambient_colors[1])*ambB)*diffuse;
        }
        const float3 scaledDiffuse=diffuse*(f(s.diffuse)*down);
        lit=(ambient+scaledDiffuse*lit)+out.specular*spe;
        if(interior.flags&kNativeTotalLight2Flag) {
            // CalcTotalLight2: a self-lit bounce rescaled toward the lit luminance.
            float bounce=metal::max(0.0f,1.0f-amb);bounce=bounce*bounce*f(s.indirect_reflect);
            const float before=inside_luminance(lit)+4.0f;
            const float3 total=lit+(lit*diffuse)*bounce;
            const float after=metal::max(0.0f,inside_luminance(total))+3.0f;
            result=total*((before+(before+128.0f)*(bounce+0.11f)*0.04f)/after);
        } else {
            unshadowed=ambient+(scaledDiffuse*unshadowed)*(amb*(squared?0.11f:0.3f));
            const float remainder=metal::max(0.0f,1.0f-amb);
            float bounce=remainder*remainder;bounce*=bounce;
            const float3 scale=diffuse*(bounce*f(s.indirect_reflect))+(remainder-bounce);
            result=lit+unshadowed*(diffuse*scale);
        }
        }
        depthFactor=metal::max(0.0f,1.0f+float(int(depth)-28000)*f(s.depth));
    } else {
        // The background map along this ray's AbsViewVec.
        result=Shade&&background?inside_float(native_background_color(material->maps,samples,inside_color(absolute),squared)):dep;
        depthFactor=metal::max(0.0f,1.0f-(60768.0f-float(depth))*f(s.depth));
    }
    if(!check(depthFactor,record))return out;
    if(!suppress_fog) {
    // CalcPixelColorSvec enters this block even without far fog; retain the
    // literal 1-(1-depth) operation order for that branch.
    if(depthFactor<1.0f){float v=1.0f-depthFactor;if(squared)v*=v;if(interior.flags&4u)v*=v;depthFactor=1.0f-v;}
    // bAddBGlight leaves the background map unscaled by the depth fog.
    if(depth<32768||!background||!(material->maps.background&kNativeBackgroundAddLight))result*=depthFactor;
    const float shadow=float(p.shadow&1023);
    float fog=(shadow-f(s.shadow_start)-f(s.shadow_z_mul)*fogZ)*f(s.shadow_gradient);
    if(!check(fog,record))return out;
    if(interior.flags&16u)fog=metal::max(0.0f,fog);
    const float fogMix=shadow*f(s.dynamic_fog_mul);
    if(!check(fogMix,record))return out;
    float fogB=metal::min(1.0f,fogMix)*fog;
    if(!check(fogB,record))return out;
    result+=dep*metal::max(0.0f,1.0f-depthFactor);
    if(interior.flags&32u){
        fog=metal::clamp(fog,0.0f,1.0f);fogB=metal::clamp(fogB,0.0f,1.0f);
        result*=1.0f-fog;out.transparency*=1.0f-fog;
    }
    result+=words(s.fog_colors[0])*(fog-fogB)+words(s.fog_colors[1])*fogB;
    } else depthFactor=1.f; // CalcPixelColorSvecTrans bDivOptions.
    if((Shade&&!check(result,record))||!check(out.specular,record)||!check(depthFactor,record))return out;
    out.transparency*=metal::min(1.0f,depthFactor);
    if(material&&(interior.flags&kNativeLightGeometryFlag)) {
        inside_visible_lights<Shade>(s,interior,*material,p,dep,view,cam,absolute,
            position-float3(f(material->mid[0]),f(material->mid[1]),f(material->mid[2])),suppress_fog,
            result,out.transparency,record);
        if(record.fault)return out;
    }
    out.radiance=metal::clamp(result*(1.0f/255.0f),0.0f,8.0f);
    return out;
}
