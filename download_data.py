#!/usr/bin/env python3
"""Download the Moving AI MAPF maps and scenario sets."""

import shutil
import sys
import tempfile
import urllib.error
import urllib.request
import zipfile
from pathlib import Path


DATA_DIR = Path(__file__).resolve().parent / "data"
BASE_URL = "https://movingai.com/benchmarks/mapf"
ARCHIVES = ("map", "scen-even", "scen-random")


def download_archive(name: str) -> None:
    url = f"{BASE_URL}/mapf-{name}.zip"
    destination = DATA_DIR / f"mapf-{name}"
    print(f"Downloading {url}...", flush=True)

    with tempfile.TemporaryFile() as archive_file:
        with urllib.request.urlopen(url, timeout=60) as response:
            shutil.copyfileobj(response, archive_file)
        archive_file.seek(0)

        with zipfile.ZipFile(archive_file) as archive:
            for entry in archive.infolist():
                target = (destination / entry.filename).resolve()
                if not target.is_relative_to(destination.resolve()):
                    raise ValueError(f"Unsafe path in archive: {entry.filename}")
                if entry.is_dir():
                    target.mkdir(parents=True, exist_ok=True)
                    continue

                target.parent.mkdir(parents=True, exist_ok=True)
                try:
                    with target.open("xb") as output:
                        try:
                            with archive.open(entry) as source:
                                shutil.copyfileobj(source, output)
                        except Exception:
                            target.unlink(missing_ok=True)
                            raise
                except FileExistsError:
                    continue

    print(f"Extracted to {destination}", flush=True)


def main() -> int:
    try:
        for name in ARCHIVES:
            download_archive(name)
    except (OSError, ValueError, zipfile.BadZipFile, urllib.error.URLError) as error:
        print(f"Download failed: {error}", file=sys.stderr)
        return 1
    print("Done. Existing files were preserved.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
