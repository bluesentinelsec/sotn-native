"""Check rendered frames and cleanup in a disposable iOS Simulator.

simctl can hang after launching an app on a cold simulator. Read the
app's opt-in completion report instead; a launch or a frame-only marker cannot
satisfy this check. The simulator and its data container are fresh for each run.
"""

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time

BUNDLE = "org.bluesentinelsec.sotn-native"
COMPLETE = "SOTN_SMOKE_TEST: PASS\nSOTN_SMOKE_TEST: CLEANUP_COMPLETE\n"


def simctl(*args, **kwargs):
    # CoreSimulator service discovery is also cold-start work on hosted runners.
    kwargs.setdefault("timeout", 180)
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


def launch_and_wait(udid, report, env):
    """Require the app's complete report even when simctl does not return."""
    command = ["xcrun", "simctl", "launch", "--console-pty", udid, BUNDLE, "--smoke-test"]
    print("Launching iOS Simulator app and waiting for its rendering/cleanup report", flush=True)
    # A file avoids pipe backpressure while CoreSimulator finishes a cold launch.
    with tempfile.TemporaryFile(mode="w+t") as output:
        process = subprocess.Popen(command, stdout=output, stderr=subprocess.STDOUT, env=env)
        try:
            deadline = time.monotonic() + 180
            last = "Report not created"
            while process.poll() is None:
                try:
                    last = report.read_text()
                except FileNotFoundError:
                    pass
                if last == COMPLETE:
                    status = process.poll()
                    if status not in (None, 0):
                        raise SystemExit(f"iOS Simulator launch failed with exit {status}")
                    print(last, end="", flush=True)
                    return
                if time.monotonic() >= deadline:
                    raise SystemExit(f"iOS launch/report timed out; last report: {last!r}")
                time.sleep(0.25)
            if process.returncode != 0:
                raise SystemExit(f"iOS Simulator launch failed with exit {process.returncode}")
            wait_for_report(report)
        finally:
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait(timeout=5)
            output.seek(0)
            print(output.read(65536), end="", flush=True)


def prepare_display(udid):
    """Require a working graphical frontend after OS/data migration completes."""
    tool = Path(subprocess.check_output(["xcrun", "--find", "simctl"],
                                        text=True, timeout=60).strip())
    if not tool.is_absolute() or len(tool.parents) < 3:
        raise SystemExit("Cannot locate the active Xcode Simulator frontend")
    frontend = tool.parents[2] / "Applications/Simulator.app"
    subprocess.run(["open", "-a", str(frontend), "--args", "-CurrentDeviceUDID", udid],
                   check=True, timeout=60)
    # Compositor readiness is a prerequisite, never evidence that this app rendered.
    with tempfile.TemporaryDirectory(prefix="sotn-ios-display-") as display:
        simctl("io", udid, "screenshot", "--type=png", str(Path(display) / "ready.png"),
               timeout=60)


def select_runtime(runtimes, sdk_version):
    identifier = "com.apple.CoreSimulator.SimRuntime.iOS-" + sdk_version.strip().replace(".", "-")
    runtime = next((r for r in runtimes if r.get("isAvailable") and
                    r["identifier"] == identifier), None)
    if runtime is None:
        raise SystemExit(f"Install the iOS {sdk_version.strip()} Simulator runtime for the active Xcode SDK")
    return runtime


def run_smoke(app):
    runtimes = json.loads(simctl("list", "runtimes", "-j"))["runtimes"]
    sdk = subprocess.check_output(["xcrun", "--sdk", "iphonesimulator", "--show-sdk-version"],
                                  text=True, timeout=60).strip()
    runtime = select_runtime(runtimes, sdk)
    print(f"Testing iOS SDK {sdk} with {runtime['identifier']}", flush=True)
    types = runtime.get("supportedDeviceTypes") or json.loads(
        simctl("list", "devicetypes", "-j"))["devicetypes"]
    device = next(d for d in types if d["name"].startswith("iPhone"))
    udid = simctl("create", "SOTN Native smoke test", device["identifier"], runtime["identifier"]).strip()
    try:
        simctl("boot", udid)
        simctl("bootstatus", udid, "-bd", timeout=300)
        prepare_display(udid)
        simctl("install", udid, str(app), timeout=240)
        container = Path(simctl("get_app_container", udid, BUNDLE, "data").strip())
        report = container / "Library/Application Support/bluesentinelsec/sotn-native/smoke-result.txt"
        if report.exists():
            raise SystemExit("Fresh simulator unexpectedly contains a stale smoke report")
        env = os.environ.copy()
        env["SIMCTL_CHILD_SOTN_SMOKE_REPORT"] = "1"
        env["SIMCTL_CHILD_SDL_AUDIO_DRIVER"] = "dummy"
        try:
            launch_and_wait(udid, report, env)
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
