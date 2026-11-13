#include "division_abi.h"
// Include after Metal, require/ns and the independent native db/bd helpers.
static NSDictionary* divisionSeedChecks(Metal& m) {
    using U128 = unsigned __int128;
    std::vector<std::pair<uint64_t,uint64_t>> values;
    const uint64_t fractions[]{0, 1, 0x000fffffffffffffULL};
    for (unsigned exponent=0; exponent<2048; ++exponent)
        for (uint64_t fraction:fractions)
            for (uint64_t sign:{0ULL,0x8000000000000000ULL})
                values.emplace_back((uint64_t(exponent)<<52)|fraction|sign,
                    fraction==1?0xbfffffffffffffffULL:0x3ff0000000000001ULL);
    uint64_t state=0x7dbe6ea0291c4513ULL;
    auto random=[&]{state^=state<<13;state^=state>>7;state^=state<<17;return state;};
    for (unsigned i=0;i<2048;++i) {
        const uint64_t a=random(),b=random();values.emplace_back(a,b);
    }
    const uint64_t special[]{0,0x8000000000000000ULL,1,0x8000000000000001ULL,
        0x000fffffffffffffULL,0x0010000000000000ULL,0x0010000000000001ULL,
        0x3ff0000000000000ULL,0xbff0000000000000ULL,0x3ff0000000000001ULL,
        0x7fefffffffffffffULL,0x7ff0000000000000ULL,0xfff0000000000000ULL,
        0x7ff0000000000001ULL,0x7ff8000000001234ULL,0xfff923456789abcdULL};
    for(uint64_t a:special) for(uint64_t b:special) values.emplace_back(a,b);
    require(values.size()==14592,"division value matrix denominator changed");
    constexpr unsigned variants=12;
    const char* names[variants]={"exact","minus_one","plus_one","minus_two","plus_two",
        "minus_three","plus_three","zero","limit_minus_one","limit","uint64_max","hardware"};
    std::vector<DivisionSeedInput> input; input.reserve(values.size()*variants+48);
    std::vector<unsigned> categories; categories.reserve(input.capacity());
    for(size_t index=0;index<values.size();++index) {
        const auto& value=values[index];
        const uint64_t denominator=(1ULL<<52)|(random()&((1ULL<<52)-1));
        const uint64_t numerator=random()%(2*denominator);
        const unsigned shift=index&1?19:18;
        const uint64_t quotient=uint64_t((U128(numerator)<<shift)/denominator);
        const float hardware=(float(numerator)*(524288.0f/float(denominator)))*(shift==18?0.5f:1.0f);
        const uint64_t hints[variants]={quotient,quotient-1,quotient+1,quotient-2,quotient+2,
            quotient-3,quotient+3,0,(1ULL<<20)-1,1ULL<<20,~0ULL,uint64_t(hardware)};
        // The value seed is independent of the digit input above. Normalize
        // with the native exponent encoding, not the software unpack helper.
        uint64_t sig=value.second&0x000fffffffffffffULL;
        const unsigned encoded=unsigned((value.second>>52)&2047);
        if(encoded&&encoded!=2047)sig|=1ULL<<52;
        else if(!encoded&&sig)while(sig<(1ULL<<52))sig<<=1;
        float reciprocal=sig?524288.0f/float(sig):0.0f;
        uint32_t seed=0;std::memcpy(&seed,&reciprocal,sizeof(seed));
        const uint32_t reciprocalBits[variants]={seed,seed?seed-1:0,seed+1,0,0x80000000U,
            0x7fc00001U,0x7f800000U,0xff800000U,0xbf800000U,0x3f800000U,1,0x7f7fffffU};
        for(unsigned v=0;v<variants;++v) {
            input.push_back({value.first,value.second,numerator,denominator,hints[v],shift,reciprocalBits[v]});
            categories.push_back(v);
        }
    }
    // Invalid digit domains must reject before shifts, products or division.
    for(uint64_t denominator:{0ULL,(1ULL<<52)-1,1ULL<<53,~0ULL})
        for(unsigned shift:{0U,18U,19U,64U})
            for(uint64_t hint:{0ULL,1ULL,~0ULL}) {
                input.push_back({0x3ff0000000000001ULL,0x3fffffffffffffffULL,
                    ~0ULL,denominator,hint,shift,0x7fc00001U});
                categories.push_back(variants);
            }
    require(input.size()==175152,"division seed record denominator changed");
    uint32_t count=uint32_t(input.size());
    auto source=m.buffer(input.size()*sizeof(DivisionSeedInput),input.data());
    auto result=m.buffer(input.size()*sizeof(DivisionSeedOutput));
    std::memset(result.contents,0xa5,result.length);
    const double gpuMs=m.run(m.pipeline(@"exact_division_seed_controls"),source,result,&count,sizeof(count),count);
    const auto* actual=static_cast<const DivisionSeedOutput*>(result.contents);
    std::array<uint64_t,variants+1> records{},requiredAccepts{},requiredRejects{},accepted{},rejected{},
        valueFailures{},digitFailures{},correctionAccepts{};
    uint64_t failures=0,totalRequiredAccepts=0,totalRequiredRejects=0,totalCorrections=0;
    NSMutableArray* examples=[NSMutableArray array];
    for(size_t i=0;i<input.size();++i) {
        const auto& p=input[i];const auto& got=actual[i];const unsigned group=categories[i];
        volatile double a=bd(p.a),b=bd(p.b);const double native=a/b;
        const uint64_t expected=std::isnan(native)?0x7ff8000000000000ULL:db(native);
        const bool valueOK=got.bits==expected&&got.restoring_bits==expected&&got.hinted_bits==expected;
        const bool domain=p.denominator>=(1ULL<<52)&&p.denominator<(1ULL<<53)&&
            p.numerator<2*p.denominator&&(p.shift==18||p.shift==19);
        const U128 target=U128(p.numerator)<<(p.shift<64?p.shift:0);
        const uint64_t quotient=domain?uint64_t(target/p.denominator):0;
        const uint64_t remainder=domain?uint64_t(target%p.denominator):0;
        const uint64_t distance=p.hint>quotient?p.hint-quotient:quotient-p.hint;
        const bool mustAccept=domain&&p.hint<(1ULL<<20)&&distance<=2;
        const bool digitOK=got.valid==unsigned(mustAccept)&&got.reserved==0&&
            (mustAccept?(got.quotient==quotient&&got.remainder==remainder&&
                U128(got.quotient)*p.denominator+got.remainder==target&&got.remainder<p.denominator):
                got.quotient==0&&got.remainder==0);
        ++records[group];requiredAccepts[group]+=mustAccept;requiredRejects[group]+=!mustAccept;
        accepted[group]+=got.valid==1;rejected[group]+=got.valid==0;
        correctionAccepts[group]+=mustAccept&&p.hint!=quotient;
        totalRequiredAccepts+=mustAccept;totalRequiredRejects+=!mustAccept;
        totalCorrections+=mustAccept&&p.hint!=quotient;
        valueFailures[group]+=!valueOK;digitFailures[group]+=!digitOK;
        failures+=got.bits!=expected;failures+=got.restoring_bits!=expected;
        failures+=got.hinted_bits!=expected;failures+=!digitOK;
        if((!valueOK||!digitOK)&&examples.count<12)[examples addObject:@{
            @"index":@(i),@"case":ns(group<variants?names[group]:"invalid_domain"),
            @"a":@(p.a),@"b":@(p.b),@"hint":@(p.hint),@"expected_bits":@(expected),
            @"actual_bits":@(got.bits),@"hinted_bits":@(got.hinted_bits),
            @"value_pass":@(valueOK),@"digit_pass":@(digitOK)}];
    }
    NSMutableArray* cases=[NSMutableArray array];
    for(unsigned group=0;group<=variants;++group)[cases addObject:@{
        @"case":ns(group<variants?names[group]:"invalid_domain"),@"records":@(records[group]),
        @"required_accepts":@(requiredAccepts[group]),@"required_rejections":@(requiredRejects[group]),
        @"accepted_digits":@(accepted[group]),@"rejected_digits":@(rejected[group]),
        @"correction_accepts":@(correctionAccepts[group]),
        @"value_failures":@(valueFailures[group]),@"digit_failures":@(digitFailures[group])}];
    return @{@"pass":@(failures==0),@"matrix_inputs":@(values.size()),@"seed_variants":@(variants),
        @"invalid_domain_checks":@48,@"records":@(count),@"checks":@(uint64_t(count)*4),
        @"value_checks":@(uint64_t(count)*3),@"digit_checks":@(count),
        @"required_accepts":@(totalRequiredAccepts),@"required_rejections":@(totalRequiredRejects),
        @"correction_accepts":@(totalCorrections),@"failures":@(failures),
        @"input_bytes":@(sizeof(DivisionSeedInput)),@"record_bytes":@(sizeof(DivisionSeedOutput)),
        @"gpu_timestamp_ms":@(gpuMs),@"cases":cases,@"failure_examples":examples,
        @"gate":@"all value bits exact with canonical NaNs; digit exact quotient/remainder or zero rejection",
        @"oracle":@"native ARM64 hardware binary64 division; independent unsigned __int128 quotient/remainder"};
}
