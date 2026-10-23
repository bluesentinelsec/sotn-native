"""Check privately extracted reference files without copying or publishing them."""

import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import sys


MANIFEST = Path(__file__).resolve().parents[1] / "config/reference-us.json"


def verify_files(directory, expected):
    """Require matching, regular files contained under directory; write nothing."""
    root = Path(directory).resolve(strict=True)
    if not root.is_dir():
        raise ValueError("Reference input must be an extracted game directory")
    if not expected:
        raise ValueError("Reference manifest contains no files")
    for name, checksum in expected.items():
        relative = PurePosixPath(name)
        if (relative.is_absolute() or ".." in relative.parts or "\\" in name
                or not relative.parts or ":" in name):
            raise ValueError(f"Unsafe reference path: {name}")
        if not re.fullmatch(r"[0-9a-f]{40}", checksum):
            raise ValueError(f"Invalid reference SHA-1: {name}")
        path = (root / relative).resolve(strict=True)
        if not path.is_relative_to(root):
            raise ValueError(f"Reference path escapes the input directory: {name}")
        if not path.is_file():
            raise ValueError(f"Reference input is not a regular file: {name}")
        # SHA-1 identifies the upstream game revision; it is not a security signature.
        digest = hashlib.sha1(usedforsecurity=False)
        with path.open("rb") as stream:
            for block in iter(lambda: stream.read(256 * 1024), b""):
                digest.update(block)
        if digest.hexdigest() != checksum:
            raise ValueError(f"Reference checksum mismatch: {name}; expected {checksum}")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path, help="Private extracted US game directory")
    args = parser.parse_args(argv)
    try:
        manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
        verify_files(args.directory, manifest["files"])
    except (OSError, ValueError, KeyError) as error:
        print(f"Reference verification failed: {error}", file=sys.stderr)
        return 1
    print(f"{manifest['id']}: {len(manifest['files'])} core reference files match.")
    print("This checks the selected files only; it does not verify the entire disc or game parity.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
