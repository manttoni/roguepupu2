#!/usr/bin/env bash

set -euo pipefail

repo_root=$(git rev-parse --show-toplevel)
cd "$repo_root"

mapfile -d '' system_files < <(
    find headers/systems sources/systems \
        -mindepth 2 \
        -type f \
        \( -name '*System.hpp' -o -name '*System.cpp' \) \
        -print0
)

declare -A planned_destinations

# Validate everything before modifying anything.
for source_file in "${system_files[@]}"; do
    case "$source_file" in
        headers/systems/*) destination_dir="headers/systems" ;;
        sources/systems/*) destination_dir="sources/systems" ;;
        *)
            echo "Unexpected path: $source_file" >&2
            exit 1
            ;;
    esac

    filename=${source_file##*/}
    extension=${filename##*.}
    old_namespace=${filename%.*}
    system_name=${old_namespace%System}

    if [[ -z "$system_name" || "$old_namespace" == "$system_name" ]]; then
        echo "Invalid system filename: $source_file" >&2
        exit 1
    fi

    destination="$destination_dir/$system_name.$extension"

    if [[ -e "$destination" ]]; then
        echo "Destination already exists: $destination" >&2
        exit 1
    fi

    if [[ -n "${planned_destinations[$destination]+exists}" ]]; then
        echo "Multiple files would become: $destination" >&2
        exit 1
    fi

    planned_destinations["$destination"]=1

    if ! git ls-files --error-unmatch -- "$source_file" >/dev/null 2>&1; then
        echo "Source is not tracked by Git: $source_file" >&2
        exit 1
    fi

    if ! grep -Eq \
        "namespace[[:space:]]+$old_namespace([[:space:]\{]|$)" \
        "$source_file"
    then
        echo "Namespace $old_namespace not found in: $source_file" >&2
        exit 1
    fi
done

# Perform the refactor.
for source_file in "${system_files[@]}"; do
    case "$source_file" in
        headers/systems/*) destination_dir="headers/systems" ;;
        sources/systems/*) destination_dir="sources/systems" ;;
    esac

    filename=${source_file##*/}
    extension=${filename##*.}
    old_namespace=${filename%.*}
    system_name=${old_namespace%System}
    destination="$destination_dir/$system_name.$extension"

    git mv -- "$source_file" "$destination"

    REFACTOR_OLD_NAMESPACE="$old_namespace" \
    REFACTOR_NEW_NAMESPACE="System::$system_name" \
        perl -pi -e '
            s{
                \bnamespace\s+\Q$ENV{REFACTOR_OLD_NAMESPACE}\E\b
            }{
                namespace $ENV{REFACTOR_NEW_NAMESPACE}
            }gx
        ' "$destination"

    echo "$source_file -> $destination"
done

# Remove only directories left empty by moving the files.
find headers/systems sources/systems \
    -mindepth 1 \
    -type d \
    -empty \
    -delete
