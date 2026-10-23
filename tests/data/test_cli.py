"""Check native argument failures without requiring a graphics driver or game data."""

import os
from pathlib import Path
import subprocess
import sys
import tempfile


app = sys.argv[1]
environment = {**os.environ, "SDL_VIDEODRIVER": "invalid-test-driver"}
# Expected argument failures must remain distinguishable from sanitizer failures.
for option, value in [("ASAN_OPTIONS", "exitcode=86"),
                      ("UBSAN_OPTIONS", "exitcode=87:halt_on_error=1")]:
    environment[option] = f"{environment.get(option, '')}:{value}".lstrip(":")
cases = [
    (["--game-data"], "requires one directory"),
    (["--game-data", "--validate-only"], "requires one directory"),
    (["--validate-only"], "requires --game-data"),
    (["--unknown"], "Unknown argument"),
    (["--game-data", ".", "--game-data", "."], "may appear only once"),
]
with tempfile.TemporaryDirectory() as directory:
    cases += [
        (["--game-data", str(Path(directory) / "missing"), "--validate-only"], "directory"),
        (["--game-data", directory, "--validate-only"], "SLUS_000.67"),
    ]
    for arguments, message in cases:
        result = subprocess.run([app, *arguments], env=environment, text=True,
                                capture_output=True, timeout=5)
        output = result.stdout + result.stderr
        if result.returncode != 1 or message not in output or "SOTN_GAME_DATA: PASS" in output:
            raise SystemExit(f"Unexpected validation result for {arguments}: {output}")
result = subprocess.run([app, "--help"], env=environment, text=True,
                        capture_output=True, timeout=5)
if result.returncode != 0 or "Usage:" not in result.stdout + result.stderr:
    raise SystemExit("Help should work without graphics initialization")
print("Native CLI failure checks passed.")
