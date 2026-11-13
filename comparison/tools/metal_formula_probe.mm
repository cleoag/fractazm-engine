// Native, offline-compiled Metal feasibility probe. Deliberately independent of
// production rendering: no M3P input, runtime formula compiler or CPU fallback.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "comparison/metal_probe/probe_math.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <sys/sysctl.h>
namespace box_reference {
#include "tests/fixtures/boxifs_twist_regressions.h"
}
using Clock=std::chrono::steady_clock;
static double elapsed(Clock::time_point t) { return std::chrono::duration<double,std::milli>(Clock::now()-t).count(); }
static NSString* ns(const std::string& s) { return [NSString stringWithUTF8String:s.c_str()]; }
static void require(bool condition,const std::string& reason) { if(!condition) throw std::runtime_error(reason); }
static double median(std::vector<double> values) {
    std::sort(values.begin(),values.end());size_t middle=values.size()/2;
    return values.size()%2 ? values.at(middle) : (values.at(middle-1)+values.at(middle))*.5;
}
static id number(double x) {
    return std::isnan(x)?(id)@"NaN":std::isinf(x)?(x>0?(id)@"+Infinity":(id)@"-Infinity"):(id)@(x);
}
static std::string bits(float x) { char text[16];std::snprintf(text,sizeof(text),"%08x",std::bit_cast<uint32_t>(x));return text; }
static void writeJSON(const std::filesystem::path& path,id value) {
    NSError* error=nil;
    NSData* data=[NSJSONSerialization dataWithJSONObject:value options:NSJSONWritingPrettyPrinted|NSJSONWritingSortedKeys error:&error];
    require(data!=nil,error?error.description.UTF8String:"JSON encoding failed");
    require([data writeToFile:ns(path.string()) options:NSDataWritingWithoutOverwriting error:&error],
            error?error.description.UTF8String:"JSON write failed");
}

struct NumericStats {
    uint64_t values=0,exact=0,strict_failures=0,class_mismatches=0,zero_sign_mismatches=0;
    double max_abs=0,max_relative=0,sum_abs=0;
    void add(double actual,double expected) {
        ++values;
        bool same_class=(std::isnan(actual)&&std::isnan(expected)) ||
            (std::isinf(actual)&&actual==expected) || (std::isfinite(actual)&&std::isfinite(expected));
        if(!same_class) { ++class_mismatches;++strict_failures;return; }
        if(std::isnan(actual)||std::isinf(actual)) { ++exact;return; }
        if(actual==0&&expected==0&&std::signbit(actual)!=std::signbit(expected)) ++zero_sign_mismatches;
        if(actual==expected && (actual!=0 || std::signbit(actual)==std::signbit(expected))) ++exact;
        double error=std::abs(actual-expected);
        double relative=error/std::max(1.,std::abs(expected));
        max_abs=std::max(max_abs,error);max_relative=std::max(max_relative,relative);sum_abs+=error;
        if(error>2e-12*std::max(1.,std::abs(expected))) ++strict_failures;
    }
    NSDictionary* json() const {
        return @{ @"values":@(values),@"exact_value_and_zero_sign":@(exact),@"strict_gate_failures":@(strict_failures),
            @"classification_mismatches":@(class_mismatches),@"zero_sign_mismatches":@(zero_sign_mismatches),
            @"max_absolute_error":number(max_abs),@"max_error_over_max_1_abs_reference":number(max_relative),
            @"mean_absolute_error_over_all_values":number(values?sum_abs/values:0),
            @"gate":@"finite: abs(actual-reference) <= 2e-12 * max(1, abs(reference)); NaN classification; signed infinity. Signed zero measured separately." };
    }
};
struct Options {
    std::filesystem::path library,output;
    uint32_t width=256,height=192,threads=6,repeats=3;
};
static Options parse(int argc,char** argv) {
    Options o;
    for(int i=1;i<argc;++i) {
        std::string key=argv[i];
        if(key=="--help") {
            std::puts("metal_formula_probe --library OFFLINE.metallib --output NEW_DIRECTORY [--width 256 --height 192 --threads 6 --repeats 3]\nEXPERIMENTAL_FP32: synthetic SphereIFS, boxIFS twist and six-iteration Menger3. Not production M3P/Pascal parity. Outputs report.json and GPU/CPU FP32/CPU FP64 PPMs. Numerical disagreements are measurements; infrastructure failure exits nonzero.");
            std::exit(0);
        }
        require(i+1<argc,"missing value for "+key);std::string value=argv[++i];
        if(key=="--library") o.library=value;
        else if(key=="--output") o.output=value;
        else {
            size_t used=0;unsigned long n=std::stoul(value,&used);
            require(used==value.size()&&n<=UINT32_MAX,"invalid integer for "+key);
            if(key=="--width") o.width=uint32_t(n);else if(key=="--height") o.height=uint32_t(n);
            else if(key=="--threads") o.threads=uint32_t(n);else if(key=="--repeats") o.repeats=uint32_t(n);
            else throw std::runtime_error("unknown option "+key);
        }
    }
    require(!o.library.empty()&&!o.output.empty(),"--library and --output are required");
    require(o.width>=16&&o.height>=16&&o.width<=4096&&o.height<=4096,"dimensions must be 16..4096");
    require(o.threads>=1&&o.threads<=64&&o.repeats>=3&&o.repeats<=15,"threads must be 1..64, repeats 3..15");
    require(!std::filesystem::exists(o.output),"output already exists: refusing to overwrite evidence");
    return o;
}
struct Metal {
    id<MTLDevice> device;
    id<MTLLibrary> library;
    id<MTLCommandQueue> queue;
    explicit Metal(const std::filesystem::path& path) {
        device=MTLCreateSystemDefaultDevice();require(device!=nil,"Metal device unavailable");
        NSError* error=nil;
        library=[device newLibraryWithURL:[NSURL fileURLWithPath:ns(std::filesystem::absolute(path).string())] error:&error];
        require(library!=nil,error?error.description.UTF8String:"offline library unavailable");
        queue=[device newCommandQueue];require(queue!=nil,"Metal queue unavailable");
    }
    id<MTLComputePipelineState> pipeline(NSString* name) {
        NSError* error=nil;id<MTLFunction> function=[library newFunctionWithName:name];
        require(function!=nil,"missing offline kernel "+std::string(name.UTF8String));
        auto p=[device newComputePipelineStateWithFunction:function error:&error];
        require(p!=nil,error?error.description.UTF8String:"pipeline creation failed");return p;
    }
    id<MTLBuffer> buffer(size_t size,const void* data=nullptr) {
        auto b=data?[device newBufferWithBytes:data length:size options:MTLResourceStorageModeShared]:
                    [device newBufferWithLength:size options:MTLResourceStorageModeShared];
        require(b!=nil,"Metal buffer allocation failed");return b;
    }
    double run(id<MTLComputePipelineState> pipeline,id<MTLBuffer> a,id<MTLBuffer> b,
               const void* params,size_t paramSize,uint32_t width,uint32_t height=1) {
        auto command=[queue commandBuffer];auto encoder=[command computeCommandEncoder];
        require(command!=nil&&encoder!=nil,"command/encoder unavailable");
        [encoder setComputePipelineState:pipeline];[encoder setBuffer:a offset:0 atIndex:0];
        if(b) [encoder setBuffer:b offset:0 atIndex:1];
        [encoder setBytes:params length:paramSize atIndex:b?2:1];
        NSUInteger tw=pipeline.threadExecutionWidth;
        NSUInteger th=height==1?1:std::min<NSUInteger>(4,pipeline.maxTotalThreadsPerThreadgroup/tw);
        [encoder dispatchThreads:MTLSizeMake(width,height,1) threadsPerThreadgroup:MTLSizeMake(tw,th,1)];
        [encoder endEncoding];[command commit];[command waitUntilCompleted];
        require(command.status==MTLCommandBufferStatusCompleted,command.error?command.error.description.UTF8String:"GPU command failed");
        require(command.GPUEndTime>=command.GPUStartTime&&command.GPUStartTime>0,"GPU timestamps unavailable");
        return (command.GPUEndTime-command.GPUStartTime)*1000;
    }
};

template<class T,int Formula> static double cpuRender(const ProbeScene& scene,uint32_t threads,std::vector<ProbePixel>& pixels) {
    auto start=Clock::now();std::atomic<uint32_t> nextRow{0};std::vector<std::thread> workers;
    for(uint32_t t=0;t<threads;++t) workers.emplace_back([&] {
        uint32_t y;while((y=nextRow.fetch_add(1,std::memory_order_relaxed))<scene.height)
            for(uint32_t x=0;x<scene.width;++x) pixels[size_t(y)*scene.width+x]=render_pixel<T,Formula>(x,y,scene);
    });
    for(auto& w:workers) w.join();return elapsed(start);
}
static uint8_t channel(float c) { return uint8_t(std::clamp(c,0.f,1.f)*255.f+.5f); }
static void saveImage(const std::filesystem::path& path,const ProbeScene& scene,const std::vector<ProbePixel>& pixels) {
    std::ofstream out(path,std::ios::binary);require(bool(out),"cannot create "+path.string());
    out<<"P6\n"<<scene.width<<" "<<scene.height<<"\n255\n";
    for(auto p:pixels) { uint8_t rgb[]={channel(p.red),channel(p.green),channel(p.blue)};out.write(reinterpret_cast<char*>(rgb),3); }
    require(bool(out),"image write failed");
}
static NSDictionary* comparePixels(const std::vector<ProbePixel>& actual,const std::vector<ProbePixel>& expected) {
    NumericStats rgb,depth,normal,ao;uint64_t differentBytes=0,differentPixels=0,hits=0,steps=0;
    unsigned maxChannel=0;
    for(size_t i=0;i<actual.size();++i) {
        const auto &a=actual[i],&e=expected[i];
        hits+=a.hit!=e.hit;steps+=a.steps!=e.steps;
        float ac[]={a.red,a.green,a.blue},ec[]={e.red,e.green,e.blue};bool changed=false;
        for(int c=0;c<3;++c) { rgb.add(ac[c],ec[c]);unsigned delta=unsigned(std::abs(int(channel(ac[c]))-int(channel(ec[c]))));
            maxChannel=std::max(maxChannel,delta);differentBytes+=delta!=0;changed|=delta!=0; }
        differentPixels+=changed;depth.add(a.depth,e.depth);
        if(a.hit&&e.hit) { normal.add(a.nx,e.nx);normal.add(a.ny,e.ny);normal.add(a.nz,e.nz);ao.add(a.ao,e.ao); }
    }
    return @{ @"pixels":@(actual.size()),@"hit_disagreements":@(hits),@"ray_step_disagreements":@(steps),
        @"different_rgb8_pixels":@(differentPixels),@"different_rgb8_channels":@(differentBytes),@"max_rgb8_channel_delta":@(maxChannel),
        @"linear_output_rgb":rgb.json(),@"depth":depth.json(),@"normals_on_mutual_hits":normal.json(),@"ao_on_mutual_hits":ao.json() };
}
static NSDictionary* imageCounts(const std::vector<ProbePixel>& pixels) {
    uint64_t hits=0,calls=0,steps=0,bounded=0,shadowBounded=0;uint32_t low=UINT32_MAX,high=0;
    for(const auto& p:pixels) {
        require((p.fault&1)==0,"nonfinite computation or ABI rejection in image");
        require(std::isfinite(p.red)&&std::isfinite(p.green)&&std::isfinite(p.blue)&&std::isfinite(p.depth)&&
                std::isfinite(p.nx)&&std::isfinite(p.ny)&&std::isfinite(p.nz)&&std::isfinite(p.ao),"nonfinite pixel buffer");
        require(p.hit<=1&&p.fault<=6,"corrupt/unwritten pixel buffer");
        hits+=p.hit;calls+=p.de_calls;steps+=p.steps;bounded+=(p.fault&2)!=0;shadowBounded+=(p.fault&4)!=0;
        uint32_t rgb=(uint32_t(channel(p.red))<<16)|(uint32_t(channel(p.green))<<8)|channel(p.blue);
        low=std::min(low,rgb);high=std::max(high,rgb);
    }
    require(hits>0&&hits<pixels.size()&&low!=high,"synthetic scene is not a nonconstant object/background render");
    return @{ @"hits":@(hits),@"misses":@(pixels.size()-hits),@"de_evaluations":@(calls),@"primary_ray_steps":@(steps),
        @"primary_step_limit_pixels":@(bounded),@"shadow_step_limit_pixels":@(shadowBounded),
        @"completion":bounded||shadowBounded?@"BOUNDED_WITH_UNRESOLVED_PIXELS":@"COMPLETE",@"nonconstant":@YES };
}
template<int Formula> static NSDictionary* renderCase(Metal& metal,const Options& options,const ProbeScene& scene,
                                                      NSString* kernel,const char* label) {
    auto start=Clock::now();auto pipeline=metal.pipeline(kernel);double pipelineMs=elapsed(start);
    size_t count=size_t(scene.width)*scene.height,bytes=count*sizeof(ProbePixel);
    start=Clock::now();auto output=metal.buffer(bytes);
    std::vector<ProbePixel> gpu(count),cpu32(count),cpu64(count);double allocationMs=elapsed(start);
    std::memset(output.contents,0xa5,bytes);
    start=Clock::now();double firstGpuMs=metal.run(pipeline,output,nil,&scene,sizeof(scene),scene.width,scene.height);
    std::memcpy(gpu.data(),output.contents,bytes);double firstWarmMs=elapsed(start);auto warm=gpu;
    cpuRender<float,Formula>(scene,options.threads,cpu32);cpuRender<double,Formula>(scene,options.threads,cpu64);
    std::vector<double> gpuTimes,warmTimes,cpu32Times,cpu64Times;
    for(uint32_t i=0;i<options.repeats;++i) {
        start=Clock::now();double gpuMs=metal.run(pipeline,output,nil,&scene,sizeof(scene),scene.width,scene.height);
        std::memcpy(gpu.data(),output.contents,bytes);warmTimes.push_back(elapsed(start));gpuTimes.push_back(gpuMs);
        require(std::memcmp(gpu.data(),warm.data(),bytes)==0,"nondeterministic GPU repeats");
        cpu32Times.push_back(cpuRender<float,Formula>(scene,options.threads,cpu32));
        cpu64Times.push_back(cpuRender<double,Formula>(scene,options.threads,cpu64));
    }
    auto gpuCounts=imageCounts(gpu),cpu32Counts=imageCounts(cpu32),cpu64Counts=imageCounts(cpu64);
    saveImage(options.output/(std::string(label)+"-gpu.ppm"),scene,gpu);
    saveImage(options.output/(std::string(label)+"-cpu-fp32.ppm"),scene,cpu32);
    saveImage(options.output/(std::string(label)+"-cpu-fp64.ppm"),scene,cpu64);
    std::ofstream raw(options.output/(std::string(label)+"-gpu-pixels.bin"),std::ios::binary);
    raw.write(reinterpret_cast<const char*>(gpu.data()),bytes);require(bool(raw),"raw pixel write failed");
    NSMutableArray* gt=[NSMutableArray array],*wt=[NSMutableArray array],*ct=[NSMutableArray array],*dt=[NSMutableArray array];
    for(uint32_t i=0;i<options.repeats;++i) { [gt addObject:@(gpuTimes[i])];[wt addObject:@(warmTimes[i])];[ct addObject:@(cpu32Times[i])];[dt addObject:@(cpu64Times[i])]; }
    std::fprintf(stderr,"%s: GPU %.3f ms, warm submit/readback %.3f ms, CPU FP32 %.3f ms (%u threads)\n",
                 label,median(gpuTimes),median(warmTimes),median(cpu32Times),options.threads);
    return @{ @"name":ns(label),@"formula_id":@(Formula),@"width":@(scene.width),@"height":@(scene.height),
        @"kernel":kernel,@"thread_execution_width":@(pipeline.threadExecutionWidth),
        @"threadgroup_width":@(pipeline.threadExecutionWidth),@"threadgroup_height":@(std::min<NSUInteger>(4,pipeline.maxTotalThreadsPerThreadgroup/pipeline.threadExecutionWidth)),
        @"pipeline_creation_ms":@(pipelineMs),@"buffer_and_host_arrays_allocation_ms":@(allocationMs),@"device_output_bytes":@(bytes),
        @"first_gpu_timestamp_ms":@(firstGpuMs),@"first_encode_submit_wait_readback_ms":@(firstWarmMs),
        @"pipeline_allocation_first_render_ms":@(pipelineMs+allocationMs+firstWarmMs),
        @"gpu_timestamp_ms":gt,@"warm_encode_submit_wait_readback_ms":wt,@"cpu_fp32_ms":ct,@"cpu_fp64_ms":dt,
        @"gpu_timestamp_median_ms":@(median(gpuTimes)),@"warm_encode_submit_wait_readback_median_ms":@(median(warmTimes)),
        @"cpu_fp32_median_ms":@(median(cpu32Times)),@"cpu_fp64_median_ms":@(median(cpu64Times)),
        @"same_algorithm_cpu_fp32_over_warm_gpu_ratio":@(median(cpu32Times)/median(warmTimes)),
        @"same_algorithm_cpu_fp64_over_warm_gpu_ratio":@(median(cpu64Times)/median(warmTimes)),
        @"gpu":gpuCounts,@"cpu_fp32":cpu32Counts,@"cpu_fp64":cpu64Counts,
        @"gpu_vs_cpu_fp32":comparePixels(gpu,cpu32),@"gpu_vs_cpu_fp64_rounded_output":comparePixels(gpu,cpu64),
        @"gpu_repeats_bit_exact":@YES,
        @"gpu_image":ns(std::string(label)+"-gpu.ppm"),@"cpu_fp32_image":ns(std::string(label)+"-cpu-fp32.ppm"),
        @"cpu_fp64_image":ns(std::string(label)+"-cpu-fp64.ppm"),@"gpu_pixel_buffer":ns(std::string(label)+"-gpu-pixels.bin") };
}

struct ArithmeticCase { const char* label;uint32_t op;double a,b,c; };
static NSDictionary* arithmeticChecks(Metal& metal) {
    const double inf=std::numeric_limits<double>::infinity(),nan=std::numeric_limits<double>::quiet_NaN();
    std::vector<ArithmeticCase> rows={
        {"cancellation_2pow24_plus_1",0,16777216.,1,0},{"input_narrowing_2pow24_plus_1",1,16777217.,1,-16777216.},
        {"multiply_add_not_fused",1,1.00000011920928955078125,0.99999988079071044921875,-1},
        {"small_increment",0,1,1e-8,0},{"one_third",2,1,3,0},{"sqrt2",3,2,0,0},
        {"sin_small",4,1e-6,0,0},{"sin_pi_approx",4,3.141592653589793,0,0},
        {"sin_large",4,1e10,0,0},{"cos_large",5,1e10,0,0},{"atan_quadrant",6,1,-1,0},
        {"log_near_one",7,1.0000001,0,0},{"exp2_fraction",8,.125,0,0},{"pow_fraction",9,1.00001,99.5,0},
        {"negative_zero_reciprocal",10,-0.,0,0},{"positive_zero_reciprocal",10,0,0,0},
        {"negative_sqrt",3,-1,0,0},{"infinity_sin",4,inf,0,0},{"nan_division",2,nan,1,0},
        {"subnormal_division",2,double(std::numeric_limits<float>::denorm_min()),2,0},
        {"float_overflow",1,double(std::numeric_limits<float>::max()),2,0},
        {"signed_zero_atan",6,-0.,1,0},{"integer_exact_control",1,3,4,2},{"sqrt_exact_control",3,9,0,0}};
    std::vector<ArithmeticInput> input;for(const auto& r:rows) input.push_back({r.op,0,0,0,float(r.a),float(r.b),float(r.c),0});
    auto src=metal.buffer(input.size()*sizeof(input[0]),input.data()),dst=metal.buffer(rows.size()*sizeof(float));
    uint32_t count=uint32_t(rows.size());metal.run(metal.pipeline(@"arithmetic_probe"),src,dst,&count,sizeof(count),count);
    auto gpu=static_cast<const float*>(dst.contents);NumericStats fp32,fp64;NSMutableArray* details=[NSMutableArray array];
    for(size_t i=0;i<rows.size();++i) {
        const auto&r=rows[i];float f=arithmetic(r.op,float(r.a),float(r.b),float(r.c));double d=arithmetic(r.op,r.a,r.b,r.c);
        fp32.add(gpu[i],f);fp64.add(gpu[i],d);
        [details addObject:@{ @"name":ns(r.label),@"operation":@(r.op),@"source_input_fp64":@[number(r.a),number(r.b),number(r.c)],
            @"gpu_input_fp32":@[number(input[i].a),number(input[i].b),number(input[i].c)],
            @"gpu":number(gpu[i]),@"gpu_bits":ns(bits(gpu[i])),@"cpu_fp32":number(f),@"cpu_fp32_bits":ns(bits(f)),@"cpu_fp64":number(d) }];
    }
    // Independent exact controls catch wrong dispatch/offset/operation wiring.
    require(gpu[22]==14.f&&gpu[23]==3.f,"independent arithmetic exact controls failed");
    return @{ @"rows":@(rows.size()),@"gpu_vs_cpu_fp32":fp32.json(),@"gpu_vs_original_fp64_inputs":fp64.json(),
        @"independent_exact_controls":@2,@"cases":details };
}
static NSDictionary* fixtureChecks(Metal& metal) {
    constexpr size_t count=sizeof(box_reference::formulaRegressions)/sizeof(box_reference::formulaRegressions[0]);
    std::vector<FormulaInput> inputs(count);
    for(size_t i=0;i<count;++i) {
        const auto& r=box_reference::formulaRegressions[i];auto& p=inputs[i];
        for(int k=0;k<16;++k) p.params[k]=float(r.options[k]);
        p.x=float(r.input[0]);p.y=float(r.input[1]);p.z=float(r.input[2]);p.w=float(r.input[3]);
    }
    auto src=metal.buffer(sizeof(FormulaInput)*count,inputs.data()),dst=metal.buffer(sizeof(FormulaOutput)*count);
    uint32_t n=count;metal.run(metal.pipeline(@"box_fixture_probe"),src,dst,&n,sizeof(n),n);
    auto gpu=static_cast<const FormulaOutput*>(dst.contents);NumericStats original,cpu32,cpu64;uint64_t failedRows=0;
    struct Cohort { uint64_t rows=0,failedRows=0;NumericStats original,cpu32,cpu64; } cohorts[3];
    const char* cohortNames[]={"finite_original_in_x87_range","x87_fsincos_out_of_range","fp32_input_overflow"};
    uint64_t originalNonfiniteRows=0,x87RangeRows=0,narrowedOverflowRows=0;
    NSMutableArray* examples=[NSMutableArray array];
    for(size_t i=0;i<count;++i) {
        const auto& r=box_reference::formulaRegressions[i];const auto& p=inputs[i];
        double params[16];std::copy_n(r.options,16,params);params[8]=float(params[8]);params[11]=float(params[11]);params[12]=float(params[12]);
        bool overflow=false;
        for(int k=0;k<r.option_count;++k) overflow|=std::isfinite(r.options[k])&&!std::isfinite(p.params[k]);
        for(int k=0;k<4;++k) overflow|=std::isfinite(r.input[k])&&!std::isfinite(float(r.input[k]));
        double angle=(r.input[2]*params[3]+params[4])*params[8];
        bool x87Outside=params[8]!=0&&(!std::isfinite(angle)||std::abs(angle)>=0x1p63);
        bool originalNonfinite=!std::isfinite(r.output[11]);
        // This frozen fixture has original NaN distances only in the x87
        // out-of-range region. They are results, not masked/unwritten lanes.
        require(!originalNonfinite||x87Outside,"unclassified original nonfinite fixture row");
        size_t group=overflow?2:x87Outside?1:0;auto& cohort=cohorts[group];++cohort.rows;
        originalNonfiniteRows+=originalNonfinite;x87RangeRows+=x87Outside;narrowedOverflowRows+=overflow;
        auto d=box_formula(V3<double>{r.input[0],r.input[1],r.input[2]},params);
        auto f=box_formula(V3<float>{p.x,p.y,p.z},p.params);
        double dv[]={d.position.x,d.position.y,d.position.z,r.input[3],d.distance,d.vary,d.trap};
        float fv[]={f.position.x,f.position.y,f.position.z,p.w,f.distance,f.vary,f.trap};
        float gv[]={gpu[i].x,gpu[i].y,gpu[i].z,gpu[i].w,gpu[i].distance,gpu[i].vary_scale,gpu[i].trap};
        const int lanes[]={0,1,2,3,11,12,14};uint64_t before=original.strict_failures;
        for(int k=0;k<7;++k) {
            original.add(gv[k],r.output[lanes[k]]);cpu32.add(gv[k],fv[k]);cpu64.add(dv[k],r.output[lanes[k]]);
            cohort.original.add(gv[k],r.output[lanes[k]]);cohort.cpu32.add(gv[k],fv[k]);cohort.cpu64.add(dv[k],r.output[lanes[k]]);
        }
        if(original.strict_failures>before) { ++failedRows;++cohort.failedRows;if(examples.count<8) [examples addObject:@{ @"fixture_row_zero_based":@(i),
            @"cohort":ns(cohortNames[group]),
            @"gpu_distance":number(gpu[i].distance),@"original_x86_distance":number(r.output[11]),@"failed_values":@(original.strict_failures-before) }]; }
    }
    NSMutableArray* groups=[NSMutableArray array];
    for(size_t i=0;i<3;++i) [groups addObject:@{ @"name":ns(cohortNames[i]),@"rows":@(cohorts[i].rows),
        @"gpu_failed_rows_at_existing_gate":@(cohorts[i].failedRows),@"gpu_vs_original_x86":cohorts[i].original.json(),
        @"gpu_vs_cpu_fp32":cohorts[i].cpu32.json(),@"cpu_fp64_libm_vs_original_x86":cohorts[i].cpu64.json(),
        @"scope":i==0?@"Finite original results; ordinary FP32 narrowing/arithmetic and libm-versus-x87 differences measured at the unchanged gate":
            i==1?@"Original FSINCOS out-of-range stack behavior is unsupported by this libm prototype; failures retained, not rounding-only evidence":
                 @"Finite original inputs overflow when narrowed to FP32; unsupported exponent range, not rounding-only evidence" }];
    return @{ @"source":@"tests/fixtures/boxifs_twist_regressions.h",@"formula":@"boxIFS",@"rows":@(count),
        @"lanes_per_row":@7,@"lanes":@[@"x",@"y",@"z",@"w",@"Rout",@"VaryScale",@"Dfree1"],
        @"gpu_vs_original_x86":original.json(),@"gpu_vs_cpu_fp32":cpu32.json(),@"cpu_fp64_libm_vs_original_x86":cpu64.json(),
        @"gpu_failed_rows_at_existing_gate":@(failedRows),@"failure_examples":examples,
        @"cohorts":groups,@"primary_finite_original_in_range":groups[0],
        @"original_nonfinite_distance_rows":@(originalNonfiniteRows),@"x87_out_of_range_rows_including_overflow":@(x87RangeRows),
        @"fp32_input_overflow_rows":@(narrowedOverflowRows),
        @"cohort_contract":@"Every original row is retained in aggregate and exactly one cohort: FP32 input overflow takes precedence, then |twist angle| >= 2^63 x87 FSINCOS range, then finite original/in-range. Original NaN outputs are actual recorded results, not masks. CPU FP64 libm does not emulate out-of-range x87 stack behavior. No original result is relabeled PASS.",
        @"qualification":@"DIAGNOSTIC_ONLY; original fixture threshold unchanged. This subset does not cover all state lanes, arbitrary parameters or hybrids." };
}
static NSDictionary* shapeChecks(Metal& metal) {
    struct alignas(16) Point {float x,y,z,w;};
    std::vector<Point> points={{0,0,0,0},{1,0,0,0},{2,0,0,0},{0,0,.8f,0},{0,1.25f,0,0}};
    uint32_t random=0x12345678;
    for(int i=0;i<2048;++i) { float xyz[3];for(int c=0;c<3;++c) {
        random=random*1664525u+1013904223u;xyz[c]=(float(random>>8)/16777216.f)*6.f-3.f; }
        points.push_back({xyz[0],xyz[1],xyz[2],0}); }
    auto src=metal.buffer(points.size()*sizeof(Point),points.data()),dst=metal.buffer(points.size()*sizeof(Point));
    uint32_t count=points.size();metal.run(metal.pipeline(@"shape_points"),src,dst,&count,sizeof(count),count);
    auto gpu=static_cast<const Point*>(dst.contents);NumericStats stats[3],same32[3];
    for(size_t i=0;i<points.size();++i) {
        V3<double> p={points[i].x,points[i].y,points[i].z};V3<float> f={points[i].x,points[i].y,points[i].z};
        double d[]={shape_de<double,0>(p,6),shape_de<double,1>(p,6),shape_de<double,2>(p,6)};
        float fp[]={shape_de<float,0>(f,6),shape_de<float,1>(f,6),shape_de<float,2>(f,6)};
        float g[]={gpu[i].x,gpu[i].y,gpu[i].z};
        for(int k=0;k<3;++k) { stats[k].add(g[k],d[k]);same32[k].add(g[k],fp[k]); }
    }
    // These are analytic sphere-axis results, independent of the shared math.
    // A tight original gate can fail after narrowing -0.8/0.8; record it as data.
    NumericStats analytic;analytic.add(gpu[0].x,-.8);analytic.add(gpu[1].x,0);analytic.add(gpu[2].x,.8);
    require(gpu[1].x==0&&gpu[0].x<0&&gpu[2].x>0,"independent shape dispatch/sign controls failed");
    NSMutableArray* formulas=[NSMutableArray array];const char* names[]={"SphereIFS","boxIFS","Menger3"};
    for(int i=0;i<3;++i) [formulas addObject:@{ @"name":ns(names[i]),@"gpu_vs_cpu_fp64":stats[i].json(),@"gpu_vs_cpu_fp32":same32[i].json() }];
    return @{ @"point_count":@(points.size()),@"source_inputs":@"identical FP32 coordinates promoted exactly for the FP64 diagnostic",
        @"per_formula":formulas,@"independent_sphere_axis_values":analytic.json(),@"independent_exact_zero_and_sign_controls":@3 };
}

int main(int argc,char** argv) {
    @autoreleasepool { try {
#if !defined(__APPLE__) || !defined(__aarch64__)
        throw std::runtime_error("requires native Apple Silicon ARM64");
#endif
        int translated=0;size_t translatedSize=sizeof(translated);
        if(sysctlbyname("sysctl.proc_translated",&translated,&translatedSize,nullptr,0)==0)
            require(translated==0,"Rosetta is forbidden");
        Options options=parse(argc,argv);
        if(!options.output.parent_path().empty()) std::filesystem::create_directories(options.output.parent_path());
        require(std::filesystem::create_directory(options.output),"output ownership lost: refusing to overwrite evidence");
        auto start=Clock::now();Metal metal(options.library);double setupMs=elapsed(start);
        // Grazing shadow rays need more iterations at full resolution. Keep
        // epsilon and every arithmetic step unchanged; exhaustion stays visible.
        ProbeScene scene={1,options.width,options.height,512,4096,5,6,0,.0008f,12.f,.7f,.37f};
        auto abi=metal.buffer(8*sizeof(uint32_t));metal.run(metal.pipeline(@"abi_probe"),abi,nil,&scene,sizeof(scene),1);
        const uint32_t expected[]={sizeof(ProbeScene),sizeof(ProbePixel),sizeof(ArithmeticInput),sizeof(FormulaInput),sizeof(FormulaOutput),1,options.width,std::bit_cast<uint32_t>(scene.epsilon)};
        require(std::memcmp(abi.contents,expected,sizeof(expected))==0,"host/device ABI mismatch");
        NSDictionary* arithmetic=arithmeticChecks(metal),*fixtures=fixtureChecks(metal),*shapes=shapeChecks(metal);
        NSMutableArray* renders=[NSMutableArray array];
        [renders addObject:renderCase<0>(metal,options,scene,@"render_sphere","SphereIFS")];
        [renders addObject:renderCase<1>(metal,options,scene,@"render_box","boxIFS")];
        [renders addObject:renderCase<2>(metal,options,scene,@"render_menger","Menger3")];
        uint64_t unresolved=0,shadowUnresolved=0;for(NSDictionary* render in renders) {
            unresolved+=[render[@"gpu"][@"primary_step_limit_pixels"] unsignedLongLongValue];
            shadowUnresolved+=[render[@"gpu"][@"shadow_step_limit_pixels"] unsignedLongLongValue];
        }
        NSDictionary* report=@{ @"schema_version":@1,@"infrastructure_status":@"PASS",@"numeric_profile":@"EXPERIMENTAL_FP32",
            @"numerical_qualification":@"NOT_QUALIFIED; differences are reported without changing the 2e-12 fixture gate",
            @"scope":@"Three synthetic formula scenes; all rays, formula/DE, normals, hard shadow, five-sample DE AO and color run on GPU. No production M3P parsing, hybrid engine, maps, reflections, transparency, volume or DOF.",
            @"cpu_compute_fallback_calls":@0,@"runtime_shader_compilation":@NO,@"abi_version":@1,@"abi_checks":@8,
            @"host_architecture":@"arm64",@"rosetta_translated":@(translated),@"device":metal.device.name,
            @"unified_memory":@(metal.device.hasUnifiedMemory),@"device_registry_id":@(metal.device.registryID),
            @"library_path":ns(std::filesystem::absolute(options.library).string()),@"cpu_threads":@(options.threads),
            @"warmup_runs_per_backend_per_scene":@1,@"measured_repeats":@(options.repeats),@"device_library_queue_setup_ms":@(setupMs),
            @"precision_build_contract":@"Offline Metal: -fno-fast-math -ffp-contract=off; CPU: -fno-fast-math -ffp-contract=off. No half, explicit fma or native:: approximations. Build provenance must be retained by the caller.",
            @"timing_contract":@"Sequential backend measurements. GPU timestamps cover command execution; warm time includes encode/submit/wait plus copying the full 48-byte/pixel output to host arrays. CPU includes thread creation/join into preallocated arrays, same algorithm and sample counts. Disk image encoding and diagnostic comparison excluded; device/library/queue, pipeline and allocation costs reported separately. Ratios do not compare against the production CPU renderer.",
            @"scene_parameters":@{ @"width":@(scene.width),@"height":@(scene.height),@"max_steps":@(scene.max_steps),@"shadow_steps":@(scene.shadow_steps),
                @"ao_samples":@(scene.ao_samples),@"menger_iterations":@(scene.iterations),@"epsilon":@(scene.epsilon),@"max_distance":@(scene.max_distance),
                @"step_scale":@(scene.step_scale),@"fov_scale":@(scene.fov_scale),@"camera":@[@3,@2.3,@4],@"target":@[@0,@0,@0],@"light":@[@4,@6,@3] },
            @"gpu_unresolved_primary_pixels":@(unresolved),@"gpu_unresolved_shadow_pixels":@(shadowUnresolved),
            @"arithmetic":arithmetic,@"original_formula_fixture":fixtures,@"formula_points":shapes,@"renders":renders };
        writeJSON(options.output/"report.json",report);
        std::printf("Report: %s\n",(options.output/"report.json").c_str());return 0;
    } catch(const std::exception& error) { std::fprintf(stderr,"metal_formula_probe: %s\n",error.what());return 2; } }
}
