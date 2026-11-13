#pragma once
// Experimental caller composition. Include the pinned original wrapper/math
// first. No production navigation entry admits this path yet.
#include "continuation_abi.h"
#include "continuation_10274.metal.h"
#include "continuation_10265.metal.h"
#include "continuation_10220.metal.h"
#include "continuation_10321.metal.h"
#include "continuation_10089.metal.h"
#include "wrapper_regions.metal.h"
namespace mb3d_native { namespace continuation {
inline bool body_supported(uint id){return id==10274||id==10265||id==10220||id==10321||id==10089;}
inline uint body_advance(thread BodyFrame& b,thread WrapperFrame& w,
 constant NativeFormulaSlot& slot,thread const NativeResources& resources,
 thread NativeFormulaFault& fault,uint local_limit) {
 bool written=bool(w.first_counter_written);
 FormulaParams view{w.a,&slot,&resources,&fault,&w.first_counter,&written,local_limit};
 FormulaVec3 point(w.p[0],w.p[1],w.p[2]),result;uint pc=0;
 switch(slot.id){
 case 10274:continuation_10274::step(b.locals.tess,point,view,w.de);pc=b.locals.tess.pc;if(pc==99)result=b.locals.tess.result;break;
 case 10265:continuation_10265::step(b.locals.pine,point,view,w.de);pc=b.locals.pine.pc;if(pc==99)result=b.locals.pine.result;break;
 case 10220:continuation_10220::step(b.locals.columns,point,view,w.de);pc=b.locals.columns.pc;if(pc==99)result=b.locals.columns.result;break;
 case 10321:continuation_10321::step(b.locals.quat,point,view,w.de);pc=b.locals.quat.pc;if(pc==99)result=b.locals.quat.result;break;
 case 10089:continuation_10089::step(b.locals.gnarly,point,view,w.de);pc=b.locals.gnarly.pc;if(pc==99)result=b.locals.gnarly.result;break;
 default:native_formula_fail(fault,UnsupportedFormula,slot.id);pc=99;break;
 }
 b.header.pc=pc;w.first_counter_written=uint(written);
 if(pc==99){w.result[0]=result.x;w.result[1]=result.y;w.result[2]=result.z;}
 return pc;
}
// PC 0 initializes once, 1 advances the child, 2 commits original wrapper
// writeback once; 99 is terminal. Semantic work guards remain in their owners.
inline uint wrapper_advance(thread WrapperFrame& w,thread BodyFrame& b,
 thread NativeFormulaState& lane,constant NativeFormulaSlot& slot,
 thread const NativeResources& resources,thread NativeFormulaFault& fault,uint local_limit,
 thread bool& success) {
 if(w.header.pc==0){
  success=false;b={};w.header.version=LayoutVersion;w.header.tag=uint(Tag::Wrapper);
  if(!body_supported(slot.id)){native_formula_fail(fault,UnsupportedFormula,slot.id);w.header.pc=99;return 99;}
  if(!wrapper_begin(w,lane,slot,resources,fault,local_limit)){w.header.pc=99;return 99;}
  b.identity=slot.id;b.header.version=LayoutVersion;b.header.tag=uint(Tag::Body);
  w.header.pc=1;
 }else if(w.header.pc==1){
  if(b.identity!=slot.id||b.header.version!=LayoutVersion){success=false;native_formula_fail(fault,InvalidFormulaAbi,slot.id);w.header.pc=99;return 99;}
  if(body_advance(b,w,slot,resources,fault,local_limit)==99)w.header.pc=2;
 }else if(w.header.pc==2){success=wrapper_finish(w,lane,slot,fault);w.header.pc=99;}
 else if(w.header.pc!=99){success=false;native_formula_fail(fault,InvalidFormulaAbi,slot.id);w.header.pc=99;}
 return w.header.pc;
}
} }
