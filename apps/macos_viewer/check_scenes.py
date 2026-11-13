#!/usr/bin/env python3
"""Guarded corpus preview audit against a frozen viewer and prepared inputs.

This tests the private service, not mouse events or full-size qualification.
The input inventory is pinned before dispatch; MC/JIT entries are never sent.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import select
import struct
import subprocess
import time

from PIL import Image


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + "\n")


class Service:
    def __init__(self, exe, folder, lane):
        self.exe, self.folder, self.lane = exe, folder, lane
        self.job = None
        self.generation = 0
        self.request_serial = 0

    def start(self):
        if self.job is not None and self.job.poll() is None:
            return
        self.close()
        self.generation += 1
        self.log = (self.folder / f"{self.lane}-{self.generation}.log").open("wb")
        self.job = subprocess.Popen([str(self.exe), "--service"], stdin=subprocess.PIPE,
                                    stdout=subprocess.PIPE, stderr=self.log, bufsize=0)

    def close(self):
        if self.job is None:
            return
        if self.job.poll() is None:
            self.job.kill()
        self.job.wait(timeout=5)
        self.job.stdin.close()
        self.job.stdout.close()
        self.log.close()
        self.job = None

    def run(self, request, label, destination, deadline):
        self.start()
        self.request_serial += 1
        request = dict(request)
        if request.get('protocol') == 1:
            request['request_id'] = self.request_serial
        started = time.monotonic()
        next_notice = started + 30
        diagnostics = []
        def read(size):
            nonlocal next_notice
            result = bytearray()
            while len(result) < size:
                now = time.monotonic()
                if now >= deadline:
                    raise TimeoutError("180 s diagnostic guard (no larger repeat)")
                if now >= next_notice:
                    print(f"WAIT {label}: {now-started:.0f} s, pid {self.job.pid}", flush=True)
                    next_notice = now + 30
                    if not diagnostics:
                        diagnostics.append(subprocess.Popen(
                            ["/usr/bin/sample", str(self.job.pid), "1", "-file", str(destination / "sample.txt")],
                            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL))
                        with (destination / "compiler.log").open("wb") as log:
                            diagnostics.append(subprocess.Popen(
                                ["/usr/bin/log", "show", "--last", "1m", "--style", "compact", "--predicate",
                                 'process == "MTLCompilerService"'], stdout=log, stderr=subprocess.STDOUT))
                if not select.select([self.job.stdout], [], [], min(3, deadline-now))[0]:
                    continue
                part = os.read(self.job.stdout.fileno(), size-len(result))
                if not part:
                    raise EOFError(f"Service closed transport, exit={self.job.poll()}")
                result.extend(part)
            return bytes(result)
        record = {"label": label, "request": {k: request[k] for k in ("mode", "edge", "phase")},
                  "lane": self.lane, "service_generation": self.generation, "pid": self.job.pid}
        rgb = b""
        try:
            data = json.dumps(request).encode()
            packet = memoryview(struct.pack("<III", 0x4d425633, len(data), 0) + data)
            while packet:
                n = os.write(self.job.stdin.fileno(), packet)
                if n <= 0:
                    raise EOFError("Service request write failed")
                packet = packet[n:]
            magic, n, m = struct.unpack("<III", read(12))
            if magic != 0x4d425633 or n > 1048576 or m > 1600*1600*3:
                raise ValueError("Invalid service packet")
            info = json.loads(read(n))
            rgb = read(m)
            record.update(info)
            if info["status"] == "completed":
                if (info["backend"] != request.get("backend", "metal") or info["fallback"] or info["faults"] or
                        len(rgb) != info["width"]*info["height"]*3 or info["pid"] != self.job.pid):
                    raise ValueError("Invalid success / backend fallback / faulty or incomplete RGB")
                record["rgb_sha256"] = hashlib.sha256(rgb).hexdigest()
                hits, misses = info["archive_hits"], info["archive_misses"]
                record["pipeline_archive"] = ("misses (compile-inclusive)" if misses else
                                              "hits" if hits else "not reported / in-memory")
            elif rgb:
                raise ValueError("Failed frame published an image")
        except (OSError, EOFError, ValueError, TimeoutError) as error:
            record.update(status="guarded" if isinstance(error, TimeoutError) else "transport_failed", error=str(error))
            rgb = b""
            self.close()
        finally:
            for job in diagnostics:
                try:
                    job.wait(timeout=2)
                except subprocess.TimeoutExpired:
                    job.kill()
                    job.wait(timeout=2)
        record["wall_seconds"] = time.monotonic()-started
        return record, rgb


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--package", type=Path, required=True,
                   help="Prepared package containing final app, inventory, requests and frozen inputs")
    a = p.parse_args()
    package = a.package.resolve()
    inventory = json.loads((package / "inventory.json").read_text())
    exe = package / "final/Mandelbulb Viewer.app/Contents/MacOS/MandelbulbViewer"
    output = package / "audit"
    output.mkdir(exist_ok=False)
    preview, full = (Service(exe, output, lane) for lane in ("preview", "full"))
    report = {"qualification": False, "source_commit": subprocess.check_output(
        ["git", "rev-parse", "HEAD"], text=True).strip(), "exe_sha256": hashlib.sha256(exe.read_bytes()).hexdigest(),
        "exclusions": inventory["excluded"], "scope": "default Points -> accumulation -> Full, 64 preflight then 640 long edge",
        "limits": "180 s parent diagnostic stop per frame/whole accumulation; service also has 300 s alarm",
        "timing": "wall time includes compilation on archive misses; not a pure GPU benchmark",
        "rows": []}
    # Preserve the queue's deferred tail. Do not promote or rerun a slow full frame.
    rows = sorted(enumerate(inventory["ordinary"]), key=lambda pair:
                  (pair[1]["priority"] == "deferred_slow_tail", pair[0]))
    try:
        for number, (_, row) in enumerate(rows, 1):
            scene = package / "inputs" / Path(row["scene"]).name
            if hashlib.sha256(scene.read_bytes()).hexdigest() != row["sha256"]:
                raise ValueError("Frozen scene changed: " + str(scene))
            request = json.loads((package / "requests" / (row["id"] + ".json")).read_text())
            record = dict(row, stages=[], status="running")
            report["rows"].append(record)
            folder = output / row["id"]
            folder.mkdir()
            print(f"START {number}/77 rank {row['rank']}: {scene.name} (UI {'yes' if row['ui_supported'] else 'rejects projection'})", flush=True)
            def run(label, mode, edge, phase=0, deadline=None):
                dest = folder / label
                dest.mkdir()
                service = full if mode == "full" else preview
                result, rgb = service.run(dict(request, mode=mode, edge=edge, phase=phase),
                    f"{row['rank']} {label}", dest, deadline or time.monotonic()+180)
                if rgb:
                    image = Image.frombytes("RGB", (result["width"], result["height"]), rgb)
                    image.save(dest / "image.png")
                    result["image"] = str((dest / "image.png").relative_to(package))
                    result["extrema"] = image.getextrema()
                record["stages"].append(result)
                save(folder / "case.json", record)
                save(output / "report.json", report)
                print(f"  {label}: {result['status']} {result['wall_seconds']:.3f}s " +
                      result.get("error", result.get("preview_mode", "")), flush=True)
                return result
            fast = run("preflight-points", "points", 64)
            # Even when fast mode fails, the full-shading diagnostic is useful:
            # it differentiates a preview-specific fault from backend rejection.
            shaded = run("preflight-full", "full", 64)
            safe = all(x["status"] == "completed" and x["wall_seconds"] < 180 for x in (fast, shaded))
            if safe:
                accumulation_start = time.monotonic()
                fast = run("points-00", "points", 640, deadline=accumulation_start+180)
                if fast["status"] == "completed":
                    phases, covered, samples = fast["phases"], fast["covered"], fast["samples"]
                    for phase in range(1, phases):
                        current = run(f"points-{phase:02d}", "points", 640, phase, accumulation_start+180)
                        if current["status"] != "completed":
                            fast = current
                            break
                        if current["phase"] != phase or current["covered"] <= covered:
                            raise ValueError("Accumulation did not advance")
                        covered = current["covered"]
                        samples += current["samples"]
                        fast = current
                    record["accumulation_wall_seconds"] = time.monotonic()-accumulation_start
                    if phases > 1 and fast["status"] == "completed":
                        assert covered == samples == fast["width"]*fast["height"], "Incomplete point coverage"
                    if fast["status"] == "completed":
                        shaded = run("full-after-preview", "full", 640)
                record["status"] = "service_chain_completed" if fast["status"] == shaded["status"] == "completed" else "viewer_size_failed"
            else:
                record["status"] = "preflight_failed_no_large_repeat"
            record["ui_chain_completed"] = record["status"] == "service_chain_completed" and row["ui_supported"]
            save(folder / "case.json", record)
            save(output / "report.json", report)
            print(f"DONE {number}/77: {record['status']}", flush=True)
    finally:
        preview.close()
        full.close()
        save(output / "report.json", report)


if __name__ == "__main__":
    main()
