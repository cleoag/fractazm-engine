#!/usr/bin/env python3
"""Phased, fail-closed collection of independent Native Metal formula evidence.

plan and assemble never dispatch GPU work. collect executes only the hash-bound
raw internal-formula jobs in a prepared plan, with a fixed argument allowlist.
Partial cases are recorded separately from qualified formula receipts. This
first corpus phase does not qualify scenes, dimensions, effects or timings.
"""
from __future__ import annotations
import argparse
from collections import Counter, defaultdict
import json
from pathlib import Path
import re
import shlex
import shutil
import struct
import subprocess
import sys
import time

import qualify_native_metal_all as q

PLAN_SCHEMA="mb3d.native-metal-all.formula-plan.v1"
EXECUTION_SCHEMA="mb3d.native-metal-all.formula-execution.v1"
LANES=[f"state_{i:02d}" for i in range(47)]
CURRENT_DESCRIPTOR_CONTROLS=7
IMPORTED_DESCRIPTOR_CONTROLS=(5,6,7)

def safe_id(value):
    q.require(isinstance(value,str) and re.fullmatch(r"[A-Za-z0-9_-]+",value),"Unsafe case/group ID")
    return value

def words(path):
    data=Path(path).read_bytes();q.require(len(data)%4==0,"Unaligned raw words")
    return list(struct.unpack("<"+"I"*(len(data)//4),data))

def bits(values):return {name:f"0x{value:08x}" for name,value in zip(LANES,values)}

def current_binding(contract,build):
    scope=q.load_scope(Path(contract).resolve(),q.ROOT)
    result={"contract_sha256":scope["contract_sha256"],"inventory_bindings":scope["inventory_bindings"],
            "source_files_sha256":q.source_snapshot(q.ROOT)}
    for key,name in (("renderer","mb3d_render"),("library","metal_native_scene.metallib")):
        path=(Path(build)/name).resolve();q.require(path.is_file(),"Missing build artifact: "+name)
        result[key]={"path":str(path),"sha256":q.sha(path)}
    return result

def input_file(base,name,expected_hash):
    path=q.contained(base,name)
    q.require(path.is_file() and q.sha(path)==expected_hash,"Changed expected input: "+str(name))
    return path

def check_program(path,formula_id):
    data=path.read_bytes();q.require(len(data)==8192,"Wrong program ABI size")
    for slot in range(6):
        version,namespace,number=struct.unpack_from("<3I",data,224+slot*1328)
        enabled=struct.unpack_from("<I",data,224+slot*1328+36)[0]
        q.require(version==1 and namespace==0 and enabled==int(slot==0),"External/deferred or unexpected active formula")
        if slot==0:q.require(number==formula_id and 0<=number<10,"Unexpected formula identity")
    return struct.unpack_from("<I",data,224+32)[0]

def fixture_jobs(step_path,orbit_path):
    """Validate original expectations before any output directory is created."""
    step=q.read_json(step_path);orbit=q.read_json(orbit_path);jobs=[]
    q.require(step.get("schema")=="mb3d.independent-native-internal-corpus.v1","Wrong step corpus schema")
    q.require(orbit.get("schema")=="mb3d.native-internal-orbit-controls.v1","Wrong orbit corpus schema")
    for corpus in (step,orbit):
        for path,value in corpus["source_sha256"].items():q.require(q.sha(q.ROOT/path)==value,"Expectation source changed: "+path)
    for group in step["groups"]:
        name="step-"+safe_id(group["id"]);paths={}
        for key,row in group["files"].items():paths[key]=input_file(step_path.parent,row["path"],row["sha256"])
        expected=q.read_json(paths["expected"])
        q.require(len(expected["rows"])==group["count"],"Step count mismatch")
        power=check_program(paths["program"],group["formula_id"])
        jobs.append({"id":name,"kind":"point","formula_id":group["formula_id"],"name":group["formula_name"],
                     "variant":f"power-{power}" if group["formula_id"] in (0,6) else group["id"],
                     "program_path":paths["program"],"states_path":paths["states"],"cases":expected["rows"]})
    names=["Integer Power","Real Power","Quaternion","Tricorn","Amazing Box","Bulbox","Folding Int Pow","test","testIFS","Aexion C"]
    for case in orbit["cases"]:
        name="orbit-"+safe_id(case["id"])
        program=input_file(orbit_path.parent,case["program"],orbit["fixture_sha256"][case["program"]])
        states=input_file(orbit_path.parent,case["states"],orbit["fixture_sha256"][case["states"]])
        number=struct.unpack_from("<I",program.read_bytes(),224+8)[0];power=check_program(program,number)
        partition=case.get("partition","heldout")
        q.require(partition in ("fit","heldout"),"Invalid independent corpus partition")
        jobs.append({"id":name,"kind":"orbit","formula_id":number,"name":names[number],
                     "variant":f"power-{power}" if number in (0,6) else case["id"],
                     "program_path":program,"states_path":states,
                     "cases":[{**case,"partition":partition,"allowed_written_lanes":[i for i,(a,b) in enumerate(zip(case["input_words"],case["expected_state_words"])) if a!=b]}]})
    q.require(len({j["id"] for j in jobs})==len(jobs),"Duplicate job ID")
    for job in jobs:
        state_words=words(job["states_path"])
        q.require(len(state_words)==47*len(job["cases"]),"State corpus size mismatch")
        seen=set()
        for i,case in enumerate(job["cases"]):
            safe_id(case["id"]);q.require(case["id"] not in seen,"Duplicate case ID");seen.add(case["id"])
            q.require(case["partition"] in ("fit","heldout") and case["input_words"]==state_words[i*47:(i+1)*47],"Case partition/input mismatch")
            q.require(len(case["expected_state_words"])==47 and case["expected_fault"]==0,"Incomplete expected state")
    return jobs

def plan(args):
    jobs=fixture_jobs(args.steps.resolve(),args.orbits.resolve())
    q.require(not args.output.exists(),"Output must be new")
    binding=current_binding(args.contract,args.build)
    probe=(args.build/"mb3d_native_formula_probe").resolve();q.require(probe.is_file(),"Missing formula probe")
    args.output.mkdir(parents=True);base=args.output.resolve()
    for job in jobs:
        target=base/"inputs"/job["id"];target.mkdir(parents=True)
        for key,filename in (("program","program.bin"),("states","states.bin")):
            origin=job.pop(key+"_path");identity=q.sha(origin);destination=target/filename
            shutil.copyfile(origin,destination);q.require(q.sha(destination)==identity,"Input copy mismatch")
            job[key]={"path":str(destination.relative_to(base)),"sha256":identity,"origin":str(origin)}
    value={"schema":PLAN_SCHEMA,"binding":binding,"probe":{"path":str(probe),"sha256":q.sha(probe)},
           "contract":str(args.contract.resolve()),"build":str(args.build.resolve()),"producer":"native_formula_phase_collector",
           "expected_sources":{"steps":{"path":str(args.steps.resolve()),"sha256":q.sha(args.steps)},
                               "orbits":{"path":str(args.orbits.resolve()),"sha256":q.sha(args.orbits)}},"jobs":jobs,
           "scope":{"step_cases":sum(len(j["cases"]) for j in jobs if j["kind"]=="point"),"orbit_cases":sum(len(j["cases"]) for j in jobs if j["kind"]=="orbit")},
           "next_commands":[f"python3 -B comparison/tools/collect_native_metal_evidence.py collect --plan {shlex.quote(str(base/'plan.json'))}",
                            f"python3 -B comparison/tools/collect_native_metal_evidence.py assemble --plan {shlex.quote(str(base/'plan.json'))}"],
           "remaining_phases":["Independent fit orbit cases for each formula", "All457 external formulas", "Full467 heldout point+orbit review",
                               "1187 pinned ordinary-scene/classification obligations plus separately pinned additional cases", "Full active-effect scenes with3exact repeats and NativeCPU reference",
                               "Main renderer cold/warm:3fresh processes/backend, discardedwarmups and>=3complete warm renders/process", "All24feature controls and8global receipts"]}
    (base/"plan.json").write_text(json.dumps(value,indent=2)+"\n")
    print(json.dumps({"status":"PLAN_ONLY_NO_GPU","jobs":len(jobs),**value["scope"]}))

def load_plan(path):
    value=q.read_json(path);q.require(value.get("schema")==PLAN_SCHEMA,"Wrong plan schema")
    q.require(Path(value["probe"]["path"]).resolve()==(Path(value["build"])/"mb3d_native_formula_probe").resolve(),"Unexpected probe executable path")
    q.unique(value.get("jobs"),lambda row:safe_id(row.get("id")),"plan jobs")
    for job in value["jobs"]:
        check_program(q.artifact(path.parent,job["program"]),job["formula_id"])
        q.artifact(path.parent,job["states"])
        q.require(job["kind"] in ("point","orbit") and 0<=job["formula_id"]<10,"Unsupported plan job")
    return value

def screen_probe_command(argv, probe):
    """Only the collector's fixed probe protocol; never execute a shell string."""
    require=q.require
    require(isinstance(argv,list) and len(argv)==15 and all(isinstance(a,str) for a in argv),
            "Malformed probe command")
    require(Path(argv[0]).resolve()==probe.resolve(),"Unexpected probe executable")
    require(argv[1::2]==["--program","--states","--mode","--slot","--repeats","--library","--output"],
            "Unexpected probe options")
    require(argv[6] in ("step","orbit") and argv[8]=="0" and argv[10]=="3",
            "Unexpected probe mode/limits")
    for index in (2,4,12,14):
        require(Path(argv[index]).is_absolute() and "\0" not in argv[index],"Probe paths must be absolute")

def collect(args):
    path=args.plan.resolve();value=load_plan(path);base=path.parent
    q.require(current_binding(value["contract"],value["build"])==value["binding"],"Stale source/build plan; prepare a new plan")
    probe=Path(value["probe"]["path"]);q.require(q.sha(probe)==value["probe"]["sha256"],"Probe changed")
    destination=base/"observed";q.require(not destination.exists(),"Observations already exist")
    q.require(shutil.disk_usage(base).free>=13*1024**3,"Free disk below13GiB")
    destination.mkdir();results=[]
    for job in value["jobs"]:
        q.require(shutil.disk_usage(base).free>=13*1024**3,"Free disk below13GiB")
        report=destination/(job["id"]+".json");log=destination/(job["id"]+".log")
        argv=[str(probe),"--program",str(q.artifact(base,job["program"])),"--states",str(q.artifact(base,job["states"])),
              "--mode","step" if job["kind"]=="point" else "orbit","--slot","0","--repeats","3",
              "--library",value["binding"]["library"]["path"],"--output",str(report)]
        screen_probe_command(argv,probe)
        start=time.monotonic()
        try:
            process=subprocess.run(argv,capture_output=True,text=True,timeout=90)
            output=process.stdout+process.stderr;code=process.returncode
        except subprocess.TimeoutExpired as error:
            output=f"TIMEOUT after90seconds: {error}";code=124
        log.write_text(output)
        row={"id":job["id"],"argv":argv,"exit_code":code,"wall_seconds":time.monotonic()-start,
             "log":{"path":str(log.relative_to(base)),"sha256":q.sha(log)},"screen":"ok",
             "screen_policy":"fixed_probe_argv_v1_no_shell"}
        if report.is_file():row["report"]={"path":str(report.relative_to(base)),"sha256":q.sha(report)}
        results.append(row)
        print(job["id"],code,flush=True)
        if code==124:break
    stable=current_binding(value["contract"],value["build"])==value["binding"] and q.sha(probe)==value["probe"]["sha256"]
    receipt={"schema":EXECUTION_SCHEMA,"plan_sha256":q.sha(path),"binding":value["binding"],"probe":value["probe"],
             "identity_unchanged":stable,"results":results}
    (base/"execution.json").write_text(json.dumps(receipt,indent=2)+"\n")
    q.require(stable,"Source/build changed during execution")
    return 0 if len(results)==len(value["jobs"]) and all(r["exit_code"]==0 for r in results) else 1

def validate_observation(report,job,program_words,controls=CURRENT_DESCRIPTOR_CONTROLS):
    q.require(report.get("schema")=="mb3d.native-formula-probe.v1" and report.get("status")=="COMPLETE"
              and report.get("profile")=="native_gpu_fp32" and report.get("library_profile")==[2,4,32]
              and report.get("formula_abi")==[1,8192,188,236,32] and bool(report.get("device")),"Missing actual shader ABI/profile marker")
    q.require(report.get("mode")==int(job["kind"]=="orbit") and report.get("slot")==0 and
              report.get("program_words")==program_words,"Wrong program/mode/slot")
    q.require(report.get("repeats",0)>=3 and report.get("exact_state_repeats") is True and
              report.get("input_immutable") is True and report.get("allocation_canaries") is True and
              type(report.get("invalid_descriptor_controls")) is int and
              report["invalid_descriptor_controls"]==controls and report.get("fault_count")==0,"Probe repeat/guard failure")
    rows=report.get("rows",[])
    q.require(report.get("point_count")==len(job["cases"]) and len(rows)==len(job["cases"]),"Missing/extra formula case")
    result=[]
    for index,(expected,actual) in enumerate(zip(job["cases"],rows)):
        output=actual.get("result_words",[])
        q.require(actual.get("index")==index and actual.get("input_words")==expected["input_words"],"Wrong observed input/order")
        q.require(len(output)==59 and output[:47]==expected["expected_state_words"],"Independent full47state mismatch: "+expected["id"])
        q.require(actual.get("fault")==0 and output[47]==0 and type(actual.get("work")) is int and actual["work"]>0
                  and output[51]==actual["work"] and output[53:55]==[0,0] and output[56]==1 and output[58]==0,"Invalid/inactive formula execution")
        if job["kind"]=="orbit":
            q.require(output[55]==expected["expected_distance_word"] and actual["work"]==expected["expected_formula_work"],"Independent orbit distance/work mismatch")
        result.append({"case_id":job["id"]+":"+expected["id"],"identity":["internal",job["formula_id"],job["name"]],
                       "kind":job["kind"],"partition":expected["partition"],"variant":job["variant"],"gpu_invocations":actual["work"],"faults":0,
                       "input_state_bits":bits(expected["input_words"]),"parameter_bits":[f"0x{word:08x}" for word in program_words],
                       "expected_state_bits":bits(expected["expected_state_words"]),"gpu_state_bits":bits(output[:47]),
                       "read_lanes":[],"written_lanes":[LANES[i] for i in expected["allowed_written_lanes"]],
                       "all_input_and_output_lanes_checked":LANES})
    return result

def import_observations(args):
    """Retain prior hash-bound probe observations without inventing source proof."""
    path=args.plan.resolve();value=load_plan(path);base=path.parent
    current=current_binding(value["contract"],value["build"])
    q.require(all(current[key]==value["binding"][key] for key in ("renderer","library","contract_sha256","inventory_bindings")),"Stale import artifacts/scope")
    q.require(q.sha(value["probe"]["path"])==value["probe"]["sha256"],"Stale import probe")
    q.require(not (base/"execution.json").exists() and not (base/"observed").exists(),"Observations already exist")
    sources={"point":args.steps.resolve(),"orbit":args.orbits.resolve()};admitted=[]
    for kind,receipt_path in sources.items():
        receipt=q.read_json(receipt_path)
        if kind=="point":
            bound=receipt["binding"]
            q.require(receipt.get("identity_unchanged") is True,"Step execution identity changed")
            binary,library=bound["binary"]["sha256"],bound["library"]["sha256"]
            q.require(bound["manifest_sha256"]==value["expected_sources"]["steps"]["sha256"],"Wrong step expectation manifest")
        else:
            q.require(receipt["binding_before"]==receipt["binding_after"],"Orbit execution identity changed")
            bound=receipt["binding_before"];binary,library=bound["binary"],bound["library"]
            q.require(bound["expected"]==value["expected_sources"]["orbits"]["sha256"],"Wrong orbit expectation manifest")
        q.require(binary==value["probe"]["sha256"] and library==value["binding"]["library"]["sha256"],"Stale observed probe/library")
        records=q.unique(receipt["results"],lambda row:safe_id(row["id"]),"imported job")
        selected=[job for job in value["jobs"] if job["kind"]==kind]
        q.require(set(records)=={job["id"].split("-",1)[1] for job in selected},"Missing/extra imported job")
        for job in selected:
            row=records[job["id"].split("-",1)[1]]
            report_path=receipt_path.parent/(row["id"]+".json")
            q.require(report_path.is_file() and q.sha(report_path)==row["report_sha256"],"Changed imported observation")
            report=q.read_json(report_path)
            q.require(report.get("library")==value["binding"]["library"]["path"] and report.get("program")==job["program"]["origin"],"Imported resource path mismatch")
            controls=report.get("invalid_descriptor_controls")
            q.require(type(controls) is int and controls in IMPORTED_DESCRIPTOR_CONTROLS,"Unsupported imported guard inventory")
            validate_observation(report,job,words(q.artifact(base,job["program"])),controls=controls)
            argv=row.get("command")
            q.require(isinstance(argv,list) and argv and argv[0]==value["probe"]["path"],"Imported executable mismatch")
            q.require(argv[argv.index("--states")+1]==job["states"]["origin"] and argv[argv.index("--output")+1]==str(report_path),"Imported command/input binding mismatch")
            admitted.append((job,row,report_path,receipt_path))
    destination=base/"observed";destination.mkdir();results=[]
    for job,row,origin,receipt_path in admitted:
        target=destination/(job["id"]+".json");expected_hash=q.sha(origin);shutil.copyfile(origin,target)
        q.require(q.sha(target)==expected_hash==row["report_sha256"],"Import copy changed")
        record_path=destination/(job["id"]+"-origin.json")
        record_path.write_text(json.dumps({"origin_report":str(origin),"origin_report_sha256":expected_hash,
            "origin_receipt":str(receipt_path),"origin_receipt_sha256":q.sha(receipt_path),"original_command":row["command"],
            "note":"This is an import record, not a fabricated process log."},indent=2)+"\n")
        results.append({"id":job["id"],"exit_code":0,"argv":row["command"],"imported":True,
                        "report":{"path":str(target.relative_to(base)),"sha256":expected_hash},
                        "log":{"path":str(record_path.relative_to(base)),"sha256":q.sha(record_path)}})
    execution={"schema":EXECUTION_SCHEMA,"plan_sha256":q.sha(path),"binding":value["binding"],"probe":value["probe"],
               "identity_unchanged":True,"imported":True,"full_source_execution_binding":False,
               "limits":"Prior receipts bind actual probe/library/expectations, not the new full source snapshot. Partial cases only; no current formula proof is promoted.","results":results}
    q.require(q.sha(value["probe"]["path"])==value["probe"]["sha256"] and
              q.sha(value["binding"]["library"]["path"])==value["binding"]["library"]["sha256"],"Probe/library changed during import")
    (base/"execution.json").write_text(json.dumps(execution,indent=2)+"\n")
    print(json.dumps({"status":"IMPORTED_PARTIAL_ONLY","jobs":len(results)}))

def readiness(cases):
    result={}
    for number in range(10):
        selected=[row for row in cases if row["identity"][1]==number]
        count=Counter((row["partition"],row["kind"]) for row in selected)
        result[number]={"fit_point":count["fit","point"],"fit_orbit":count["fit","orbit"],
                        "heldout_point":count["heldout","point"],"heldout_orbit":count["heldout","orbit"],
                        "working_point_orbit_ready":count["fit","point"]>0 and count["fit","orbit"]>0,
                        "heldout_point_orbit_ready":count["heldout","point"]>0 and count["heldout","orbit"]>0}
    return result

def execution_binding(plan,execution,current):
    if execution.get("imported"):
        q.require(execution.get("full_source_execution_binding") is False,"Imported observation cannot claim a new source binding")
        q.require(all(current[key]==plan["binding"][key] for key in ("renderer","library","contract_sha256","inventory_bindings")),"Stale imported artifact/scope")
        return current,False
    q.require(current==plan["binding"],"Stale plan cannot become current evidence")
    return current,True

def observation_controls(report,row,execution):
    """A historical row cannot acquire a fresh source binding by itself."""
    imported=execution.get("imported",False)
    q.require(type(imported) is bool and row.get("imported",False) is imported,
              "Mixed imported/current execution rows")
    if imported:
        q.require(execution.get("full_source_execution_binding") is False,
                  "Imported observation cannot claim a new source binding")
        controls=report.get("invalid_descriptor_controls")
        q.require(type(controls) is int and controls in IMPORTED_DESCRIPTOR_CONTROLS,
                  "Unknown imported guard inventory")
        return controls
    return CURRENT_DESCRIPTOR_CONTROLS

def assemble(args):
    path=args.plan.resolve();value=load_plan(path);base=path.parent;binding=value["binding"]
    current=current_binding(value["contract"],value["build"])
    output=base/"assembled";q.require(not output.exists(),"Assembly already exists; retain previous evidence")
    execution_path=base/"execution.json"
    observations={};errors=[];cases=[];artifacts=[];source_bound=False
    if execution_path.is_file():
        execution=q.read_json(execution_path)
        q.require(execution.get("schema")==EXECUTION_SCHEMA and execution.get("plan_sha256")==q.sha(path)
                  and execution.get("binding")==binding and execution.get("probe")==value["probe"]
                  and execution.get("identity_unchanged") is True,"Stale/unbound execution receipt")
        observations=q.unique(execution.get("results"),lambda r:r.get("id"),"observed job")
        binding,source_bound=execution_binding(value,execution,current)
        q.require(set(observations)<={j["id"] for j in value["jobs"]},"Unexpected observed job")
    else:q.require(current==binding,"Stale empty plan")
    for job in value["jobs"]:
        row=observations.get(job["id"])
        if row is None:errors.append({"job":job["id"],"status":"OPEN","reason":"No current actual GPU observation"});continue
        try:
            q.require(row["exit_code"]==0,"Probe process failed")
            report_path=q.artifact(base,row.get("report"));q.artifact(base,row.get("log"))
            report=q.read_json(report_path)
            expected_program=job["program"]["origin"] if row.get("imported") else str(q.artifact(base,job["program"]))
            q.require(report.get("library")==binding["library"]["path"] and report.get("program")==expected_program,"Observed resource path mismatch")
            controls=observation_controls(report,row,execution)
            cases.extend(validate_observation(report,job,words(q.artifact(base,job["program"])),controls=controls))
            artifacts.extend([row["report"],row["log"],job["program"],job["states"]])
        except (q.InvalidEvidence,KeyError,TypeError,ValueError,OSError) as error:
            errors.append({"job":job["id"],"status":"FAILED","reason":str(error)})
    output.mkdir();binding_id=q.digest(binding)
    # Relative paths in the final evidence remain within the plan directory.
    evidence={"schema":q.SCHEMA,"binding":binding,"formulas":[],"scenes":[],"additional_scenes":[],"features":[],"receipts":[]}
    ready=readiness(cases);review=None
    if args.review:
        review=q.read_json(args.review)
        q.require(review.get("schema")=="mb3d.native-metal-all.formula-review.v1" and review.get("plan_sha256")==q.sha(path)
                  and execution_path.is_file() and review.get("execution_sha256")==q.sha(execution_path)
                  and isinstance(review.get("reviewer"),str) and review["reviewer"] not in ("",value["producer"]),"Missing independent bound review")
        reviewed_hash=q.sha(args.review);retained_review=output/"independent-review.json"
        shutil.copyfile(args.review,retained_review)
        q.require(q.sha(retained_review)==reviewed_hash,"Independent review changed while retaining it")
        artifacts.extend([{"path":"assembled/independent-review.json","sha256":reviewed_hash},
                          {"path":"plan.json","sha256":q.sha(path)},
                          {"path":"execution.json","sha256":q.sha(execution_path)}])
    for number in range(10):
        for partition in ("fit","heldout"):
            selected=[row for row in cases if row["identity"][1]==number and row["partition"]==partition]
            if not selected:continue
            observation={"schema":"mb3d.native-metal-all.formula-observations.v1","binding_sha256":binding_id,
                         "library_marker":[2,4,32],"execution":"metal_gpu","cpu_pixel_calls":0,"cases":selected}
            name=f"internal-{number}-{partition}.json";(output/name).write_text(json.dumps(observation,indent=2)+"\n")
            observed={"path":"assembled/"+name,"sha256":q.sha(output/name)}
            failed_for_formula=any(e["job"] in {j["id"] for j in value["jobs"] if j["formula_id"]==number} for e in errors)
            if partition!="fit" or not ready[number]["working_point_orbit_ready"] or failed_for_formula or review is None or not source_bound:continue
            identity=selected[0]["identity"]
            q.require(identity in review.get("approved_identities",[]),"Formula lacks independent review")
            # Reuse the final predicate's raw-state checks before emitting proof.
            q.formula_observations(base,observed,tuple(identity),binding_id)
            proof={"schema":q.RECEIPT_SCHEMA,"kind":"formula_gpu_point_orbit","status":"PASS","binding_sha256":binding_id,
                   "producer":value["producer"],"independent_reviewer":review["reviewer"],"identity":identity,
                   "checks":[{"id":"independent-full-state-and-program","status":"PASS"}],"artifacts":artifacts,
                   "observations":observed,"gpu_point_cases":ready[number]["fit_point"],"gpu_orbit_cases":ready[number]["fit_orbit"],
                   "faults":0,"state_contract_verified":True,"parameter_contract_verified":True,"cpu_pixel_calls":0}
            proof_name=f"internal-{number}-proof.json";(output/proof_name).write_text(json.dumps(proof,indent=2)+"\n")
            evidence["formulas"].append({"namespace":"internal","id":number,"name":identity[2],
                                         "proof":{"path":"assembled/"+proof_name,"sha256":q.sha(output/proof_name)}})
    (base/"evidence.json").write_text(json.dumps(evidence,indent=2)+"\n")
    scope=q.load_scope(Path(value["contract"]),q.ROOT)
    qualification=q.evaluate(scope,evidence,base,binding)
    ledger={"schema":"mb3d.native-metal-all.formula-phase-ledger.v1","status":"PARTIAL_GOAL_OPEN","readiness":ready,
            "observed_cases":len(cases),"fit_cases":sum(r["partition"]=="fit" for r in cases),"heldout_cases":sum(r["partition"]=="heldout" for r in cases),
            "errors":errors,"source_bound_execution":source_bound,"qualified_formula_receipts":len(evidence["formulas"]),"full_predicate":qualification,
            "remaining_phases":value["remaining_phases"],"limits":["Raw step/orbit cases do not prove full scene rendering or all parameter branches.",
                "Heldout cases are never moved into fit to close missing working orbit obligations.","No scene, feature, timing, full heldout, build-provenance or no-CPU-work global receipt is fabricated."]}
    (output/"ledger.json").write_text(json.dumps(ledger,indent=2)+"\n")
    print(json.dumps({"status":ledger["status"],"observed_cases":len(cases),"formula_receipts":len(evidence["formulas"]),"remaining":qualification["remaining"]}))
    return 0

def main(argv=None):
    parser=argparse.ArgumentParser(description=__doc__);commands=parser.add_subparsers(dest="phase",required=True)
    p=commands.add_parser("plan")
    for option in ("contract","build","steps","orbits","output"):p.add_argument("--"+option,type=Path,required=True)
    p.set_defaults(run=plan)
    for name,function in (("collect",collect),("assemble",assemble)):
        p=commands.add_parser(name);p.add_argument("--plan",type=Path,required=True);p.set_defaults(run=function)
        if name=="assemble":p.add_argument("--review",type=Path)
    p=commands.add_parser("import-observations")
    for option in ("plan","steps","orbits"):p.add_argument("--"+option,type=Path,required=True)
    p.set_defaults(run=import_observations)
    args=parser.parse_args(argv)
    try:return args.run(args) or 0
    except (q.InvalidEvidence,OSError,KeyError,ValueError,TypeError) as error:
        print("INVALID: "+str(error),file=sys.stderr);return 2

if __name__=="__main__":raise SystemExit(main())
