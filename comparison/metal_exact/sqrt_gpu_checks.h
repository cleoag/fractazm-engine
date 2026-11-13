// Included after the narrow-prefix controls. The wide-prefix oracle uses
// integer binary search and native sqrt, never the software sqrt helpers.
static uint64_t independentWideSqrtHigh(uint64_t bits) {
    const unsigned encoded=unsigned((bits>>52)&2047);
    uint64_t sig=bits&0x000fffffffffffffULL;
    if((bits>>63)||encoded==2047||(!encoded&&!sig))return 0;
    int exponent=encoded?int(encoded)-1023:-1022;
    if(encoded)sig|=1ULL<<52;
    else while(sig<(1ULL<<52)){sig<<=1;--exponent;}
    return sig<<(exponent%2!=0?11:10);
}
static uint64_t independentWidePrefixRoot(uint64_t high) {
    uint64_t low=0,upper=1ULL<<32;
    while(low+1<upper) {
        const uint64_t middle=low+(upper-low)/2;
        if(static_cast<unsigned __int128>(middle)*middle<=high)low=middle;
        else upper=middle;
    }
    return low;
}
static NSDictionary* sqrtWideSeedChecks(Metal& m) {
    auto values=sqrtTargetInputs();
    const uint64_t special[]={0x8000000000000000ULL,0xbff0000000000000ULL,0x8000000000000001ULL,
      0xffefffffffffffffULL,0x7ff0000000000000ULL,0xfff0000000000000ULL,
      0x7ff8000000000001ULL,0x7ff0000000000001ULL,0xfff8000000000001ULL};
    values.insert(values.end(),std::begin(special),std::end(special));
    require(values.size()==19712,"wide sqrt value matrix denominator changed");
    std::vector<std::pair<uint64_t,uint64_t>> matrix;
    for(uint64_t value:values)matrix.emplace_back(value,independentWideSqrtHigh(value));
    const uint64_t highEdges[]={0,1,2,3,4,15,16,17,
      (1ULL<<32)-1,1ULL<<32,(1ULL<<32)+1,
      (1ULL<<62)-1,1ULL<<62,(1ULL<<62)+1,
      (1ULL<<63)-1,1ULL<<63,(1ULL<<63)+1,~0ULL-2048,~0ULL-1024,~0ULL-1,~0ULL};
    std::vector<uint64_t> boundaries(std::begin(highEdges),std::end(highEdges));
    const uint64_t roots[]={0,1,2,3,4,31,(1ULL<<16)-1,1ULL<<16,(1ULL<<16)+1,
      (1ULL<<24)-1,1ULL<<24,(1ULL<<24)+1,(1ULL<<31)-1,1ULL<<31,(1ULL<<31)+1,
      (1ULL<<32)-2,(1ULL<<32)-1,1ULL<<32};
    for(uint64_t root:roots) {
        const unsigned __int128 square=static_cast<unsigned __int128>(root)*root;
        for(int offset=-2;offset<=2;++offset) {
            if(offset<0&&square<uint64_t(-offset))continue;
            const unsigned __int128 adjacent=offset<0?square-uint64_t(-offset):square+uint64_t(offset);
            if(adjacent<=~uint64_t(0))boundaries.push_back(uint64_t(adjacent));
        }
    }
    uint64_t boundaryState=0x6a09e667f3bcc909ULL;
    for(unsigned i=0;i<64;++i) {
        boundaryState^=boundaryState<<13;boundaryState^=boundaryState>>7;boundaryState^=boundaryState<<17;
        boundaries.push_back(boundaryState);
    }
    require(boundaries.size()==169,"wide sqrt prefix boundary denominator changed");
    for(uint64_t high:boundaries)matrix.emplace_back(0x3ff0000000000000ULL,high);
    constexpr unsigned variants=20,categories=21;
    const char* names[categories]={"exact","minus_one","plus_one","minus_four","plus_four",
      "minus_256","plus_256","minus_1023","plus_1023","minus_1024","plus_1024",
      "minus_1025","plus_1025","zero","root_max","limit","uint64_max","hardware32",
      "random32","random64","root_max_clamp"};
    std::vector<SqrtSeedInput> input;std::vector<unsigned> category;
    input.reserve(matrix.size()*variants+16);category.reserve(input.capacity());
    uint64_t state=0x67129b4de8a03cf5ULL;
    auto random=[&]{state^=state<<13;state^=state>>7;state^=state<<17;return state;};
    for(const auto& entry:matrix) {
        const uint64_t root=independentWidePrefixRoot(entry.second);
        const float estimate=std::sqrt(float(entry.second));
        const uint64_t hardware=estimate>=0.0f&&estimate<4294967296.0f?uint64_t(estimate):1ULL<<32;
        const uint64_t hints[variants]={root,root-1,root+1,root-4,root+4,root-256,root+256,
          root-1023,root+1023,root-1024,root+1024,root-1025,root+1025,
          0,(1ULL<<32)-1,1ULL<<32,~0ULL,hardware,uint64_t(uint32_t(random())),random()};
        for(unsigned seed=0;seed<variants;++seed) {
            input.push_back({entry.first,hints[seed],entry.second});category.push_back(seed);
        }
    }
    // These residual corrections round toward the one-past-root proposal;
    // only the integer rootMax clamp plus exact bracket makes them admissible.
    for(uint64_t high:{~0ULL,~0ULL-1,~0ULL-1024,~0ULL-2048})
        for(uint64_t distance:{127ULL,128ULL,255ULL,256ULL}) {
            input.push_back({0x400fffffffffffffULL,(1ULL<<32)-1-distance,high});
            category.push_back(variants);
        }
    require(input.size()==397636,"wide sqrt seed record denominator changed");
    uint32_t count=uint32_t(input.size());
    auto source=m.buffer(input.size()*sizeof(SqrtSeedInput),input.data());
    auto result=m.buffer(input.size()*sizeof(SqrtSeedOutput));
    std::memset(result.contents,0xa5,result.length);
    const double gpuMs=m.run(m.pipeline(@"exact_sqrt_wide_seed_controls"),source,result,&count,sizeof(count),count);
    const auto* actual=static_cast<const SqrtSeedOutput*>(result.contents);
    std::array<uint64_t,categories> records{},requiredAccepts{},requiredRejects{},optional{},
      accepted{},rejected{},sqrtFailures{},prefixFailures{};
    uint64_t failures=0,nans=0,zeros=0;NSMutableArray* examples=[NSMutableArray array];
    for(size_t i=0;i<input.size();++i) {
        const auto& p=input[i];const auto& got=actual[i];const unsigned group=category[i];
        volatile double value=bd(p.a);const uint64_t expected=db(std::sqrt(value));
        const bool nan=nanbits(expected);nans+=nan;zeros+=(expected&0x7fffffffffffffffULL)==0;
        const bool valueOK=nan?nanbits(got.bits):got.bits==expected;
        const uint64_t root=independentWidePrefixRoot(p.high);
        const uint64_t distance=p.hint>root?p.hint-root:root-p.hint;
        const bool mustAccept=p.hint==root||(p.high>=(1ULL<<62)&&p.hint<(1ULL<<32)&&distance<=256)||
                              (group==17&&p.hint<(1ULL<<32));
        bool mustReject=p.hint>=(1ULL<<32)||(!p.hint&&p.high);
        if(p.hint&&p.hint<(1ULL<<32)) {
            const uint64_t square=p.hint*p.hint;
            const uint64_t magnitude=square>p.high?square-p.high:p.high-square;
            mustReject|=magnitude>4096*p.hint;
        }
        require(!(mustAccept&&mustReject),"wide sqrt independent coverage classification overlaps");
        const unsigned __int128 q=got.root;
        const bool prefixOK=got.valid<=1&&got.reserved==0&&
          (got.valid?(got.root==root&&q*q<=p.high&&(q+1)*(q+1)>p.high&&
                       got.remainder==p.high-root*root):(got.root==0&&got.remainder==0))&&
          (!mustAccept||got.valid==1)&&(!mustReject||got.valid==0);
        ++records[group];requiredAccepts[group]+=mustAccept;requiredRejects[group]+=mustReject;
        optional[group]+=!mustAccept&&!mustReject;accepted[group]+=got.valid!=0;rejected[group]+=got.valid==0;
        sqrtFailures[group]+=!valueOK;prefixFailures[group]+=!prefixOK;
        failures+=!valueOK;failures+=!prefixOK;
        if((!valueOK||!prefixOK)&&examples.count<12)[examples addObject:@{
          @"index":@(i),@"case":ns(names[group]),@"a":@(p.a),@"hint":@(p.hint),@"high":@(p.high),
          @"expected_bits":@(expected),@"actual_bits":@(got.bits),@"sqrt_pass":@(valueOK),@"prefix_pass":@(prefixOK)}];
    }
    require(nans==140&&zeros==40,"wide sqrt class coverage changed");
    NSMutableArray* cases=[NSMutableArray array];
    for(unsigned group=0;group<categories;++group)[cases addObject:@{
      @"case":ns(names[group]),@"records":@(records[group]),@"required_accepts":@(requiredAccepts[group]),
      @"required_rejections":@(requiredRejects[group]),@"optional_prefixes":@(optional[group]),
      @"accepted_prefixes":@(accepted[group]),@"rejected_prefixes":@(rejected[group]),
      @"sqrt_failures":@(sqrtFailures[group]),@"prefix_failures":@(prefixFailures[group])}];
    return @{@"pass":@(failures==0),@"matrix_inputs":@(values.size()),@"boundary_prefix_inputs":@(boundaries.size()),
      @"seed_variants":@(variants),@"clamp_checks":@16,@"records":@(count),@"checks":@(uint64_t(count)*2),
      @"sqrt_checks":@(count),@"prefix_checks":@(count),@"nan_class_checks":@(nans),@"signed_zero_checks":@(zeros),
      @"input_bytes":@(sizeof(SqrtSeedInput)),@"record_bytes":@(sizeof(SqrtSeedOutput)),
      @"failures":@(failures),@"gpu_timestamp_ms":@(gpuMs),@"cases":cases,@"failure_examples":examples,
      @"gate":@"sqrt non-NaN bits exact; NaN class; accepted prefix exact bracket/remainder; rejected prefix zero",
      @"oracle":@"native ARM64 std::sqrt(double); independent unsigned __int128 prefix binary search"};
}
