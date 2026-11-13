#include "render/native_metal/formula_resources.h"
#include "core/external_formulas.h"
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace mb3d;using namespace mb3d::native_metal;
static unsigned checks=0;
static void check(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
template<class F>static void reject(F f){bool threw=false;try{f();}catch(const std::invalid_argument&){threw=true;}
    check(threw,"Invalid callback identity or ID admitted");}
int main(){try{
    InitExternalFormulaRegistry();
    for(const auto& binding:NativeFormulaMapBindings){
        const auto* info=FindExternalFormula(std::string(binding.name).c_str());
        check(info&&info->id==int(binding.id),"Registry identity drift");
        mb3d_native::NativeFormulaSlot slot{};slot.enabled=1;slot.namespace_id=mb3d_native::ExternalFormula;
        slot.id=info->id;slot.param_count=info->paramCount;
        check(NativeFormulaMapRequirement(slot,info->name)==&binding,"Callback binding lost");
        for(float value:{0.f,-0.f,1.f,55.f,1633.f,32000.f,65535.f}){
            slot.params[binding.parameter]=value;
            check(NativeFormulaMapId(slot,binding)==int(value),"Uploaded map ID differs from callback integer");
        }
        for(float value:{-1.f,0.5f,1.5f,2147483648.f,std::numeric_limits<float>::infinity(),
                         std::numeric_limits<float>::quiet_NaN()}){
            slot.params[binding.parameter]=value;reject([&]{(void)NativeFormulaMapId(slot,binding);});
        }
        slot.params[binding.parameter]=55;
        reject([&]{(void)NativeFormulaMapRequirement(slot,"incorrect formula");});
        ++slot.id;reject([&]{(void)NativeFormulaMapRequirement(slot,info->name);});--slot.id;
        slot.param_count=binding.parameter;reject([&]{(void)NativeFormulaMapRequirement(slot,info->name);});
        reject([&]{(void)NativeFormulaMapId(slot,binding);});slot.param_count=info->paramCount;
        slot.enabled=0;reject([&]{(void)NativeFormulaMapRequirement(slot,info->name);});slot.enabled=1;
        slot.namespace_id=mb3d_native::InternalFormula;
        reject([&]{(void)NativeFormulaMapRequirement(slot,info->name);});
    }
    for(const char* name:{"TgladDihed","TgladDihed2","TgladTetra","TgladTetra2"}){
        const auto* info=FindExternalFormula(name);check(info&&info->mapValueMask==2,"Legacy Pascal prefetch hint changed");
        mb3d_native::NativeFormulaSlot slot{};slot.enabled=1;slot.namespace_id=mb3d_native::ExternalFormula;
        slot.id=info->id;slot.param_count=info->paramCount;
        check(!NativeFormulaMapRequirement(slot,name),"Log2MapPower became a callback asset");
    }
    check(FindExternalFormula("HeightMapIFS")->mapValueMask==0x0401,"Legacy channel prefetch hint changed");
    check(FindExternalFormula("CylinderHeightMap")->mapValueMask==3,"Legacy channel prefetch hint changed");
    std::cout<<"PASS native formula resource checks="<<checks<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<" checks="<<checks<<'\n';return 1;}}
