#!/usr/bin/env python3
"""Copy-only initialized inside-owner snapshots around primary and smooth calls."""
from gen_native_inside_continuations import regions,generate_inside
from gen_native_orbit_continuations import function
from gen_native_continuations import locate
from gen_native_metal_formulas import tokens,emit_text

def capture(site,phase,candidate=False):
 attempt='attempt' if site==1 else '0u';point='float3(0.f)' if site==1 else ('float3(f.de.point[0],f.de.point[1],f.de.point[2])' if candidate else 'point');stop='0.f' if site==1 else 'as_type<float>(pixel.final_stop)'
 child='result' if site==2 and phase==2 else 'NativeOrbitResult{}'
 source=['camera','x','y','normal_params','normal_record','normal_result']
 if candidate:source=['f.camera','f.owner.x','f.owner.y','f.normal_params','f.normal_record','f.normal_result']
 camera,x,y,params,record,result=source
 return f'observer.record({site}u,{phase}u,{site}u,state,{attempt},policy,{camera},{x},{y},{params},{point},{stop},pixel,lane,geometry,{record},{result},activity,{child});'

def original_observed(source):
 regions(source);v=function(source,'general_primary_inside');patches=[]
 for site,pattern in [(1,'pixel=general_primary_camera_attempt('),(2,'const auto result=general_de(')]:
  i=locate(v,tokens(pattern));end=v.index(';',i)+1;patches.append((i,end,tokens(capture(site,1))+v[i:end]+tokens(capture(site,2))))
 for a,b,replacement in sorted(patches,reverse=True):v[a:b]=replacement
 return emit_text(v)

def candidate_observed(source):
 code=generate_inside(source)
 def replace(old,new):
  nonlocal code
  if code.count(old)!=1:raise ValueError('Missing/ambiguous inside trace binding: '+old)
  code=code.replace(old,new)
 replace('inline uint inside_advance(', 'template<class Observer> inline uint inside_advance(')
 replace('thread NativeResources& resources){', 'thread NativeResources& resources,thread Observer& observer){')
 replace('f.primary={};',capture(1,1,True)+'f.primary={};')
 anchor='if(primary_advance(f,scene,program,resources)!=99)return 2;';replace(anchor,anchor+capture(1,2,True))
 replace('f.de.smooth=1;', 'f.de.smooth=1;'+capture(2,1,True))
 replace('const auto result=f.de.child;', 'const auto result=f.de.child;'+capture(2,2,True))
 return code
