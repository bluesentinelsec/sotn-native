"""Install the bootstrap APK and require its native rendering smoke test to pass."""

import subprocess
import sys
import time

subprocess.run(["adb", "install", "-r", sys.argv[1]], check=True)
# Cold-boot setup can replace system resource overlays and recreate SDLActivity.
# Drain boot/package broadcasts before starting the rendering test.
subprocess.run([
    "adb", "shell", "am", "wait-for-broadcast-idle", "--flush-broadcast-loopers",
], check=True, timeout=60)
subprocess.run(["adb", "logcat", "-c"], check=True)
subprocess.run([
    "adb", "shell", "am", "start", "-W", "-n",
    "org.bluesentinelsec.sotn/.MainActivity", "--ez", "smoke_test", "true",
], check=True)
deadline = time.monotonic() + 60
while time.monotonic() < deadline:
    output = subprocess.check_output(["adb", "logcat", "-d"], text=True)
    if "SOTN_SMOKE_TEST: PASS" in output:
        print("Android rendering smoke test passed.")
        break
    if "FATAL EXCEPTION" in output or "Fatal signal" in output:
        raise SystemExit(output)
    time.sleep(1)
else:
    raise SystemExit("Android smoke test timed out:\n" + output)
