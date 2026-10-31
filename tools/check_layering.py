#!/usr/bin/env python3
"""Enforce the layer contract described in docs/architecture.md.

Project includes must use an explicit layer prefix, e.g.

    #include "domain/grid.h"
    #include "model/life_model.h"

The checker rejects:
  * includes that point at a forbidden layer,
  * path traversal, absolute paths and the non-canonical ``src/`` prefix,
  * Qt/UI headers in layers that must stay UI-free,
  * Chinese display text in domain/model,
  * sources that are not listed in CMakeLists.txt,
  * CMake target links that point the wrong way.

Run directly or via CTest (test name: layering).
"""

from __future__ import annotations

import re
import sys
from pathlib import Path, PurePosixPath

LAYERS = ("domain", "shared", "model", "service", "view", "controller", "app")

ALLOWED = {
    "domain": {"domain"},
    "shared": {"shared"},
    "model": {"domain", "shared", "model"},
    "service": {"domain", "shared", "service"},
    "view": {"domain", "shared", "view"},
    "controller": {"domain", "shared", "model", "service", "view", "controller"},
    "app": set(LAYERS),
}

# UI/framework headers that must not leak into pure/model/service layers.
QT_FORBIDDEN = {
    "model": {
        "QFile", "QSettings", "QFileDialog", "QMessageBox", "QWidget", "QDialog",
        "QMainWindow", "QProgressDialog", "QApplication", "QStatusBar", "QMenu",
        "QMenuBar", "QToolBar", "QImage",
    },
    "service": {
        "QWidget", "QDialog", "QMessageBox", "QFileDialog", "QMainWindow",
        "QApplication", "QProgressDialog", "QMenu", "QToolBar",
    },
}

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]')
QT_INCLUDE_RE = re.compile(r'^\s*#\s*include\s*[<"]Q[A-Za-z0-9_/]*[>"]')
CJK_RE = re.compile(r'[\u4e00-\u9fff]')
TARGET_RE = re.compile(r'target_link_libraries\s*\(\s*([A-Za-z0-9_]+)(.*?)\)\s*;?\s*', re.S)


def layer_of(path: Path, root: Path) -> str | None:
    try:
        rel = path.relative_to(root)
    except ValueError:
        return None
    if len(rel.parts) < 2:
        return None
    layer = rel.parts[0]
    return layer if layer in LAYERS else None


def normalize_include(target: str) -> str | None:
    """Return the canonical layer-prefixed include, or None for non-project headers."""
    target = target.replace("\\", "/")
    if target.startswith("/"):
        return ""
    if target.startswith("src/"):
        return ""
    parts = PurePosixPath(target).parts
    if ".." in parts:
        return ""
    if not parts or parts[0] not in LAYERS:
        return None
    return target


def target_layer(name: str) -> str | None:
    if name == "lifegame":
        return "app"
    if name.startswith("lifegame_"):
        layer = name[len("lifegame_"):]
        return layer if layer in LAYERS else None
    return None


def check_cmake(root: Path, sources: list[Path]) -> list[str]:
    problems: list[str] = []
    cmake_path = root / "CMakeLists.txt"
    if not cmake_path.is_file():
        return ["CMakeLists.txt is missing"]
    text = cmake_path.read_text(encoding="utf-8", errors="replace")

    for source in sources:
        rel = source.relative_to(root).as_posix()
        if rel not in text:
            problems.append(f"{source}: source file is not listed in CMakeLists.txt")

    for match in TARGET_RE.finditer(text):
        owner = match.group(1)
        owner_layer = target_layer(owner)
        if owner_layer is None:
            continue
        for dependency in re.findall(r'lifegame(?:_[A-Za-z0-9_]+)?', match.group(2)):
            dependency_layer = target_layer(dependency)
            if dependency_layer is None:
                continue
            if dependency_layer not in ALLOWED[owner_layer]:
                problems.append(
                    f"CMakeLists.txt: {owner} ({owner_layer}) links {dependency} ({dependency_layer})"
                )
    return problems


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    src = root / "src"
    if not src.is_dir():
        print(f"layering: source directory not found: {src}")
        return 2

    problems: list[str] = []
    checked = 0
    sources: list[Path] = []

    for path in sorted(src.rglob("*")):
        if path.suffix not in {".h", ".hpp", ".cpp", ".cc", ".cxx"}:
            continue
        layer = layer_of(path, src)
        if layer is None:
            problems.append(f"{path}: file is not inside a known layer directory")
            continue
        checked += 1
        sources.append(path)
        text = path.read_text(encoding="utf-8", errors="replace")

        if layer in {"domain", "shared", "model"} and CJK_RE.search(text):
            problems.append(f"{path}: Chinese display text is forbidden in {layer}/")

        for lineno, line in enumerate(text.splitlines(), 1):
            if layer in {"domain", "shared"} and QT_INCLUDE_RE.match(line):
                problems.append(f"{path}:{lineno}: {layer} must not include Qt headers")
                continue
            match = INCLUDE_RE.match(line)
            if not match:
                continue
            raw_target = match.group(2)
            canonical = normalize_include(raw_target)
            if canonical == "":
                problems.append(
                    f"{path}:{lineno}: non-canonical or escaping include is forbidden: {raw_target}"
                )
                continue
            if canonical is None:
                # Qt/system header; check the UI-free lists below.
                header = raw_target.split("/")[0]
                header = header.split(".")[0]
                if layer in QT_FORBIDDEN and header in QT_FORBIDDEN[layer]:
                    problems.append(f"{path}:{lineno}: {layer} may not include {raw_target}")
                continue
            target_layer = canonical.split("/", 1)[0]
            if target_layer not in ALLOWED[layer]:
                problems.append(
                    f"{path}:{lineno}: {layer} may not include {target_layer} ({canonical})"
                )

    problems.extend(check_cmake(root, sources))

    if problems:
        print("Architecture violations:")
        for problem in problems:
            print(f"  - {problem}")
        print(f"\n{len(problems)} violation(s) in {checked} files.")
        return 1

    print(f"Layering OK: {checked} files checked.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
