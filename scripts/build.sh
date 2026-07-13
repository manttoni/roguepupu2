#!/bin/bash
set -eEo pipefail

LOG_FILE="logs/logs.log"
mkdir -p logs

trap '
echo "$msg" >&2
echo "$msg $(date)" >> "$LOG_FILE"
' ERR

CXX_COMPILER=clang++

BUILD_DEBUG="build-debug"
BUILD_RELEASE="build-release"

configure_debug() {
    cmake -S . -B "$BUILD_DEBUG" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_COMPILER="$CXX_COMPILER"
}

build_debug() {
    echo "Building debug... $(date)" >> "$LOG_FILE"
    cmake --build "$BUILD_DEBUG"
}

run_tests() {
    ./build-debug/roguepupu2_tests
}

configure_release() {
    cmake -S . -B "$BUILD_RELEASE" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER="$CXX_COMPILER"
}

build_release() {
    echo "Building release... $(date)" >> "$LOG_FILE"
    cmake --build "$BUILD_RELEASE"
}

for arg in "$@"; do
    case "$arg" in
        debug)
            configure_debug
            build_debug
            ;;
        test)
            run_tests
            ;;
        release)
            configure_release
            build_release
            ;;
        clean)
            rm -rf "$BUILD_DEBUG" "$BUILD_RELEASE"
            ;;
        *)
            echo "Unknown command: $arg"
            echo "Usage: $0 [clean] [debug] [test] [release]"
            exit 1
            ;;
    esac
done
