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
    return subprocess.check_output(["xcrun", "simctl", *args], text=True, **kwargs)


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
        simctl("bootstatus", udid, "-b", timeout=240)
        simctl("install", udid, str(app))
        container = Path(simctl("get_app_container", udid, BUNDLE, "data").strip())
        report = container / "Library/Application Support/bluesentinelsec/sotn-native/smoke-result.txt"
        if report.exists():
            raise SystemExit("Fresh simulator unexpectedly contains a stale smoke report")
        env = os.environ.copy()
        env["SIMCTL_CHILD_SOTN_SMOKE_REPORT"] = "1"
        print(simctl("launch", udid, BUNDLE, "--smoke-test", env=env), end="", flush=True)
        try:
            wait_for_report(report)
        except SystemExit:
            subprocess.run(["xcrun", "simctl", "spawn", udid, "log", "show", "--last", "2m",
                            "--style", "compact", "--predicate", 'process == "sotn-native"'],
                           check=False, timeout=30)
            raise
    finally:
        subprocess.run(["xcrun", "simctl", "shutdown", udid], check=False, timeout=60)
        subprocess.run(["xcrun", "simctl", "delete", udid], check=False, timeout=60)


if __name__ == "__main__":
    app = Path(sys.argv[1]).resolve()
    if not app.is_dir():
        raise SystemExit(f"App bundle not found: {app}")
    run_smoke(app)
