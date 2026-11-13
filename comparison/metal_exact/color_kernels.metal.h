#include "color_math.h"
kernel void exact_color(constant ExactColorScene& scene [[buffer(0)]],
                        device const ExactTrig* columns [[buffer(1)]],
                        device const ExactTrig* rows [[buffer(2)]],
                        device const ExactSurface* surface [[buffer(3)]],
                        device ExactColorPixel* output [[buffer(4)]],
                        constant uint* launch [[buffer(5)]],
                        uint2 position [[thread_position_in_grid]]) {
    // Launch extent comes from allocated buffers, separately from the scene
    // capability. Reject changed dimensions before reading trig/surface arrays.
    const uint width=launch[1],height=launch[2];
    const uint x=position.x,y=position.y+launch[0];
    if(x>=width||y>=height)return;
    const uint index=y*width+x;
    if(scene.width!=width||scene.height!=height) {
        ExactColorPixel rejected{};rejected.fault=1;output[index]=rejected;return;
    }
    output[index]=mb3d_metal_exact::exact_color_pixel(scene,columns[x],rows[y],surface[index],y);
}
