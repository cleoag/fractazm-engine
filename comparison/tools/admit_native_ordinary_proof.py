#!/usr/bin/env python3
"""Revalidate a captured ordinary run against the current frozen identity.

This binds only the ordinary obligation. Missing observer tools, formulas,
scenes and other proofs stay OPEN. Run while the target queue is idle.
"""
import argparse,copy,sys
from pathlib import Path


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('repository','package','contract','build','capture'):
        p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root=a.repository.resolve();sys.path.insert(0,str(root/'comparison/tools'))
    from prepare_gpu_parallel_bundle import ref,refresh,write
    from qualify_gpu_parallel_pipeline import scope,evaluate
    from qualify_native_metal_all import digest,RECEIPT_SCHEMA
    from gpu_pipeline_binding import core_context
    from gpu_pipeline_proofs import cpu_ordinary
    from run_m3parameter_gpu import read,require
    package=a.package.resolve();contract=a.contract.resolve();build=a.build.resolve()
    capture=read(a.capture);require(capture['schema']=='mb3d.native-ordinary-capture.v1' and capture['status']=='PASS_SCOPED','Unverified capture')
    ev=refresh(package,contract,build)
    require(not ev['proofs'] and not ev['formulas'],'Explicit rebinding required for existing global/formula proofs')
    b=copy.deepcopy(ev['binding'])
    require(capture['renderer']==b['renderer'] and capture['library']==b['library'],'Ordinary capture uses a different final identity')
    b.setdefault('tools',{})['cpu_ordinary']=capture['tool'];b['ordinary_commands']=capture['ordinary_commands']
    c,formulas,scenes=scope(contract);ctx=core_context(root,b,c,formulas,scenes,root)
    cpu_ordinary(root,capture['proof'],ctx)
    binding=digest(b);data=dict(capture['proof'],schema='mb3d.gpu-pipeline-proof.v1',kind='cpu_ordinary',binding_sha256=binding)
    folder=package/'ordinary-proof'/binding;require(not folder.exists(),'Proof folder already exists')
    obs=folder/'observations.json';write(obs,data)
    receipt={'schema':RECEIPT_SCHEMA,'kind':'cpu_ordinary','status':'PASS','binding_sha256':binding,
       'producer':'collect_native_ordinary_proof.py actual CTest processes',
       'independent_reviewer':'gpu_pipeline_proofs.cpu_ordinary raw inventory/JUnit/input replay',
       'checks':[{'id':'all_current_suites_bound_and_passed','status':'PASS'}],
       'artifacts':[ref(obs),ref(a.capture.resolve())],'observations':ref(obs)}
    path=folder/'receipt.json';write(path,receipt)
    ev['binding']=b;write(package/'evidence.json',ev)
    # Refresh scene receipts for the new binding before adding the scoped proof.
    ev=refresh(package,contract,build);require(digest(ev['binding'])==binding,'Unexpected binding change')
    ev['proofs'].append({'kind':'cpu_ordinary','artifact':ref(path)});write(package/'evidence.json',ev)
    result=evaluate(contract,package);write(package/'qualification.json',result)
    require(['proofs','cpu_ordinary'] in result['accepted'],'Common gate rejected ordinary proof')
    print('ORDINARY PROOF ACCEPTED; full contract',result['status'],'remaining='+str(result['remaining']))

if __name__=='__main__':main()
