"""Run the actual app in a disposable iOS Simulator and check its result."""

import json
import os
from pathlib import Path
import subprocess
import sys


def simctl(*args, **kwargs):
    return subprocess.check_output(["xcrun", "simctl", *args], text=True, **kwargs)


app = Path(sys.argv[1]).resolve()
if not app.is_dir():
    raise SystemExit(f"App bundle not found: {app}")
runtimes = json.loads(simctl("list", "runtimes", "-j"))["runtimes"]
runtime = next((r for r in reversed(runtimes) if r.get("isAvailable") and
                r["identifier"].startswith("com.apple.CoreSimulator.SimRuntime.iOS-")), None)
if runtime is None:
    raise SystemExit("Install an iOS Simulator runtime in Xcode first")
types = runtime.get("supportedDeviceTypes") or json.loads(simctl("list", "devicetypes", "-j"))["devicetypes"]
device = next(d for d in types if d["name"].startswith("iPhone"))
udid = simctl("create", "SOTN Native smoke test", device["identifier"], runtime["identifier"]).strip()
try:
    simctl("boot", udid)
    simctl("bootstatus", udid, "-b", timeout=240)
    simctl("install", udid, str(app))
    environment = dict(os.environ, SIMCTL_CHILD_SOTN_SMOKE_TEST="1")
    result = subprocess.run([
        "xcrun", "simctl", "launch", "--console", udid, "org.bluesentinelsec.sotn-native",
    ], env=environment, text=True, capture_output=True, timeout=90)
    output = result.stdout + result.stderr
    print(output)
    if result.returncode != 0 or "SOTN_SMOKE_TEST: PASS" not in output:
        raise SystemExit("iOS rendering smoke test failed")
finally:
    subprocess.run(["xcrun", "simctl", "shutdown", udid], check=False)
    subprocess.run(["xcrun", "simctl", "delete", udid], check=False)
