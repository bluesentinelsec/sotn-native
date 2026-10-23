"""Use original synthetic bytes to test private reference validation."""

import contextlib
import hashlib
import importlib.util
import io
from pathlib import Path
import tempfile
import unittest


script = Path(__file__).resolve().parents[1] / "scripts/verify_reference.py"
spec = importlib.util.spec_from_file_location("reference", script)
reference = importlib.util.module_from_spec(spec)
spec.loader.exec_module(reference)


class ReferenceTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.content = b"original synthetic test data\n" * 20000
        self.expected = {"BIN/test.dat": hashlib.sha1(self.content).hexdigest()}
        (self.root / "BIN").mkdir()
        (self.root / "BIN/test.dat").write_bytes(self.content)

    def test_matching_files_are_read_without_modification_or_output(self):
        output = io.StringIO()
        before = sorted(path.relative_to(self.root) for path in self.root.rglob("*"))
        with contextlib.redirect_stdout(output), contextlib.redirect_stderr(output):
            reference.verify_files(self.root, self.expected)
        self.assertEqual((self.root / "BIN/test.dat").read_bytes(), self.content)
        self.assertEqual(sorted(path.relative_to(self.root) for path in self.root.rglob("*")), before)
        self.assertEqual(output.getvalue(), "")

    def test_modified_data_fails(self):
        (self.root / "BIN/test.dat").write_bytes(b"changed")
        with self.assertRaisesRegex(ValueError, "checksum mismatch"):
            reference.verify_files(self.root, self.expected)

    def test_missing_input_fails(self):
        (self.root / "BIN/test.dat").unlink()
        with self.assertRaises(FileNotFoundError):
            reference.verify_files(self.root, self.expected)

    def test_directory_cannot_replace_file(self):
        with self.assertRaisesRegex(ValueError, "not a regular file"):
            reference.verify_files(self.root, {"BIN": "0" * 40})

    def test_unsafe_manifest_paths_fail(self):
        for name in ["../outside", "/outside", "C:/outside", "BIN\\outside", ""]:
            with self.subTest(name=name), self.assertRaisesRegex(ValueError, "Unsafe reference path"):
                reference.verify_files(self.root, {name: "0" * 40})

    def test_empty_and_invalid_checksums_fail(self):
        for expected in [{}, {"BIN/test.dat": "not a checksum"}]:
            with self.subTest(expected=expected), self.assertRaises(ValueError):
                reference.verify_files(self.root, expected)

    def test_external_symlink_fails(self):
        with tempfile.TemporaryDirectory() as outside:
            target = Path(outside) / "outside.dat"
            target.write_bytes(self.content)
            link = self.root / "link.dat"
            try:
                link.symlink_to(target)
            except OSError:
                self.skipTest("Creating symlinks requires privileges on this platform")
            with self.assertRaisesRegex(ValueError, "escapes the input directory"):
                reference.verify_files(self.root, {"link.dat": next(iter(self.expected.values()))})

    def test_cli_failure_is_clear_and_does_not_claim_success(self):
        output, errors = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(output), contextlib.redirect_stderr(errors):
            result = reference.main([str(self.root)])
        self.assertEqual(result, 1)
        self.assertEqual(output.getvalue(), "")
        self.assertIn("Reference verification failed", errors.getvalue())


if __name__ == "__main__":
    unittest.main()
