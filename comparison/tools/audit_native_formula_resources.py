#!/usr/bin/env python3
"""Bind typed Native map requirements to every emitted ordinary callback.

Source audit only: this neither executes formulas nor grants device coverage.
Unknown callback expressions fail closed and require a new manual read-set audit.
"""
import argparse,hashlib,importlib.util,json,re
from pathlib import Path

def require(ok,message):
    if not ok:raise ValueError(message)
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True);a=p.parse_args();root=a.source.resolve()
    require(not a.output.exists(),'Preserve existing audit')
    inputs={}
    def source(rel):
        data=(root/rel).read_bytes();inputs[rel]=hashlib.sha256(data).hexdigest();return data.decode()
    module_path=root/'scripts/gen_native_metal_formulas.py';source(str(module_path.relative_to(root)))
    spec=importlib.util.spec_from_file_location('native_resource_registry',module_path)
    module=importlib.util.module_from_spec(spec)
    import sys
    sys.modules[spec.name]=module;spec.loader.exec_module(module)
    for path in module.TABLES:source(str(path.relative_to(root)))
    entries=module.registry()
    body=source('src/render/native_metal/formula_resources.h')
    rows=re.findall(r'\{(\d+),(\d+),(\d+),"([^"]+)"\}',body)
    bindings={int(i):{'parameter':int(param),'expanded_option':int(option),'name':name} for i,param,option,name in rows}
    require(len(rows)==len(bindings)==9,'Changed typed callback inventory requires review')
    observed={};calls=0
    def parameter(expression):
        match=re.fullmatch(r'(?:int|static_cast<int>)\(params\[(\d+)\]\)',expression)
        require(match is not None,'Unknown map ID expression: '+expression)
        return int(match.group(1))
    for entry in entries:
        rel=f'src/render/native_metal/generated/formula_{entry.id}.metal.h'
        if not (root/rel).exists():
            require(10353<=entry.id<=10357,'Missing ordinary generated body '+entry.name);continue
        values=module.tokens(source(rel));indices=[]
        for index,token in enumerate(values):
            if not token.startswith('native_formula_map'):continue
            require(token in ('native_formula_map','native_formula_map_direct','native_formula_map_sphere'),
                    'Unknown map callback '+token)
            require(values[index+1]=='(','Non-call map symbol')
            end=module.matching(values,index+1,'(',')');args=module.split_token_args(values[index+2:end])
            position=4 if token.endswith('_sphere') else 3
            require(len(args)==(5 if token!='native_formula_map_direct' else 4),'Unexpected callback signature')
            expression=''.join(args[position])
            if re.fullmatch(r'[A-Za-z_]\w*',expression):
                # Pinned bodies use one immutable local alias. Reject multiple
                # assignments or an expression whose dataflow is no longer clear.
                aliases=[]
                for j in range(1,len(values)-1):
                    if values[j-1]==expression and values[j]=='=':
                        finish=values.index(';',j+1);aliases.append(''.join(values[j+1:finish]))
                require(len(aliases)==1,'Ambiguous map ID alias '+entry.name)
                expression=aliases[0]
            indices.append(parameter(expression));calls+=1
        if not indices:
            require(entry.id not in bindings,'Declared resource without callback '+entry.name);continue
        require(entry.id in bindings,'Callback absent from typed requirements '+entry.name)
        binding=bindings[entry.id]
        require(binding['name']==entry.name and set(indices)=={binding['parameter']},'Callback binding mismatch '+entry.name)
        require(binding['parameter']<len(entry.parameter_types) and entry.parameter_types[binding['parameter']]==2,
                'Callback ID is not integer-expanded '+entry.name)
        observed[entry.id]={'name':entry.name,'parameter':binding['parameter'],'calls':len(indices)}
    require(set(observed)==set(bindings),'Incomplete resource callback inventory')
    report={'status':'PASS_SOURCE_CALLBACK_RESOURCE_BINDINGS','ordinary_external_formulas':len(entries),
        'callback_formulas':len(observed),'callback_calls':calls,'bindings':observed,'source_files':inputs,
        'device_qualification':False,'cpu_prefetch_changed':False}
    a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(report,indent=2)+'\n')
    print(report['status'],len(entries),'formulas',len(observed),'resource formulas',calls,'callbacks')
if __name__=='__main__':main()
