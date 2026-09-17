#!/usr/bin/env python3

"""Audit X-macro component definitions and their dependency graph."""

from __future__ import annotations

import argparse
import re
import sys
from collections import Counter, defaultdict
from pathlib import Path


CATEGORIES = ("Tag", "Value", "List", "Resource")


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//.*", "", text)


def x_calls(text: str) -> list[str]:
    """Return the contents of balanced X(...) calls, including multiline calls."""
    calls: list[str] = []
    text = strip_comments(text)
    for match in re.finditer(r"\bX\s*\(", text):
        start = match.end()
        depth = 1
        quote = ""
        escaped = False
        for index in range(start, len(text)):
            char = text[index]
            if quote:
                if escaped:
                    escaped = False
                elif char == "\\":
                    escaped = True
                elif char == quote:
                    quote = ""
            elif char in "\"'":
                quote = char
            elif char == "(":
                depth += 1
            elif char == ")":
                depth -= 1
                if depth == 0:
                    calls.append(text[start:index].strip())
                    break
    return calls


def split_arguments(call: str) -> list[str]:
    """Split arguments without breaking nested templates or function calls."""
    arguments: list[str] = []
    start = 0
    nesting = {"(": 0, "[": 0, "{": 0, "<": 0}
    closing = {")": "(", "]": "[", "}": "{", ">": "<"}
    quote = ""
    escaped = False
    for index, char in enumerate(call):
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = ""
            continue
        if char in "\"'":
            quote = char
        elif char in nesting:
            nesting[char] += 1
        elif char in closing and nesting[closing[char]]:
            nesting[closing[char]] -= 1
        elif char == "," and not any(nesting.values()):
            arguments.append(call[start:index].strip())
            start = index + 1
    arguments.append(call[start:].strip())
    return arguments


def clean_symbol(symbol: str) -> str:
    return re.sub(r"\s+", "", symbol).removeprefix("::")


def definition_name(category: str, symbol: str) -> str:
    symbol = clean_symbol(symbol)
    if symbol.startswith("Component::"):
        return symbol
    if any(symbol.startswith(f"{kind}::") for kind in CATEGORIES):
        return f"Component::{symbol}"
    return f"Component::{category}::{symbol}"


def resolve_reference(symbol: str, declared: set[str]) -> tuple[str, bool]:
    symbol = clean_symbol(symbol)
    candidates = [symbol]
    if any(symbol.startswith(f"{kind}::") for kind in CATEGORIES):
        candidates.insert(0, f"Component::{symbol}")
    for candidate in candidates:
        if candidate in declared:
            return candidate, True

    # Allow short dependency names only when they identify exactly one component.
    short_matches = [name for name in declared if name.rsplit("::", 1)[-1] == symbol]
    if len(short_matches) == 1:
        return short_matches[0], True
    return candidates[0], False


def strongly_connected_components(
    nodes: set[str], dependencies: dict[str, set[str]]
) -> list[list[str]]:
    index = 0
    indices: dict[str, int] = {}
    lowlink: dict[str, int] = {}
    stack: list[str] = []
    on_stack: set[str] = set()
    result: list[list[str]] = []

    def visit(node: str) -> None:
        nonlocal index
        indices[node] = lowlink[node] = index
        index += 1
        stack.append(node)
        on_stack.add(node)

        for dependency in dependencies.get(node, set()):
            if dependency not in nodes:
                continue
            if dependency not in indices:
                visit(dependency)
                lowlink[node] = min(lowlink[node], lowlink[dependency])
            elif dependency in on_stack:
                lowlink[node] = min(lowlink[node], indices[dependency])

        if lowlink[node] == indices[node]:
            component: list[str] = []
            while True:
                member = stack.pop()
                on_stack.remove(member)
                component.append(member)
                if member == node:
                    break
            if len(component) > 1:
                result.append(sorted(component))

    for node in sorted(nodes):
        if node not in indices:
            visit(node)
    return result


def short(name: str) -> str:
    return name.removeprefix("Component::")


def is_component_reference(name: str) -> bool:
    name = clean_symbol(name)
    return name.startswith("Component::") or any(
        name.startswith(f"{category}::") for category in CATEGORIES
    )


def category_of(name: str, declared: set[str]) -> str:
    if name not in declared:
        return "External"
    match = re.match(r"^Component::([^:]+)::", name)
    return match.group(1) if match else "Other"


def bullet_section(lines: list[str], title: str, items: list[str]) -> None:
    lines.append(f"\n{title} ({len(items)})")
    lines.append("-" * (len(title) + len(str(len(items))) + 3))
    lines.extend(f"  {item}" for item in items) if items else lines.append("  none")


def build_report(component_dir: Path) -> str:
    declarations: dict[str, list[tuple[str, int]]] = defaultdict(list)
    metadata: dict[str, str] = {}
    category_members: dict[str, set[str]] = {category: set() for category in CATEGORIES}
    warnings: list[str] = []

    for category in CATEGORIES:
        path = component_dir / f"{category}.def"
        if not path.is_file():
            warnings.append(f"missing definition file: {path}")
            continue
        for number, call in enumerate(x_calls(path.read_text(encoding="utf-8")), 1):
            arguments = split_arguments(call)
            if not arguments or not arguments[0]:
                warnings.append(f"empty X(...) call in {path.name}, parsed call #{number}")
                continue
            name = definition_name(category, arguments[0])
            declarations[name].append((path.name, number))
            category_members[category].add(name)
            metadata[name] = ", ".join(arguments[1:]) or "-"

    declared = set(declarations)
    dependency_path = component_dir / "Dependency.def"
    raw_edges: list[tuple[str, str]] = []
    malformed_dependencies: list[str] = []
    if dependency_path.is_file():
        for number, call in enumerate(x_calls(dependency_path.read_text(encoding="utf-8")), 1):
            arguments = split_arguments(call)
            if len(arguments) < 2:
                malformed_dependencies.append(f"call #{number}: X({call})")
                continue
            raw_edges.append((arguments[0], arguments[1]))
    else:
        warnings.append(f"missing dependency file: {dependency_path}")

    edges: list[tuple[str, str]] = []
    unresolved: list[str] = []
    external_references: list[str] = []
    for raw_component, raw_dependency in raw_edges:
        component, component_found = resolve_reference(raw_component, declared)
        dependency, dependency_found = resolve_reference(raw_dependency, declared)
        edges.append((component, dependency))
        if not component_found:
            unresolved.append(f"dependent not declared: {component}")
        if not dependency_found:
            if is_component_reference(raw_dependency):
                unresolved.append(f"dependency not declared: {dependency}")
            else:
                external_references.append(f"{dependency} <- {component}")

    edge_counts = Counter(edges)
    unique_edges = set(edges)
    dependencies: dict[str, set[str]] = defaultdict(set)
    required_by: dict[str, set[str]] = defaultdict(set)
    for component, dependency in unique_edges:
        dependencies[component].add(dependency)
        required_by[dependency].add(component)

    mapped = {name for edge in unique_edges for name in edge}
    isolated = sorted(declared - mapped)
    duplicate_declarations = [
        f"{short(name)}: " + ", ".join(f"{file} call #{line}" for file, line in places)
        for name, places in sorted(declarations.items())
        if len(places) > 1
    ]
    duplicate_edges = [
        f"{short(component)} -> {short(dependency)} ({count} times)"
        for (component, dependency), count in sorted(edge_counts.items())
        if count > 1
    ]
    self_dependencies = [
        short(component) for component, dependency in sorted(unique_edges) if component == dependency
    ]
    cycles = strongly_connected_components(declared, dependencies)
    tags = category_members["Tag"]
    tag_dependencies = {
        tag: {dependency for dependency in dependencies[tag] if dependency in tags}
        for tag in tags
    }
    tag_required_by = {
        tag: {dependent for dependent in required_by[tag] if dependent in tags}
        for tag in tags
    }
    tag_roots = sorted(tag for tag in tags if not tag_dependencies[tag])
    tag_leaves = sorted(tag for tag in tags if not tag_required_by[tag])

    edge_matrix: Counter[tuple[str, str]] = Counter(
        (category_of(component, declared), category_of(dependency, declared))
        for component, dependency in unique_edges
    )

    lines = [
        "COMPONENT AUDIT",
        "===============",
        f"Directory: {component_dir}",
        "Edge meaning: component -> required dependency",
        "",
        "SUMMARY",
        "-------",
        f"Declared components : {len(declared)}",
        f"Dependency pairs    : {len(raw_edges)} ({len(unique_edges)} unique)",
        f"Mapped components   : {len(declared & mapped)}",
        f"Isolated components : {len(isolated)}",
        f"Coverage            : {(100.0 * len(declared & mapped) / len(declared)) if declared else 0:.1f}%",
        "",
        f"{'Category':<12}{'Defined':>8}{'Mapped':>8}{'Depends':>9}{'Required':>10}{'Isolated':>10}",
        "-" * 57,
    ]
    for category in CATEGORIES:
        members = category_members[category]
        lines.append(
            f"{category:<12}{len(members):>8}{len(members & mapped):>8}"
            f"{sum(bool(dependencies[name]) for name in members):>9}"
            f"{sum(bool(required_by[name]) for name in members):>10}"
            f"{len(members - mapped):>10}"
        )

    lines.extend(["", "DEPENDENCY PAIRS BY CATEGORY", "----------------------------"])
    for (source_category, target_category), count in sorted(edge_matrix.items()):
        lines.append(f"  {source_category:<10} -> {target_category:<10} {count:>4}")

    bullet_section(lines, "ISOLATED (not present on either side of any pair)", [short(x) for x in isolated])
    bullet_section(lines, "TAG HIERARCHY ROOTS (no Tag prerequisite)", [short(x) for x in tag_roots])
    bullet_section(lines, "TAG HIERARCHY LEAVES (no derived Tag)", [short(x) for x in tag_leaves])
    bullet_section(
        lines,
        "NO DECLARED DEPENDENCIES",
        [short(x) for x in sorted(declared) if not dependencies[x]],
    )
    bullet_section(
        lines,
        "NEVER REQUIRED BY ANOTHER COMPONENT",
        [short(x) for x in sorted(declared) if not required_by[x]],
    )

    ranked_dependencies = sorted(declared, key=lambda x: (-len(dependencies[x]), x))
    ranked_required = sorted(declared, key=lambda x: (-len(required_by[x]), x))
    bullet_section(
        lines,
        "MOST DIRECT DEPENDENCIES",
        [f"{len(dependencies[x]):>3}  {short(x)}" for x in ranked_dependencies[:10] if dependencies[x]],
    )
    bullet_section(
        lines,
        "MOST DEPENDED-ON",
        [f"{len(required_by[x]):>3}  {short(x)}" for x in ranked_required[:10] if required_by[x]],
    )

    bullet_section(lines, "UNDECLARED REFERENCES", sorted(set(unresolved)))
    bullet_section(lines, "EXTERNAL DEPENDENCIES", sorted(set(external_references)))
    bullet_section(lines, "DUPLICATE DEFINITIONS", duplicate_declarations)
    bullet_section(lines, "DUPLICATE DEPENDENCY PAIRS", duplicate_edges)
    bullet_section(lines, "SELF-DEPENDENCIES", self_dependencies)
    bullet_section(lines, "MALFORMED DEPENDENCY CALLS", malformed_dependencies)
    bullet_section(lines, "WARNINGS", warnings)
    bullet_section(
        lines,
        "DEPENDENCY CYCLES",
        [" <-> ".join(short(member) for member in cycle) for cycle in cycles],
    )

    lines.extend(["", "DIRECT DEPENDENCY MAP", "---------------------"])
    mapped_dependents = sorted(name for name in declared if dependencies[name])
    if mapped_dependents:
        for name in mapped_dependents:
            dependency_list = ", ".join(short(x) for x in sorted(dependencies[name]))
            lines.append(f"  {short(name)} -> {dependency_list}")
    else:
        lines.append("  none")

    lines.extend(["", "ALL DECLARATIONS", "----------------"])
    for category in CATEGORIES:
        lines.append(f"  [{category}] {len(category_members[category])}")
        for name in sorted(category_members[category]):
            lines.append(
                f"    {short(name):<42} deps={len(dependencies[name]):<2} "
                f"used_by={len(required_by[name]):<2} data={metadata[name]}"
            )

    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "component_dir",
        nargs="?",
        type=Path,
        default=Path("headers/components"),
        help="directory containing the .def files (default: headers/components)",
    )
    parser.add_argument(
        "-o", "--output", type=Path, default=Path("component_info.txt"),
        help="report file (default: component_info.txt)",
    )
    args = parser.parse_args()

    report = build_report(args.component_dir)
    args.output.write_text(report, encoding="utf-8")
    sys.stdout.write(report)
    print(f"Report written to {args.output}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

