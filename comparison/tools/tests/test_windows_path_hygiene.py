#!/usr/bin/env python3
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
FORBIDDEN = ["/Users/den/", r"\\Mac\Home", "C:\\Mac\\Home", "Z:"]
TARGETS = [
    REPO / "tests" / "test_renderer.cpp",
    REPO / "tests" / "test_m3p_parser.cpp",
]


def main() -> int:
    for path in TARGETS:
        text = path.read_text(encoding="utf-8")
        for marker in FORBIDDEN:
            assert marker not in text, f"forbidden machine-specific path '{marker}' found in {path.name}"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
