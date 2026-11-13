"""Sequential ordinary CLI backend checks; no Pascal, Monte Carlo or JIT work.

The portable subset runs against a build without Metal. The Metal subset also
compares the main CLI with the independent scene-host entry point when supplied.
Run only when no benchmark, VM workload or other renderer is active.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import struct
import subprocess
import tempfile
import zlib


SIZE = (64, 48)
WORKERS = 6
SCENES = ("sphereifs-ao-shadows", "boxifs-ao-shadows", "integerpower8-ao-shadows")


def execution_work(execution):
    """Compare every work field; validate and exclude only declared wall times
    and the pipeline archive's [hits, misses], which a repeat turns into hits."""
    timings = {"ao_gpu_seconds", "ao_max_command_seconds", "ao_preparation_seconds"}
    for name in timings:
        value = execution[name]
        assert type(value) in (int, float) and math.isfinite(value) and value >= 0, (name, value)
    archive = execution["pipeline_archive"]
    # [0, 0] when MB3D_METAL_PIPELINE_CACHE is off.
    assert (type(archive) is list and len(archive) == 2 and
            all(type(v) is int and v >= 0 for v in archive)), ("pipeline_archive", archive)
    return {key: value for key, value in execution.items() if key not in timings | {"pipeline_archive"}}


def png(path):
    """Read and CRC-check the production writer's bounded, noninterlaced PNG."""
    data = path.read_bytes()
    assert data[:8] == b"\x89PNG\r\n\x1a\n", (path, "PNG signature")
    offset, header, payload, ended = 8, None, bytearray(), False
    while offset < len(data):
        assert offset + 12 <= len(data), (path, "truncated PNG chunk")
        length = struct.unpack_from(">I", data, offset)[0]
        name = data[offset + 4:offset + 8]
        end = offset + 8 + length
        assert end + 4 <= len(data), (path, "truncated PNG payload")
        body = data[offset + 8:end]
        expected = struct.unpack_from(">I", data, end)[0]
        assert zlib.crc32(name + body) & 0xffffffff == expected, (path, "PNG CRC", name)
        if name == b"IHDR":
            assert header is None and offset == 8 and length == 13, (path, "PNG header")
            header = struct.unpack(">IIBBBBB", body)
        elif name == b"IDAT":
            assert header is not None, (path, "PNG data before header")
            payload.extend(body)
        elif name == b"IEND":
            assert length == 0 and end + 4 == len(data), (path, "PNG ending")
            ended = True
            break
        offset = end + 4
    assert ended and header and payload, (path, "incomplete PNG")
    assert header[:2] == SIZE and header[4:] == (0, 0, 0), (path, header)
    return header, bytes(payload)


def rgb(path):
    header, compressed = png(path)
    assert header[2:4] == (8, 2), (path, "expected 8-bit RGB PNG", header)
    stride = header[0] * 3
    raw = zlib.decompress(compressed)
    assert len(raw) == header[1] * (stride + 1), (path, "PNG scanline length")
    previous, decoded = bytearray(stride), bytearray()
    for y in range(header[1]):
        start = y * (stride + 1)
        method = raw[start]
        assert 0 <= method <= 4, (path, "unknown PNG filter", method)
        row = bytearray(raw[start + 1:start + stride + 1])
        for x in range(stride):
            left = row[x - 3] if x >= 3 else 0
            above = previous[x]
            upper_left = previous[x - 3] if x >= 3 else 0
            if method == 1:
                prediction = left
            elif method == 2:
                prediction = above
            elif method == 3:
                prediction = (left + above) // 2
            elif method == 4:
                p = left + above - upper_left
                a, b, c = abs(p - left), abs(p - above), abs(p - upper_left)
                prediction = left if a <= b and a <= c else (above if b <= c else upper_left)
            else:
                prediction = 0
            row[x] = (row[x] + prediction) & 255
        decoded.extend(row)
        previous = row
    return bytes(decoded)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--renderer", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--expect-metal", choices=("yes", "no"), required=True)
    parser.add_argument("--scratch", type=Path)
    parser.add_argument("--native-library", type=Path)
    parser.add_argument("--compatible-library", type=Path)
    parser.add_argument("--metal-host", type=Path)
    args = parser.parse_args(argv)
    renderer = args.renderer.resolve(strict=True)
    source = args.source_root.resolve(strict=True)
    has_metal = args.expect_metal == "yes"
    if has_metal and args.native_library is None:
        parser.error("Metal checks require --native-library")
    native = args.native_library.resolve(strict=True) if args.native_library else None
    compatible = args.compatible_library.resolve(strict=True) if args.compatible_library else None
    metal_host = args.metal_host.resolve(strict=True) if args.metal_host else None
    if metal_host and not has_metal:
        parser.error("--metal-host requires --expect-metal yes")
    parent = args.scratch.resolve() if args.scratch else None
    if parent:
        parent.mkdir(parents=True, exist_ok=True)
    environment = {k: v for k, v in os.environ.items() if not k.upper().startswith("MB3D_")}
    scene_root = source / "comparison/benchmark_scenes/effects-v1"
    scenes = {name: scene_root / (name + ".m3p") for name in SCENES}
    ordinary = scenes[SCENES[0]]
    checks = 0
    runs = 0
    input_hashes = {}

    with tempfile.TemporaryDirectory(dir=parent, prefix="render-backend-") as temporary:
        scratch = Path(temporary)

        def check(condition, detail):
            nonlocal checks
            assert condition, detail
            checks += 1

        def invoke(label, extra=(), *, scene=ordinary, code=0, profile=True,
                   output=None, sentinel=None):
            nonlocal runs
            input_hashes.setdefault(scene, hashlib.sha256(scene.read_bytes()).hexdigest())
            output = output or scratch / (label + ".png")
            if sentinel is not None:
                output.write_bytes(sentinel)
            sidecar = scratch / (label + ".json")
            command = [str(renderer), str(scene), "--width", str(SIZE[0]), "--height", str(SIZE[1]),
                       "--threads", str(WORKERS), "--deterministic", "--output", str(output)]
            if profile:
                command += ["--profile-json", str(sidecar)]
            command += list(map(str, extra))
            result = subprocess.run(command, cwd=scratch, env=environment, text=True,
                                    capture_output=True, timeout=90)
            runs += 1
            detail = {"command": command, "expected": code, "returncode": result.returncode,
                      "stdout": result.stdout, "stderr": result.stderr}
            check(result.returncode == code, detail)
            if code:
                check(output.read_bytes() == sentinel if sentinel is not None else not output.exists(),
                      ("Failed operation changed/published output", detail))
            data = json.loads(sidecar.read_text()) if sidecar.exists() else None
            return result, data, output

        def metadata(result, profile, requested, effective, *, fallback=False,
                     fallback_codes=(), precision="native", code=0):
            check(isinstance(profile, dict), ("Missing backend profile", result.stderr))
            check(profile["backend_requested"] == requested and profile["backend"] == effective, profile)
            check(profile["backend_fallback"] is fallback, profile)
            check(profile["precision"] == precision, profile)
            expected_numeric = "native_metal_fp32" if effective == "metal" else (
                "native_cpu_fp64" if precision == "native" else "compatible_cpu")
            check(profile["numeric_profile"] == expected_numeric, profile)
            expected_schema = "mb3d.render-stage-profile.v1" if effective == "metal" else "mb3d.cpu-stage-profile.v1"
            check(profile["schema"] == expected_schema and profile["exit_code"] == code, profile)
            seconds = profile["backend_selection_seconds"]
            check(type(seconds) in (float, int) and math.isfinite(seconds) and seconds >= 0, profile)
            reason, reason_code = profile["backend_fallback_reason"], profile["backend_fallback_code"]
            if fallback:
                check(bool(reason) and reason_code in fallback_codes, profile)
                check(reason in result.stderr and requested in result.stderr and "cpu" in result.stderr,
                      ("Fallback not visible without --verbose", result.stderr, profile))
            else:
                check(reason == "" and reason_code == "", profile)
                check(requested in result.stderr and effective in result.stderr,
                      ("Requested/effective backend not logged", result.stderr))
            if code == 0:
                check(profile["status"] == "complete" and profile["render_status"] == "completed", profile)
                execution = profile["metal_execution"]
                if effective == "metal":
                    pixels = SIZE[0] * SIZE[1]
                    check(execution["engine"] in ("exact_capability", "general_formula"), execution)
                    check(execution["validated"] is True and execution["library_profile"] == [2, 4, 32], execution)
                    check(all(execution[k] == 0 for k in ("faults", "nonfinite", "work_exhausted")), execution)
                    check(execution["primary_records"] == pixels and execution["normal_records"] == pixels and
                          execution["surface_records"] == 2 * pixels and execution["color_records"] == pixels, execution)
                    shadow_factor = 2 if execution["engine"] == "exact_capability" else 1
                    check(execution["shadow_records"] == (shadow_factor * pixels if profile["stages"]["shadows"]["state"] == "completed" else 0), execution)
                    check(execution["reflection_records"] == (pixels if profile["stages"]["reflections"]["state"] == "completed" else 0), execution)
                else:
                    check(execution["engine"] == "none", execution)
                    check(execution["validated"] is False and execution["library_profile"] == [0, 0, 0], execution)
                actual_workers = 0 if effective == "metal" else WORKERS
                check((profile["width"], profile["height"], profile["workers"]) == (*SIZE, actual_workers), profile)
                check(profile["threads_requested"] == WORKERS, profile)
                for stage in ("load", "preparation", "primary", "color", "final_surface", "save"):
                    check(profile["stages"][stage]["state"] == "completed", (stage, profile))
            if effective == "metal" and code == 0:
                check(bool(profile["metal_device"]) and bool(profile["metal_library"]), profile)

        # Omitted backend is exactly the old explicit CPU selection/default.
        result, default, default_image = invoke("default")
        default_precision = default["precision"]
        check(default_precision in ("compatible", "native"), default)
        metadata(result, default, "cpu", "cpu", precision=default_precision)
        result, explicit, explicit_image = invoke("explicit-cpu", ["--backend", "cpu"])
        metadata(result, explicit, "cpu", "cpu", precision=default_precision)
        check(rgb(default_image) == rgb(explicit_image), "Explicit CPU changed default image")
        cpu_native = {}
        for mode in ("compatible", "native"):
            result, profile, image = invoke("cpu-" + mode, ["--backend", "cpu", "--precision", mode])
            metadata(result, profile, "cpu", "cpu", precision=mode)
            if mode == default_precision:
                check(rgb(image) == rgb(default_image), "CPU precision default override changed image")
            if mode == "native":
                cpu_native[ordinary] = rgb(image)

        # Argument order cannot turn explicit compatible arithmetic into Native.
        for backend in ("metal", "auto"):
            for index, flags in enumerate((["--backend", backend, "--precision", "compatible"],
                                            ["--precision", "compatible", "--backend", backend])):
                invoke(f"incompatible-{backend}-{index}", flags, code=1)
            invoke("info-" + backend, ["--backend", backend, "--info"], code=1, profile=False)
        invoke("invalid-backend", ["--backend", "gpu"], code=1)
        invoke("missing-backend", ["--backend"], code=1)
        invoke("cpu-library", ["--backend", "cpu", "--metal-library", scratch / "unused.metallib"], code=1)

        # Both explicit Metal and automatic selection must give a real reason
        # and the exact Native CPU output when capability/options are unsupported.
        transparent_bytes = bytearray((scene_root / "integerpower8-reflections.m3p").read_bytes())
        check(struct.unpack_from("<i", transparent_bytes)[0] == 44 and transparent_bytes[336] == 1,
              "Expected raw opaque reflection fixture")
        # Documented TMandHeader10.bCalcSRautomatic: transmission (bit 2)
        # restricted to dIFS surfaces (bit 4) stays unsupported on Metal.
        transparent_bytes[336] = 7
        transparent_scene = scratch / "reflection-difs-transmission.m3p"
        transparent_scene.write_bytes(transparent_bytes)
        unsupported_scenes = (
            transparent_scene,
        )
        for index, scene in enumerate(unsupported_scenes):
            _, _, baseline = invoke(f"fallback-scene-cpu-{index}", ["--backend", "cpu", "--precision", "native"], scene=scene)
            for backend in ("metal", "auto"):
                flags = ["--backend", backend]
                if native:
                    flags += ["--metal-library", native]
                result, profile, image = invoke(f"fallback-scene-{index}-{backend}", flags, scene=scene)
                codes = ("unsupported_scene",) if has_metal else ("unsupported_scene", "metal_not_built")
                metadata(result, profile, backend, "cpu", fallback=True, fallback_codes=codes)
                check(rgb(image) == rgb(baseline), "Unsupported scene fallback changed CPU image")

        option_cases = [("no-shadows", ["--no-shadows"]),
                        ("no-postprocess", ["--no-postprocess"]), ("unlimited-control", ["--work-budget", "0"])]
        for label, options in option_cases:
            _, _, baseline = invoke(label + "-cpu", ["--backend", "cpu", "--precision", "native", *options])
            for backend in ("metal", "auto"):
                flags = ["--backend", backend, *options]
                if native:
                    flags += ["--metal-library", native]
                result, profile, image = invoke(label + "-" + backend, flags)
                metadata(result, profile, backend, "cpu", fallback=True,
                         fallback_codes=("unsupported_options", "metal_not_built") if not has_metal else ("unsupported_options",))
                check(rgb(image) == rgb(baseline), "Option fallback ignored CPU override: " + label)

        for backend in ("metal", "auto"):
            diagnostic = scratch / ("diagnostics-" + backend + ".txt")
            result, profile, image = invoke("diagnostic-" + backend,
                ["--backend", backend, "--diag-points", "16,12", "--diag-dump", diagnostic])
            metadata(result, profile, backend, "cpu", fallback=True,
                     fallback_codes=("unsupported_options", "metal_not_built") if not has_metal else ("unsupported_options",))
            check(diagnostic.is_file() and diagnostic.stat().st_size > 0, "Diagnostic fallback lost requested dump")
            check(rgb(image) == cpu_native[ordinary], "Diagnostic fallback changed Native CPU RGB")
            sentinel = b"existing image must survive exhausted fallback\n"
            result, profile, _ = invoke("limited-" + backend,
                ["--backend", backend, "--work-budget", "1"], code=6, sentinel=sentinel)
            metadata(result, profile, backend, "cpu", fallback=True, code=6,
                     fallback_codes=("unsupported_options", "metal_not_built") if not has_metal else ("unsupported_options",))
            check(profile["render_status"] == "work_exhausted" and profile["stages"]["save"]["state"] == "not_run", profile)

        for backend in ("metal", "auto"):
            missing = scratch / "does-not-exist.metallib"
            result, profile, image = invoke("missing-library-" + backend,
                                            ["--backend", backend, "--metal-library", missing])
            metadata(result, profile, backend, "cpu", fallback=True,
                     fallback_codes=("metal_unavailable",) if has_metal else ("metal_not_built",))
            check(rgb(image) == cpu_native[ordinary], "Unavailable backend changed Native CPU RGB")

        # Same-profile repeats, not CPU/GPU equality: Native GPU is FP32.
        metal_reference = {}
        for name, scene in scenes.items():
            _, _, first = invoke(name + "-cpu-first", ["--backend", "cpu", "--precision", "native"], scene=scene)
            _, _, second = invoke(name + "-cpu-repeat", ["--backend", "cpu", "--precision", "native"], scene=scene)
            expected_cpu = rgb(first)
            check(expected_cpu == rgb(second), "Native CPU repeat differs: " + name)
            first_metal = None
            for backend in ("metal", "auto"):
                for repeat in range(2):
                    flags = ["--backend", backend]
                    if native:
                        flags += ["--metal-library", native]
                    result, profile, image = invoke(f"{name}-{backend}-{repeat}", flags, scene=scene)
                    if has_metal:
                        metadata(result, profile, backend, "metal")
                        values = rgb(image)
                        first_metal = values if first_metal is None else first_metal
                        check(values == first_metal, "Metal/auto repeat differs: " + name)
                        check(Path(profile["metal_library"]).resolve() == native, profile)
                    else:
                        metadata(result, profile, backend, "cpu", fallback=True, fallback_codes=("metal_not_built",))
                        check(rgb(image) == expected_cpu, "No-Metal build fallback changed Native CPU RGB")
            if has_metal:
                metal_reference[scene] = first_metal
            if metal_host:
                output = scratch / (name + "-standalone")
                command = [str(metal_host), "--scene", str(scene), "--output", str(output),
                    "--precision", "native", "--library", str(native), "--benchmark-render", "--full-shading",
                    "--width", str(SIZE[0]), "--height", str(SIZE[1]), "--threads", str(WORKERS),
                    "--warmups", "0", "--repeats", "3"]
                result = subprocess.run(command, cwd=scratch, env=environment, text=True,
                                        capture_output=True, timeout=90)
                runs += 1
                check(result.returncode == 0, (command, result.stdout, result.stderr))
                report = json.loads((output / "report.json").read_text())
                check(report["status"] == "COMPLETE" and report["precision"] == "native", report)
                check(rgb(output / "cold.png") == first_metal, "Main CLI differs from standalone Native GPU: " + name)

        # Tricorn exercises the general formula engine, outside the standalone
        # scene host's admitted set. A CPU fallback cannot satisfy this check in
        # a Metal build; the portable build must still reproduce Native CPU.
        general_scene = source / "comparison/scenes/test_06_tricorn.m3p"
        general_stages = {stage: ("completed" if stage in
            ("load", "preparation", "primary", "ao", "color", "final_surface", "save") else "not_run")
            for stage in ("load", "preparation", "volume", "primary", "normals", "shadows",
                          "ao", "color", "reflections", "dof", "final_surface", "save")}

        def general_stage_records(profile, expected_stages=general_stages):
            actual = profile["stages"]
            check(set(actual) == set(expected_stages), ("General stage inventory", profile))
            for stage, expected in expected_stages.items():
                record = actual[stage]
                check(record["state"] == expected and type(record["seconds"]) in (int, float) and
                      math.isfinite(record["seconds"]) and record["seconds"] >= 0,
                      ("General stage omitted or invalid", stage, record))
                if expected == "not_run":
                    check(record["seconds"] == 0, ("Inactive general stage reported work", stage, record))

        general_cpu = None
        for repeat in range(2):
            result, profile, image = invoke(f"general-tricorn-cpu-{repeat}",
                ["--backend", "cpu", "--precision", "native"], scene=general_scene)
            metadata(result, profile, "cpu", "cpu")
            general_stage_records(profile)
            values = rgb(image)
            general_cpu = values if general_cpu is None else general_cpu
            check(values == general_cpu, "General Native CPU RGB repeat changed")
        general_metal = None
        general_work = None
        for backend in ("metal", "auto"):
            for repeat in range(3):
                flags = ["--backend", backend, "--precision", "native"]
                if native:
                    flags += ["--metal-library", native]
                result, profile, image = invoke(f"general-tricorn-{backend}-{repeat}", flags,
                                                scene=general_scene)
                general_stage_records(profile)
                values = rgb(image)
                if has_metal:
                    metadata(result, profile, backend, "metal")
                    check(Path(profile["metal_library"]).resolve() == native, profile)
                    execution = profile["metal_execution"]
                    check(execution["engine"] == "general_formula" and
                          execution["distance_calls"] > 0 and execution["formula_work"] > 0,
                          ("Tricorn did not execute GPU formulas", execution))
                    observed_work = execution_work(execution)
                    general_work = observed_work if general_work is None else general_work
                    check(observed_work == general_work, "General GPU work changed between Metal/auto repeats")
                    general_metal = values if general_metal is None else general_metal
                    check(values == general_metal, "General Metal/auto RGB repeat changed")
                else:
                    metadata(result, profile, backend, "cpu", fallback=True,
                             fallback_codes=("metal_not_built",))
                    check(values == general_cpu, "No-Metal general fallback changed Native CPU RGB")
                check(len({values[i:i + 3] for i in range(0, len(values), 3)}) > 1,
                      "General scene produced a constant image")

        # Active ordinary DOF uses the general GPU renderer. Exercise the real
        # CLI route and actual blur work, independently of the no-DOF override.
        dof_scene = scene_root / "integerpower8-dof.m3p"
        dof_bytes = dof_scene.read_bytes()
        check(struct.unpack_from("<i", dof_bytes)[0] == 44 and dof_bytes[181] == 1,
              "Expected one-pass sorted ordinary DOF fixture")
        dof_stages = {**general_stages, "shadows": "completed", "dof": "completed"}
        dof_images = {}
        dof_activity = None
        for backend in ("cpu", "metal", "auto"):
            first = None
            for repeat in range(2 if backend == "cpu" else 3):
                flags = ["--backend", backend, "--precision", "native"]
                if backend != "cpu" and native:
                    flags += ["--metal-library", native]
                result, profile, image = invoke(f"dof-active-{backend}-{repeat}", flags, scene=dof_scene)
                effective = "metal" if has_metal and backend != "cpu" else "cpu"
                fallback = backend != "cpu" and not has_metal
                metadata(result, profile, backend, effective, fallback=fallback,
                         fallback_codes=("metal_not_built",) if fallback else ())
                general_stage_records(profile, dof_stages)
                check(profile["stages"]["dof"]["seconds"] > 0, "Active DOF has no measured stage")
                values = rgb(image)
                first = values if first is None else first
                check(values == first, "Active DOF RGB changed within backend: " + backend)
                check(len({values[i:i + 3] for i in range(0, len(values), 3)}) > 1,
                      "Active DOF produced a constant image")
                if effective == "metal":
                    check(Path(profile["metal_library"]).resolve() == native, profile)
                    execution = profile["metal_execution"]
                    check(execution["engine"] == "general_formula" and
                          execution["distance_calls"] > 0 and execution["formula_work"] > 0,
                          ("DOF did not execute the general GPU renderer", execution))
                    pixels = SIZE[0] * SIZE[1]
                    activity = {k: value for k, value in execution.items() if k.startswith("dof_")}
                    check(set(activity) == {"dof_records", "dof_passes", "dof_positive_radius_sources",
                          "dof_sort_work", "dof_candidate_scan_checks", "dof_candidate_checks",
                          "dof_contributing_pairs", "dof_changed_pixels", "dof_maximum_radius", "dof_dispatches"},
                          ("Incomplete DOF activity inventory", activity))
                    check(all(type(value) is int and value > 0 for value in activity.values()) and
                          activity["dof_passes"] == 1 and activity["dof_records"] == pixels and
                          activity["dof_positive_radius_sources"] <= pixels and
                          activity["dof_changed_pixels"] <= pixels and activity["dof_maximum_radius"] <= 1000 and
                          activity["dof_contributing_pairs"] <= activity["dof_candidate_checks"],
                          ("Missing or invalid GPU blur activity", activity))
                    dof_activity = activity if dof_activity is None else dof_activity
                    check(activity == dof_activity, "DOF activity changed across Metal/auto repeats")
                else:
                    check(all(value == 0 for key, value in profile["metal_execution"].items()
                              if key.startswith("dof_")), "CPU DOF reported GPU activity")
            dof_images[backend] = first
        check(dof_images["auto"] == dof_images["metal"], "Auto DOF differs from explicit Metal")
        if not has_metal:
            check(dof_images["metal"] == dof_images["cpu"], "No-Metal DOF fallback changed Native CPU RGB")

        # Disabled effects are admitted independently from their active variants.
        # Each override must match the corresponding file-level off control on
        # the same backend; CPU and Metal are intentionally not compared exactly.
        disabled_effects = (
            ("ao", "--no-ao", "integerpower8-ao-shadows", "integerpower8-ao-shadows-ao-off"),
            ("dof", "--no-dof", "integerpower8-dof", "integerpower8-dof-off"),
            ("reflections", "--no-reflections", "integerpower8-reflections", "integerpower8-reflections-off"),
        )
        for stage, option, active_name, disabled_name in disabled_effects:
            active_scene = scene_root / (active_name + ".m3p")
            disabled_scene = scene_root / (disabled_name + ".m3p")
            for backend in ("cpu", "metal", "auto"):
                flags = ["--backend", backend, "--precision", "native"]
                if backend != "cpu" and native:
                    flags += ["--metal-library", native]
                result, profile, control = invoke("disabled-control-" + stage + "-" + backend, flags, scene=disabled_scene)
                effective = "metal" if has_metal and backend != "cpu" else "cpu"
                fallback = backend != "cpu" and not has_metal
                metadata(result, profile, backend, effective, fallback=fallback,
                         fallback_codes=("metal_not_built",) if fallback else ())
                expected = rgb(control)
                if stage == "dof":
                    check(expected != dof_images[backend], "DOF off control did not change RGB: " + backend)
                for repeat in range(3):
                    result, profile, image = invoke(f"disabled-override-{stage}-{backend}-{repeat}",
                        [*flags, option], scene=active_scene)
                    metadata(result, profile, backend, effective, fallback=fallback,
                             fallback_codes=("metal_not_built",) if fallback else ())
                    check(profile["stages"][stage]["state"] == "not_run", "Disabled effect still ran: " + stage)
                    check(rgb(image) == expected, "Effect override differs from off control: " + stage)
                    if stage == "dof":
                        check(all(value == 0 for key, value in profile["metal_execution"].items()
                                  if key.startswith("dof_")), "Disabled DOF reported GPU blur work")
                    if stage == "reflections" and effective == "metal":
                        check(not profile["metal_reflections"]["enabled"] and profile["metal_reflections"]["rays"] == 0,
                              "Disabled reflection submitted secondary rays")

        # Main-API-only opaque reflection capability. Standalone exact/native
        # qualification remains a different admitted scene set.
        reflection_scene = scene_root / "integerpower8-reflections.m3p"
        reflection_off = scene_root / "integerpower8-reflections-off.m3p"
        reflection_images = {}
        reflection_activity = None
        for backend in ("cpu", "metal", "auto"):
            first = None
            for repeat in range(3):
                flags = ["--backend", backend, "--precision", "native"]
                if backend != "cpu" and native:
                    flags += ["--metal-library", native]
                result, profile, image = invoke(f"reflection-{backend}-{repeat}", flags, scene=reflection_scene)
                effective = "metal" if has_metal and backend != "cpu" else "cpu"
                fallback = not has_metal and backend != "cpu"
                metadata(result, profile, backend, effective, fallback=fallback,
                         fallback_codes=("metal_not_built",) if fallback else ())
                check(profile["stages"]["reflections"]["state"] == "completed" and
                      profile["stages"]["reflections"]["seconds"] > 0, "Reflection stage did not run")
                values = rgb(image)
                first = values if first is None else first
                check(values == first, "Reflection RGB changed within backend")
                if effective == "metal":
                    activity = profile["metal_reflections"]
                    check(activity["enabled"] is True and activity["rays"] > 0 and
                          0 < activity["changed_pixels"] <= activity["active_pixels"] <= SIZE[0] * SIZE[1] and
                          activity["faults"] == 0 and 1 <= activity["max_bounces"] <= 2 and
                          activity["hits"] + activity["misses"] == activity["rays"] and
                          all(activity[k] > 0 for k in ("ray_steps", "ao_rays", "ao_steps", "shadow_rays", "shadow_steps")),
                          "Missing/faulted GPU reflection and secondary-effect activity")
                    counts = {k: v for k, v in activity.items() if k != "max_command_seconds"}
                    reflection_activity = counts if reflection_activity is None else reflection_activity
                    check(counts == reflection_activity, "Reflection GPU activity changed between frames")
            reflection_images[backend] = first
        check(reflection_images["auto"] == reflection_images["metal"], "Auto reflection differs from explicit Metal")
        if has_metal:
            first_no_ao = None
            for repeat in range(3):
                result, profile, image = invoke(f"reflection-image-ao-disabled-{repeat}",
                    ["--backend", "metal", "--precision", "native", "--metal-library", native, "--no-ao"],
                    scene=reflection_scene)
                metadata(result, profile, "metal", "metal")
                activity = profile["metal_reflections"]
                check(profile["stages"]["ao"]["state"] == "not_run" and
                      profile["stages"]["reflections"]["state"] == "completed" and
                      activity["hits"] > 0 and activity["ao_rays"] > 0 and activity["ao_steps"] > 0,
                      "Image AO override disabled required reflection point AO")
                values = rgb(image)
                first_no_ao = values if first_no_ao is None else first_no_ao
                check(values == first_no_ao, "Reflection with image AO disabled is not repeatable")
        if not has_metal:
            check(reflection_images["metal"] == reflection_images["cpu"], "No-Metal reflection fallback changed Native CPU RGB")
        for backend in ("cpu", "metal"):
            flags = ["--backend", backend, "--precision", "native"]
            if backend == "metal" and native:
                flags += ["--metal-library", native]
            result, profile, image = invoke("reflection-off-" + backend, flags, scene=reflection_off)
            fallback = backend == "metal" and not has_metal
            metadata(result, profile, backend, "cpu" if fallback else backend, fallback=fallback,
                     fallback_codes=("metal_not_built",) if fallback else ())
            check(profile["stages"]["reflections"]["state"] == "not_run" and
                  profile["stages"]["reflections"]["seconds"] == 0, "Reflection-off scene ran reflection stage")
            check(rgb(image) != reflection_images[backend], "Reflection effect-off control did not change RGB")
            if backend == "metal" and has_metal:
                activity = profile["metal_reflections"]
                check(activity["enabled"] is False and all(value == 0 for key, value in activity.items()
                      if key != "enabled"), "Reflection-off scene reported GPU reflection work")

        if has_metal:
            # A misleading cwd-relative library must not shadow the executable's
            # installed default. Explicit library overrides are covered below.
            adjacent = renderer.parent / "metal_native_scene.metallib"
            if adjacent.is_file():
                (scratch / "metal_native_scene.metallib").write_bytes(b"invalid cwd library must be ignored")
                for backend in ("metal", "auto"):
                    result, profile, image = invoke("adjacent-library-" + backend, ["--backend", backend])
                    metadata(result, profile, backend, "metal")
                    check(Path(profile["metal_library"]).resolve() == adjacent.resolve(), profile)
                    check(rgb(image) == metal_reference[ordinary], "Default library lookup used current working directory")

            # Profile mismatch is an execution error, never an availability
            # excuse for CPU fallback; preserve existing output on that error.
            corrupt = scratch / "corrupt-existing.metallib"
            corrupt.write_bytes(b"an existing file is not an unavailable Metal backend")
            bad_libraries = [("corrupt", corrupt)]
            if compatible:
                bad_libraries.append(("compatible", compatible))
            for label, library in bad_libraries:
                for backend in ("metal", "auto"):
                    sentinel = b"do not overwrite on wrong GPU library\n"
                    result, profile, _ = invoke("wrong-library-" + label + "-" + backend,
                        ["--backend", backend, "--metal-library", library], code=3, sentinel=sentinel)
                    metadata(result, profile, backend, "metal", code=3)
                    check(bool(profile["error"]) and profile["stages"]["save"]["state"] == "not_run", profile)
                    preparation = profile["stages"]["preparation"]
                    check(preparation["state"] == "interrupted" and
                          type(preparation["seconds"]) in (int, float) and
                          math.isfinite(preparation["seconds"]) and preparation["seconds"] > 0,
                          "Failed Metal library lost interrupted preparation timing")
                    check(profile["stages"]["primary"]["state"] == "not_run" and
                          profile["stages"]["primary"]["seconds"] == 0 and
                          profile["stages"]["save"]["seconds"] == 0,
                          "Failed Metal library invented primary/save work")

            exports = {}
            for backend in ("metal", "auto"):
                paths = {name: scratch / ("export-" + backend + suffix) for name, suffix in
                         (("depth", "-depth.png"), ("normal", "-normal.png"), ("surface", "-surface.bin"))}
                result, profile, image = invoke("exports-" + backend,
                    ["--backend", backend, "--metal-library", native, "--zbuffer", paths["depth"],
                     "--zbuffer-format", "png16", "--normalmap", paths["normal"], "--silight5-dump", paths["surface"]])
                metadata(result, profile, backend, "metal")
                check(paths["surface"].read_bytes().startswith(b"SIL5DMP1"), "Missing/invalid GPU surface export")
                depth_header, _ = png(paths["depth"])
                normal_header, _ = png(paths["normal"])
                check(depth_header[2] == 16 and normal_header[2:4] == (8, 2), "GPU auxiliary export PNG layout")
                exports[backend] = {name: path.read_bytes() for name, path in paths.items()}
                exports[backend]["rgb"] = rgb(image)
            check(exports["metal"] == exports["auto"], "Backend selection changed auxiliary surface exports")

            # Depth conversion uses the effective render header. A 64x48
            # override of the stored 512x384 scene must equal the same scene
            # stored directly at 64x48, including exported PNG16 depth.
            stored = bytearray(ordinary.read_bytes())
            check(len(stored) >= 840 and struct.unpack_from("<i", stored)[0] == 44,
                  "Expected raw version44 dimension-regression fixture")
            struct.pack_into("<ii", stored, 4, *SIZE)
            resized_scene = scratch / "stored-64x48.m3p"
            resized_scene.write_bytes(stored)
            resized_depth = scratch / "stored-64x48-depth.png"
            result, profile, image = invoke("stored-dimensions",
                ["--backend", "metal", "--metal-library", native,
                 "--zbuffer", resized_depth, "--zbuffer-format", "png16"], scene=resized_scene)
            metadata(result, profile, "metal", "metal")
            check(rgb(image) == exports["metal"]["rgb"], "Stored/overridden scene dimensions changed GPU RGB")
            check(resized_depth.read_bytes() == exports["metal"]["depth"],
                  "GPU depth export used stored header dimensions instead of effective dimensions")

            changed = []
            for backend in ("metal", "auto"):
                result, profile, image = invoke("iteration-override-" + backend,
                    ["--backend", backend, "--metal-library", native, "--max-iter", "2"])
                metadata(result, profile, backend, "metal")
                changed.append(rgb(image))
            check(changed[0] == changed[1], "Iteration override differs between Metal and auto")
            check(changed[0] != exports["metal"]["rgb"], "Metal ignored effective maximum-iteration override")

        for path, expected in input_hashes.items():
            check(hashlib.sha256(path.read_bytes()).hexdigest() == expected, "Test modified source fixture: " + str(path))
        print(f"Backend CLI: {checks} checks passed across {runs} sequential processes; "
              f"Metal build={has_metal}. Native CPU/GPU image differences are permitted.")


if __name__ == "__main__":
    main()
