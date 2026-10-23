"""Download checksum-pinned Windows software OpenGL for CI; never install it system-wide."""

import hashlib
from pathlib import Path
import shutil
import subprocess
import sys
import urllib.request


URL = ("https://github.com/pal1000/mesa-dist-win/releases/download/26.2.4/"
       "mesa3d-26.2.4-release-msvc.7z")
SHA256 = "351fc8c8b695878ffb3eaa044b3ead08672a48b1a045e3c3e3975811df0f6695"
directory = Path(sys.argv[1]).resolve()
directory.mkdir(parents=True, exist_ok=True)
archive = directory / "mesa.7z"
with urllib.request.urlopen(URL, timeout=60) as source, archive.open("wb") as output:
    shutil.copyfileobj(source, output)
with archive.open("rb") as stream:
    if hashlib.file_digest(stream, "sha256").hexdigest() != SHA256:
        archive.unlink()
        raise SystemExit("Windows Mesa archive checksum mismatch")
extractor = shutil.which("7z") or r"C:\Program Files\7-Zip\7z.exe"
subprocess.run([extractor, "x", "-y", str(archive), f"-o{directory}"], check=True,
               stdout=subprocess.DEVNULL)
if not (directory / "x64/opengl32.dll").is_file():
    raise SystemExit("Windows Mesa archive is missing x64/opengl32.dll")
print("Pinned Windows Mesa driver is ready for CI testing only.")
