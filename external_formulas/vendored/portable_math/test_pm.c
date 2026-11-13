/* test_pm.c — sanity check: print bits of pm_sin/cos/log for known inputs.
 * Compare against Python math.sin/cos/log to verify polynomial is correct.
 */
#include "portable_math.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static void print_bits(const char* name, double v) {
    uint64_t b;
    memcpy(&b, &v, sizeof(b));
    printf("%s = %.17g  bits=0x%016llX\n", name, v, (unsigned long long)b);
}

int main(void) {
    print_bits("pm_sin(1.23456789)",  pm_sin(1.23456789));
    print_bits("pm_cos(1.23456789)",  pm_cos(1.23456789));
    print_bits("pm_sin(0.5)",          pm_sin(0.5));
    print_bits("pm_cos(0.5)",          pm_cos(0.5));
    print_bits("pm_sin(0.0)",          pm_sin(0.0));
    print_bits("pm_sin(-0.0)",         pm_sin(-0.0));
    print_bits("pm_cos(0.0)",          pm_cos(0.0));
    print_bits("pm_sin(3.14159265)",   pm_sin(3.14159265));
    print_bits("pm_log(2.718281828)",  pm_log(2.718281828));
    print_bits("pm_log(2.0)",          pm_log(2.0));
    print_bits("pm_sqrt(2.0)",         pm_sqrt(2.0));
    return 0;
}
