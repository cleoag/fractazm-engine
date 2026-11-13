// Same-process 0 -> 1 -> 0 control: immutable PSOs shared, mutable inputs owned.
#include "comparison/metal_exact/metal_device.h"
#include "src/render/native_metal/formula_abi.h"
#include "src/render/native_metal/formula_selector.h"
#include "src/render/native_metal/wrapper_policy.h"
#include <fstream>
#include <iostream>
#include <vector>
using namespace mb3d::metal_exact::gpu;
using namespace mb3d_native;
std::vector<char> bytes(const char* path){std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
int main(int argc,char**argv){@autoreleasepool{try{
    require(argc==4,"usage: wrapper-cache LIBRARY PROGRAM STATES");
    auto raw=bytes(argv[2]),inputs=bytes(argv[3]);require(raw.size()==sizeof(NativeFormulaProgram)&&inputs.size()>0&&inputs.size()%sizeof(NativeFormulaState)==0,"input sizes");
    NativeFormulaProgram p;std::memcpy(&p,raw.data(),sizeof(p));std::array<uint32_t,6> ids;
    require(ProductionWrapperPolicy(p)==1,"qualified chain policy");
    auto mixed=p;mixed.slots[3]=p.slots[0];mixed.slots[3].id=9401;
    require(ProductionWrapperPolicy(mixed)==0,"enabled mixed Makin must be generic");
    mixed.slots[3].enabled=0;require(ProductionWrapperPolicy(mixed)==1,"disabled Makin is inert");
    auto only=p;only.slots[0].id=9401;
    for(unsigned i=1;i<6;++i)only.slots[i].enabled=0;
    require(ProductionWrapperPolicy(only)==0,"Makin-only must be generic");
    for(unsigned slot=0;slot<3;++slot){auto changed=p;changed.slots[slot].enabled=0;require(ProductionWrapperPolicy(changed)==0,"incomplete chain must be generic");}
    auto swapped=p;std::swap(swapped.slots[0],swapped.slots[1]);require(ProductionWrapperPolicy(swapped)==0,"reordered chain must be generic");
    unsigned mutated=0;
    for(unsigned field=0;field<7;++field){auto changed=p;uint32_t value;
        auto address=reinterpret_cast<char*>(&changed)+field*sizeof(uint32_t);
        std::memcpy(&value,address,sizeof(value));value^=1;std::memcpy(address,&value,sizeof(value));
        require(ProductionWrapperPolicy(changed)==0,"changed program selector admitted");++mutated;}
    for(unsigned slot=0;slot<3;++slot)for(unsigned field=0;field<8;++field){auto changed=p;uint32_t value;
        auto address=reinterpret_cast<char*>(&changed.slots[slot])+field*sizeof(uint32_t);
        std::memcpy(&value,address,sizeof(value));value^=1;std::memcpy(address,&value,sizeof(value));
        require(ProductionWrapperPolicy(changed)==0,"changed formula descriptor admitted");++mutated;}
    std::cout<<"PASS production selector: "<<mutated<<" descriptor/driver mutations; Makin-only/mixed/disabled/partial/reordered controls\n";
    for(unsigned i=0;i<6;++i)ids[i]=FormulaPipelineSelector(p.slots[i].namespace_id,p.slots[i].id,p.slots[i].enabled);
    const uint32_t n=uint32_t(inputs.size()/sizeof(NativeFormulaState));require(n<=4096,"input bound");
    std::vector<char> reference;id<MTLComputePipelineState> first=nil;
    for(unsigned policy:{0u,1u,0u}){
        Metal m(argv[1],4,0,0,false,true,&ids,p.driver,{},1,true,policy);
        auto program=m.buffer(raw.size(),raw.data());auto audit=m.buffer(18*sizeof(uint32_t));
        dispatchStage(m,m.pipeline(@"native_formula_wrapper_audit"),6,[&](id<MTLComputeCommandEncoder> e){[e setBuffer:program offset:0 atIndex:0];[e setBuffer:audit offset:0 atIndex:1];});
        auto words=static_cast<const uint32_t*>(audit.contents);
        for(unsigned i=0;i<6;++i)require(words[i*3]==1&&words[i*3+1]==policy,"wrong cached specialization policy");
        require(words[2]==(policy?10350u:0u),"missing fixed eligibility");
        auto pipeline=m.pipeline(@"native_formula_probe");
        if(!first)first=pipeline;
        else if(policy==0)require(first==pipeline,"return to policy0 did not reuse pipeline");
        else require(first!=pipeline,"policy1 reused policy0 probe pipeline");
        auto input=m.buffer(inputs.size(),inputs.data()),output=m.buffer(n*sizeof(NativeOrbitResult));
        auto maps=m.buffer(sizeof(NativeMapDescriptor)),pixels=m.buffer(sizeof(float));
        const uint32_t launch[]{n,0,0,n},counts[]{0,0};
        dispatchStage(m,pipeline,n,[&](id<MTLComputeCommandEncoder> e){
            [e setBuffer:program offset:0 atIndex:0];[e setBuffer:input offset:0 atIndex:1];[e setBuffer:output offset:0 atIndex:2];
            [e setBuffer:maps offset:0 atIndex:3];[e setBuffer:pixels offset:0 atIndex:4];
            [e setBytes:launch length:sizeof(launch) atIndex:5];[e setBytes:counts length:sizeof(counts) atIndex:6];});
        require(!std::memcmp(input.contents,inputs.data(),inputs.size()),"input mutated");
        const auto results=static_cast<const NativeOrbitResult*>(output.contents);
        for(uint32_t i=0;i<n;++i)require(results[i].completed && !results[i].fault.code,"formula probe did not complete successfully");
        const auto data=static_cast<const char*>(output.contents);
        if(reference.empty())reference.assign(data,data+output.length);
        else require(reference.size()==output.length&&!std::memcmp(reference.data(),data,output.length),"59-word cached policy results differ");
    }
    std::cout<<"PASS same-process policies0/1/0; GPU eligibility; PSO reuse; "<<n<<" complete59-word states\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}}
