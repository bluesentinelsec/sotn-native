"""Check rendered frames and cleanup in a disposable iOS Simulator.

simctl's console attachment can hang after launch on a cold simulator. Read the
app's opt-in completion report instead; a launch or a frame-only marker cannot
satisfy this check. The simulator and its data container are fresh for each run.
"""

import json
import os
from pathlib import Path
import subprocess
import sys
import time

BUNDLE = "org.bluesentinelsec.sotn-native"
COMPLETE = "SOTN_SMOKE_TEST: PASS\nSOTN_SMOKE_TEST: CLEANUP_COMPLETE\n"


def simctl(*args, **kwargs):
    kwargs.setdefault("timeout", 60)
    print(f"simctl {' '.join(args)}", flush=True)
    try:
        output = subprocess.check_output(["xcrun", "simctl", *args], text=True, **kwargs)
    except subprocess.TimeoutExpired as error:
        for partial in (error.stdout, error.stderr):
            if partial:
                print(partial.decode(errors="replace") if isinstance(partial, bytes) else partial,
                      flush=True)
        raise
    print(output, end="", flush=True)
    return output


def wait_for_report(report, timeout=90):
    deadline = time.monotonic() + timeout
    last = "Report not created"
    while time.monotonic() < deadline:
        try:
            last = report.read_text()
        except FileNotFoundError:
            pass
        if last == COMPLETE:
            print(last, end="", flush=True)
            return
        time.sleep(0.25)
    raise SystemExit(f"iOS rendering/cleanup smoke test timed out; last report: {last!r}")


def run_smoke(app):
    runtimes = json.loads(simctl("list", "runtimes", "-j"))["runtimes"]
    runtime = next((r for r in reversed(runtimes) if r.get("isAvailable") and
                    r["identifier"].startswith("com.apple.CoreSimulator.SimRuntime.iOS-")), None)
    if runtime is None:
        raise SystemExit("Install an iOS Simulator runtime in Xcode first")
    types = runtime.get("supportedDeviceTypes") or json.loads(
        simctl("list", "devicetypes", "-j"))["devicetypes"]
    device = next(d for d in types if d["name"].startswith("iPhone"))
    udid = simctl("create", "SOTN Native smoke test", device["identifier"], runtime["identifier"]).strip()
    try:
        simctl("boot", udid)
        simctl("bootstatus", udid, "-bd", timeout=300)
        simctl("install", udid, str(app), timeout=240)
        container = Path(simctl("get_app_container", udid, BUNDLE, "data").strip())
        report = container / "Library/Application Support/bluesentinelsec/sotn-native/smoke-result.txt"
        if report.exists():
            raise SystemExit("Fresh simulator unexpectedly contains a stale smoke report")
        env = os.environ.copy()
        env["SIMCTL_CHILD_SOTN_SMOKE_REPORT"] = "1"
        simctl("launch", udid, BUNDLE, "--smoke-test", env=env, timeout=180)
        try:
            wait_for_report(report)
        except SystemExit:
            subprocess.run(["xcrun", "simctl", "spawn", udid, "log", "show", "--last", "2m",
                            "--style", "compact", "--predicate", 'process == "sotn-native"'],
                           check=False, timeout=30)
            raise
    finally:
        for operation in ("shutdown", "delete"):
            try:
                subprocess.run(["xcrun", "simctl", operation, udid], check=False, timeout=60)
            except subprocess.TimeoutExpired:
                print(f"Warning: simulator {operation} timed out for {udid}", file=sys.stderr)


if __name__ == "__main__":
    app = Path(sys.argv[1]).resolve()
    if not app.is_dir():
        raise SystemExit(f"App bundle not found: {app}")
    run_smoke(app)
