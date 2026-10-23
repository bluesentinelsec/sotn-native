"""Serve the Wasm app and verify rendering in a real headless Chromium browser."""

from functools import partial
import http.server
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import threading

root = Path(sys.argv[1]).resolve()
chrome = os.environ.get("CHROME_BIN") or shutil.which("google-chrome") or shutil.which("chromium")
if not chrome:
    raise SystemExit("Install Chromium or set CHROME_BIN to a Chrome/Chromium executable")
handler = partial(http.server.SimpleHTTPRequestHandler, directory=str(root))
server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), handler)
thread = threading.Thread(target=server.serve_forever, daemon=True)
thread.start()
try:
    with tempfile.TemporaryDirectory(prefix="sotn-browser-") as profile:
        result = subprocess.run([
            chrome, "--headless=new", "--no-sandbox", "--use-angle=swiftshader",
            "--enable-unsafe-swiftshader", "--disable-dev-shm-usage", "--dump-dom",
            "--virtual-time-budget=15000", f"--user-data-dir={profile}",
            f"http://127.0.0.1:{server.server_port}/index.html?smoke-test=1",
        ], capture_output=True, text=True, timeout=90)
        if result.returncode != 0 or 'data-smoke-result="pass"' not in result.stdout:
            raise SystemExit("Browser rendering smoke test failed:\n" + result.stderr + result.stdout)
        print("Browser rendering smoke test passed.")
finally:
    server.shutdown()
    server.server_close()
