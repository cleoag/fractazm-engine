#include "formula_host.h"
#include "formula_resources.h"
#include "pixel_independence.h"
#include "native_inside_math.h"
#include "generated/formula_metadata.h"
#include "core/external_formulas.h"
#include "core/formula_registry.h"
#include "core/formulas.h"
#include "core/header_trafos.h"
#include "core/lightmap.h"
#include "core/numeric_precision.h"
#include "core/ray_marcher.h"
#include "core/render_context.h"
#include <algorithm>
#include <bit>
#include <cctype>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <limits>
#include <stdexcept>

namespace mb3d::native_metal {
using namespace mb3d_native;
namespace {
float finiteFloat(double value, const char* label) {
    float result = static_cast<float>(value);
    if (!std::isfinite(value) || !std::isfinite(result))
        throw std::invalid_argument(std::string("Native formula FP32 range: ") + label);
    return result;
}
// A positive radius stop beyond binary32 is unreachable by a finite FP32
// radius: the orbit escapes only by an FP32 range exit, as with FLT_MAX.
float radiusStop(double value, const char* label) {
    constexpr double largest = std::numeric_limits<float>::max();
    return value > largest ? std::numeric_limits<float>::max() : finiteFloat(value, label);
}
double readDouble(const void* p, int offset) {
    double v; std::memcpy(&v, static_cast<const char*>(p)+offset, sizeof(v)); return v;
}
void appendMap(PreparedFormulaProgram& out,int id) {
    for(const auto& existing:out.maps)if(existing.map_id==id)return;
    if(id<0)throw std::invalid_argument("Native formula map ID is negative");
    NativeMapDescriptor descriptor{};descriptor.map_id=id;descriptor.channels=3;
    if(!id) {descriptor.flags=FormulaMapEmpty;out.maps.push_back(descriptor);return;}
    unsigned nonempty=0;for(const auto& existing:out.maps)nonempty+=existing.map_id!=0;
    if(nonempty>=3)throw std::invalid_argument("Native formula map count exceeds the CPU three-map cache");
    TLightMap map{};
    struct MapRelease {TLightMap& map;~MapRelease(){FreeLightMap(&map);}} release{map};
    if(!LoadLightMapNr(id,&map))throw std::invalid_argument("Native formula map is unavailable: "+std::to_string(id));
    // An absent sequence frame is an explicit empty map in ordinary loading.
    if(!map.LMa || !map.LMWidth || !map.LMHeight) {
        descriptor.flags=FormulaMapEmpty;out.maps.push_back(descriptor);return;
    }
    if(map.LMWidth<0 || map.LMHeight<0 || map.LMWidth>16384 || map.LMHeight>16384 ||
        (map.iMapType!=0 && map.iMapType!=1) || map.sLMXfactor!=float(map.LMWidth) || map.sLMYfactor!=float(map.LMHeight))
        throw std::invalid_argument("Native formula map layout is unsupported");
    const std::size_t pixels=(std::size_t(map.LMWidth)+1)*(std::size_t(map.LMHeight)+1);
    if(pixels*3>(1u<<26) || out.map_pixels.size()+pixels*3>(1u<<26))
        throw std::invalid_argument("Native formula map resource work bound exceeded");
    descriptor.width=U32(map.LMWidth);descriptor.height=U32(map.LMHeight);
    descriptor.offset=U32(out.map_pixels.size());descriptor.element_count=U32(pixels*3);
    descriptor.flags=map.iMapType==1?FormulaMapWords:0;
    const auto* bytes=reinterpret_cast<const unsigned char*>(map.LMa);
    out.map_pixels.reserve(out.map_pixels.size()+pixels*3);
    for(std::size_t pixel=0;pixel<pixels;++pixel)for(unsigned channel=0;channel<3;++channel) {
        if(map.iMapType==1) {
            std::uint16_t value;std::memcpy(&value,bytes+pixel*6+(2-channel)*2,2);
            out.map_pixels.push_back(float(value));
        } else out.map_pixels.push_back(float((map.LMa[pixel]>>(8*(2-channel)))&255u));
    }
    out.maps.push_back(descriptor);
}
unsigned selectDriver(bool custom, TMandFunctionDE functionDE, TMandFunction function) {
    if (custom) {
        if (functionDE == doHybridPasDE) return Hybrid3DAnalytic;
        if (functionDE == doHybrid4DDEPas) return Hybrid4DAnalytic;
        if (functionDE == doInterpolHybridPasDE) return Interpolated3DAnalytic;
        if (functionDE == doInterpolHybridPas4DDE) return Interpolated4DAnalytic;
        if (functionDE == doHybridIFS3D) return HybridDifs;
        if (functionDE == doHybridIFS3DnoVecIni) return HybridDifsNoInitialization;
    } else {
        if (function == doHybridPas) return Hybrid3D;
        if (function == doHybrid4DPas) return Hybrid4D;
        if (function == doInterpolHybridPas) return Interpolated3D;
        if (function == doInterpolHybridPas4D) return Interpolated4D;
    }
    throw std::invalid_argument("Native Metal formula driver is not implemented");
}
unsigned selectDriver(const TMCTparameter& m) {
    return selectDriver(m.IsCustomDE != 0, m.mMandFunctionDE, m.mMandFunction);
}
} // namespace

PreparedFormulaProgram PrepareFormulaProgramWithMaps(const TMCTparameter& m,
    const THeaderCustomAddon& addon, const PreparedFormulaProgram* snapshot, unsigned work_limit) {
    if (!NativePrecisionEnabled())
        throw std::invalid_argument("Native formula preparation requires Native arithmetic");
    const bool combined = m.FormulaType != 0;
    if (!m.bMCTisValid || m.FormulaType < 0 || m.FormulaType > 6 ||
        (combined ? m.CalcDE != CalcDEfull : (m.CalcDE != CalcDEnoADE && m.CalcDE != CalcDEanalytic)) ||
        !mb3d_inside::flag_word(m.bCalcInside) || m.bCalcInside!=m.bInsideRendering)
        throw std::invalid_argument("Native formula DE combination/inside policy is invalid");
    if (combined && (m.iMaxitF2 < 1 || m.iMaxitF2 > (1<<20) ||
        m.StartFrom2 > m.iEnd2 || m.RepeatFrom2 > m.iEnd2 || m.iEnd2 >= 6 || m.iEnd2 < 0))
        throw std::invalid_argument("Invalid native DE combination work/slot bounds");
    if (!work_limit || work_limit > (1u<<24) || m.iMaxIt < 1 || m.iMaxIt > (1<<20) ||
        m.StartFrom1 > m.wEndTo || m.RepeatFrom1 > m.wEndTo || m.wEndTo >= 6)
        throw std::invalid_argument("Invalid native formula work/slot bounds");
    PreparedFormulaProgram out;
    if(snapshot) { out.maps=snapshot->maps; out.map_pixels=snapshot->map_pixels; }
    auto appendResource=[&](int id) {
        if(snapshot) {
            if(std::none_of(out.maps.begin(),out.maps.end(),[&](const auto& v){return v.map_id==id;}))
                throw std::invalid_argument("Prepared formula resource identity mismatch");
        } else appendMap(out,id);
    };
    auto& p=out.device;
    p.version=kFormulaAbiVersion; p.driver=selectDriver(m); p.slot_count=6;
    p.start_slot=m.StartFrom1; p.repeat_slot=m.RepeatFrom1; p.end_slot=m.wEndTo;
    p.de_option=m.DEoption; p.max_iterations=m.iMaxIt; p.julia=m.bDoJulia!=0;
    p.inside=static_cast<U32>(m.bCalcInside); p.calc_smooth=m.bCalcSIT!=0;
    p.work_limit=work_limit; p.local_work_limit=1u<<20;
    p.radius_stop=radiusStop(m.dRStop,"radius stop");
    p.log_radius_stop=finiteFloat(m.dLNRStop,"log radius stop");
    p.de_scale=finiteFloat(m.dDEscale,"DE scale");
    p.de_offset=finiteFloat(m.mctDEoffset,"DE offset");
    p.de_offset_floor=finiteFloat(m.mctDEoffset006,"DE denominator floor");
    p.gradient_radius_stop=radiusStop(m.Rstop3D,"gradient radius stop");
    p.stop_scale=finiteFloat(m.sStepWm103,"dIFS stop scale");
    if (combined) {
        // Both programs carry the combination words (CalcDEfull's mct fields).
        p.formula_type=static_cast<U32>(m.FormulaType); p.mix_color=static_cast<U32>(m.DEmixCol);
        if (m.DEmixCol < 0 || m.DEmixCol > 2)
            throw std::invalid_argument("Native DE combination color option is invalid");
        p.mix_power=finiteFloat(m.FmixPow,"DE combination mix power");
        p.combination_smooth=finiteFloat(m.sDEcombSmooth,"DE combination smoothness");
        if (!(p.combination_smooth > 0.f))
            throw std::invalid_argument("Native DE combination smoothness is invalid");
    }
    const double julia[]={m.dJUx,m.dJUy,m.dJUz,m.dJUw};
    for(unsigned i=0;i<4;++i) {
        p.julia_value[i]=finiteFloat(julia[i],"Julia");
        for(unsigned j=0;j<4;++j)p.rotation[i][j]=finiteFloat(m.Smatrix4d[i][j],"4D rotation");
    }
    const bool interpolated=p.driver>=Interpolated3D && p.driver<=Interpolated4DAnalytic;
    for(unsigned i=0;i<6;++i) {
        p.hybrid_counts[i]=m.nHybrid[i];
        p.smoothing_power[i]=finiteFloat(m.fHln[i],"smoothing power");
        auto& s=p.slots[i]; s.version=kFormulaAbiVersion;
        // CPU installs EmptyFormula for unprepared disabled slots. A later
        // conditional count change must keep that explicit no-op behavior.
        if (!m.fHPVar[i] || (!interpolated && !m.nHybrid[i])) continue;
        const auto& f=addon.Formulas[i]; const void* vars=m.fHPVar[i];
        s.enabled=1;
        std::memcpy(s.raw_words,static_cast<const char*>(vars)-256,sizeof(s.raw_words));
        // Unused packed integer/pointer lanes may not be finite doubles. Keep
        // their raw words; only actually decoded numeric parameters are checked.
        for(int j=0;j<32;++j) {
            s.negative_params[j]=static_cast<float>(readDouble(vars,-8*(j+1)));
            s.constants[j]=static_cast<float>(readDouble(vars,8*j));
        }
        if (IsInternalFormula(f.iFnr)) {
            s.namespace_id=InternalFormula; s.id=static_cast<U32>(f.iFnr);
            s.de_option=GetDEoptionForFormula(f.iFnr);
            // Resolve the already prepared callback, avoiding a second option
            // conversion with different tie/overflow behavior.
            if (f.iFnr==0 || f.iFnr==6) {
                ThybridIteration fn=reinterpret_cast<ThybridIteration>(m.fHybrid[i]);
                if(f.iFnr==6)std::memcpy(&fn,static_cast<const char*>(vars)-56,sizeof(fn));
                for(unsigned power=2;power<=8;++power)
                    if(fn==fHIntFunctions[power-2])s.internal_power=power;
                if(!s.internal_power)throw std::invalid_argument("Unsupported internal power callback");
                if(f.iFnr==6) {
                    // Resolve the CPU callback to an integer selector, then
                    // remove its host address from every uploaded view.
                    std::memset(reinterpret_cast<char*>(s.raw_words)+256-56,0,sizeof(fn));
                    s.negative_params[6]=0.f;
                }
            }
            if(f.iFnr==4 && reinterpret_cast<ThybridIteration>(m.fHybrid[i])==fHybridCubeDE)
                s.wrapper_flags|=2u; // internal box derivative tracking
            out.names.push_back(InternFormulaNames[s.id]);
            if(f.iFnr==8) {
                const auto map=std::bit_cast<I32>(s.raw_words[(256-52)/4]);
                if(map && (s.raw_words[(256-56)/4]&3u)==3u)
                    throw std::invalid_argument("Native testIFS map channel exceeds its three-channel sample");
                appendResource(map);
            }
        } else {
            const char* begin=reinterpret_cast<const char*>(f.CustomFname);
            const std::string name(begin,std::find(begin,begin+sizeof(f.CustomFname),'\0'));
            const auto* info=FindExternalFormula(name.c_str());
            if(!info)throw std::invalid_argument("Unknown native formula: "+name);
            s.namespace_id=ExternalFormula; s.id=info->id; s.de_option=info->DEoption;
            double values[64]{};
            int count=0;
            if(info->paramMap) {
                if(info->paramMap->count<0 || info->paramMap->count>58)
                    throw std::invalid_argument("Native formula parameter map exceeds ABI");
                count=ReadFormulaParams(vars,*info->paramMap,values);
            }
            else if(info->id>=95001 && info->id<=95003) {
                count=2;values[0]=readDouble(vars,-16);values[1]=readDouble(vars,-24);
            } else throw std::invalid_argument("Unsupported native formula parameter ABI: "+name);
            if(count<0 || count>58)throw std::invalid_argument("Native formula parameter count exceeds ABI");
            s.param_count=static_cast<U32>(count);
            for(int j=0;j<count;++j) {
                // FP32 range policy for parameters: a finite binary64 value
                // beyond binary32 (the loader's reciprocal options expand a
                // zero input to 1/1e-40, BOXSCALE to Scale/1e-80) uploads as
                // its binary32 rounding, +-infinity. A pass that carries it
                // into the orbit state leaves binary32 and takes the counted
                // range exit; unread lanes change nothing. Integer and raw
                // lanes are not values and keep the strict check.
                const bool real=!info->paramMap || info->paramMap->entries[j].valueType==0;
                if(real && std::isfinite(values[j]) && !std::isfinite(float(values[j])))
                    s.params[j]=static_cast<float>(values[j]);
                else
                    s.params[j]=finiteFloat(values[j],("formula parameter "+std::to_string(j)+" of "+name).c_str());
                const auto bits=std::bit_cast<std::uint64_t>(values[j]);
                s.param_bits[j][0]=static_cast<U32>(bits);
                s.param_bits[j][1]=static_cast<U32>(bits>>32);
            }
            if(!configure_native_external_slot(s))
                throw std::invalid_argument("Native Metal lowering is unavailable: "+name);
            if(const auto* resource=NativeFormulaMapRequirement(s,info->name))
                appendResource(NativeFormulaMapId(s,*resource));
            out.names.push_back(name);
        }
    }
    const bool four=p.driver==Hybrid4D||p.driver==Hybrid4DAnalytic||
        p.driver==Interpolated4D||p.driver==Interpolated4DAnalytic;
    out.requires_persistent_lane=four||p.driver>=HybridDifs||
        (p.driver==Hybrid3DAnalytic&&(p.de_option&0x38)==32);
    for(const auto& slot:p.slots)
        out.requires_persistent_lane|=slot.enabled&&slot.namespace_id==ExternalFormula;
    if(IndependentPrimaryPixels(p))out.requires_persistent_lane=false;
    if (combined) {
        // PrepareF1DEcomb: the second hybrid part shares slots, counts, stops
        // and offsets; it has its own range, driver, DE option/scale and maximum.
        auto& q=out.second; q=p;
        q.driver=selectDriver(m.IsCustomDE2 != 0, m.mMandFunctionDE2, m.mMandFunction2);
        q.start_slot=m.StartFrom2; q.repeat_slot=m.RepeatFrom2; q.end_slot=static_cast<U32>(m.iEnd2);
        q.de_option=m.DEoption2; q.max_iterations=m.iMaxitF2;
        q.de_scale=finiteFloat(m.dDEscale2,"second DE scale");
        if ((q.driver>=Interpolated3D&&q.driver<=Interpolated4DAnalytic) ||
            (p.driver>=Interpolated3D&&p.driver<=Interpolated4DAnalytic))
            throw std::invalid_argument("Native interpolated DE combination is not implemented");
        // The lane is shared and never audited across two parts.
        out.requires_persistent_lane=true;
    }
    return out;
}

PreparedFormulaProgram PrepareFormulaProgram(const TMCTparameter& m,
    const THeaderCustomAddon& addon, unsigned work_limit) {
    return PrepareFormulaProgramWithMaps(m,addon,nullptr,work_limit);
}

PreparedFormulaProgram PrepareLoadedFormulaProgram(const TMandHeader10& header,
    const THeaderCustomAddon& addon,const std::string& root,unsigned work_limit) {
    auto context=std::make_shared<RenderContext>(root);
    context->precision=PrecisionMode::Native;
    ScopedRenderContext binding(context);
    TMandHeader10 copy=header; THeaderCustomAddon formulas=addon;
    if(copy.MCoptions&1u)throw std::invalid_argument("Deferred input mode excluded");
    for(const auto& formula:formulas.Formulas) {
        if(IsInternalFormula(formula.iFnr))continue;
        const char* first=reinterpret_cast<const char*>(formula.CustomFname);
        std::string name(first,std::find(first,first+sizeof(formula.CustomFname),'\0'));
        std::transform(name.begin(),name.end(),name.begin(),[](unsigned char c){return char(std::tolower(c));});
        if(name.find("jit")!=std::string::npos)throw std::invalid_argument("Deferred formula input excluded");
    }
    const auto mct=GetMCTparasFromHeader(copy,&formulas,0);
    return PrepareFormulaProgram(mct,formulas,work_limit);
}

void LoadRawFormulaMapResources(PreparedFormulaProgram& out,
    const std::vector<int>& ids,const std::string& root) {
    if(!out.maps.empty() || !out.map_pixels.empty() || ids.empty() || ids.size()>4)
        throw std::invalid_argument("Raw formula map resources require empty output and one to four unique IDs");
    unsigned nonzero=0;
    for(std::size_t i=0;i<ids.size();++i) {
        if(ids[i]<0 || std::find(ids.begin(),ids.begin()+i,ids[i])!=ids.begin()+i)
            throw std::invalid_argument("Raw formula map IDs must be unique and nonnegative");
        nonzero+=ids[i]!=0;
    }
    if(nonzero>3)throw std::invalid_argument("Raw formula map IDs exceed the CPU three-map cache");
    if(root.empty() || !std::filesystem::is_directory(root))
        throw std::invalid_argument("Raw formula map resource root must be an existing directory");
    auto context=std::make_shared<RenderContext>(std::filesystem::canonical(root).string());
    context->precision=PrecisionMode::Native;
    ScopedRenderContext binding(context);
    PreparedFormulaProgram resources;
    for(int id:ids)appendMap(resources,id);
    out.maps.swap(resources.maps);out.map_pixels.swap(resources.map_pixels);
}

NativeFormulaState MakeFormulaPointState(const NativeFormulaProgram& p,
    float x,float y,float z,float stop) {
    NativeFormulaState s{};
    s.c[0]=x;s.c[1]=y;s.c[2]=z;
    s.max_iterations=p.max_iterations;s.rstop=p.radius_stop;
    s.rstop_d=stop*p.stop_scale;s.rold=s.rstop_d;
    s.julia=p.julia;s.inside=p.inside;s.calc_smooth=p.calc_smooth;
    for(unsigned i=0;i<4;++i)s.j[i]=s.original_j[i]=p.julia_value[i];
    s.derivative[0]=1.f;
    for(unsigned i=0;i<6;++i)s.hybrid_counts[i]=p.hybrid_counts[i];
    return s;
}
} // namespace mb3d::native_metal
