#!/usr/bin/env python3
from __future__ import annotations

import json
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(REPO / "comparison" / "tools"))

from windows_workspace import load_workspace_config


def main() -> int:
    with tempfile.TemporaryDirectory() as td:
        root = Path(td).resolve()
        repo = root / "repository"
        cfg = root / "windows_workspace.local.json"
        cfg.write_text(
            json.dumps(
                {
                    "workflow_mode": "local-windows",
                    "pascal_work_root": "D:\\work\\mb3d_pascal_work\\mb3d_work",
                    "legacy_windows_repo_root": "C:\\Mac\\Home\\work\\_projects\\mb3d_cpp_port",
                    "repo_root": "D:\\stale\\checkout",
                }
            ),
            encoding="utf-8",
        )

        data = load_workspace_config(
            config_path=cfg,
            repo_root=repo,
        )
        assert data["workflow_mode"] == "local-windows"
        assert data["pascal_work_root"] == r"D:\work\mb3d_pascal_work\mb3d_work"
        # The explicit native checkout wins over both saved Windows paths.
        # An absolute D:\... fixture is relative on POSIX and cannot test this.
        assert Path(data["repo_root"]) == repo
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
