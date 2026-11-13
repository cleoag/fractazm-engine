// Host-only regression for source-derived caller ownership; no device dispatch.
#include "render/native_metal/pixel_state_contract.h"
#include "render/native_metal/pixel_independence.h"
#include <initializer_list>
#include <iostream>
#include <stdexcept>
using namespace mb3d_native;
namespace {
unsigned checks=0;
void check(bool ok,const char* text){++checks;if(!ok)throw std::runtime_error(text);}
NativeFormulaSlot slot(U32 ns,U32 id,I32 de,U32 params,U32 traits=0,U32 flags=1){
    NativeFormulaSlot s{};s.version=1;s.enabled=1;s.namespace_id=ns;s.id=id;s.de_option=de;
    s.param_count=params;s.traits=traits;s.wrapper_flags=flags;return s;
}
NativeFormulaProgram program(U32 driver,I32 option,std::initializer_list<NativeFormulaSlot> slots){
    NativeFormulaProgram p{};p.version=1;p.slot_count=6;p.driver=driver;p.de_option=option;
    for(auto& s:p.slots)s.version=1;
    unsigned n=0;for(const auto& s:slots){p.slots[n]=s;p.hybrid_counts[n++]=1;}
    p.end_slot=n-1;return p;
}
void live_inputs(){
    using namespace pixel_state;
    auto translate=program(Hybrid3D,0,{slot(ExternalFormula,8065,-1,3)});
    const auto t=AnalyzePrimaryPixelState(translate);
    check(t.known&&t.independent&&t.generated,"stateless Translate lost independence");
    check((t.live_inputs&Vary)&&!(t.initialized&Vary)&&!(t.driver_may_write&Vary)&&!(t.formula_may_write&Vary),"Vary falsely reset/written");
    check((t.driver_may_write&SSE)==SSE&&!(t.initialized&SSE),"return SSE became entry initialization");
    auto quat=program(Hybrid3D,0,{slot(InternalFormula,2,4,0,0,0)});
    for(U32 julia:{0u,1u}){
        quat.julia=julia;const auto q=AnalyzePrimaryPixelState(quat);
        check(q.known&&q.independent&&(q.live_inputs&J4)&&!(q.initialized&J4)&&!(q.driver_may_write&J4),"ordinary J4 preservation/live input");
    }
    quat.slots[1]=slot(InternalFormula,7,0,0,0,0);quat.hybrid_counts[1]=1;quat.end_slot=1;
    const auto written=AnalyzePrimaryPixelState(quat);
    check(written.known&&!written.independent&&(written.formula_may_write&J4)&&!(written.live_inputs&J4),"mutable J4 falsely immutable");
    for(U32 id:{9452u,7046u,3676u,10010u,7076u,10171u}){
        NativeFormulaSlot s{};
        for(const auto& e:generated_effects::kEntries)if(e.namespace_id==ExternalFormula&&e.id==id)
            s=slot(ExternalFormula,id,e.de_option,e.param_count,e.traits,e.wrapper_flags);
        const auto hand=formula(s),generated=generated_formula(s);
        check(hand.known&&generated.known&&hand.may_write==generated.may_write&&!(hand.may_write&(Vary|First)),"pass-through formula claimed First/Vary mutation");
    }
}
void mutable_vary(){
    using namespace pixel_state;
    for(const auto& body:{slot(ExternalFormula,1509,11,15),slot(ExternalFormula,8024,2,8)}){
        auto p=program(Hybrid3DAnalytic,body.de_option,{body});
        const auto c=AnalyzePrimaryPixelState(p);
        check(c.known&&c.independent==(body.id==1509)&&(c.formula_may_write&Vary)&&
              !(c.initialized&Vary)&&!(c.live_inputs&Vary),"conditional body init erased carry");
        check(c.entry_equivalent==(body.id==1509?Vary:0)&&!(c.contained&Vary),"entry relation confused with containment");
        check(!IndependentPrimaryPixels(p),"closed Vary whitelist still admitted");
    }
    auto rotated=program(Hybrid3DAnalytic,11,{slot(ExternalFormula,7060,-1,9),slot(ExternalFormula,1509,11,15)});
    rotated.repeat_slot=1;
    check(!IndependentPrimaryPixels(rotated)&&!AnalyzePrimaryPixelState(rotated).independent,"Rotate before Surf falsely independent");
    auto four=program(Hybrid4DAnalytic,5,{slot(ExternalFormula,7060,-1,9),slot(ExternalFormula,10324,5,9,2)});
    check(IndependentPrimaryPixels(four),"stateless closed4D chain lost proof");
    auto boxes=program(Hybrid3DAnalytic,2,{slot(ExternalFormula,8900,2,10,0,0),slot(ExternalFormula,10014,-1,13,576)});
    check(IndependentPrimaryPixels(boxes),"stateless existing box subset lost proof");
}
void combined(){
    using namespace pixel_state;
    auto a=program(Hybrid3DAnalytic,2,{slot(ExternalFormula,10041,2,25),slot(ExternalFormula,4952,20,10)});
    a.formula_type=1;a.end_slot=0;a.slots[1].difs_style=1;
    auto b=a;b.driver=HybridDifs;b.de_option=20;b.start_slot=b.repeat_slot=b.end_slot=1;
    const auto later=AnalyzePrimaryPixelState(a,&b);
    check(later.known&&later.conditional&&!later.independent&&(later.driver_may_write&Vary)&&!(later.live_inputs&Vary),"later dIFS write masked as live input");
    auto first=b;first.start_slot=first.repeat_slot=first.end_slot=1;
    auto second=a;second.start_slot=second.repeat_slot=second.end_slot=0;
    const auto local=AnalyzePrimaryPixelState(first,&second);
    check(local.known&&local.conditional&&local.independent&&(local.initialized&Vary),"first dIFS query-local initialization lost");
    first.formula_type=0;
    const auto standalone=AnalyzePrimaryPixelState(first);
    check(standalone.known&&standalone.independent&&(standalone.initialized&Vary)&&(standalone.driver_may_write&Vary)&&!(standalone.live_inputs&Vary),"dIFS own reset lost");
}
void drivers(){
    using namespace pixel_state;
    for(U32 driver=Hybrid3D;driver<=Interpolated4DAnalytic;++driver){
        auto p=program(driver,0,{slot(ExternalFormula,8065,-1,3)});
        const auto writes=part_driver_may_write(p);
        check((writes&SSE)==SSE&&!(writes&Vary),"ordinary SSE/Vary ownership");
        const bool four=driver==Hybrid4D||driver==Hybrid4DAnalytic||driver==Interpolated4D||driver==Interpolated4DAnalytic;
        check(bool(writes&J4)==four,"J4 driver write ownership");
        check(bool(writes&bit(35))==(driver<Interpolated3D),"interpolation tmp preservation");
    }
}
void strict_first_vary(){
    // Source-equivalent first callback, not a fabricated orbit-entry reset.
    auto p=program(Hybrid3DAnalytic,2,{slot(ExternalFormula,1509,11,15),
        slot(ExternalFormula,8065,-1,3),slot(ExternalFormula,7039,2,3)});
    p.hybrid_counts[0]=30;p.hybrid_counts[1]=-2147483647;p.hybrid_counts[2]=5;
    const auto entry=AnalyzePrimaryPixelState(p);
    check(entry.independent,"Surf-first source equivalence missing");
    check(entry.version==2&&entry.entry_equivalent==pixel_state::Vary&&entry.production_chain==0,
          "entry equivalence lacks explicit contract identity");
    check(!((entry.initialized|entry.live_inputs|entry.contained)&pixel_state::Vary),
          "entry relation mislabeled as reset, immutable or contained");
    for(auto driver:{Hybrid3D,Hybrid3DAnalytic}){
        auto q=p;q.driver=driver;q.de_option=driver==Hybrid3D?0:11;
        check(AnalyzePrimaryPixelState(q).independent,"source-proven ordinary entry lost");
    }
    unsigned rejection=0;
    auto reject=[&](NativeFormulaProgram q){
        check(!AnalyzePrimaryPixelState(q).independent,
            (std::string("unproved first-write schedule admitted: ")+std::to_string(++rejection)).c_str());};
    auto q=p;q.hybrid_counts[0]=0;reject(q);
    q=p;q.hybrid_counts[0]=-1;reject(q);
    q=p;q.slots[0].enabled=0;
    check(AnalyzePrimaryPixelState(q).entry_equivalent==0&&AnalyzePrimaryPixelState(q).independent,
          "disabled initializer borrowed a first-write proof instead of stateless live input");
    q=p;q.start_slot=1;reject(q);
    q=p;q.slots[0]=slot(ExternalFormula,8065,-1,3);q.slots[1]=slot(ExternalFormula,1509,11,15);reject(q);
    q=p;q.slots[0]=slot(ExternalFormula,8024,2,8);reject(q);
    q=p;q.slots[1]=slot(ExternalFormula,10353,-1,6);reject(q); // may mutate Counts
    q=p;q.formula_type=1;reject(q);
    q=p;q.driver=Hybrid4D;q.de_option=4;reject(q);
    q=p;q.start_slot=6;reject(q);
    q=p;q.end_slot=6;reject(q);
    q=p;q.repeat_slot=6;reject(q);
    for(unsigned field=0;field<9;++field){
        q=p;auto& s=q.slots[0];
        switch(field){case 0:++s.version;break;case 1:s.namespace_id=InternalFormula;break;
        case 2:s.de_option=2;break;case 3:--s.param_count;break;case 4:s.traits=1;break;
        case 5:s.wrapper_flags=0;break;case 6:s.difs_style=1;break;
        case 7:s.internal_power=2;break;case 8:s.reserved[0]=1;break;}
        reject(q);
    }
}
}
int RunNativePixelStateContractChecks(){try{live_inputs();mutable_vary();combined();drivers();strict_first_vary();std::cout<<checks<<" ownership checks PASS\n";return 0;}
catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
#ifndef MB3D_PIXEL_STATE_CONTRACT_EMBEDDED
int main(){return RunNativePixelStateContractChecks();}
#endif
