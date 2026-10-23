"""Package only the built APK, bootstrap documentation, and dependency licenses."""

from pathlib import Path
import sys
import zipfile

root = Path(__file__).resolve().parents[1]
apk = Path(sys.argv[1])
version = (root / "VERSION").read_text().strip()
out = root / "dist" / f"sotn-native-{version}-android-unsigned.zip"
out.parent.mkdir(exist_ok=True)
sdl_license = next((root / "platforms/android/app/.cxx").glob("**/sdl3-src/LICENSE.txt"))
with zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED) as archive:
    archive.write(apk, "sotn-native-unsigned.apk")
    for name in ["LICENSE", "README.md", "THIRD_PARTY.md"]:
        archive.write(root / name, name)
    archive.write(sdl_license, "licenses/SDL3.txt")
with zipfile.ZipFile(apk) as archive:
    libraries = [n for n in archive.namelist() if n.startswith("lib/") and n.endswith(".so")]
    assert sorted(libraries) == ["lib/arm64-v8a/libmain.so", "lib/x86_64/libmain.so"], libraries
print(out)
