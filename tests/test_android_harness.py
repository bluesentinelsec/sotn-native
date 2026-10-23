"""Check that incomplete emulator setup cannot be mistaken for an app smoke pass."""

import importlib.util
from pathlib import Path
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location(
    "android_harness", Path(__file__).resolve().parents[1] / "scripts/test_android.py")
harness = importlib.util.module_from_spec(spec)
spec.loader.exec_module(harness)

READY = "\n".join(f"[x] com.android.systemui:{name}"
                  for name in ("neutral", "accent", "dynamic"))


class AndroidReadinessTest(unittest.TestCase):
    def test_disabled_overlay_does_not_satisfy_readiness(self):
        pending = READY.replace("[x] com.android.systemui:dynamic", "[ ] com.android.systemui:dynamic")
        with patch.object(harness.subprocess, "check_output", side_effect=[pending, READY]) as read, \
             patch.object(harness.time, "monotonic", return_value=0), \
             patch.object(harness.time, "sleep") as sleep:
            harness.wait_for_aosp_theme()
        self.assertEqual(read.call_count, 2)
        sleep.assert_called_once_with(1)

    def test_timeout_fails_with_setup_diagnostics(self):
        with patch.object(harness.subprocess, "check_output", return_value="[ ] pending"), \
             patch.object(harness.time, "monotonic", side_effect=[0, 0, 61]), \
             patch.object(harness.time, "sleep"):
            with self.assertRaisesRegex(SystemExit, "theme setup timed out.*"):
                harness.wait_for_aosp_theme()

    def test_failed_setup_prevents_app_launch(self):
        with patch.object(harness, "wait_for_aosp_theme", side_effect=SystemExit("setup failed")), \
             patch.object(harness.subprocess, "run") as run:
            with self.assertRaisesRegex(SystemExit, "setup failed"):
                harness.run_smoke("synthetic.apk", wait_for_theme=True)
        self.assertEqual(run.call_count, 1)
        self.assertEqual(run.call_args.args[0], ["adb", "install", "-r", "synthetic.apk"])
