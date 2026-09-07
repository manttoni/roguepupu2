#!/usr/bin/env bash

set -euo pipefail

rg -l -0 '^[[:space:]]*using namespace[[:space:]]+[^;]+;[[:space:]]*$' headers sources |
    xargs -0 -r sed -i \
        '/^[[:space:]]*using namespace[[:space:]][^;]*;[[:space:]]*$/d'

rg 'using namespace' headers sources || true
