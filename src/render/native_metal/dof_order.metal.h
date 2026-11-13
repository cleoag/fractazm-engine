// Exact permutation of core/dof.cpp::QuickSortImpl, including equal keys.
// Input items are generated on GPU in row-major order, key = packed_depth>>8;
// keys > 8388352 become -1. No CPU sorting or pixel preparation is permitted.
#include <metal_stdlib>
#include "dof_abi.h"
using namespace metal;
namespace mb3d_dof {

// One invocation owns the permutation. Quota counts comparisons and mutations,
// so even a deliberately adversarial pivot sequence yields between dispatches.
// The host must poll cancellation, validate fault/done/work, and re-dispatch.
inline void sort_slice(device Item* items,thread SortState& s,constant SortLaunch& p) {
    if (s.version != 1 || p.reserved || !p.count || p.count > 16777216u ||
        p.count > p.allocation_count || s.count != p.count || p.quota == 0 ||
        p.quota > 65536 || !p.total_limit || s.reserved0 || s.reserved1 ||
        s.reserved2 || s.stack_size > 32 || s.phase > 3 || s.done > 1 ||
        (s.phase && (s.lo < 0 || s.hi < s.lo || uint(s.hi) >= p.count ||
          s.left < s.lo-1 || s.left > s.hi || s.right < s.lo-1 || s.right > s.hi)) ||
        (s.phase == 3 && s.left < s.lo)) { s.fault = 1; return; }
    if (s.done || s.fault) return;
    for (uint k = 0; k < p.quota; ++k) {
        if (s.work >= p.total_limit) { s.fault = 2; return; }
        ++s.work;
        if (s.phase == 0) {
            if (!s.stack_size) { s.done = 1; return; }
            --s.stack_size;
            s.lo = s.stack_lo[s.stack_size]; s.hi = s.stack_hi[s.stack_size];
            if (s.lo < 0 || s.hi < s.lo || uint(s.hi) >= p.count) { s.fault = 3; return; }
            if (s.lo == s.hi) continue;
            s.pivot = items[s.hi].key; s.left = s.lo-1; s.right = s.hi; s.phase = 1;
        } else if (s.phase == 1) {
            ++s.left;
            if (s.left < s.lo || s.left > s.hi) { s.fault = 3; return; }
            if (items[s.left].key >= s.pivot) s.phase = 2;
        } else if (s.phase == 2) {
            --s.right;
            if (s.right < s.lo-1 || s.right >= s.hi) { s.fault = 3; return; }
            if (!(s.right > s.left && items[s.right].key > s.pivot)) s.phase = 3;
        } else if (s.left < s.right) {
            const Item tmp = items[s.left]; items[s.left] = items[s.right]; items[s.right] = tmp;
            s.phase = 1;
        } else {
            const Item tmp = items[s.left]; items[s.left] = items[s.hi]; items[s.hi] = tmp;
            // Partition order is independent: both children touch disjoint ranges.
            // Visit the smaller child first and stack the larger to bound space.
            int a = s.lo, b = s.left-1, c = s.left+1, d = s.hi;
            if (b-a > d-c) { int t=a; a=c; c=t; t=b; b=d; d=t; }
            if (d > c) {
                if (s.stack_size == 32) { s.fault = 4; return; }
                s.stack_lo[s.stack_size] = c; s.stack_hi[s.stack_size++] = d;
            }
            if (b > a) {
                s.lo=a; s.hi=b; s.pivot=items[b].key; s.left=a-1; s.right=b; s.phase=1;
            } else s.phase=0;
        }
    }
}
// Only this invocation owns the state. Keep its scalar cursor/stack private
// during the bounded slice, then publish once; integer permutation and work
// order are identical to the device-state implementation.
[[host_name("native_dof_sort")]] kernel void staged_dof_sort(device Item* items [[buffer(0)]],
    device SortState& output_state [[buffer(1)]],constant SortLaunch& p [[buffer(2)]],
    uint thread_id [[thread_position_in_grid]]) {
    if(thread_id)return;
    SortState local=output_state;
    sort_slice(items,local,p);
    output_state=local;
}
} // namespace mb3d_dof
