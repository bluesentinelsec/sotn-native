"""Require the native app's real OpenGL framebuffer checks to pass."""

import argparse
import os
from pathlib import Path
import subprocess


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("app", type=Path)
parser.add_argument("--mesa-dir", type=Path, help="CI-only Windows software-driver directory")
args = parser.parse_args()
environment = dict(os.environ)
if args.mesa_dir:
    mesa = args.mesa_dir.resolve(strict=True)
    driver = mesa / "opengl32.dll"
    if not driver.is_file():
        raise SystemExit(f"Missing software OpenGL driver: {driver}")
    environment["SDL_OPENGL_LIBRARY"] = str(driver)
    environment["PATH"] = str(mesa) + os.pathsep + environment.get("PATH", "")
    environment["GALLIUM_DRIVER"] = "llvmpipe"
    environment["LIBGL_ALWAYS_SOFTWARE"] = "true"
result = subprocess.run([str(args.app.resolve(strict=True)), "--graphics-test"],
                        env=environment, text=True, capture_output=True, timeout=90)
output = result.stdout + result.stderr
print(output)
if result.returncode != 0 or "SOTN_GRAPHICS_TEST: PASS" not in output:
    raise SystemExit(f"OpenGL framebuffer test failed (exit {result.returncode})")
