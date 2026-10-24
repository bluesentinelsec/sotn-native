"""A launch, missing report, or frames without cleanup cannot pass iOS CI."""

import importlib.util
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location(
    "ios_harness", Path(__file__).resolve().parents[1] / "scripts/test_ios.py")
harness = importlib.util.module_from_spec(spec)
spec.loader.exec_module(harness)


class IOSReportTest(unittest.TestCase):
    def test_runtime_matches_active_sdk_instead_of_newest_installed(self):
        runtimes = [{"identifier": f"com.apple.CoreSimulator.SimRuntime.iOS-{version}",
                     "isAvailable": True} for version in ("18-5", "18-6", "26-2")]
        self.assertEqual(harness.select_runtime(runtimes, "18.5"), runtimes[0])
        self.assertEqual(harness.select_runtime(runtimes, "26.2"), runtimes[2])
        with self.assertRaisesRegex(SystemExit, "18.4 Simulator runtime"):
            harness.select_runtime(runtimes, "18.4")
        runtimes[0]["isAvailable"] = False
        with self.assertRaisesRegex(SystemExit, "18.5 Simulator runtime"):
            harness.select_runtime(runtimes, "18.5")

    def test_frames_without_cleanup_do_not_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            report = Path(directory) / "result.txt"
            report.write_text("SOTN_SMOKE_TEST: PASS\n")
            with patch.object(harness.time, "monotonic", side_effect=[0, 0, 91]), \
                 patch.object(harness.time, "sleep"):
                with self.assertRaisesRegex(SystemExit, "rendering/cleanup.*timed out"):
                    harness.wait_for_report(report)

    def test_missing_report_does_not_pass(self):
        with tempfile.TemporaryDirectory() as directory:
            with patch.object(harness.time, "monotonic", side_effect=[0, 0, 91]), \
                 patch.object(harness.time, "sleep"):
                with self.assertRaisesRegex(SystemExit, "Report not created"):
                    harness.wait_for_report(Path(directory) / "missing.txt")

    def test_complete_report_passes(self):
        with tempfile.TemporaryDirectory() as directory:
            report = Path(directory) / "result.txt"
            report.write_text(harness.COMPLETE)
            with patch.object(harness.time, "monotonic", return_value=0), \
                 patch.object(harness.time, "sleep") as sleep:
                harness.wait_for_report(report)
            sleep.assert_not_called()
