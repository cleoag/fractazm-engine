#include "native_camera_host.h"
#include "core/header_trafos.h"
#include "core/math3d.h"
#include "core/ray_marcher.h"
#include "core/numeric_precision.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
namespace mb3d::native_metal {
namespace {
float f32(double value,const char* name){
    const float result=static_cast<float>(value);
    if(!std::isfinite(value)||!std::isfinite(result))
        throw std::invalid_argument(std::string("Native camera FP32 range: ")+name);
    return result;
}
mb3d_camera::Axis axis(double angle){
    return {f32(angle,"angle"),f32(std::sin(angle),"sine"),f32(std::cos(angle),"cosine"),0};
}
}
PreparedNativeCamera PrepareNativeCamera(const TMandHeader10& header,const TMCTparameter& m,Lattice lattice){
    const ScopedNumericPrecision precision(PrecisionMode::Native);
    if(header.Width<=0||header.Height<=0||header.Width>4096||header.Height>4096||
       !m.calc3D||m.MCTCameraOptic<0||m.MCTCameraOptic>2||
       m.iMandWidth!=header.Width||m.iMandHeight!=header.Height)
        throw std::invalid_argument("Native camera extent/optic mismatch");
    if(header.TilingOptions&&(!(header.TilingOptions&127)||!((header.TilingOptions>>7)&127)))
        throw std::invalid_argument("Native camera zero tile count");
    int width,height,left,top;GetPaintTileSizes(&header,width,height,left,top);
    if(width<=0||height<=0||width>4096||height>4096||
       m.CalcRect.Left!=left||m.CalcRect.Top!=top||m.CalcRect.Right!=left+width-1||
       m.CalcRect.Bottom!=top+height-1)
        throw std::invalid_argument("Native camera calculation rectangle mismatch");
    const int stride=lattice.stride;
    if(!lattice.dense()){
        if(stride<2||stride>4||lattice.x<0||lattice.y<0||lattice.x>=stride||lattice.y>=stride||
           header.TilingOptions||lattice.x>=width||lattice.y>=height)
            throw std::invalid_argument("Native camera lattice is not 2..4 pixels inside an untiled frame");
        left=lattice.x;top=lattice.y;width=(width-lattice.x+stride-1)/stride;height=(height-lattice.y+stride-1)/stride;
    }
    // File cameras accept0..2. Direct runtime mask3 selects spherical primary
    // but a different paint branch, so it needs a separately specified control.
    if(header.bPlanarOptic>2||m.MCTCameraOptic!=header.bPlanarOptic||
       m.iCutOptions<0||(unsigned(m.iCutOptions)&~0x77u))
        throw std::invalid_argument("Native camera selector or cut bits unsupported");
    PreparedNativeCamera out;auto& p=out.device;
    p.version=mb3d_camera::Version;p.optic=m.MCTCameraOptic;
    p.full_width=header.Width;p.full_height=header.Height;p.width=width;p.height=height;p.left=left;p.top=top;
    p.stride_extra=unsigned(stride-1);
    p.primary_planar_z=f32(m.mctPlOpticZ,"primary planar focal length");
    const float paint_fov=header.bPlanarOptic==2?float(Pi):float(header.dFOVy*Pid180);
    const double half=std::clamp(double(paint_fov)*.5,.001,1.5);
    p.paint_planar_z=f32(std::cos(half)*half/std::sin(half),"paint planar focal length");
    p.cut_options=unsigned(m.iCutOptions);
    for(unsigned i=0;i<3;++i){
        p.origin[i]=f32(m.Ystart[i],"origin");
        for(unsigned j=0;j<3;++j)p.matrix[i][j]=f32(m.Vgrads[i][j],"matrix");
    }
    // Dormant cut coordinates are not consumed by CPU rendering. Do not reject
    // an ordinary scene because an inactive saved field exceeds FP32 range.
    const double cuts[]{m.dCOX,m.dCOY,m.dCOZ};
    for(unsigned i=0;i<3;++i)if(p.cut_options&(1u<<i))p.cut[i]=f32(cuts[i],"active cut coordinate");
    const float aspect=header.bPlanarOptic==2?2.f:float(header.Width)/float(header.Height);
    const float xoff=CalcXoff(&header);
    for(int x=0;x<width;++x){
        const int screen=x*stride+left;
        out.columns.push_back(axis((double(m.FOVXoff)-double(screen+1))*double(m.FOVXmul)));
        const float pos=float(screen+1)/float(header.Width);
        const float a=(xoff-pos)*paint_fov;
        const float paint_angle=a*aspect;
        out.paint_columns.push_back(axis(double(paint_angle)));
    }
    for(int y=0;y<height;++y){
        const int screen=y*stride+top;
        out.rows.push_back(axis(CalcCameraRowAngle(screen,m.iMandHeight,m.FOVy)));
        const float pos=float(screen)/float(header.Height);
        const float paint_angle=(pos-.5f)*paint_fov;
        out.paint_rows.push_back(axis(double(paint_angle)));
    }
    return out;
}
}
