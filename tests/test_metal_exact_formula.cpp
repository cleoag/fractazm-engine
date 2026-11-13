#include "comparison/metal_exact/formula_fixture.h"
#include "comparison/metal_exact/soft_double.h"

#include <cstdio>
#include <stdexcept>

namespace {
void check(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
}

int main() {
    try {
        using namespace mb3d_exact;
        using mb3d_metal_exact::F64;
        const auto counts = fixture_host::box_counts();
        const auto inputs = fixture_host::box_inputs();
        const auto original = fixture_host::box_expected();
        check(inputs.size() == original.size() && counts.total == inputs.size(), "Fixture import lost original rows");
        std::vector<ExactBoxOutput> outputs;
        outputs.reserve(inputs.size());
        for (const auto& input : inputs) outputs.push_back(exact_box<F64>(input));
        const auto stats = fixture_host::check_box_outputs(outputs);
        check(stats.rows == 1540 && stats.values == 30800 && stats.unsupported_rows == 70,
              "Fixture qualification denominator changed");
        check(stats.failed_rows == 0 && stats.failed_values == 0 && stats.unsupported_rejection_failures == 0,
              "Software binary64 boxIFS disagrees with original fixture or unsupported input policy");

        // A successful oracle replay must not mask invalid parameter records.
        auto input = inputs.front();
        input.reserved = 1;
        check(exact_box<F64>(input).status == box_invalid_input, "Reserved ABI field accepted");
        input = inputs.front();
        input.trap_option = 4;
        check(exact_box<F64>(input).status == box_invalid_input, "Unnormalized trap selector accepted");
        input = inputs.front();
        input.twist_bits = 0x80000000u;
        check(exact_box<F64>(input).status == box_unsupported_twist, "Negative-zero Twist silently bypassed x87 branch");

        // Verify the acceptance checker detects finite, exceptional and exact
        // zero-sign mismatches instead of relying only on image appearance.
        auto broken = outputs;
        broken[0].state[0] = std::bit_cast<std::uint64_t>(100.0);
        check(fixture_host::check_box_outputs(broken).failed_values == 1, "Point gate missed finite mismatch");
        broken = outputs;
        broken[0].state[0] = 0x7ff0000000000000ULL;
        check(fixture_host::check_box_outputs(broken).class_failures == 1, "Point gate missed finite/infinite mismatch");
        broken = outputs;
        broken[0].state[7] = 0x8000000000000000ULL;
        check(fixture_host::check_box_outputs(broken).signed_zero_failures == 1, "Point gate missed signed zero mismatch");
        broken.pop_back();
        bool rejected = false;
        try { fixture_host::check_box_outputs(broken); }
        catch (const std::runtime_error&) { rejected = true; }
        check(rejected, "Point gate accepted truncated output");
        std::printf("Metal exact boxIFS CPU adapter: %zu rows, %zu unchanged-gate values, %zu exact bits; "
                    "%zu Twist rows explicitly unsupported; 7 rejection/validator checks passed.\n",
                    stats.rows, stats.values, stats.exact_bits, stats.unsupported_rows);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Metal exact boxIFS CPU adapter FAILED: %s\n", error.what());
        return 1;
    }
}
