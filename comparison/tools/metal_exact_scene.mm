// Native Metal software binary64 and restricted M3P primary-stage qualification.
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#include "comparison/metal_exact/soft_double.h"
#include "comparison/metal_exact/formula_fixture.h"
#include "comparison/metal_exact/normal_arithmetic_vectors.h"
#include "comparison/metal_exact/scene_host.h"
#include "comparison/metal_exact/scene_math.h"
#include <cmath>
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

#include "comparison/metal_exact/metal_device.h"
using mb3d::metal_exact::gpu::Metal;


#include "comparison/metal_exact/extended_gpu_checks.h"
#include "comparison/metal_exact/single_gpu_checks.h"
#include "comparison/metal_exact/log_gpu_checks.h"
using namespace mb3d_metal_exact;
struct ArithmeticInput { uint64_t a,b; uint32_t op,reserved; };
static_assert(sizeof(ArithmeticInput)==24);
static uint64_t db(double d) { return std::bit_cast<uint64_t>(d); }
static double bd(uint64_t d) { return std::bit_cast<double>(d); }
static bool nanbits(uint64_t x) { return (x&0x7fffffffffffffffULL)>0x7ff0000000000000ULL; }
static uint64_t oracle(const ArithmeticInput& p) {
    // These volatile operands force the native ARM64 hardware operation.
    volatile double a=bd(p.a),b=bd(p.b);
    switch(p.op) {
      case 0:return db(a+b);case 1:return db(a-b);case 2:return db(a*b);case 3:return db(a/b);
      case 4:return db(std::sqrt(a));
      case 5:return db(double(std::bit_cast<float>(uint32_t(p.a))));
      case 6:return std::bit_cast<uint32_t>(float(a));
      case 7:return a==b;case 8:return a<b;case 9:return a<=b;
    } throw std::runtime_error("unknown arithmetic op");
}
static std::vector<uint64_t> sqrtTargetInputs() {
    // Every exponent boundary, including subnormals, and either side of exact
    // integer squares. These inputs are shared by production-sqrt and injected
    // seed checks; their expected values use native hardware, not this helper.
    const uint64_t fractions[]={0,1,2,0x0007ffffffffffffULL,0x0008000000000000ULL,
      0x0008000000000001ULL,0x000ffffffffffffdULL,0x000ffffffffffffeULL,0x000fffffffffffffULL};
    std::vector<uint64_t> values;values.reserve(19703);
    for(unsigned exponent=0;exponent<2047;++exponent)
      for(uint64_t fraction:fractions)values.push_back((uint64_t(exponent)<<52)|fraction);
    for(unsigned n=1;n<=256;++n) {
      const uint64_t square=db(double(n*n));
      for(int delta=-2;delta<=2;++delta)values.push_back(square+delta);
    }
    require(values.size()==19703,"targeted sqrt input denominator changed");
    return values;
}
static NSDictionary* arithmeticChecks(Metal& m) {
    const uint64_t edge[]={0,0x8000000000000000ULL,1,2,0x8000000000000001ULL,0xfffffffffffffULL,
      0x10000000000000ULL,0x10000000000001ULL,0x3ff0000000000000ULL,0xbff0000000000000ULL,
      0x3fefffffffffffffULL,0x3ff0000000000001ULL,0x3ca0000000000000ULL,0x7fefffffffffffffULL,
      0xffefffffffffffffULL,0x7ff0000000000000ULL,0xfff0000000000000ULL,0x7ff8000000001234ULL,
      0xfff0000000000001ULL,0x3810000000000000ULL,0x36a0000000000000ULL,0x47efffffe0000000ULL};
    std::vector<ArithmeticInput> input;
    for(uint64_t a:edge)for(uint64_t b:edge)for(uint32_t op=0;op<10;++op)input.push_back({a,b,op,0});
    uint64_t state=0x52dce7297ae31b45ULL;
    auto random=[&] { state^=state<<13;state^=state>>7;state^=state<<17;return state; };
    for(int i=0;i<50000;++i) { uint64_t a=random(),b=random();for(uint32_t op=0;op<10;++op)input.push_back({a,b,op,0}); }
    const uint64_t numeratorEdges[]={0,1,2,3,4,5,0x000ffffffffffffeULL,
      0x000fffffffffffffULL,0x0010000000000000ULL,0x0010000000000001ULL,
      0x0010000000000002ULL,0x3fdfffffffffffffULL,0x3fe0000000000000ULL,
      0x3fe0000000000001ULL,0x3fefffffffffffffULL,0x3ff0000000000000ULL,
      0x3ff0000000000001ULL,0x3ff0000000000002ULL,0x3fffffffffffffffULL,
      0x4000000000000000ULL,0x4000000000000001ULL,0x7feffffffffffffeULL,
      0x7fefffffffffffffULL,0x7ff0000000000000ULL,0x7ff8000000000001ULL,0x7ff0000000000001ULL};
    uint64_t powerTwoChecks=0,powerTwoMultiplyLeftChecks=0,powerTwoMultiplyRightChecks=0;
    for(int e=-1074;e<=1023;++e) {
      uint64_t magnitude=e>=-1022?uint64_t(e+1023)<<52:1ULL<<(e+1074);
      for(auto a:numeratorEdges)for(unsigned signs=0;signs<4;++signs) {
        const uint64_t operand=a|(uint64_t(signs&1)<<63),power=magnitude|(uint64_t(signs>>1)<<63);
        input.push_back({operand,power,3,0});++powerTwoChecks;
        // Cover both shortcut positions independently, including all
        // subnormal powers, four sign combinations, and boundary operands.
        // The oracle remains native volatile-double multiplication.
        input.push_back({power,operand,2,0});++powerTwoMultiplyLeftChecks;
        input.push_back({operand,power,2,0});++powerTwoMultiplyRightChecks;
      }
    }
    const auto targetedSqrt=sqrtTargetInputs();
    for(uint64_t a:targetedSqrt)input.push_back({a,0,4,0});
    uint64_t normalAddChecks=0,normalSubtractChecks=0,normalSwappedChecks=0;
    uint64_t normalMultiplyChecks=0,normalHalfwayChecks=0;
    const auto normalAdditionPairs=for_each_normal_addition_pair(
        [&](uint64_t a,uint64_t b,bool reversed) {
            input.push_back({a,b,0,0});++normalAddChecks;
            input.push_back({a,b,1,0});++normalSubtractChecks;
            if(reversed)normalSwappedChecks+=2;
        });
    const auto normalMultiplicationPairs=for_each_normal_multiplication_pair(
        [&](uint64_t a,uint64_t b,bool halfway) {
            input.push_back({a,b,2,0});++normalMultiplyChecks;
            if(halfway)++normalHalfwayChecks;
        });
    require(normalAdditionPairs==normal_addition_boundary_pairs&&
            normalAddChecks==normal_addition_boundary_pairs&&
            normalSubtractChecks==normal_addition_boundary_pairs&&
            normalSwappedChecks==normal_addition_boundary_swapped_checks&&
            normalMultiplicationPairs==normal_multiplication_boundary_checks&&
            normalMultiplyChecks==normal_multiplication_boundary_checks&&
            normalHalfwayChecks==normal_multiplication_halfway_checks,
            "normal arithmetic boundary matrix denominator changed");
    // Independent Decimal/powers-of-two fixtures retained in test_portable_sqrt.cpp.
    const uint64_t roots[][2]={{0,0},{0x8000000000000000ULL,0x8000000000000000ULL},
      {1,0x1e60000000000000ULL},{2,0x1e66a09e667f3bcdULL},{0xfffffffffffffULL,0x1fffffffffffffffULL},
      {0x10000000000000ULL,0x2000000000000000ULL},{0x7fefffffffffffffULL,0x5fefffffffffffffULL},
      {0x3ff0000000000001ULL,0x3ff0000000000000ULL},{0x4000000000000000ULL,0x3ff6a09e667f3bcdULL},
      {0x4008000000000000ULL,0x3ffbb67ae8584caaULL},{0x4022000000000000ULL,0x4008000000000000ULL}};
    size_t independentStart=input.size();for(auto& r:roots)input.push_back({r[0],0,4,0});
    uint32_t count=uint32_t(input.size());auto src=m.buffer(input.size()*sizeof(input[0]),input.data());
    auto dst=m.buffer(input.size()*8);std::memset(dst.contents,0xa5,input.size()*8);
    double gpuMs=m.run(m.pipeline(@"exact_arithmetic"),src,dst,&count,sizeof(count),count);
    auto result=static_cast<const uint64_t*>(dst.contents);
    uint64_t failures=0,classifiedNaNs=0,checks[10]={},bad[10]={};NSMutableArray* examples=[NSMutableArray array];
    for(size_t i=0;i<input.size();++i) {
      uint64_t expected=i<independentStart?oracle(input[i]):roots[i-independentStart][1];auto op=input[i].op;
      bool nan=op<6?nanbits(expected):(op==6&&(expected&0x7fffffffU)>0x7f800000U);
      bool same=nan ? (op==6?((result[i]&0x7fffffffU)>0x7f800000U):nanbits(result[i])) : expected==result[i];
      ++checks[op];classifiedNaNs+=nan;
      if(!same) { ++failures;++bad[op];if(examples.count<12)[examples addObject:@{@"index":@(i),@"operation":@(op),@"a":@(input[i].a),@"b":@(input[i].b),@"expected_bits":@(expected),@"actual_bits":@(result[i])}]; }
    }
    NSMutableArray* perOp=[NSMutableArray array];for(int op=0;op<10;++op)[perOp addObject:@{@"operation":@(op),@"checks":@(checks[op]),@"failures":@(bad[op])}];
    return @{@"checks":@(input.size()),@"failures":@(failures),@"independent_sqrt_checks":@(std::size(roots)),@"power_two_division_checks":@(powerTwoChecks),@"targeted_sqrt_checks":@(targetedSqrt.size()),
      @"power_two_multiplication_checks":@(powerTwoMultiplyLeftChecks+powerTwoMultiplyRightChecks),
      @"power_two_multiplication_left_checks":@(powerTwoMultiplyLeftChecks),@"power_two_multiplication_right_checks":@(powerTwoMultiplyRightChecks),
      @"normal_addition_boundary_checks":@(normalAddChecks+normalSubtractChecks),
      @"normal_addition_boundary_add_checks":@(normalAddChecks),@"normal_addition_boundary_subtract_checks":@(normalSubtractChecks),
      @"normal_addition_boundary_swapped_checks":@(normalSwappedChecks),
      @"normal_multiplication_boundary_checks":@(normalMultiplyChecks),@"normal_multiplication_halfway_checks":@(normalHalfwayChecks),
      @"nan_class_checks":@(classifiedNaNs),@"nan_policy":@"NaN class; payload, sign and exception flags outside profile. All non-NaN bits including signed zero exact.",
      @"rounding":@"nearest, ties to even; gradual underflow; no fused multiply-add",@"gpu_timestamp_ms":@(gpuMs),@"operations":perOp,@"failure_examples":examples};
}
struct SqrtSeedInput {uint64_t a,hint,high;};
struct SqrtSeedOutput {uint64_t bits,root,remainder;uint32_t valid,reserved;};
static_assert(sizeof(SqrtSeedInput)==24&&sizeof(SqrtSeedOutput)==32);
static uint64_t independentSqrtHigh(uint64_t bits) {
    const unsigned encoded=unsigned((bits>>52)&2047);
    uint64_t sig=bits&0x000fffffffffffffULL;
    if((bits>>63)||encoded==2047||(!encoded&&!sig))return 0;
    int exponent=encoded?int(encoded)-1023:-1022;
    if(encoded)sig|=1ULL<<52;
    else while(sig<(1ULL<<52)){sig<<=1;--exponent;}
    if(exponent%2!=0)sig<<=1;
    return sig>>6;
}
static uint64_t independentPrefixRoot(uint64_t high) {
    // Binary search is independent of the bounded-correction/restoring helper.
    uint64_t low=0,upper=1ULL<<24;
    while(low+1<upper) {
      const uint64_t middle=(low+upper)/2;
      if(middle*middle<=high)low=middle;else upper=middle;
    }
    return low;
}
static NSDictionary* sqrtSeedChecks(Metal& m) {
    auto values=sqrtTargetInputs();
    const uint64_t special[]={0x8000000000000000ULL,0xbff0000000000000ULL,0x8000000000000001ULL,
      0xffefffffffffffffULL,0x7ff0000000000000ULL,0xfff0000000000000ULL,
      0x7ff8000000000001ULL,0x7ff0000000000001ULL,0xfff8000000000001ULL};
    values.insert(values.end(),std::begin(special),std::end(special));
    require(values.size()==19712,"sqrt seed matrix denominator changed");
    const char* names[]={"exact","minus_one","plus_one","minus_four","plus_four","minus_five",
      "plus_five","zero","limit","uint64_max","random32","random64","invalid_high"};
    std::vector<SqrtSeedInput> input;input.reserve(values.size()*12+6);
    std::vector<unsigned> categories;categories.reserve(input.capacity());
    uint64_t state=0x67129b4de8a03cf5ULL;
    auto random=[&]{state^=state<<13;state^=state>>7;state^=state<<17;return state;};
    for(uint64_t value:values) {
      const uint64_t high=independentSqrtHigh(value),root=independentPrefixRoot(high);
      const uint64_t hints[]={root,root-1,root+1,root-4,root+4,root-5,root+5,0,1ULL<<24,
        UINT64_MAX,uint64_t(uint32_t(random())),random()};
      for(unsigned kind=0;kind<12;++kind){input.push_back({value,hints[kind],high});categories.push_back(kind);}
    }
    const uint64_t invalidHigh[]={1ULL<<48,(1ULL<<48)+1,UINT64_MAX};
    for(uint64_t high:invalidHigh)for(uint64_t hint:{0ULL,1ULL<<24}) {
      input.push_back({0x3ff0000000000000ULL,hint,high});categories.push_back(12);
    }
    require(input.size()==236550,"sqrt seed record denominator changed");
    uint32_t count=uint32_t(input.size());auto src=m.buffer(input.size()*sizeof(input[0]),input.data());
    auto dst=m.buffer(input.size()*sizeof(SqrtSeedOutput));std::memset(dst.contents,0xa5,dst.length);
    const double gpuMs=m.run(m.pipeline(@"exact_sqrt_seed_controls"),src,dst,&count,sizeof(count),count);
    const auto* result=static_cast<const SqrtSeedOutput*>(dst.contents);
    uint64_t rows[13]{},sqrtBad[13]{},prefixBad[13]{},accepted[13]{},rejected[13]{};
    uint64_t failures=0,nanChecks=0,zeroChecks=0;NSMutableArray* examples=[NSMutableArray array];
    for(size_t i=0;i<input.size();++i) {
      const auto& p=input[i];const auto& actual=result[i];const unsigned kind=categories[i];++rows[kind];
      volatile double native=bd(p.a);const uint64_t expected=db(std::sqrt(native));
      const bool nan=nanbits(expected);nanChecks+=nan;zeroChecks+=(expected&0x7fffffffffffffffULL)==0;
      const bool valueOk=nan?nanbits(actual.bits):actual.bits==expected;
      uint64_t root=0,remainder=0;bool valid=false;
      if(p.high<(1ULL<<48)&&p.hint<=(1ULL<<24)) {
        root=independentPrefixRoot(p.high);
        const uint64_t distance=p.hint>root?p.hint-root:root-p.hint;
        valid=distance<=4;if(valid)remainder=p.high-root*root;
      }
      if(!valid)root=remainder=0;
      const bool prefixOk=actual.valid==unsigned(valid)&&actual.root==root&&actual.remainder==remainder&&actual.reserved==0;
      accepted[kind]+=valid;rejected[kind]+=!valid;sqrtBad[kind]+=!valueOk;prefixBad[kind]+=!prefixOk;
      failures+=!valueOk;failures+=!prefixOk;
      if((!valueOk||!prefixOk)&&examples.count<12)[examples addObject:@{
        @"index":@(i),@"case":ns(names[kind]),@"a":@(p.a),@"hint":@(p.hint),@"high":@(p.high),
        @"expected_bits":@(expected),@"actual_bits":@(actual.bits),@"sqrt_pass":@(valueOk),@"prefix_pass":@(prefixOk)}];
    }
    NSMutableArray* cases=[NSMutableArray array];
    for(unsigned kind=0;kind<13;++kind)[cases addObject:@{
      @"case":ns(names[kind]),@"records":@(rows[kind]),@"sqrt_failures":@(sqrtBad[kind]),@"prefix_failures":@(prefixBad[kind]),
      @"accepted_prefixes":@(accepted[kind]),@"rejected_prefixes":@(rejected[kind])}];
    return @{@"pass":@(failures==0),@"matrix_inputs":@(values.size()),@"seed_variants":@12,
      @"invalid_high_checks":@6,@"records":@(input.size()),@"checks":@(2*input.size()),@"failures":@(failures),
      @"sqrt_checks":@(input.size()),@"prefix_checks":@(input.size()),@"nan_class_checks":@(nanChecks),
      @"signed_zero_checks":@(zeroChecks),@"input_bytes":@(sizeof(SqrtSeedInput)),@"record_bytes":@(sizeof(SqrtSeedOutput)),
      @"cases":cases,@"gpu_timestamp_ms":@(gpuMs),@"failure_examples":examples,
      @"gate":@"sqrt non-NaN bits exact; NaN class; prefix root/remainder/valid exact",
      @"oracle":@"native ARM64 std::sqrt(double); independent integer prefix binary search"};
}
#include "comparison/metal_exact/sqrt_gpu_checks.h"
#include "comparison/metal_exact/division_gpu_checks.h"
static NSDictionary* boxChecks(Metal& m) {
    auto input=mb3d_exact::fixture_host::box_inputs();uint32_t n=uint32_t(input.size());
    std::vector<mb3d_exact::ExactBoxOutput> output(n);
    auto src=m.buffer(input.size()*sizeof(input[0]),input.data()),dst=m.buffer(output.size()*sizeof(output[0]));
    std::memset(dst.contents,0xa5,output.size()*sizeof(output[0]));
    double t=m.run(m.pipeline(@"exact_box_fixture"),src,dst,&n,sizeof(n),n);
    std::memcpy(output.data(),dst.contents,output.size()*sizeof(output[0]));
    auto c=mb3d_exact::fixture_host::check_box_outputs(output);
    return @{@"supported_rows":@(c.rows),@"values":@(c.values),@"failed_rows":@(c.failed_rows),@"failed_values":@(c.failed_values),
      @"class_failures":@(c.class_failures),@"signed_zero_failures":@(c.signed_zero_failures),@"exact_bits":@(c.exact_bits),
      @"unsupported_twist_rows":@(c.unsupported_rows),@"unsupported_rejection_failures":@(c.unsupported_rejection_failures),
      @"gate":@"2e-12 * max(1,abs(reference)); same nonfinite class and signed zero",@"gpu_timestamp_ms":@(t)};
}
struct Options {std::filesystem::path library,scene,output;unsigned width=128,height=96,threads=6,repeats=3,warmups=1;bool fullShading=false,hardShadows=false,shadowSweep=false,describe=false,benchmarkRender=false,warmupsSpecified=false;mb3d::PrecisionMode precision=mb3d::PrecisionMode::Compatible;std::string divisionMode="verified",normalMode="parallel",shadowMode="parallel";unsigned normalThreads=32;unsigned shadowPrepThreads=32,shadowTraceThreads=32,shadowRows=256;unsigned primaryRows=256,primaryThreads=32,primaryTileWidth=8;bool primarySweep=false;};
static Options parse(int argc,char**argv) {
    Options o;
    for(int i=1;i<argc;++i) {
      std::string k=argv[i];if(k=="--help") {puts("mb3d_metal_exact_scene --library OFFLINE.metallib --scene SUPPORTED_SCENE.m3p --output NEW_DIR [--width 128 --height 96 --threads 6 --repeats 3] [--primary-rows-per-command 256] [--primary-threads 32] [--primary-tile-width 8] [--primary-dispatch-sweep] [--precision compatible|native] [--division-mode verified|restoring] [--normal-mode parallel|inline] [--normal-threads 32] [--full-shading [--hard-shadows] [--shadow-mode parallel|prepared|legacy] [--shadow-prep-threads 32] [--shadow-trace-threads 32] [--shadow-rows-per-command 256] [--shadow-dispatch-sweep]]\nAdd --describe --scene INPUT.m3p for native preparation metadata without GPU dispatch or output files.\nExact arithmetic + real M3P primary geometry; --full-shading computes the final admitted scene render on GPU. --hard-shadows explicitly enables original-scene-disabled shadows. --primary-tile-width 0 selects a linear threadgroup.\nAdd --benchmark-render --full-shading [--warmups 1 --repeats 3] for render-only cold/warm timings and PNGs; compatible requires separate qualification. Native uses the separate metal_native_scene.metallib hardware FP32 library; exact qualification is unavailable.");exit(0);}
      if(k=="--describe"){o.describe=true;continue;}
      if(k=="--benchmark-render"){o.benchmarkRender=true;continue;}
      if(k=="--full-shading"){o.fullShading=true;continue;}
      if(k=="--primary-dispatch-sweep"){o.primarySweep=true;continue;}
      if(k=="--hard-shadows"){o.hardShadows=true;continue;}
      if(k=="--shadow-dispatch-sweep"){o.shadowSweep=true;continue;}
      require(i+1<argc,"missing value for "+k);std::string v=argv[++i];
      if(k=="--library")o.library=v;else if(k=="--scene")o.scene=v;else if(k=="--output")o.output=v;
            else if(k=="--precision"){require(mb3d::ParsePrecisionMode(v.c_str(),o.precision),"invalid precision (expected compatible or native)");}
      else if(k=="--division-mode"){require(v=="verified"||v=="restoring","invalid division mode");o.divisionMode=v;}
      else if(k=="--normal-mode"){require(v=="parallel"||v=="inline","invalid normal mode");o.normalMode=v;}
      else if(k=="--shadow-mode"){require(v=="legacy"||v=="prepared"||v=="parallel","invalid shadow mode");o.shadowMode=v;}
      else {require(!v.empty()&&v.find_first_not_of("0123456789")==std::string::npos,"invalid unsigned value");size_t end=0;auto n=std::stoul(v,&end);require(end==v.size()&&n<=4096,"invalid range");
        if(k=="--warmups"){o.warmups=unsigned(n);o.warmupsSpecified=true;}else if(k=="--normal-threads")o.normalThreads=unsigned(n);else if(k=="--primary-rows-per-command")o.primaryRows=unsigned(n);else if(k=="--primary-threads")o.primaryThreads=unsigned(n);else if(k=="--primary-tile-width")o.primaryTileWidth=unsigned(n);else if(k=="--width")o.width=unsigned(n);else if(k=="--height")o.height=unsigned(n);else if(k=="--threads")o.threads=unsigned(n);else if(k=="--repeats")o.repeats=unsigned(n);else if(k=="--shadow-prep-threads")o.shadowPrepThreads=unsigned(n);else if(k=="--shadow-trace-threads")o.shadowTraceThreads=unsigned(n);else if(k=="--shadow-rows-per-command")o.shadowRows=unsigned(n);else throw std::runtime_error("unknown option "+k);}
    }
    require(o.precision!=mb3d::PrecisionMode::Native||o.describe||(o.benchmarkRender&&!o.primarySweep&&!o.shadowSweep),"native precision requires --benchmark-render --full-shading; exact qualification is compatible-only");
    require(o.precision!=mb3d::PrecisionMode::Native||o.divisionMode=="verified","restoring software division is unavailable in native precision");
    require(o.normalThreads>0&&o.normalThreads<=1024,"invalid normal threadgroup size");
    require(!o.primarySweep||o.fullShading,"--primary-dispatch-sweep requires --full-shading");
    require(o.primaryRows>0&&o.primaryRows<=4096&&o.primaryThreads>0&&o.primaryThreads<=1024,"invalid primary dispatch settings");
    require(o.primaryTileWidth==0||(o.primaryTileWidth<=o.primaryThreads&&o.primaryThreads%o.primaryTileWidth==0),"primary tile width must be zero or a positive factor of primary threads");
    require(!o.shadowSweep||(o.fullShading&&o.hardShadows&&o.shadowMode=="prepared"),"--shadow-dispatch-sweep requires --full-shading --hard-shadows and prepared mode");
    require(o.shadowTraceThreads>0&&o.shadowTraceThreads<=1024&&o.shadowPrepThreads>0&&o.shadowPrepThreads<=1024&&o.shadowRows>0&&o.shadowRows<=4096,"invalid shadow dispatch settings");
    require(o.fullShading||(o.shadowMode=="parallel"&&o.shadowPrepThreads==32&&o.shadowTraceThreads==32&&o.shadowRows==256),"shadow dispatch options require --full-shading");
    require(!o.hardShadows||o.fullShading,"--hard-shadows requires --full-shading");
    require(!o.warmupsSpecified||o.benchmarkRender,"--warmups requires --benchmark-render");
    require(!o.benchmarkRender||(o.fullShading&&!o.describe&&!o.primarySweep&&!o.shadowSweep),"--benchmark-render requires --full-shading and forbids --describe or diagnostic sweeps");
    require(!o.scene.empty()&&(o.describe||(!o.library.empty()&&!o.output.empty())),"scene required; rendering also requires library and output");
    require(o.width&&o.height&&o.threads&&o.threads<=64&&o.threads<=o.height&&o.repeats>=3&&o.repeats<=(o.benchmarkRender?4096u:15u),"invalid dimensions/threads/repeats (threads must not exceed height)");
    require(o.describe||!std::filesystem::exists(o.output),"output exists; refusing overwrite");return o;
}
#include "comparison/metal_exact/primary_dispatch.h"
#include "comparison/metal_exact/normal_dispatch.h"
static void saveImage(const std::filesystem::path& file,unsigned w,unsigned h,const uint32_t* rgba) {
    std::ofstream f(file,std::ios::binary);f<<"P6\n"<<w<<" "<<h<<"\n255\n";
    for(size_t i=0;i<size_t(w)*h;++i) {char p[3]={char(rgba[i]),char(rgba[i]>>8),char(rgba[i]>>16)};f.write(p,3);}require(bool(f),"cannot save preview");
}
static std::vector<uint32_t> referencePreview(const std::vector<ExactPixel>& p) {
    std::vector<uint32_t> image(p.size());for(size_t i=0;i<p.size();++i) {auto& n=p[i];uint32_t r=12,g=18,b=30;if(n.rm_result){r=uint32_t(n.normal[0]+32767)*255/65534;g=uint32_t(n.normal[1]+32767)*255/65534;b=uint32_t(n.normal[2]+32767)*255/65534;}image[i]=r|(g<<8)|(b<<16)|0xff000000U;}return image;
}
static NSDictionary* compare(const std::vector<ExactPixel>& a,const std::vector<ExactPixel>& b,const ExactScene& scene) {
    require(a.size()==b.size(),"incomplete reference");uint64_t cameraBad=0,geometryBad=0,normalBad=0,discreteBad=0,bitBad=0,hits=0,faults=0,checks=0;
    uint64_t zeroSignBad=0,sanityBad=0,sanityValues=0;
    const uint32_t resultKindLimit=scene.formula_kind==3?2:1;
    const uint32_t iterationLimit=scene.max_iterations+(scene.formula_kind==3?1:0);
    NSMutableArray* examples=[NSMutableArray array];
    auto test=[&](uint64_t x,uint64_t y,double tol) {
      ++checks;bitBad+=x!=y;double actual=bd(x),expected=bd(y);
      const bool wrongZero=actual==0&&expected==0&&std::signbit(actual)!=std::signbit(expected);
      zeroSignBad+=wrongZero;
      return !wrongZero&&std::isfinite(actual)&&std::isfinite(expected)&&std::abs(actual-expected)<=tol*std::max(1.,std::abs(expected));
    };
    for(size_t i=0;i<a.size();++i) {
      auto& p=a[i];auto& q=b[i];hits+=p.rm_result!=0;faults+=p.fault!=0;bool bad=false;
      // Check every declared numeric field on hits and misses independently of
      // the shader fault flag. A NaN hidden in an unused field is not complete
      // exact-stage evidence, and a missing dispatch cannot look like a miss.
      for(const ExactPixel* record:{&p,&q}) {
        auto finite=[&](uint64_t value){++sanityValues;if(!std::isfinite(bd(value))){++sanityBad;bad=true;}};
        for(int j=0;j<3;++j){finite(record->ray[j]);finite(record->start[j]);finite(record->position[j]);finite(record->raw_normal[j]);}
        finite(record->depth);finite(record->axial_depth);finite(record->final_de);finite(record->orbit_trap);
        for(uint32_t value:{record->step_count,record->smooth_iterations,record->final_stop}) {
          ++sanityValues;if(!std::isfinite(std::bit_cast<float>(value))){++sanityBad;bad=true;}
        }
        if(record->rm_result>resultKindLimit||record->start_inside>1||record->final_iterations>iterationLimit||record->reserved_tail!=0||
           record->normal[0]<-32768||record->normal[0]>32767||record->normal[1]<-32768||record->normal[1]>32767||
           record->normal[2]<-32768||record->normal[2]>32767) {++sanityBad;bad=true;}
      }
      for(int j=0;j<3;++j){if(!test(p.ray[j],q.ray[j],2e-15)){++cameraBad;bad=true;}if(!test(p.start[j],q.start[j],2e-15)){++cameraBad;bad=true;}}
      for(auto pair:{std::pair(p.depth,q.depth),std::pair(p.axial_depth,q.axial_depth)})if(!test(pair.first,pair.second,2e-12)){++geometryBad;bad=true;}
      if(p.rm_result!=q.rm_result||p.start_inside!=q.start_inside||p.step_count!=q.step_count||p.final_iterations!=q.final_iterations||p.final_stop!=q.final_stop){++discreteBad;bad=true;}
      if(p.rm_result&&q.rm_result) {
        for(int j=0;j<3;++j){if(!test(p.position[j],q.position[j],2e-12)){++geometryBad;bad=true;}if(!test(p.raw_normal[j],q.raw_normal[j],2e-12)){++normalBad;bad=true;}}
        if(!test(p.final_de,q.final_de,2e-12)){++geometryBad;bad=true;}
        if(!test(p.orbit_trap,q.orbit_trap,2e-12)){++geometryBad;bad=true;}
        if(p.normal[0]!=q.normal[0]||p.normal[1]!=q.normal[1]||p.normal[2]!=q.normal[2]||p.smooth_iterations!=q.smooth_iterations){++normalBad;bad=true;}
      }
      if(bad&&examples.count<12)[examples addObject:@{@"pixel":@(i),@"gpu_depth":number(bd(p.depth)),@"cpu_depth":number(bd(q.depth)),@"gpu_final_de":number(bd(p.final_de)),@"cpu_final_de":number(bd(q.final_de)),@"gpu_orbit_trap":number(bd(p.orbit_trap)),@"cpu_orbit_trap":number(bd(q.orbit_trap)),@"gpu_steps_bits":@(p.step_count),@"cpu_steps_bits":@(q.step_count),@"gpu_hit":@(p.rm_result),@"cpu_hit":@(q.rm_result)}];
    }
    const uint64_t coverageBad=hits==0||hits==a.size()?1:0;
    return @{@"pixels":@(a.size()),@"result_kind_limit":@(resultKindLimit),@"final_iteration_limit":@(iterationLimit),@"hits":@(hits),@"fault_pixels":@(faults),@"numeric_checks":@(checks),@"different_numeric_bits":@(bitBad),
      @"misses":@(a.size()-hits),@"nontrivial_hit_mask":@(coverageBad==0),@"coverage_failures":@(coverageBad),
      @"signed_zero_failures":@(zeroSignBad),@"output_sanity_failures":@(sanityBad),@"output_sanity_values":@(sanityValues),
      @"camera_failures_2e_15":@(cameraBad),@"geometry_failures_2e_12":@(geometryBad),@"normal_failures_2e_12_and_exact_packing":@(normalBad),@"discrete_failures":@(discreteBad),
      @"failures":@(cameraBad+geometryBad+normalBad+discreteBad+faults+sanityBad+coverageBad),@"failure_examples":examples};
}

// Exercise formula admission on the real device independently of successful
// images. Saved output buffers and benchmark timing never include these runs.
static NSDictionary* formulaRejectionChecks(Metal& m,id<MTLComputePipelineState> pipeline,
                                            ExactScene scene,id<MTLBuffer> cols,id<MTLBuffer> rows,
                                            id<MTLBuffer> output,const Options& o,
                                            const std::function<void(id<MTLBuffer>,const ExactScene&)>& complete={}) {
    const size_t count=size_t(scene.width)*scene.height;
    uint64_t checks=0,failures=0;
    for(unsigned mode=0;mode<5;++mode) {
        auto bad=scene;
        if(mode==0)bad.formula_kind=99;
        if(mode==1)bad.capability_id=scene.capability_id==1?101:0xffffffffU;
        if(mode==2)bad.params[0]=0x7ff8000000000000ULL;
        if(mode==3)bad.reserved_formula[1]=1;
        if(mode==4)bad.bulb.power=scene.formula_kind==3?(scene.bulb.power==2?3:2):8;
        std::memset(output.contents,0,output.length);
        runScene(m,pipeline,output,cols,rows,bad,o.primaryRows,o.primaryThreads,nullptr,o.primaryTileWidth);
        if(complete)complete(output,bad);
        const auto* pixels=static_cast<const ExactPixel*>(output.contents);
        for(size_t pixel=0;pixel<count;++pixel){++checks;failures+=pixels[pixel].fault!=1||pixels[pixel].rm_result!=0;}
    }
    uint64_t noMinimumCases=0,noMinimumChecks=0,noMinimumFailures=0;
    if(scene.formula_kind==1||scene.formula_kind==2) {
        auto stateful=scene;
        // The pipeline is specialized for the admitted capability. Pinned
        // profiles retain every formula coefficient and iteration count.
        if(scene.capability_id==101||scene.capability_id==102) {
            stateful.max_iterations=stateful.slot_count=1;
            std::fill(std::begin(stateful.params),std::end(stateful.params),0);
            const uint64_t one=0x3ff0000000000000ULL,two=0x4000000000000000ULL;
            if(scene.formula_kind==1) {
                stateful.params[0]=stateful.params[5]=stateful.params[6]=stateful.params[7]=one;
                stateful.params[1]=two;stateful.params[8]=stateful.params[10]=stateful.params[11]=one;
            } else {
                stateful.params[0]=stateful.params[1]=stateful.params[2]=one;
                stateful.params[3]=two;stateful.params[9]=stateful.params[11]=stateful.params[12]=one;
            }
        }
        for(auto& coordinate:stateful.y_start)coordinate=std::bit_cast<uint64_t>(100000000.0);
        require(mb3d_metal_exact::valid_scene(stateful),"No-minimum GPU witness is not a valid IFS scene");
        std::memset(output.contents,0,output.length);
        runScene(m,pipeline,output,cols,rows,stateful,o.primaryRows,o.primaryThreads,nullptr,o.primaryTileWidth);
        if(complete)complete(output,stateful);
        const auto* pixels=static_cast<const ExactPixel*>(output.contents);
        noMinimumCases=1;
        for(size_t pixel=0;pixel<count;++pixel) {
            ++noMinimumChecks;
            noMinimumFailures+=pixels[pixel].fault!=4||pixels[pixel].rm_result!=0;
        }
    }
    return @{@"pass":@(failures==0&&noMinimumFailures==0),@"cases":@5,@"checks":@(checks),@"failures":@(failures),
             @"no_minimum_cases":@(noMinimumCases),@"no_minimum_checks":@(noMinimumChecks),
             @"no_minimum_failures":@(noMinimumFailures)};
}
static NSDictionary* sceneChecks(Metal& m,const Options& o,const mb3d::metal_exact::PreparedExactScene& prepared) {
    auto scene=prepared.device;size_t count=size_t(scene.width)*scene.height,bytes=count*sizeof(ExactPixel);
    auto preview=m.pipeline(@"exact_preview");
    NormalStages normals(m,o,uint32_t(count));
    auto cols=m.buffer(prepared.columns.size()*sizeof(ExactTrig),prepared.columns.data());
    auto rows=m.buffer(prepared.rows.size()*sizeof(ExactTrig),prepared.rows.data());
    auto out=m.buffer(bytes),image=m.buffer(count*4);std::memset(out.contents,0xa5,bytes);
    double maxPrimaryMs=0;
    auto start=Clock::now();double firstGpu=normals.run(out,cols,rows,scene,maxPrimaryMs,true);double firstWall=elapsed(start);
    std::vector<ExactPixel> gpu(count);std::memcpy(gpu.data(),out.contents,bytes);auto first=gpu;
    start=Clock::now();auto cpu=mb3d::metal_exact::ReferencePrimary(prepared,o.threads);double cpuMs=elapsed(start);
    auto comparison=compare(gpu,cpu,scene);
    std::vector<double> gpuTimes,wallTimes;bool exactRepeat=true;
    for(unsigned i=0;i<o.repeats;++i) {
      start=Clock::now();gpuTimes.push_back(normals.run(out,cols,rows,scene,maxPrimaryMs,false));
      std::memcpy(gpu.data(),out.contents,bytes);wallTimes.push_back(elapsed(start));
      exactRepeat=exactRepeat&&std::memcmp(gpu.data(),first.data(),bytes)==0;
    }
    std::vector<ExactPixel> baseline;
    auto dispatch=primaryDispatchChecks(m,normals.selected(),scene,cols,rows,out,first,o.primaryRows,o.primaryThreads,o.primarySweep,maxPrimaryMs,baseline,o.primaryTileWidth,
        [&](id<MTLBuffer> buffer,const ExactScene& sc){normals.complete(buffer,sc);});
    auto normalReport=normals.report(scene,cols,rows,first);
    uint32_t n=uint32_t(count);m.run(preview,out,image,&n,sizeof(n),n);
    saveImage(o.output/"gpu-primary-normals.ppm",scene.width,scene.height,static_cast<const uint32_t*>(image.contents));
    auto refImage=referencePreview(cpu);saveImage(o.output/"cpu-primary-normals.ppm",scene.width,scene.height,refImage.data());
    uint64_t rgbDiff=0;for(size_t i=0;i<count;++i)rgbDiff+=refImage[i]!=static_cast<const uint32_t*>(image.contents)[i];
    for(auto item:{std::pair("gpu-primary.bin",&gpu),std::pair("cpu-primary.bin",&cpu),std::pair("gpu-baseline-primary.bin",&baseline)}) {
      std::ofstream f(o.output/item.first,std::ios::binary);f.write(reinterpret_cast<const char*>(item.second->data()),bytes);require(bool(f),"cannot save primary buffer");
    }
    std::ofstream packed(o.output/"prepared-scene.bin",std::ios::binary);packed.write(reinterpret_cast<const char*>(&scene),sizeof(scene));require(bool(packed),"cannot save scene ABI");
    // Negative controls dispatch the same kernel with incompatible ABI and an
    // intentionally exhausted bound. They must not publish completed geometry.
    auto reject=[&](const ExactScene& bad){runScene(m,normals.selected(),out,cols,rows,bad,o.primaryRows,o.primaryThreads,nullptr,o.primaryTileWidth);normals.complete(out,bad);};
    auto bad=scene;bad.version=scene.version+1;reject(bad);uint64_t rejected=0;
    auto pixels=static_cast<const ExactPixel*>(out.contents);for(size_t i=0;i<count;++i)rejected+=pixels[i].fault==1;
    uint64_t malformedRejected=0;
    for(unsigned variant=0;variant<3;++variant) {
      bad=scene;
      if(variant==0) {
        bad.params[1]=0x7ff8000000000000ULL;
        require(!mb3d_metal_exact::valid_scene(bad),"Malformed coefficient must be outside the admitted formula domain");
      }
      if(variant==1)bad.width+=1;
      if(variant==2)bad.y_start[0]=0x7ff8000000000000ULL;
      reject(bad);
      for(size_t i=0;i<count;++i)malformedRejected+=pixels[i].fault==1;
    }
    bad=scene;bad.max_steps=1;reject(bad);uint64_t exhausted=0;
    for(size_t i=0;i<count;++i)exhausted+=(pixels[i].fault&2)!=0;
    auto formulaControls=formulaRejectionChecks(m,normals.selected(),scene,cols,rows,out,o,
        [&](id<MTLBuffer> buffer,const ExactScene& sc){normals.complete(buffer,sc);});
    NSMutableArray* gt=[NSMutableArray array],*wt=[NSMutableArray array];for(size_t i=0;i<gpuTimes.size();++i){[gt addObject:@(gpuTimes[i])];[wt addObject:@(wallTimes[i])];}
    return @{@"scene_sha256":ns(prepared.source_sha256),@"scene_id":ns(prepared.scene_id),@"capability_id":@(scene.capability_id),@"formula_kind":@(scene.formula_kind),@"formula_name":ns(prepared.formula_name),@"width":@(scene.width),@"height":@(scene.height),@"comparison":comparison,@"primary_dispatch":dispatch,@"formula_rejections":formulaControls,
      @"normal_pipeline":normalReport,@"complete":@([comparison[@"fault_pixels"] unsignedLongLongValue]==0),@"gpu_repeats_bit_exact":@(exactRepeat),@"diagnostic_rgb_pixel_differences":@(rgbDiff),
      @"abi_rejection_pixels":@(rejected),@"malformed_rejection_pixels":@(malformedRejected),@"malformed_variants":@3,
      @"budget_exhausted_pixels":@(exhausted),@"negative_controls_pass":@(rejected==count&&malformedRejected==3*count&&exhausted>0),
      @"first_gpu_timestamp_ms":@(firstGpu),@"first_encode_submit_wait_ms":@(firstWall),@"gpu_timestamp_ms":gt,@"encode_submit_wait_readback_ms":wt,
      @"gpu_timestamp_median_ms":@(median(gpuTimes)),@"encode_submit_wait_readback_median_ms":@(median(wallTimes)),
      @"cpu_production_primary_reference_observation_ms":@(cpuMs),@"cpu_reference_threads":@(o.threads),
      @"timing_note":@"CPU reference also captures validation records and replays raw normal probes. It is one observation, not an equivalent production benchmark; no speedup claim.",
      @"host_work":@"M3P parse/parameter preparation and O(width+height) original scalar sin/cos tables; dispatch/save; independent CPU validation separately.",
      @"gpu_work":@"camera ray assembly/normalization/rotation, selected formula chain, DE, primary stepping, x87-style refinement, six-probe normals, extended normal packing, diagnostic normal colors",
      @"excluded":@"production hard shadows, AO, palette/color, reflections, volume, DOF, arbitrary M3P/formulas, device cancellation; diagnostic normal visualization is not final scene shading"};
}
#include "comparison/metal_exact/shading_gpu_checks.h"

static NSDictionary* sceneDescription(const mb3d::metal_exact::PreparedExactScene& prepared) {
    const auto& scene=prepared.device;
    NSMutableArray* parameters=[NSMutableArray array];
    for(auto value:scene.params) {
        char encoded[17];std::snprintf(encoded,sizeof(encoded),"%016llx",(unsigned long long)value);
        [parameters addObject:ns(encoded)];
    }
    return @{@"scene_sha256":ns(prepared.source_sha256),@"scene_id":ns(prepared.scene_id),
        @"capability_id":@(scene.capability_id),@"formula_kind":@(scene.formula_kind),
        @"formula_name":ns(prepared.formula_name),@"max_iterations":@(scene.max_iterations),
        @"slot_count":@(scene.slot_count),@"original_shadow_bits":@(prepared.original_shadow_bits),
        @"parameter_bits":parameters,@"builtin_power":@(scene.bulb.power)};
}
static NSString* numericProfile(const mb3d::metal_exact::PreparedExactScene& scene,bool full) {
    if(scene.precision==mb3d::PrecisionMode::Native)
        return [NSString stringWithFormat:@"NATIVE_FP32_%@_FULL_SHADING",[ns(scene.formula_name) uppercaseString]];
    return [NSString stringWithFormat:full?@"EXACT_%@_FULL_SHADING":@"EXACT_BINARY64_PRIMARY_%@",
            [ns(scene.formula_name) uppercaseString]];
}
#include "comparison/metal_exact/render_benchmark.h"

int main(int argc,char** argv) {
    std::filesystem::path output;
    bool benchmarkRender=false;
    @autoreleasepool { try {
      auto o=parse(argc,argv);output=o.output;benchmarkRender=o.benchmarkRender;
      int translated=0;size_t length=sizeof(translated);int rc=sysctlbyname("sysctl.proc_translated",&translated,&length,nullptr,0);
#if !defined(__aarch64__)
      throw std::runtime_error("native ARM64 required");
#endif
      require((rc==0&&translated==0)||(rc==-1&&errno==ENOENT),"translated execution is forbidden");
      // Reject capabilities before allocating a GPU pipeline or any output.
      auto prepStart=Clock::now();auto scene=mb3d::metal_exact::PrepareExactScene(o.scene.string(),o.width,o.height,65536,o.hardShadows,o.precision);double prepareMs=elapsed(prepStart);
      if(o.describe) {
        NSDictionary* description=@{@"schema":@"mb3d.metal-exact-scene-description.v1",
          @"native_arm64":@YES,@"translated":@NO,@"gpu_dispatches":@0,
          @"scene_abi_version":@(scene.device.version),@"scene_bytes":@(sizeof(ExactScene)),
          @"pixel_bytes":@(sizeof(ExactPixel)),@"precision":ns(mb3d::PrecisionModeName(o.precision)),@"scene":sceneDescription(scene)};
        NSError* error=nil;
        NSData* data=[NSJSONSerialization dataWithJSONObject:description options:NSJSONWritingSortedKeys error:&error];
        require(data!=nil,error?error.description.UTF8String:"description JSON encoding failed");
        require(std::fwrite(data.bytes,1,data.length,stdout)==data.length,"description output failed");
        std::fputc('\n',stdout);return 0;
      }
      if(o.benchmarkRender) {
        renderBenchmark(o,scene,prepareMs,prepStart);
        return 0;
      }
      require(std::filesystem::create_directory(output),"cannot exclusively create output directory");
      auto setupStart=Clock::now();Metal metal(o.library,scene.device.formula_kind,scene.device.bulb.power,scene.device.capability_id,o.divisionMode=="restoring");double setupMs=elapsed(setupStart);
      auto abi=metal.buffer(14*sizeof(uint32_t));metal.run(metal.pipeline(@"exact_abi"),abi,nil,&scene.device,sizeof(scene.device),1);
      const uint32_t expected[]={sizeof(ExactScene),sizeof(ExactTrig),sizeof(ExactPixel),sizeof(ArithmeticInput),sizeof(mb3d_exact::ExactBoxInput),sizeof(mb3d_exact::ExactBoxOutput),scene.device.version,scene.device.width,scene.device.height,scene.device.de_stop,uint32_t(scene.device.params[0]),uint32_t(scene.device.params[0]>>32),scene.device.formula_kind,scene.device.capability_id};
      require(std::memcmp(abi.contents,expected,sizeof(expected))==0,"device ABI echo mismatch");
      auto nativeLog=nativeLogChecks(metal);auto arithmetic=arithmeticChecks(metal);auto sqrtSeeds=sqrtSeedChecks(metal);auto wideSqrtSeeds=sqrtWideSeedChecks(metal);auto divisionSeeds=divisionSeedChecks(metal);auto boxes=boxChecks(metal);auto extended=extendedArithmeticChecks(metal);
      if(o.fullShading) {
        auto single=singleArithmeticChecks(metal);
        auto shading=shadingChecks(metal,o,scene);
        const bool pass=[nativeLog[@"failures"] unsignedLongLongValue]==0&&[sqrtSeeds[@"pass"] boolValue]&&[wideSqrtSeeds[@"pass"] boolValue]&&[divisionSeeds[@"pass"] boolValue]&&[shading[@"pass"] boolValue]&&[single[@"failures"] unsignedLongLongValue]==0&&[arithmetic[@"failures"] unsignedLongLongValue]==0&&[extended[@"failures"] unsignedLongLongValue]==0&&[boxes[@"failed_rows"] unsignedLongLongValue]==0&&[boxes[@"unsupported_rejection_failures"] unsignedLongLongValue]==0;
        writeJSON(output/"report.json",@{@"schema":@"mb3d.metal-exact-shading.v1",@"status":pass?@"PASS_SCOPED":@"FAIL",@"numeric_profile":numericProfile(scene,true),@"device":metal.device.name,@"native_arm64":@YES,@"translated":@NO,@"cpu_compute_fallback_calls":@0,@"runtime_shader_compilation":@NO,@"abi_checks":@24,@"scene_abi_version":@(scene.device.version),@"scene_bytes":@(sizeof(ExactScene)),@"pixel_bytes":@(sizeof(ExactPixel)),@"scene_description":sceneDescription(scene),@"hardware":@{@"unified_memory":@(metal.device.hasUnifiedMemory),@"max_buffer_length":@(metal.device.maxBufferLength),@"max_threads_per_threadgroup":@[@(metal.device.maxThreadsPerThreadgroup.width),@(metal.device.maxThreadsPerThreadgroup.height),@(metal.device.maxThreadsPerThreadgroup.depth)],@"recommended_max_working_set_size":@(metal.device.recommendedMaxWorkingSetSize)},@"single_arithmetic":single,@"scene_preparation_ms":@(prepareMs),@"pipeline_creation_ms":@(metal.pipelineCreationMs),@"pipeline_specialization":@{@"formula_kind":@(metal.formulaKind),@"builtin_power":@(metal.bulbPower),@"capability_id":@(metal.capabilityId)},@"device_library_queue_setup_ms":@(setupMs),@"native_log":nativeLog,@"arithmetic":arithmetic,@"sqrt_seed_controls":sqrtSeeds,@"sqrt_wide_seed_controls":wideSqrtSeeds,@"division_seed_controls":divisionSeeds,@"division_mode":ns(o.divisionMode),@"extended_arithmetic":extended,@"box_original_fixtures":boxes,@"shading":shading,@"normal_pipeline":shading[@"normal_pipeline"]});
        fprintf(stderr,"Exact Metal shading: %s; RGB differences %llu; GPU %.3f ms; CPU %.3f ms\n",pass?"PASS_SCOPED":"FAIL",[shading[@"final_rgb"][@"different_pixels"] unsignedLongLongValue],[shading[@"gpu_timestamp_median_ms"] doubleValue],[shading[@"cpu_full_render_median_ms"] doubleValue]);
        return pass?0:1;
      }
      auto primary=sceneChecks(metal,o,scene);
      bool pass=[nativeLog[@"failures"] unsignedLongLongValue]==0&&[sqrtSeeds[@"pass"] boolValue]&&[wideSqrtSeeds[@"pass"] boolValue]&&[divisionSeeds[@"pass"] boolValue]&&[extended[@"failures"] unsignedLongLongValue]==0&&[arithmetic[@"failures"] unsignedLongLongValue]==0&&[boxes[@"failed_rows"] unsignedLongLongValue]==0&&[boxes[@"unsupported_rejection_failures"] unsignedLongLongValue]==0&&
        [primary[@"normal_pipeline"][@"pass"] boolValue]&&[primary[@"primary_dispatch"][@"pass"] boolValue]&&[primary[@"comparison"][@"failures"] unsignedLongLongValue]==0&&[primary[@"comparison"][@"different_numeric_bits"] unsignedLongLongValue]==0&&[primary[@"gpu_repeats_bit_exact"] boolValue]&&[primary[@"negative_controls_pass"] boolValue]&&[primary[@"formula_rejections"][@"pass"] boolValue]&&[primary[@"diagnostic_rgb_pixel_differences"] unsignedLongLongValue]==0;
      NSDictionary* report=@{@"schema":@"mb3d.metal-exact-scene.v1",@"status":pass?@"PASS_SCOPED":@"FAIL",@"numeric_profile":numericProfile(scene,false),@"device":metal.device.name,
        @"native_arm64":@YES,@"translated":@NO,@"offline_library":ns(std::filesystem::absolute(o.library).string()),@"abi_checks":@14,@"scene_abi_version":@(scene.device.version),@"scene_bytes":@(sizeof(ExactScene)),@"pixel_bytes":@(sizeof(ExactPixel)),@"scene_description":sceneDescription(scene),
        @"scene_preparation_ms":@(prepareMs),@"pipeline_creation_ms":@(metal.pipelineCreationMs),@"pipeline_specialization":@{@"formula_kind":@(metal.formulaKind),@"builtin_power":@(metal.bulbPower),@"capability_id":@(metal.capabilityId)},@"device_library_queue_setup_ms":@(setupMs),@"native_log":nativeLog,@"arithmetic":arithmetic,@"sqrt_seed_controls":sqrtSeeds,@"sqrt_wide_seed_controls":wideSqrtSeeds,@"division_seed_controls":divisionSeeds,@"division_mode":ns(o.divisionMode),@"extended_arithmetic":extended,@"cpu_compute_fallback_calls":@0,@"runtime_shader_compilation":@NO,@"box_original_fixtures":boxes,@"primary":primary,@"normal_pipeline":primary[@"normal_pipeline"]};
      writeJSON(output/"report.json",report);
      fprintf(stderr,"Exact Metal: %s; arithmetic failures %llu; box failures %llu; scene failures %llu; GPU median %.3f ms\n",pass?"PASS_SCOPED":"FAIL",[arithmetic[@"failures"] unsignedLongLongValue],[boxes[@"failed_rows"] unsignedLongLongValue],[primary[@"comparison"][@"failures"] unsignedLongLongValue],[primary[@"gpu_timestamp_median_ms"] doubleValue]);
      return pass?0:1;
    } catch(const std::exception& e) {
      fprintf(stderr,"Exact Metal error: %s\n",e.what());
      if(!output.empty()&&std::filesystem::is_directory(output)&&!std::filesystem::exists(output/"report.json")) {
        try {writeJSON(output/"report.json",@{@"schema":benchmarkRender?@"mb3d.metal-render-benchmark.v1":@"mb3d.metal-exact-scene.v1",@"status":@"FAILED_INFRASTRUCTURE",@"error":ns(e.what())});}catch(...){}
      } return 1;
    }}
}
