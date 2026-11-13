#!/usr/bin/env python3
"""Extract P1 distance regions around orbit calls and gradient retry boundaries."""
import hashlib,re
from gen_native_metal_formulas import tokens,matching,emit_text
from gen_native_continuations import locate
from gen_native_orbit_continuations import ORBIT_SHA,function

def split_call(v):
 start=locate(v,['native_run_orbit','<','Part','>','(']);end=v.index(';',start)+1
 return v[:start],v[start:end],v[end:]

def sections(source):
 if hashlib.sha256(source.encode()).hexdigest()!=ORBIT_SHA:raise ValueError('Distance source changed; repeat P1 audit')
 v=function(source,'native_distance_controlled')
 first=locate(v,['float','value','=']);end=v.index(';',first)+1
 ni=locate(v,['if','(','numerical',')','{']);ne=matching(v,ni+4)
 n=v[ni+5:ne];ib=n.index('if');condend=matching(n,ib+1,'(',')');eb=n.index('{',condend);ee=matching(n,eb)
 gradient=n[eb+1:ee];retry=locate(gradient,['for','(','bool','retried']);rb=gradient.index('{',retry);re_=matching(gradient,rb)
 loop=gradient[rb+1:re_];axis=locate(loop,['for','(','uint','axis']);ab=loop.index('{',axis);ae=matching(loop,ab)
 axisbody=loop[ab+1:ae];ac=locate(axisbody,['native_run_orbit']);ace=axisbody.index(';',ac)+1
 post=loop[ae+1:];zero=locate(post,['if','(','steps','<=','1','&&']);zb=post.index('{',zero);ze=matching(post,zb)
 zbody=post[zb+1:ze];zc=locate(zbody,['native_run_orbit']);zce=zbody.index(';',zc)+1
 rest=post[ze+1:];rc=locate(rest,['native_run_orbit']);rce=rest.index(';',rc)+1
 finalstart=locate(v,['if','(','driver','<','HybridDifs','&&'])
 return dict(prefix=v[:first],afterfirst=v[end:ni],radius=n[:ib],inside=n[ib+2:condend],gradient=gradient[:retry],
             retry=loop[:axis],axis=axisbody[:ac],afteraxis=axisbody[ace:],postaxis=post[:zero],zero_cond=post[zero+2:zb-1],
             zero=zbody[:zc],afterzero=zbody[zce:],previous=rest[:rc],afterprevious=rest[rce:],
             restore=gradient[re_+1:],final=v[finalstart:])

def phase_text(v,continuation=None,break_pc=None):
 s=emit_text(v).replace('return out;', 'return distance_phase(d,99);')
 if continuation:s=s.replace('continue;',continuation)
 if break_pc is not None:s=s.replace('break;',f'return distance_phase(d,{break_pc});')
 return s

def generate_distance(source):
 q=sections(source)
 prefix=q['prefix'];p=locate(prefix,['fault','.','limit']);prefix=prefix[p:]
 radius=emit_text(q['radius']).replace('float radius =','radius =').replace('int iterations =','iterations =').replace('uint smooth =','smooth =')
 setup=emit_text(q['gradient']).replace('float center =','center =').replace('int steps =','steps =').replace('uint limit =','limit =')
 after=phase_text(q['afterfirst']).replace('bool numerical =','numerical =').replace('bool bounded_minimum =','bounded_minimum =')
 retry=emit_text(q['retry']).replace('float3 delta;', '').replace('bool representable =','representable =')
 axis=phase_text(q['axis']).replace('float original =','original =').replace('mb3d_native_gradient :: RadiusRange range;', '')
 axis_tail=phase_text(q['afteraxis'],'++axis;return distance_phase(d,4);')
 # Delta is a float3 in P1; bind its three scalar storage elements only at cuts.
 axis_tail=axis_tail.replace('return distance_phase(d,4);','for(uint k=0;k<3;++k)d.delta[k]=delta[k];return distance_phase(d,4);')
 return f'''namespace mb3d_native {{ namespace continuation {{
inline uint distance_phase(thread DistanceFrame& d,uint pc){{d.header.pc=pc;return pc;}}
inline uint distance_orbit(thread DistanceFrame& d,thread OrbitReplayFrame& r,uint pc){{
 r={{}};r.header.parent_tag=uint(Tag::Distance);r.header.parent_pc=pc;return distance_phase(d,pc);
}}
template<uint Part=0,bool Combined=false,uint EntryContext=0> inline uint distance_advance(thread DistanceFrame& d,thread OrbitReplayFrame& r,thread OrbitPassFrame& o,
 thread FormulaStepFrame& g,thread WrapperFrame& w,thread BodyFrame& b,
 constant NativeFormulaProgram& p,thread NativeResources& resources,thread NativeFormulaState& lane,thread uint* range_clamps){{
 const uint driver=native_part_driver<Part>();thread auto& out=d.result;thread auto& s=out.state;thread auto& fault=out.fault;
 const float3 point(d.point[0],d.point[1],d.point[2]);const float stop=d.stop;
 const uint calc_smooth=d.calc_smooth,inside_mode=d.inside_mode;const int max_iterations=d.max_iterations;
 thread float& value=d.value;thread float& radius=d.radius;thread float& center=d.center;thread float& original=d.original;
 thread int& iterations=d.iterations;thread int& steps=d.steps;thread uint& smooth=d.saved_smooth;thread uint& limit=d.limit;thread uint& axis=d.axis;
 thread bool& numerical=d.numerical;thread bool& bounded_minimum=d.bounded_minimum;thread bool& retried=d.retried;thread bool& representable=d.representable;
 float3 delta(d.delta[0],d.delta[1],d.delta[2]);
 if(d.header.pc==0){{
  d.header.version=LayoutVersion;d.header.tag=uint(Tag::Distance);d.part=Part;out={{}};out.state=lane;
  {phase_text(prefix)}
  return distance_orbit(d,r,1);
 }}
 if(d.header.pc==1||d.header.pc==5||d.header.pc==8||d.header.pc==10){{
  mb3d_native_gradient::RadiusRange range;range.certified=d.radius_certified;
  auto rp=d.header.pc==5?&range:nullptr;uint first_limit=d.header.pc==5?limit:d.header.pc==8?0u:0xffffffffu;
  uint child;
  if((EntryContext!=2||driver!=Hybrid3DAnalytic||native_formula_id0==10089u||native_formula_id0==(kInternalFormulaSelector|2u))&&!(EntryContext==2&&driver>=HybridDifs&&native_formula_id0==10089u)&&(!Combined||(native_selected_formula_type()==0&&driver!=Interpolated3DAnalytic)||(driver>=Interpolated4D&&driver<HybridDifs)))child=orbit_replay_advance<Part,false,true,Combined,EntryContext>(r,o,g,w,b,p,s,resources,fault,rp,first_limit);
  else child=orbit_replay_advance<Part,false,false,Combined,EntryContext>(r,o,g,w,b,p,s,resources,fault,rp,first_limit);
  if(child!=99){{d.radius_certified=range.certified;return d.header.pc;}}
  d.radius_certified=range.certified;
  if(d.header.pc==1)value=r.result;
  return distance_phase(d,d.header.pc+1);
 }}
 if(d.header.pc==2){{
  {after}
  if(numerical){{
   {radius}
   if({emit_text(q['inside'])}){{value=0.f;return distance_phase(d,90);}}
   {setup}retried=false;return distance_phase(d,3);
  }}
  value*=p.de_scale;return distance_phase(d,90);
 }}
 if(d.header.pc==3){{{retry}axis=0;return distance_phase(d,4);}}
 if(d.header.pc==4){{
  if(!(axis<3&&representable))return distance_phase(d,7);
  {axis}
  d.sample_coordinate=sample.coordinate;d.sample_scale=sample.scale;d.radius_certified=false;
  return distance_orbit(d,r,5);
 }}
 if(d.header.pc==6){{
  const mb3d_native_gradient::Sample sample{{d.sample_coordinate,d.sample_scale,true}};
  mb3d_native_gradient::RadiusRange range;range.certified=d.radius_certified;
  {axis_tail}
  for(uint k=0;k<3;++k)d.delta[k]=delta[k];++axis;return distance_phase(d,4);
 }}
 if(d.header.pc==7){{
  {phase_text(q['postaxis'],break_pc=80)}
  if({emit_text(q['zero_cond'])}){{{phase_text(q['zero'])}return distance_orbit(d,r,8);}}
  {phase_text(q['previous'])}return distance_orbit(d,r,10);
 }}
 if(d.header.pc==9){{{phase_text(q['afterzero'],'retried=true;return distance_phase(d,3);')}}}
 if(d.header.pc==11){{{phase_text(q['afterprevious'])}retried=true;return distance_phase(d,3);}}
 if(d.header.pc==80){{{phase_text(q['restore'])}return distance_phase(d,90);}}
 if(d.header.pc==90){{{phase_text(q['final'])}}}
 if(d.header.pc!=99)native_fail(fault,InvalidFormulaAbi,s);
 return distance_phase(d,99);
}}
}} }}
'''

def generate_distance_observer(source):
 sections(source) # Pinned source and structural audit.
 v=function(source,'native_distance_controlled');sites=[i for i,t in enumerate(v) if t=='native_run_orbit']
 assert len(sites)==4
 for site,start in reversed(list(enumerate(sites,1))):
  begin=v.index('(',start);end=matching(v,begin,'(',')');args=emit_text(v[begin+1:end]).strip()
  if site in [1,4]:args+=',0xffffffffu'
  if site==1:held='distance_observation(0.f,0.f,0.f,float3(0),0.f,0.f,0.f,stop,max_iterations,0,0,0,calc_smooth,inside_mode,0,0,false,false,false,false)'
  elif site==2:held='distance_observation(value,radius,center,float3(axis>0?delta[0]:0.f,axis>1?delta[1]:0.f,0.f),original,sample.coordinate,sample.scale,stop,max_iterations,iterations,steps,smooth,calc_smooth,inside_mode,axis,limit,retried,bounded_minimum,representable,true)'
  else:held='distance_observation(value,radius,center,float3(0),0.f,0.f,0.f,stop,max_iterations,iterations,steps,smooth,calc_smooth,inside_mode,0,limit,retried,bounded_minimum,representable,false)'
  v[start:end+1]=tokens(f'distance_observed_orbit<Part>({args},observer,{site},{held})')
 return f'''namespace mb3d_native {{ namespace continuation {{
template<uint Part=0,class Observer> inline NativeOrbitResult distance_original_observed(float3 point,float stop,constant NativeFormulaProgram& p,
 thread NativeResources& resources,thread NativeFormulaState& lane,uint calc_smooth,int max_iterations,
 thread uint* range_clamps,uint inside_mode,thread Observer& observer){{
 {emit_text(v)}
}}
}} }}
'''
