#!/usr/bin/env bash

set -euo pipefail

if [[ $# -ne 2 ]]; then
        printf 'Usage: %s OLD_STRING NEW_STRING\n' "$0" >&2
        printf 'Example: %s "utils/Screen.hpp" "ncurses/Screen.hpp"\n' "$0" >&2
        exit 2
fi

old_string=$1
new_string=$2

if [[ -z $old_string ]]; then
        printf 'Error: OLD_STRING must not be empty.\n' >&2
        exit 2
fi

if [[ ! -d headers || ! -d sources ]]; then
        printf 'Error: run this from the project root containing headers/ and sources/.\n' >&2
        exit 1
fi

printf 'Matching occurrences before replacement:\n'
rg --fixed-strings --line-number \
        --glob '*.{h,hh,hpp,hxx,c,cc,cpp,cxx}' \
        --glob '!headers/external/**' \
        -- "$old_string" headers sources || true

OLD_STRING=$old_string NEW_STRING=$new_string \
        find headers sources \
        -path headers/external -prune -o \
        -type f \
        \( -name '*.h' -o -name '*.hh' -o -name '*.hpp' -o -name '*.hxx' \
           -o -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' \) \
        -exec perl -pi -e 's/\Q$ENV{OLD_STRING}\E/$ENV{NEW_STRING}/g' {} +

printf '\nReplacement complete. Remaining old-string matches:\n'
rg --fixed-strings --line-number \
        --glob '*.{h,hh,hpp,hxx,c,cc,cpp,cxx}' \
        --glob '!headers/external/**' \
        -- "$old_string" headers sources || printf 'None.\n'
