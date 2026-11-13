// Included inside the real-device lane test, reusing its bounded dispatcher.
{
    marker("native_camera_multi_shadow_abi",{1,128,32,352,56,32});
    auto multiKernel=pipeline("native_camera_multi_hard_shadow_lanes",true);
    auto singleKernel=pipeline("native_camera_inside_hard_shadow_lanes",true);
    p=program();s=scene();s.max_ray_length=f64(100);s.refinement_steps=0;
    ExactShadowParams sh{};sh.version=1;sh.enabled=1;sh.max_steps=128;
    sh.z_corr=f64(1);sh.z_c_mul=f64(8388351.5);sh.length_multiplier=f64(20);
    sh.max_iterations_result=2;
    // Alternating +/-X: independent plane oracle gives hit/backface bits.
    for(unsigned i=0;i<6;++i)sh.light_vectors[i*3]=f64(i&1?-1:1);
    mb3d_camera::Params camera{};camera.version=1;camera.width=camera.full_width=width;
    camera.height=camera.full_height=height;
    mb3d_inside::Params inside{};inside.version=1;inside.full_width=width;
    std::array<ExactPixel,pixel_count> geom{};
    std::array<ExactSurface,pixel_count> input{};
    for(unsigned i=0;i<pixel_count;++i){
        geom[i].start[0]=f64(4);geom[i].ray[2]=f64(1);
        input[i].normal[0]=-32767;input[i].shadow=0x155;
        input[i].depth_packed=0;input[i].amb_shadow=5000;
        if(i%width==1)input[i].depth_packed=0x80000000u; // decrement across background
    }
    auto gb=buffer(sizeof(geom)),ib=buffer(sizeof(input)),ob=buffer(sizeof(input)+48);
    auto mb=buffer((pixel_count+1)*sizeof(NativeMultiShadowRecord));
    auto cache=buffer(sizeof(NativeMultiShadowRowState)+56);
    auto state=buffer(sizeof(NativeFormulaState)+188);
    auto desc=buffer(sizeof(NativeLogicalLane)),status=buffer(sizeof(NativeLogicalLaneRecord));
    std::memcpy(gb.contents,geom.data(),sizeof(geom));std::memcpy(ib.contents,input.data(),sizeof(input));
    auto dispatchShadow=[&](NativeLogicalLane lane,bool single=false){
        std::memcpy(desc.contents,&lane,sizeof(lane));
        const U32 launch[]{0,1,1,pixel_count},resources[]{0,0},batchRows=height;
        dispatch(single?singleKernel:multiKernel,2,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:gb offset:0 atIndex:0];[e setBuffer:ib offset:0 atIndex:1];
            [e setBuffer:ob offset:0 atIndex:2];[e setBuffer:mb offset:0 atIndex:3];
            [e setBytes:&s length:sizeof(s) atIndex:4];[e setBytes:&p length:sizeof(p) atIndex:5];
            [e setBytes:&sh length:sizeof(sh) atIndex:6];[e setBuffer:maps offset:0 atIndex:7];
            [e setBuffer:data offset:0 atIndex:8];[e setBytes:launch length:sizeof(launch) atIndex:9];
            [e setBytes:resources length:sizeof(resources) atIndex:10];[e setBuffer:desc offset:0 atIndex:11];
            [e setBuffer:state offset:0 atIndex:12];[e setBuffer:cache offset:0 atIndex:13];
            [e setBuffer:status offset:0 atIndex:14];[e setBytes:&batchRows length:4 atIndex:15];
            [e setBytes:&camera length:sizeof(camera) atIndex:23];[e setBytes:&inside length:sizeof(inside) atIndex:27];
        });
        return *static_cast<const NativeLogicalLaneRecord*>(status.contents);
    };
    auto snapshot=[&](unsigned mask,unsigned batch,bool single=false){
        sh.light_mask=mask;
        for(auto b:{ob,mb,cache,state})std::memset(b.contents,0xa5,b.length);
        for(unsigned cursor=0;cursor<pixel_count;){
            const unsigned n=std::min(batch,pixel_count-cursor);
            auto r=dispatchShadow({0,1,height,cursor,n,pixel_count,{0,0}},single);
            require(!r.fault&&r.processed==n&&r.next_cursor==cursor+n,"multi-shadow lane completion");cursor+=n;
        }
        std::vector<unsigned char> result;
        for(auto pair:{std::pair<id<MTLBuffer>,size_t>{ob,sizeof(input)},
            {mb,pixel_count*(single?sizeof(NativeShadowRecord):sizeof(NativeMultiShadowRecord))},
            {state,sizeof(NativeFormulaState)},
            {cache,single?sizeof(NativeShadowRowState):sizeof(NativeMultiShadowRowState)}}){
            auto begin=static_cast<const unsigned char*>(pair.first.contents);
            result.insert(result.end(),begin,begin+pair.second);
            check(std::all_of(begin+pair.second,begin+pair.first.length,[](auto v){return v==0xa5;}),"multi-shadow ownership canary");
        }
        return result;
    };
    for(unsigned mask:{1u,2u,32u,37u,15u,63u}){
        const auto full=snapshot(mask,pixel_count),split=snapshot(mask,2),repeat=snapshot(mask,1);
        check(full==split&&full==repeat,"all shadow image/work/cache/47 state words exact across repeats and dispatch splits");
        auto out=static_cast<const ExactSurface*>(ob.contents);
        auto rec=static_cast<const NativeMultiShadowRecord*>(mb.contents);
        for(unsigned i=0;i<pixel_count;++i){
            check(mb3d::native_metal::ValidMultiShadowRecord(rec[i],input[i],out[i],mask,sh.max_steps),"multi-shadow per-light bounds/sums/surface preservation");
            check(out[i].shadow==(0x155u|((i%width==1?0u:mask)<<10)),"independent signed-plane shadow bit oracle");
            for(unsigned j=0;j<6;++j)if((mask&(1u<<j))&&i%width!=1){
                check(j&1?rec[i].lights[j].backface==1:rec[i].lights[j].hits==1,"independent hit/backface light oracle");
            }
        }
        auto corrupt=rec[0];corrupt.lights[0].steps=sh.max_steps+1;
        check(!mb3d::native_metal::ValidMultiShadowRecord(corrupt,input[0],out[0],mask,sh.max_steps),"per-light exhaustion rejected by host");
        auto changed=out[0];changed.shadow^=(mask&-mask)<<10;
        check(!mb3d::native_metal::ValidMultiShadowRecord(rec[0],input[0],changed,mask,sh.max_steps),"selected bit must match actual light outcome");
        for(unsigned i=0;i<pixel_count;++i)if(i%width!=1)
            check(rec[i].total.eval.formula_work==3*rec[i].total.eval.de_calls-(i==0?1:0),"independent conditional count oracle: formula state is not reset between lights or rows");
    }
    // Same selected light through two distinct device implementations.
    snapshot(1,2);std::array<NativeMultiShadowRecord,pixel_count> multi{};
    std::memcpy(multi.data(),mb.contents,sizeof(multi));
    std::array<ExactSurface,pixel_count> surface{};std::memcpy(surface.data(),ob.contents,sizeof(surface));
    NativeFormulaState finalState{};std::memcpy(&finalState,state.contents,sizeof(finalState));
    snapshot(1,2,true);
    check(!std::memcmp(surface.data(),ob.contents,sizeof(surface))&&!std::memcmp(&finalState,state.contents,sizeof(finalState)),"old single-light and multi path surface/full carried state exact");
    for(unsigned i=0;i<pixel_count;++i)check(!std::memcmp(&multi[i].total,&static_cast<const NativeShadowRecord*>(mb.contents)[i],sizeof(NativeShadowRecord)),"old single-light full work record exact");
    // Perpendicular directions leave signed-plane distance constant and must
    // establish the open-air cache before a later foreground pixel skips.
    auto originalShadow=sh;
    for(unsigned i=2;i<6;++i){sh.light_vectors[3*i]=0;sh.light_vectors[3*i+1+i/4]=f64(i&1?-1:1);}
    const auto mixed=snapshot(63,pixel_count),mixedSplit=snapshot(63,2);
    check(mixed==mixedSplit,"hit/miss/open-air cache exact across a mid-row dispatch split");
    auto mixedWork=static_cast<const NativeMultiShadowRecord*>(mb.contents);
    auto mixedSurface=static_cast<const ExactSurface*>(ob.contents);
    unsigned misses=0,skips=0;
    for(unsigned i=0;i<pixel_count;++i){
        check(mb3d::native_metal::ValidMultiShadowRecord(mixedWork[i],input[i],mixedSurface[i],63,sh.max_steps),"mixed light record bounds and output-bit binding");
        check(mixedSurface[i].shadow==(0x155u|((i%width==1?0u:3u)<<10)),"independent plane miss directions preserve clear bits");
        misses+=mixedWork[i].total.misses;skips+=mixedWork[i].total.skipped;
        if(i%width!=1)check(mixedWork[i].total.eval.formula_work==3*mixedWork[i].total.eval.de_calls-(i==0?1:0),"conditional state survives miss and cache branches");
    }
    check(misses>0&&skips>0,"real misses and open-air reuse exercised");
    sh=originalShadow;
    auto reject=[&](const char* label){
        std::memset(ob.contents,0xa5,ob.length);
        auto r=dispatchShadow({0,1,height,0,1,pixel_count,{0,0}});
        check(r.fault!=0&&r.processed==0,label);
        auto b=static_cast<const unsigned char*>(ob.contents);
        check(std::all_of(b,b+ob.length,[](auto v){return v==0xa5;}),"invalid multi descriptor publishes no surface");
    };
    {
        // Position lights (calcHShard): lights far along -X/+X march like the
        // matching directional lights; past the squared reach a light is
        // shadowed without a march and records beyond.
        sh.position_mask=3;sh.step_width=f64(1);
        for(unsigned j=0;j<2;++j){sh.light_vectors[3*j]=0;sh.light_positions[3*j]=f64(j?4+1e6:4-1e6);
            sh.light_reach[j]=std::bit_cast<U32>(1e13f);}
        const auto near=snapshot(3,pixel_count),nearSplit=snapshot(3,2);
        check(near==nearSplit,"position lights exact across dispatch splits");
        auto out=static_cast<const ExactSurface*>(ob.contents);
        auto rec=static_cast<const NativeMultiShadowRecord*>(mb.contents);
        for(unsigned i=0;i<pixel_count;++i){
            check(mb3d::native_metal::ValidMultiShadowRecord(rec[i],input[i],out[i],3,sh.max_steps,false,3),"position light record bounds");
            check(out[i].shadow==(0x155u|((i%width==1?0u:3u)<<10)),"position lights match the directional oracle");
            if(i%width!=1)check(rec[i].lights[0].hits==1&&rec[i].lights[1].backface==1&&
                !rec[i].lights[0].beyond&&!rec[i].lights[1].beyond,"position light hit/backface oracle");
        }
        sh.light_reach[0]=0;snapshot(3,pixel_count);
        for(unsigned i=0;i<pixel_count;++i){
            check(mb3d::native_metal::ValidMultiShadowRecord(rec[i],input[i],out[i],3,sh.max_steps,false,3),"beyond record bounds");
            check(out[i].shadow==(0x155u|((i%width==1?0u:3u)<<10)),"beyond light keeps its shadow bit");
            if(i%width!=1)check(rec[i].lights[0].beyond==1&&!rec[i].lights[0].rays&&!rec[i].lights[0].steps,"beyond light marches nothing");
            if(i%width!=1)check(!mb3d::native_metal::ValidMultiShadowRecord(rec[i],input[i],out[i],3,sh.max_steps,false,2),
                "beyond outside the position mask rejected by host");
        }
        sh.light_mask=3;
        sh.position_mask=7;reject("position light outside the light mask rejected");sh.position_mask=3;
        sh.step_width=0;reject("position light without step width rejected");sh.step_width=f64(1);
        sh.light_reach[0]=std::bit_cast<U32>(__builtin_nanf(""));reject("nonfinite reach rejected");sh.light_reach[0]=0;
        sh.light_mask=1;std::memset(ob.contents,0xa5,ob.length);
        auto single=dispatchShadow({0,1,height,0,1,pixel_count,{0,0}},true);
        check(single.fault!=0&&!single.processed,"single-light entry rejects position lights");
        sh=originalShadow;
    }
    sh.light_mask=64;reject("multi mask out of range rejected");
    sh.light_mask=2;auto saved=sh.light_vectors[3];sh.light_vectors[3]=0;reject("selected zero light rejected");
    sh.light_vectors[3]=f64(__builtin_nan(""));reject("nonfinite light rejected");sh.light_vectors[3]=saved;
    sh.version=2;reject("stale shadow descriptor rejected");sh.version=1;
    snapshot(15,2);
    auto* bad=static_cast<NativeMultiShadowRowState*>(cache.contents);bad->threshold[5]=4;
    auto r=dispatchShadow({0,1,height,1,1,pixel_count,{0,0}});
    check(r.fault!=0&&!r.processed,"corrupted later-light cache rejected before writes");
}
