"""Reject private game inputs and unexpected binaries from the Git index."""

import hashlib
from pathlib import Path
import subprocess
import sys

BLOCKED_DIRS = {"local", "roms", "bios", "game-data", "extracted"}
BLOCKED_SUFFIXES = {
    ".bin", ".cue", ".iso", ".img", ".chd", ".pbp", ".rom", ".7z", ".zip",
    ".dat", ".tim", ".vab", ".vh", ".vb", ".xa", ".str", ".ccd", ".sub",
    ".exe", ".dll", ".so", ".dylib", ".a", ".o", ".psf", ".wav", ".mp3",
    ".ogg", ".png", ".jpg", ".bmp", ".nes", ".z64", ".n64", ".v64",
}
WRAPPER = "platforms/android/gradle/wrapper/gradle-wrapper.jar"
WRAPPER_SHA256 = "2db75c40782f5e8ba1fc278a5574bab070adccb2d21ca5a6e5ed840888448046"


def violation(name, content):
    """Return a policy violation for a tracked path and its staged bytes, if any."""
    path = Path(name.lower())
    if BLOCKED_DIRS.intersection(path.parts):
        return "private input directory"
    if path.suffix in BLOCKED_SUFFIXES:
        return "game-data or compiled-binary extension"
    if name == WRAPPER:
        if hashlib.sha256(content).hexdigest() != WRAPPER_SHA256:
            return "unexpected Gradle wrapper binary"
        return None
    if b"\0" in content or len(content) > 1024 * 1024:
        return "unexpected binary or oversized file"
    return None


def main():
    root = Path(__file__).resolve().parents[1]
    entries = subprocess.check_output(["git", "ls-files", "--stage", "-z"], cwd=root)
    problems = []
    count = 0
    for entry in entries.split(b"\0"):
        if not entry:
            continue
        metadata, raw_name = entry.split(b"\t", 1)
        mode, oid, stage = metadata.decode().split()
        name = raw_name.decode()
        content = subprocess.check_output(["git", "cat-file", "blob", oid], cwd=root)
        reason = violation(name, content)
        if mode not in {"100644", "100755"} or stage != "0":
            reason = "symlink, submodule, or unresolved index entry"
        if reason:
            problems.append(f"{name}: {reason}")
        count += 1
    if problems:
        raise SystemExit("Repository policy failed:\n" + "\n".join(problems))
    print(f"Repository policy passed ({count} indexed files checked).")


if __name__ == "__main__":
    main()
