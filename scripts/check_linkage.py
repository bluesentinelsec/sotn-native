"""Check that the app does not require a separately installed SDL library."""

from pathlib import Path
import platform
import subprocess
import sys

binary = Path(sys.argv[1])
system = platform.system()
command = {"Darwin": ["otool", "-L"], "Linux": ["ldd"], "Windows": ["dumpbin", "/dependents"]}[system]
result = subprocess.run(command + [str(binary)], text=True, capture_output=True, check=True)
print(result.stdout)
dependencies = result.stdout.splitlines()[1:]
if any("sdl" in line.lower() for line in dependencies):
    raise SystemExit("The app depends on a shared SDL library")
if system == "Darwin":
    for dependency in dependencies:
        if not dependency.strip().startswith(("/usr/lib/", "/System/Library/")):
            raise SystemExit("The app depends on a non-system dynamic library")
print("Static SDL linkage check passed.")
