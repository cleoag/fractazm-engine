#!/usr/bin/env python3
from __future__ import annotations

import json
import os
from pathlib import Path


def workflow_mode(config: dict[str, str]) -> str:
    return config.get("workflow_mode", "local-windows")


def load_workspace_config(*, config_path: Path | None = None, repo_root: Path | None = None) -> dict[str, str]:
    repo_root = (repo_root or Path(__file__).resolve().parents[2]).resolve()
    config_path = config_path or (repo_root / "comparison" / "tools" / "windows_workspace.local.json")

    data: dict[str, str] = {
        "workflow_mode": os.environ.get("MB3D_WORKFLOW_MODE", "local-windows"),
        "pascal_work_root": os.environ.get("MB3D_PASCAL_WORK_ROOT", ""),
        "repo_root": str(repo_root),
    }
    if config_path.exists():
        loaded = json.loads(config_path.read_text(encoding="utf-8"))
        data.update({k: str(v) for k, v in loaded.items()})
    data["repo_root"] = str(repo_root)
    if not data["pascal_work_root"]:
        raise SystemExit("pascal_work_root must be set in windows_workspace.local.json or MB3D_PASCAL_WORK_ROOT")
    return data
