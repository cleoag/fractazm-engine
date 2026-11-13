#include "log_fixture.h"
#include <cstdio>
using namespace mb3d_metal_exact;
int main() {
    const auto cases=log_inputs();
    unsigned failures=0,logs=0,fmas=0;
    for(const auto& p:cases) {
        p.op?++logs:++fmas;
        const F64 expected=native_log_expected(p),actual=evaluate_log_input(p);
        if(!log_result_equal(actual,expected)&&failures++<20)
            std::fprintf(stderr,"%s a=%016llx b=%016llx c=%016llx got=%016llx expected=%016llx\n",
                p.op?"log":"fma",p.a,p.b,p.c,actual.bits,expected.bits);
    }
    std::printf("native log %u, fused multiply-add %u, mismatches %u\n",logs,fmas,failures);
    return failures?1:0;
}
