#include "ao_math.h"
#include "shadow_abi.h"

kernel void exact_ao_init(device ExactSurface* surface [[buffer(0)]],
                          device uint* depth [[buffer(1)]],
                          device short* angles [[buffer(2)]],
                          constant ExactAOScene& scene [[buffer(3)]],
                          constant uint& count [[buffer(4)]],
                          uint pixel [[thread_position_in_grid]]) {
    if (pixel >= count) return;
    ExactAOScene local = scene;
    if (!mb3d_metal_exact::valid_ao(local) || local.width*local.height != count) {
        surface[pixel].fault |= 8; return;
    }
    const uint raw = surface[pixel].depth_packed;
    depth[pixel] = raw & 0x80000000u ? 0 : (raw&0xffffff00u)>>1;
    for (uint direction = 0; direction < 32; ++direction) angles[pixel*32+direction] = -32768;
}

kernel void exact_ao_sample(device ExactSurface* surface [[buffer(0)]],
                            device const uint* depth [[buffer(1)]],
                            device short* angles [[buffer(2)]],
                            device const ulong* atan_terms [[buffer(3)]],
                            constant ExactAOScene& scene [[buffer(4)]],
                            constant ExactAOLevel& level [[buffer(5)]],
                            constant uint& count [[buffer(6)]],
                            uint pixel [[thread_position_in_grid]]) {
    if (pixel >= count) return;
    ExactAOScene local = scene; ExactAOLevel step = level;
    if (!mb3d_metal_exact::valid_ao(local) || local.width*local.height != count ||
        !mb3d_metal_exact::valid_ao_level(local,step)) { surface[pixel].fault |= 8; return; }
    if (surface[pixel].fault || (surface[pixel].depth_packed & 0x80000000u)) return;
    if (!mb3d_metal_exact::ao_scan_pixel(local,step,depth,angles,pixel,surface[pixel].depth_packed)) {
        surface[pixel].fault |= 8; return;
    }
    if (step.index == local.num_levels)
        surface[pixel].amb_shadow = mb3d_metal_exact::ao_sum_pixel(angles,atan_terms,step,pixel);
}

kernel void exact_ao_wavelet_h(device const uint* input [[buffer(0)]],
                               device uint* output [[buffer(1)]],
                               constant ExactAOScene& scene [[buffer(2)]],
                               constant uint& step [[buffer(3)]],
                               constant uint& count [[buffer(4)]],
                               uint pixel [[thread_position_in_grid]]) {
    if (pixel >= count) return;
    ExactAOScene local = scene;
    if (!mb3d_metal_exact::valid_ao(local) || local.width*local.height != count || step == 0 || step > 32768) return;
    const int x = int(pixel%scene.width), row = int(pixel)-x;
    const int a = max(0,x-int(step)), b = min(int(scene.width)-1,x+int(step));
    output[pixel] = (input[pixel]+(input[row+a]+input[row+b])/2)/2;
}

kernel void exact_ao_wavelet_v(device const uint* input [[buffer(0)]],
                               device uint* output [[buffer(1)]],
                               constant ExactAOScene& scene [[buffer(2)]],
                               constant uint& step [[buffer(3)]],
                               constant uint& count [[buffer(4)]],
                               uint pixel [[thread_position_in_grid]]) {
    if (pixel >= count) return;
    ExactAOScene local = scene;
    if (!mb3d_metal_exact::valid_ao(local) || local.width*local.height != count || step == 0 || step > 32768) return;
    const int x = int(pixel%scene.width), y = int(pixel/scene.width), width = int(scene.width);
    const int a = max(0,y-int(step)), b = min(int(scene.height)-1,y+int(step));
    output[pixel] = (input[pixel]+(input[a*width+x]+input[b*width+x])/2)/2;
}
