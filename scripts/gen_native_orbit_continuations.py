#!/usr/bin/env python3
"""Source-bound P1 formula-step/orbit regions. No arithmetic rewriting."""
import hashlib
import re
from gen_native_metal_formulas import tokens,matching,emit_text
from gen_native_continuations import locate

ORBIT_SHA='cfe1cde4c4483f61e612f5be6492e02a13614550a2c94916fe48a2748d47aabe'

def function(source,name,occurrence=0):
    v=tokens(source);starts=[i for i,t in enumerate(v[:-1]) if t==name and v[i+1]=='(']
    a=v.index('{',starts[occurrence]);return v[a+1:matching(v,a)]

def return_calls(v,helper):
    result=[];i=0
    while i<len(v):
        if v[i]=='return':
            j=v.index(';',i);result+=['return',helper,'(']+v[i+1:j]+[')', ';'];i=j+1
        else:result.append(v[i]);i+=1
    return emit_text(result)

def generate_step(source):
    if hashlib.sha256(source.encode()).hexdigest()!=ORBIT_SHA:raise ValueError('Orbit source changed; repeat P1 caller audit')
    v=function(source,'native_step');a=locate(v,['bool','ok','=']);b=v.index(';',a)+1
    prefix=return_calls(v[:a],'step_finish(g,overflow,');tail=return_calls(v[b:],'step_finish(g,overflow,')
    # Helper prefix already contains argument separators; remove the inserted
    # opening parenthesis after that prefix, leaving the original expressions.
    prefix=prefix.replace('step_finish(g,overflow, (','step_finish(g,overflow, ')
    tail=tail.replace('step_finish(g,overflow, (','step_finish(g,overflow, ')
    return f'''namespace mb3d_native {{ namespace continuation {{
// Caller storage context is part of the pinned P1 proof; expressions remain original.
struct AtomicStepResult {{NativeFormulaState lane;uint success,overflow;}};
inline AtomicStepResult atomic_step_lane(NativeFormulaState s,constant NativeFormulaProgram& p,thread const NativeResources& resources,thread NativeFormulaFault& f,uint slot){{
 bool overflow=false;bool success=native_step(s,p,resources,f,slot,overflow);return {{s,uint(success),uint(overflow)}};
}}
// Copy-only W/X/Y/Z binding matches internal formula 2 scalar declarations.
struct AtomicCoordinates {{float w,x,y,z;}};
inline AtomicStepResult atomic_step_ordered(AtomicCoordinates coordinates,NativeFormulaState s,constant NativeFormulaProgram& p,thread const NativeResources& resources,thread NativeFormulaFault& f,uint slot){{
 s.xyz[3]=coordinates.w;s.xyz[0]=coordinates.x;s.xyz[1]=coordinates.y;s.xyz[2]=coordinates.z;
 bool overflow=false;bool success=native_step(s,p,resources,f,slot,overflow);return {{s,uint(success),uint(overflow)}};
}}
inline uint step_finish(thread FormulaStepFrame& g,bool overflow,bool success){{g.overflow=uint(overflow);g.child_success=uint(success);g.header.pc=99;return 99;}}
template<uint Part=0,bool ReplayContext=false,bool DistanceContext=false,uint EntryContext=0> inline uint formula_step_advance(thread FormulaStepFrame& g,thread WrapperFrame& w,thread BodyFrame& b,
 thread NativeFormulaState& s,constant NativeFormulaProgram& p,thread const NativeResources& resources,thread NativeFormulaFault& f){{
 const bool Replay=ReplayContext||(DistanceContext&&(native_part_driver<Part>()==Hybrid3DAnalytic||native_part_driver<Part>()==Interpolated3DAnalytic));
 uint slot=g.slot;bool overflow=bool(g.overflow);
 if(g.header.pc==0){{
 if(slot<6){{constant auto& spec=p.slots[slot];
  if(EntryContext==2&&DistanceContext&&native_part_driver<Part>()==Hybrid3DAnalytic&&native_formula_id0==(kInternalFormulaSelector|2u)&&spec.namespace_id==InternalFormula&&spec.id==2u){{
   auto result=atomic_step_ordered({{s.xyz[3],s.xyz[0],s.xyz[1],s.xyz[2]}},s,p,resources,f,slot);s=result.lane;return step_finish(g,bool(result.overflow),bool(result.success));
  }}
  if(((!Replay&&(native_part_driver<Part>()==Interpolated3D||native_part_driver<Part>()==Interpolated3DAnalytic))&&(g.header.parent_pc==2||g.header.parent_pc==4)||(Replay&&native_part_driver<Part>()==Hybrid3DAnalytic&&(!DistanceContext||native_formula_id0!=10089))||(DistanceContext&&native_part_driver<Part>()>=HybridDifs&&native_formula_id0==10089))&&(spec.namespace_id==InternalFormula||(spec.id>=10353&&spec.id<=10357)||!body_supported(spec.id))){{
   auto result=atomic_step_lane(s,p,resources,f,slot);s=result.lane;return step_finish(g,bool(result.overflow),bool(result.success));
  }}
 }}
 {prefix}
 if(spec.namespace_id!=InternalFormula && !(spec.id>=10353&&spec.id<=10357)&&body_supported(spec.id)){{
  w={{}};g.header.pc=1;return 1;
 }}
 {emit_text(v[a:b])}
 g.child_success=uint(ok);g.header.pc=2;return 2;
 }}
 if(slot>=6)return step_finish(g,overflow,native_fail(f,InvalidFormulaAbi,s));
 constant auto& spec=p.slots[slot];
 if(g.header.pc==1){{bool ok=false;
  if(wrapper_advance(w,b,s,spec,resources,f,p.local_work_limit,ok)==99){{g.child_success=uint(ok);g.header.pc=2;}}
  return g.header.pc;
 }}
 if(g.header.pc==2){{bool ok=bool(g.child_success);{tail}}}
 if(g.header.pc!=99)return step_finish(g,overflow,native_fail(f,InvalidFormulaAbi,s));
 return 99;
}}
}} }}
'''

def generate_orbit(source):
    if hashlib.sha256(source.encode()).hexdigest()!=ORBIT_SHA:raise ValueError('Orbit source changed; repeat P1 caller audit')
    v=function(source,'native_orbit_passes');do=locate(v,['do','{']);end=matching(v,do+1)
    init=v[:do];loop=v[do+2:end];tailstart=v.index(';',end)+1;tail=v[tailstart:]
    metaend=locate(init,['if','(','driver','!=','HybridDifsNoInitialization',')'])
    meta=init[:metaend];init=init[metaend:]
    reset=locate(meta,['if','(','radius_range',')']);resetend=meta.index(';',reset)+1
    resetcode=meta[reset:resetend];meta=meta[:reset]+meta[resetend:]
    # Frame-bound integer/bool locals retain their exact original types.
    for typ,name in [('uint','slot'),('int','remaining'),('int','minimum_iteration'),('bool','improved'),('bool','last_improved'),('uint','pass')]:
        i=locate(init,[typ,name,'=']);del init[i]
    numeric_end=locate(init,['slot','='])
    initial_numeric=init[:numeric_end];initial_metadata=init[numeric_end:]
    # Independent rotation rows retain their exact arithmetic, bound Y before X
    # only for the first direct internal-2 Hybrid4D normal sample.
    first_four=list(initial_numeric)
    rotation=tokens('if(four)for(uint i=0;i<4;++i)')
    begin_rotation=locate(first_four,rotation);row=begin_rotation+len(rotation)
    end_rotation=first_four.index(';',row)+1
    first_four[begin_rotation:end_rotation]=tokens('if(four)for(uint order=0;order<4;++order){const uint indices[4]={1,0,2,3};const uint i=indices[order];')+first_four[row:end_rotation]+['}']
    branch=locate(loop,['if','(','interpolated',')','{']);close=matching(loop,branch+4)
    begin=loop[:branch];interp=loop[branch+5:close]
    hb=close+2;assert loop[hb]=='{';hybrid=loop[hb+1:matching(loop,hb)]
    calls=[]
    for region in [interp,hybrid]:
        indices=[i for i in range(len(region)-4) if region[i:i+4]==['if','(','!','native_step']]
        calls.append([(i,region.index(';',i)+1) for i in indices])
    (a,b),(c,d)=calls[0];(e,f),=calls[1]
    before0=interp[:a];middle=interp[b:c];after1=interp[d:]
    beforeh=hybrid[:e];afterh=hybrid[f:]
    # saved and first belong to different child-return intervals.
    before0=emit_text(before0).replace('float saved [ 4 ] , first [ 4 ];','float saved [ 4 ];')
    middle=emit_text(middle)
    fields='''thread uint& slot=o.slot;thread int& remaining=o.remaining;
thread int& minimum_iteration=o.minimum_iteration;
thread bool& improved=o.improved;thread bool& last_improved=o.last_improved;
thread uint& pass=o.pass;thread uint& index=o.index;thread bool& overflow=o.overflow;
thread bool& disabled_exit=o.disabled_exit;thread uint& exit_pass=o.exit_pass;thread bool& inert=o.inert;'''
    def region(v):
        text=return_calls(v,'orbit_finish(o,').replace('orbit_finish(o, (','orbit_finish(o, ')
        return re.sub(r'\bcontinue\s*;', 'return orbit_phase(o,20);',re.sub(r'\bbreak\s*;', 'return orbit_phase(o,90);',text))
    begin=region(begin).replace('const uint index =','index =').replace('bool overflow =','overflow =')
    beforeh=region(beforeh).replace('bool disabled_exit =','disabled_exit =')
    def coords(v,base):
        for i,name in enumerate(['x','y','z','w']):
            needle=tokens(base)+['[',str(i),']'];j=0
            while j<=len(v)-len(needle):
                if v[j:j+len(needle)]==needle:v[j:j+len(needle)]=[name]
                j+=1
        return v
    radius_body=emit_text(coords(function(source,'native_radius'),'s.xyz'))
    first_start=locate(after1,['float','first_r','=']);target_start=locate(after1,['float','target','='])
    first_body=emit_text(coords(after1[first_start:target_start].copy(),'first'))
    original_first=after1[first_start:target_start].copy();del original_first[0]
    replacement=tokens('float first_r; if((four||BlendReplay)&&!FourNumericalDistance&&!(CombinedContext&&native_selected_formula_type()!=0&&Part==0&&driver==Interpolated4DAnalytic&&!(EntryContext&&(native_formula_id0==10089u||EntryContext==2)))) first_r=ordered_first_radius({first[1],first[0],first[2],first[3]},four); else {')+original_first+['}']
    after1[first_start:target_start]=replacement
    target_start=locate(after1,['float','target','=']);target_end=after1.index(';',target_start)+1
    target=emit_text(after1[target_start:target_end]).replace('native_radius ( s , four )','(((four||BlendReplay)&&!FourNumericalDistance&&!(CombinedContext&&native_selected_formula_type()!=0&&Part==0&&driver==Interpolated4DAnalytic&&!(EntryContext&&(native_formula_id0==10089u||EntryContext==2))))?ordered_radius({s.xyz[1],s.xyz[0],s.xyz[2],s.xyz[3]},four):native_radius(s,four))')
    after1[target_start:target_end]=tokens(target)
    factor_start=locate(after1,['float','factor','='])
    factor_end=after1.index(';',factor_start)+1
    factor=emit_text(after1[factor_start:factor_end]).replace('native_radius ( s , four )','(((EntryContext==2&&!CombinedContext&&ReplayContext&&driver==Interpolated3D)||(DistanceContext&&four&&!FourNumericalDistance&&!(CombinedContext&&native_selected_formula_type()!=0&&Part==0&&!(EntryContext&&driver==Interpolated4DAnalytic&&(native_formula_id0==10089u||EntryContext==2)))))?ordered_radius({s.xyz[1],s.xyz[0],s.xyz[2],s.xyz[3]},four):native_radius(s,four))')
    after1[factor_start:factor_end]=tokens(factor)
    code=f'''namespace mb3d_native {{ namespace continuation {{
struct OrderedCoordinates {{float y,x,z,w;}};
inline float ordered_radius(OrderedCoordinates v,bool four){{float y=v.y,x=v.x,z=v.z,w=v.w;{radius_body}}}
inline float ordered_first_radius(OrderedCoordinates v,bool four){{float y=v.y,x=v.x,z=v.z,w=v.w;{first_body}return first_r;}}
template<uint Part=0,bool CombinedContext=false> inline NativeFormulaState orbit_initial_lane(NativeFormulaState s,constant NativeFormulaProgram& p){{
{emit_text(meta)}
{emit_text(initial_numeric).replace("native_radius ( s , four )", "((CombinedContext&&driver==Interpolated3DAnalytic)?ordered_radius({s.xyz[1],s.xyz[0],s.xyz[2],s.xyz[3]},four):native_radius(s,four))")}
return s;
}}
template<uint Part=0> inline NativeFormulaState orbit_initial_first_normal(OrderedCoordinates coordinates,NativeFormulaState s,constant NativeFormulaProgram& p){{
 s.c[1]=coordinates.y;s.c[0]=coordinates.x;s.c[2]=coordinates.z;s.c[3]=coordinates.w;
{emit_text(meta)}
{emit_text(initial_numeric).replace("native_radius ( s , four )", "ordered_radius({s.xyz[1],s.xyz[0],s.xyz[2],s.xyz[3]},four)")}
return s;
}}
template<uint Part=0> inline NativeFormulaState orbit_initial_first_four(NativeFormulaState s,constant NativeFormulaProgram& p){{
{emit_text(meta)}
{emit_text(first_four).replace("native_radius ( s , four )", "ordered_radius({s.xyz[1],s.xyz[0],s.xyz[2],s.xyz[3]},four)")}
return s;
}}
template<uint Part=0> inline NativeFormulaState orbit_initial_ordered(OrderedCoordinates coordinates,NativeFormulaState s,constant NativeFormulaProgram& p){{
 s.c[1]=coordinates.y;s.c[0]=coordinates.x;s.c[2]=coordinates.z;s.c[3]=coordinates.w;
 return orbit_initial_lane<Part,true>(s,p);
}}
inline uint orbit_phase(thread OrbitPassFrame& o,uint pc){{o.header.pc=pc;return pc;}}
inline uint orbit_finish(thread OrbitPassFrame& o,float result){{o.result=result;return orbit_phase(o,99);}}
inline void orbit_child(thread FormulaStepFrame& g,uint slot,uint parent_pc){{g={{}};g.header.parent_tag=uint(Tag::OrbitPass);g.header.parent_pc=parent_pc;g.header.version=LayoutVersion;g.header.tag=uint(Tag::FormulaStep);g.slot=slot;}}
// Static caller storage: 0 isolated, 1 geometry DE entry, 2 normal sample,
// 3 first ordered direct-normal sample. Expressions remain source-derived.
// These bindings preserve source expressions; each caller has its own exact gate.
template<uint Part=0,bool ReplayContext=false,bool DistanceContext=false,bool CombinedContext=false,uint EntryContext=0> inline uint orbit_pass_advance(thread OrbitPassFrame& o,
 thread FormulaStepFrame& g,thread WrapperFrame& w,thread BodyFrame& b,
 constant NativeFormulaProgram& p,thread NativeFormulaState& lane,
 thread const NativeResources& resources,thread NativeFormulaFault& f,
 thread mb3d_native_gradient::RadiusRange* radius_range){{
 thread auto& s=lane;
 {emit_text(meta)}
 const bool Replay=ReplayContext||(DistanceContext&&(driver==Hybrid3DAnalytic||driver==Interpolated3DAnalytic));
 const bool BlendReplay=ReplayContext;
 const bool FourNumericalDistance=DistanceContext&&driver==Interpolated4D&&!(EntryContext==2&&!CombinedContext&&native_selected_formula_type()==0&&native_formula_id0==10089u);
 {fields}
 const uint pass_limit=o.pass_limit;
 if(o.header.pc==0){{
  {emit_text(resetcode)}
  if(EntryContext==3&&driver==Hybrid3D)s=orbit_initial_first_normal<Part>({{s.c[1],s.c[0],s.c[2],s.c[3]}},s,p);
  else if(EntryContext==4&&driver==Hybrid4D&&native_formula_id0==(kInternalFormulaSelector|2u))s=orbit_initial_first_four<Part>(s,p);
  else if(Replay&&interpolated&&!four){{{emit_text(initial_numeric).replace("native_radius ( s , four )", "(((CombinedContext&&driver==Interpolated3DAnalytic)||(EntryContext==2&&!CombinedContext&&native_selected_formula_type()==0&&driver==Interpolated3D&&native_formula_id0==10089u))?ordered_radius({s.xyz[1],s.xyz[0],s.xyz[2],s.xyz[3]},four):native_radius(s,four))")}}}
  else if(CombinedContext&&driver==Interpolated3DAnalytic)s=orbit_initial_ordered<Part>({{s.c[1],s.c[0],s.c[2],s.c[3]}},s,p);
  else s=orbit_initial_lane<Part,CombinedContext>(s,p);
  {emit_text(initial_metadata)}
  o.driver=driver;o.difs=difs;o.interpolated=interpolated;o.four=four;o.analytic=analytic;
  o.header.version=LayoutVersion;o.header.tag=uint(Tag::OrbitPass);
  return orbit_phase(o,1);
 }}
 if(o.header.pc==1){{
  {begin}
  if(interpolated){{
   {before0}
   for(uint k=0;k<4;++k)o.saved[k]=saved[k];o.old_derivative=old_derivative;
   orbit_child(g,0,2);return orbit_phase(o,2);
  }}else{{
   {beforeh}
   orbit_child(g,slot,7);return orbit_phase(o,7);
  }}
 }}
 if(o.header.pc==2||o.header.pc==4||o.header.pc==7){{
  uint child;
  if(EntryContext==2&&CombinedContext&&native_selected_formula_type()==0&&driver==Hybrid3DAnalytic&&native_formula_id0==10089u){{
   NativeFormulaState local=s;child=formula_step_advance<Part,ReplayContext,DistanceContext,EntryContext>(g,w,b,local,p,resources,f);s=local;
  }}else child=formula_step_advance<Part,ReplayContext,DistanceContext,EntryContext>(g,w,b,s,p,resources,f);
  if(child!=99)return o.header.pc;
  overflow=bool(g.overflow);
  return orbit_phase(o,o.header.pc==2?3:o.header.pc==4?5:8);
 }}
 if(o.header.pc==3){{
  if(!g.child_success)return orbit_finish(o,0.f);
  float saved[4],first[4];for(uint k=0;k<4;++k)saved[k]=o.saved[k];float old_derivative=o.old_derivative;
  {region(tokens(middle))}
  for(uint k=0;k<4;++k)o.first[k]=first[k];o.first_derivative=first_derivative;
  orbit_child(g,1,4);return orbit_phase(o,4);
 }}
 if(o.header.pc==5){{
  if(!g.child_success)return orbit_finish(o,0.f);
  float first[4];
  if((four||BlendReplay)&&!FourNumericalDistance&&!(CombinedContext&&native_selected_formula_type()!=0&&Part==0&&driver==Interpolated4DAnalytic&&!(EntryContext&&(native_formula_id0==10089u||EntryContext==2)))){{first[1]=o.first[1];first[0]=o.first[0];first[2]=o.first[2];first[3]=o.first[3];}}
  else for(uint k=0;k<4;++k)first[k]=o.first[k];
  float first_derivative=o.first_derivative;
  if((!four&&!BlendReplay)||FourNumericalDistance){{
   NativeFormulaState s=lane;
   {region(after1).replace('return orbit_finish', 'lane=s;return orbit_finish')}
   lane=s;
  }}else{{
   {region(after1)}
  }}
  return orbit_phase(o,20);
 }}
 if(o.header.pc==8){{
  if(!g.child_success)return orbit_finish(o,0.f);
  {region(afterh)}
  return orbit_phase(o,20);
 }}
 if(o.header.pc==20){{return orbit_phase(o,(s.iteration<s.max_iterations&&(difs||!(s.rout>s.rstop)))?1:90);}}
 if(o.header.pc==90){{{region(tail)}}}
 if(o.header.pc!=99)native_fail(f,InvalidFormulaAbi,s);
 return orbit_phase(o,99);
}}
}} }}
'''
    return code.replace('s . rout = native_radius ( s , four );', 's.rout=(Replay||(!(CombinedContext&&native_selected_formula_type()!=0)&&driver==Hybrid4DAnalytic&&(EntryContext!=2||native_formula_id0==10089u||native_formula_id0==7060u)))?ordered_radius({s.xyz[1],s.xyz[0],s.xyz[2],s.xyz[3]},four):native_radius(s,four);')



def generate_observer(source):
    if hashlib.sha256(source.encode()).hexdigest()!=ORBIT_SHA:raise ValueError('Orbit source changed; repeat P1 caller audit')
    v=function(source,'native_orbit_passes')
    sites=[i for i in range(len(v)-4) if v[i:i+4]==['if','(','!','native_step']]
    assert len(sites)==3
    for number,start in reversed(list(enumerate(sites))):
        end=v.index(';',start)+1;closing=matching(v,start+4,'(',')')
        call=emit_text(v[start+3:closing+1])
        phase=[2,4,7][number]
        extra='float4(0),0.f'
        if number==0:extra='float4(saved[0],saved[1],saved[2],saved[3]),old_derivative'
        if number==1:extra='float4(first[0],first[1],first[2],first[3]),first_derivative'
        args='s,f,slot,remaining,minimum_iteration,improved,last_improved,pass,index,overflow,inert,radius_range,'+extra
        replacement=f' observer.record({phase},false,{args}); bool ok{number}={call}; observer.record({phase+100},ok{number},{args}); if(!ok{number})return 0.f; '
        v[start:end]=tokens(replacement)
    code=emit_text(v).replace('s . rout = native_radius ( s , four );','s.rout=(driver==Hybrid4DAnalytic)?ordered_radius({s.xyz[1],s.xyz[0],s.xyz[2],s.xyz[3]},four):native_radius(s,four);')
    return f'''namespace mb3d_native {{ namespace continuation {{
template<uint Part=0,class Observer> inline float orbit_original_observed(constant NativeFormulaProgram& p,thread NativeFormulaState& s,
 thread const NativeResources& resources,thread NativeFormulaFault& f,thread mb3d_native_gradient::RadiusRange* radius_range,
 uint pass_limit,thread uint& exit_pass,thread bool& inert,thread Observer& observer){{
{code}
}}
}} }}
'''
