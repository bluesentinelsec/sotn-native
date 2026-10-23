"""Install the bootstrap APK and require its native rendering smoke test to pass."""

import argparse
import subprocess
import time

def wait_for_aosp_theme():
    """Wait for the CI emulator's asynchronous initial resource-overlay transaction."""
    required = {f"com.android.systemui:{name}" for name in ("neutral", "accent", "dynamic")}
    deadline = time.monotonic() + 60
    output = ""
    while time.monotonic() < deadline:
        output = subprocess.check_output([
            "adb", "shell", "cmd", "overlay", "list", "--user", "0", "android",
        ], text=True, timeout=10)
        enabled = {line.strip()[4:] for line in output.splitlines()
                   if line.strip().startswith("[x] ")}
        if required <= enabled:
            return
        time.sleep(1)
    raise SystemExit("Android emulator theme setup timed out:\n" + output)


def run_smoke(apk, wait_for_theme=False):
    """Install and launch only after the requested readiness checks succeed."""
    subprocess.run(["adb", "install", "-r", apk], check=True)
    # Theme generation runs asynchronously, outside the broadcast queues. Once its
    # overlays are enabled, drain their package notifications before app launch.
    if wait_for_theme:
        # The disposable AOSP image can still be in SDK setup even after its
        # boot-completed property is set. Theme application waits for user setup.
        for command in [
            ["settings", "put", "global", "device_provisioned", "1"],
            ["settings", "put", "secure", "user_setup_complete", "1"],
            ["am", "start", "-W", "-a", "android.intent.action.MAIN", "-c", "android.intent.category.HOME"],
        ]:
            subprocess.run(["adb", "shell", *command], check=True, timeout=30)
        wait_for_aosp_theme()
    subprocess.run([
        "adb", "shell", "am", "wait-for-broadcast-idle", "--flush-broadcast-loopers",
    ], check=True, timeout=60)
    subprocess.run(["adb", "logcat", "-c"], check=True)
    subprocess.run([
        "adb", "shell", "am", "start", "-W", "-n",
        "org.bluesentinelsec.sotn/.MainActivity", "--ez", "smoke_test", "true",
    ], check=True)
    deadline = time.monotonic() + 60
    output = ""
    while time.monotonic() < deadline:
        output = subprocess.check_output(["adb", "logcat", "-d"], text=True, timeout=10)
        if "SOTN_SMOKE_TEST: PASS" in output:
            print("Android rendering smoke test passed.")
            return
        if "FATAL EXCEPTION" in output or "Fatal signal" in output:
            raise SystemExit(output)
        time.sleep(1)
    raise SystemExit("Android smoke test timed out:\n" + output)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("apk")
    parser.add_argument("--wait-for-aosp-theme", action="store_true",
                        help="require the CI AOSP emulator's initial dynamic-color overlays")
    arguments = parser.parse_args()
    run_smoke(arguments.apk, arguments.wait_for_aosp_theme)
