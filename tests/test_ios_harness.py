"""A launch, missing report, or frames without cleanup cannot pass iOS CI."""

import importlib.util
from pathlib import Path
import tempfile
import subprocess
import unittest
from unittest.mock import Mock, patch

spec = importlib.util.spec_from_file_location(
    "ios_harness", Path(__file__).resolve().parents[1] / "scripts/test_ios.py")
harness = importlib.util.module_from_spec(spec)
spec.loader.exec_module(harness)


SIMCTL = Path(tempfile.gettempdir()) / "active-Xcode.app/Contents/Developer/usr/bin/simctl"


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


class IOSDeviceTest(unittest.TestCase):
    def test_compact_phone_must_be_in_selected_runtime_supported_types(self):
        devices = [{"name": "iPhone 16 Pro", "identifier": "modern"},
                   {"name": "iPhone SE (3rd generation)", "identifier": "compact"}]
        self.assertEqual(harness.select_device(devices), devices[1])

    def test_runtime_without_compact_model_uses_its_own_supported_phone(self):
        devices = [{"name": "iPad Pro", "identifier": "tablet"},
                   {"name": "iPhone 17", "identifier": "modern"}]
        self.assertEqual(harness.select_device(devices), devices[1])

    def test_missing_supported_phone_fails(self):
        with self.assertRaisesRegex(SystemExit, "iPhone device type"):
            harness.select_device([{"name": "iPad Pro", "identifier": "tablet"}])


class IOSDisplayTest(unittest.TestCase):
    def test_graphical_frontend_and_compositor_are_required(self):
        with patch.object(harness.subprocess, "run") as frontend, \
             patch.object(harness.subprocess, "check_output",
                          return_value=str(SIMCTL) + "\n"), \
             patch.object(harness, "simctl") as compositor:
            harness.prepare_display("fresh-device")
        frontend.assert_called_once_with(
            ["open", "-a", str(SIMCTL.parents[2] / "Applications/Simulator.app"),
             "--args", "-CurrentDeviceUDID", "fresh-device"],
            check=True, timeout=60)
        args = compositor.call_args.args
        self.assertEqual(args[:4], ("io", "fresh-device", "screenshot", "--type=png"))
        self.assertEqual(Path(args[4]).name, "ready.png")
        self.assertEqual(compositor.call_args.kwargs, {"timeout": 180})

    def test_frontend_failure_cannot_be_ignored(self):
        with patch.object(harness.subprocess, "run",
                          side_effect=subprocess.CalledProcessError(1, "open")), \
             patch.object(harness.subprocess, "check_output",
                          return_value=str(SIMCTL) + "\n"), \
             patch.object(harness, "simctl") as compositor:
            with self.assertRaises(subprocess.CalledProcessError):
                harness.prepare_display("fresh-device")
        compositor.assert_not_called()

    def test_compositor_timeout_stays_a_failure(self):
        with patch.object(harness.subprocess, "run"), \
             patch.object(harness.subprocess, "check_output",
                          return_value=str(SIMCTL) + "\n"), \
             patch.object(harness, "simctl",
                          side_effect=subprocess.TimeoutExpired("screenshot", 180)):
            with self.assertRaises(subprocess.TimeoutExpired):
                harness.prepare_display("fresh-device")


class IOSLaunchTest(unittest.TestCase):
    def test_hung_launcher_requires_completed_app_and_reaps_process(self):
        with tempfile.TemporaryDirectory() as directory:
            report = Path(directory) / "result.txt"
            report.write_text(harness.COMPLETE)
            process = Mock(returncode=None)
            process.poll.return_value = None
            with patch.object(harness.subprocess, "Popen", return_value=process), \
                 patch.object(harness.time, "monotonic", return_value=0):
                harness.launch_and_wait("fresh-device", report, {})
            process.terminate.assert_called_once()
            process.wait.assert_called_once_with(timeout=5)
            process.kill.assert_not_called()

    def test_launcher_captures_application_output(self):
        with tempfile.TemporaryDirectory() as directory:
            report = Path(directory) / "result.txt"
            report.write_text(harness.COMPLETE)
            process = Mock(returncode=None)
            process.poll.return_value = None
            with patch.object(harness.subprocess, "Popen", return_value=process) as launch:
                harness.launch_and_wait("fresh-device", report, {})
            self.assertEqual(launch.call_args.args[0],
                             ["xcrun", "simctl", "launch", "--console-pty", "fresh-device",
                              harness.BUNDLE, "--smoke-test"])

    def test_frames_without_cleanup_cannot_rescue_hung_launch(self):
        with tempfile.TemporaryDirectory() as directory:
            report = Path(directory) / "result.txt"
            report.write_text("SOTN_SMOKE_TEST: PASS\n")
            process = Mock(returncode=None)
            process.poll.return_value = None
            with patch.object(harness.subprocess, "Popen", return_value=process), \
                 patch.object(harness.time, "monotonic", side_effect=[0, 0, 181]), \
                 patch.object(harness.time, "sleep"):
                with self.assertRaisesRegex(SystemExit, "launch/report timed out"):
                    harness.launch_and_wait("fresh-device", report, {})
            process.terminate.assert_called_once()

    def test_failed_launch_is_not_hidden_by_a_report(self):
        with tempfile.TemporaryDirectory() as directory:
            report = Path(directory) / "result.txt"
            report.write_text(harness.COMPLETE)
            process = Mock(returncode=7)
            process.poll.return_value = 7
            with patch.object(harness.subprocess, "Popen", return_value=process):
                with self.assertRaisesRegex(SystemExit, "launch failed with exit 7"):
                    harness.launch_and_wait("fresh-device", report, {})
            process.terminate.assert_not_called()

    def test_successful_launch_still_waits_for_complete_report(self):
        process = Mock(returncode=0)
        process.poll.return_value = 0
        report = Path("not-a-real-report")
        with patch.object(harness.subprocess, "Popen", return_value=process), \
             patch.object(harness, "wait_for_report", side_effect=SystemExit("missing report")) as wait:
            with self.assertRaisesRegex(SystemExit, "missing report"):
                harness.launch_and_wait("fresh-device", report, {})
        wait.assert_called_once_with(report)

    def test_hung_launcher_kill_fallback_preserves_verified_result(self):
        with tempfile.TemporaryDirectory() as directory:
            report = Path(directory) / "result.txt"
            report.write_text(harness.COMPLETE)
            process = Mock(returncode=None)
            process.poll.return_value = None
            process.wait.side_effect = [subprocess.TimeoutExpired("simctl", 5), 0]
            with patch.object(harness.subprocess, "Popen", return_value=process):
                harness.launch_and_wait("fresh-device", report, {})
            process.kill.assert_called_once()

    def test_launch_failure_during_report_read_stays_a_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            report = Path(directory) / "result.txt"
            report.write_text(harness.COMPLETE)
            process = Mock(returncode=7)
            process.poll.side_effect = [None, 7, 7]
            with patch.object(harness.subprocess, "Popen", return_value=process):
                with self.assertRaisesRegex(SystemExit, "launch failed with exit 7"):
                    harness.launch_and_wait("fresh-device", report, {})
