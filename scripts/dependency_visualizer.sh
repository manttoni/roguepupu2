#!/usr/bin/env bash

set -euo pipefail

dependency_file="${1:-headers/components/Dependency.def}"
output_file="${2:-dependencies.svg}"

awk '
BEGIN {
print "digraph Dependencies {"
print "    rankdir=TB;"
print "    node [shape=box, style=filled];"
}

function color(name) {
if (name ~ /^Tag::/)   return "lightblue"
	if (name ~ /^Value::/) return "lightgreen"
		if (name ~ /^List::/)  return "lightyellow"
			if (name ~ /^Type::/)  return "lightpink"
				return "white"
			}

			/^[[:space:]]*X\(/ {
			line = $0

			sub(/^[[:space:]]*X\(/, "", line)
			sub(/\)[[:space:]]*$/, "", line)

			split(line, parts, ",")

			parent = parts[1]
			child = parts[2]

			gsub(/^[[:space:]]+|[[:space:]]+$/, "", parent)
			gsub(/^[[:space:]]+|[[:space:]]+$/, "", child)

			printf "    \"%s\" [fillcolor=\"%s\"];\n", parent, color(parent)
			printf "    \"%s\" [fillcolor=\"%s\"];\n", child, color(child)
			printf "    \"%s\" -> \"%s\";\n", parent, child
		}

	END {
	print "}"
}
' "$dependency_file" |
	dot -Tsvg -o "$output_file"

printf "Generated %s\n" "$output_file"

if command -v explorer.exe >/dev/null 2>&1; then
	explorer.exe "$(wslpath -w "$output_file")"
fi
