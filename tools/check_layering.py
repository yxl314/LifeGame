#!/usr/bin/env python3
"""Enforce the layer contract described in docs/architecture.md.

Project includes must use an explicit layer prefix, e.g.

    #include "domain/grid.h"
    #include "model/life_model.h"

The checker rejects includes that point at a forbidden layer, any Qt include
inside src/domain, and relative project includes escaping the source tree.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

LAYERS = ("domain", "model", "service", "view", "controller", "app")

ALLOWED = {
    "domain": {"domain"},
    "model": {"domain", "model"},
    "service": {"domain", "model", "service"},
    "view": {"domain", "model", "view"},
    "controller": {"domain", "model", "service", "view", "controller"},
    "app": set(LAYERS),
}

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]')
QT_INCLUDE_RE = re.compile(r'^\s*#\s*include\s*[<"]Q[A-Za-z0-9_/]*[>"]')


def layer_of(path: Path, root: Path):
    try:
        rel = path.relative_to(root)
    except ValueError:
        return None
    if len(rel.parts) < 2:
        return None
    layer = rel.parts[0]
    return layer if layer in LAYERS else None


def include_layer(target: str):
    if "/" not in target:
        return None
    head = target.split("/", 1)[0]
    return head if head in LAYERS else None


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    src = root / "src"
    if not src.is_dir():
        print(f"layering: source directory not found: {src}")
        return 2

    problems = []
    checked = 0

    for path in sorted(src.rglob("*")):
        if path.suffix not in {".h", ".hpp", ".cpp", ".cc", ".cxx"}:
            continue
        layer = layer_of(path, src)
        if layer is None:
            problems.append(f"{path}: file is not inside a known layer directory")
            continue
        checked += 1
        text = path.read_text(encoding="utf-8", errors="replace")

        for lineno, line in enumerate(text.splitlines(), 1):
            if layer == "domain" and QT_INCLUDE_RE.match(line):
                problems.append(f"{path}:{lineno}: domain must not include Qt headers")
                continue
            match = INCLUDE_RE.match(line)
            if not match:
                continue
            target = match.group(2)
            if target.startswith("."):
                problems.append(f"{path}:{lineno}: relative project include is forbidden: {target}")
                continue
            target_layer = include_layer(target)
            if target_layer is None:
                continue
            if target_layer not in ALLOWED[layer]:
                problems.append(
                    f"{path}:{lineno}: {layer} may not include {target_layer} ({target})"
                )

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
