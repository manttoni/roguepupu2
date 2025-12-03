#!/usr/bin/env python3
import json

# input/output files
INPUT_JSON = "data/entities.json"
OUTPUT_HPP = "headers/entity_enums.hpp"

# utility: convert string to snake_case
def to_snake_case(s):
    return ''.join(
        c.lower() if c.isalnum() else '_' if c.isspace() else ''
        for c in s
    )

# read JSON
with open(INPUT_JSON, "r", encoding="utf-8") as f:
    data = json.load(f)

# generate enum entries
enum_entries = [f"    {to_snake_case(name)}," for name in data.keys()]

# write header
with open(OUTPUT_HPP, "w", encoding="utf-8") as f:
    f.write("// This file is generated. Do not edit manually.\n")
    f.write("#pragma once\n\n")
    f.write("enum class EntityType {\n")
    f.write("\n".join(enum_entries))
    f.write("\n};\n")

print(f"Generated {OUTPUT_HPP} with {len(enum_entries)} entities.")

