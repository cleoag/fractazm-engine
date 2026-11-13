#include "soft_single.h"
#include "single_test_cases.h"
#include <cstdio>
using namespace mb3d_metal_exact;
int main() {
    uint64_t checks=0,failures=0,per_op[12]{},bad_op[12]{};
    const auto input=mb3d_single_test::cases(100000);
    for(const auto p:input) {
        F32 a=F32::from_bits(p.a),b=F32::from_bits(p.b),value;
        switch(p.op) {
            case 0:value=a+b;break;case 1:value=a-b;break;case 2:value=a*b;break;case 3:value=a/b;break;
            case 4:value=sqrt(a);break;case 5:value=-a;break;case 6:value=abs(a);break;
            case 7:value=min(a,b);break;case 8:value=max(a,b);break;
            case 9:value=F32(int(a<b));break;case 10:value=F32(int(a==b));break;case 11:value=F32(int(a<=b));break;
        }
        const SingleOutput actual{value.bits,value.to_i32_rtz(),value.to_i32_rne(),0,value.to_f64().bits,0};
        const float expected=mb3d_single_test::native_value(p);
        const unsigned bad=mb3d_single_test::failure_count(actual,expected);
        checks+=5;per_op[p.op]+=5;failures+=bad;bad_op[p.op]+=bad;
        if(bad && failures<=30)std::fprintf(stderr,"single op%u a=%08x b=%08x got=%08x expected=%08x bad=%u\n",p.op,p.a,p.b,value.bits,std::bit_cast<uint32_t>(expected),bad);
    }
    std::printf("{\"profile\":\"binary32-rne-value-only\",\"rows\":%zu,\"checks\":%llu,\"failures\":%llu,\"operations\":[",input.size(),(unsigned long long)checks,(unsigned long long)failures);
    for(unsigned op=0;op<12;++op)std::printf("%s{\"operation\":%u,\"checks\":%llu,\"failures\":%llu}",op?",":"",op,(unsigned long long)per_op[op],(unsigned long long)bad_op[op]);
    std::printf("]}\n");return failures?1:0;
}
