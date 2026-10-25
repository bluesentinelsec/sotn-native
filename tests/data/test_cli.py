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
    (["--inspect-stage"], "requires one canonical stage"),
    (["--inspect-stage", "../NZ0"], "requires one canonical stage"),
    (["--inspect-stage", "NZ0"], "requires --game-data"),
    (["--inspect-stage", "NO3", "--validate-only", "--game-data", "."], "Choose one"),
    (["--laboratory-continue", "1"], "requires --laboratory"),
    (["--laboratory-continue"], "requires a slot"),
    (["--laboratory-continue", "0"], "requires a slot"),
    (["--laboratory-continue", "16"], "requires a slot"),
    (["--laboratory-continue", "-1"], "requires a slot"),
    (["--laboratory-continue", "999999999999999999"], "requires a slot"),
    (["--laboratory", "--laboratory-continue", "1", "--laboratory-room", "23"], "cannot combine"),
    (["--laboratory", "--laboratory-continue", "1", "--laboratory-loadout", "dual"], "cannot combine"),
    (["--laboratory-continue", "1", "--laboratory-continue", "2"], "only once"),
    (["--game-data"], "requires one directory"),
    (["--game-data", "--validate-only"], "requires one directory"),
    (["--validate-only"], "requires --game-data"),
    (["--unknown"], "Unknown argument"),
    (["--game-data", ".", "--game-data", "."], "may appear only once"),
    (["--preview-save-icons"], "requires --game-data"),
    (["--graphics-test", "--game-data", "."], "runs separately"),
    (["--inspect-laboratory"], "requires --game-data"),
    (["--inspect-laboratory", "--game-data", ".", "--validate-only"], "inspection mode"),
    (["--inspect-laboratory", "--game-data", ".", "--preview-save-icons"], "inspection mode"),
    (["--preview-laboratory"], "requires --game-data"),
    (["--preview-laboratory", "--game-data", ".", "--validate-only"], "Choose one"),
    (["--laboratory"], "requires --game-data"),
    (["--laboratory", "--game-data", ".", "--preview-laboratory"], "Choose one"),
    (["--laboratory-room", "3"], "requires --laboratory"),
    (["--laboratory-room", "14"], "requires --laboratory"),
    (["--laboratory-room", "16"], "requires --laboratory"),
    (["--laboratory-room", "17"], "requires --laboratory"),
    (["--laboratory-room", "22"], "requires --laboratory"),
    (["--laboratory-room", "18"], "requires --laboratory"),
    (["--laboratory-room", "20"], "requires --laboratory"),
    (["--laboratory-loadout", "dual"], "requires --laboratory"),
    (["--laboratory-loadout"], "requires unarmed"),
    (["--laboratory-loadout", "unknown"], "requires unarmed"),
    (["--laboratory-loadout", "dual", "--laboratory-loadout", "sword"], "once"),
    (["--laboratory-room"], "requires 1, 3, 4, 5, 6, 9, 10, 14, 16, 17, 18, 20, 22, 23, 24, or 25"),
    (["--laboratory-room", "-1"], "requires 1, 3, 4, 5, 6, 9, 10, 14, 16, 17, 18, 20, 22, 23, 24, or 25"),
    (["--laboratory-room", "4294967296"], "requires 1, 3, 4, 5, 6, 9, 10, 14, 16, 17, 18, 20, 22, 23, 24, or 25"),
    (["--laboratory-room", "3", "--laboratory-room", "1"], "only once"),
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
