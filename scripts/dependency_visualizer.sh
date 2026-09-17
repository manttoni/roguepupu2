#!/usr/bin/env bash

set -euo pipefail

dependency_file="${1:-headers/components/Dependency.def}"
output_file="${2:-dependencies.svg}"
tags_output_file="${3:-dependencies_tags.svg}"

generate_graph()
{
	local tags_only="$1"
	local destination="$2"

	awk -v tags_only="$tags_only" '
	BEGIN {
		print "digraph Dependencies {"
		print "    rankdir=TB;"
		print "    node [shape=box, style=filled];"
	}

	function color(name) {
		if (name ~ /^Component::Tag::/)   return "lightblue"
		if (name ~ /^Component::Value::/) return "lightgreen"
		if (name ~ /^Component::List::/)  return "lightyellow"
		if (name ~ /^Component::Type::/)  return "lightpink"
		return "white"
	}

	function is_tag(name) {
		return name ~ /^Component::Tag::/
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

		if (tags_only && (!is_tag(parent) || !is_tag(child)))
			next

		printf "    \"%s\" [fillcolor=\"%s\"];\n", parent, color(parent)
		printf "    \"%s\" [fillcolor=\"%s\"];\n", child, color(child)
		printf "    \"%s\" -> \"%s\";\n", parent, child
	}

	END {
		print "}"
	}
	' "$dependency_file" | dot -Tsvg -o "$destination"

	printf "Generated %s\n" "$destination"
}

generate_graph false "$output_file"
generate_graph true "$tags_output_file"

if command -v explorer.exe >/dev/null 2>&1; then
	explorer.exe "$(wslpath -w "$output_file")"
	explorer.exe "$(wslpath -w "$tags_output_file")"
fi
