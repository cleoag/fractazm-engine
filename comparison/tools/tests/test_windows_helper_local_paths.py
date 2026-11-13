#!/usr/bin/env python3
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
TARGETS = [
    REPO / "comparison/tools/windows_runtime_guard.ps1",
    REPO / "comparison/tools/clean_windows_runtime.ps1",
    REPO / "comparison/tools/build_pascal_windows.ps1",
    REPO / "comparison/tools/render_one_windows.ps1",
]
FORBIDDEN = [r"\\Mac\Home", "C:\\Mac\\Home", "net use Z:", "net use M:"]


def main() -> int:
    for path in TARGETS:
        text = path.read_text(encoding="utf-8")
        assert "Get-WindowsWorkspace" in text, f"{path.name} must use workspace resolver"
        for marker in FORBIDDEN:
            assert marker not in text, f"forbidden bridge path '{marker}' found in {path.name}"
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
