#define MB3D_METAL_NATIVE_FLOAT 1
#include "src/render/native_metal/general_geometry.metal.h"
#include "src/render/native_metal/native_camera.metal.h"
#include "native_inside_continuation_probe_abi.h"
using namespace metal;using namespace mb3d_native;
// Sixteen first-sample lanes, not a complete frame or an owner-order proof.
kernel void prepare_inputs(constant p2_inside::Input& prepared [[buffer(0)]],
 constant mb3d_camera::Params& camera [[buffer(1)]],device const mb3d_camera::Axis* columns [[buffer(2)]],
 device const mb3d_camera::Axis* rows [[buffer(3)]],device p2_inside::Input* inputs [[buffer(4)]],uint tid [[thread_position_in_grid]]){
 p2_inside::Input out=prepared;
 const uint x=((tid%4)*2+1)*out.base.scene.width/8;
 const uint y=((tid/4)*2+1)*out.base.scene.height/8;
 out.base.x=x;out.base.y=y;
 out.base.camera=mb3d_camera::camera_ray(camera,columns[x],rows[y],x,y,general_double(out.base.scene.max_ray_length));
 inputs[tid]=out;
}
