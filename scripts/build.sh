#!/usr/bin/env bash
set -Eeuo pipefail

readonly PROJECT_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

readonly LOG_DIR="$PROJECT_ROOT/logs"
readonly LOG_FILE="$LOG_DIR/build.log"
readonly DEBUG_DIR="$PROJECT_ROOT/build-debug"
readonly RELEASE_DIR="$PROJECT_ROOT/build-release"

CXX_COMPILER="${CXX:-clang++}"
BUILD_JOBS="${BUILD_JOBS:-$(nproc)}"

mkdir -p "$LOG_DIR"

current_command="startup"

on_error() {
    local exit_code=$?
    printf '[ERROR] Command "%s" failed with exit code %d at %s\n' \
        "$current_command" "$exit_code" "$(date --iso-8601=seconds)" |
        tee -a "$LOG_FILE" >&2
    exit "$exit_code"
}
trap on_error ERR

log() {
    printf '[%s] %s\n' "$(date --iso-8601=seconds)" "$*" | tee -a "$LOG_FILE"
}

configure_debug() {
    current_command="configure debug"
    log "Configuring Debug with $CXX_COMPILER"

    cmake -S . -B "$DEBUG_DIR" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_COMPILER="$CXX_COMPILER" \
        -DROGUEPUPU_BUILD_TESTS=ON \
        -DROGUEPUPU_ENABLE_ASAN=ON

    ln -sfn "$DEBUG_DIR/compile_commands.json" "$PROJECT_ROOT/compile_commands.json"
}

build_debug() {
    configure_debug

    current_command="build debug"
    log "Building Debug"
    cmake --build "$DEBUG_DIR" --parallel "$BUILD_JOBS"
}

configure_release() {
    current_command="configure release"
    log "Configuring Release with $CXX_COMPILER"

    cmake -S . -B "$RELEASE_DIR" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER="$CXX_COMPILER" \
        -DROGUEPUPU_BUILD_TESTS=OFF \
        -DROGUEPUPU_ENABLE_ASAN=OFF
}

build_release() {
    configure_release

    current_command="build release"
    log "Building Release"
    cmake --build "$RELEASE_DIR" --parallel "$BUILD_JOBS"
}

run_tests() {
    # Always build the test target first, so tests cannot silently be stale.
    configure_debug

    current_command="build tests"
    log "Building tests"
    cmake --build "$DEBUG_DIR" --target roguepupu2_tests --parallel "$BUILD_JOBS"

    current_command="run tests"
    log "Running tests"
    ctest --test-dir "$DEBUG_DIR" --output-on-failure
}

run_debug() {
    build_debug

    current_command="run debug"
    log "Running Debug executable"
    "$DEBUG_DIR/bin/roguepupu2"
}

run_release() {
    build_release

    current_command="run release"
    log "Running Release executable"
    "$RELEASE_DIR/bin/roguepupu2"
}

clean() {
    current_command="clean"
    log "Removing build directories"
    rm -rf "$DEBUG_DIR" "$RELEASE_DIR"
    rm -f "$PROJECT_ROOT/compile_commands.json"
}

print_usage() {
    cat <<'EOF'
Usage: ./scripts/build.sh COMMAND [COMMAND...]

Commands:
  debug          Configure and build Debug with ASan and tests
  release        Configure and build Release without ASan or tests
  test           Configure, build, and run tests through CTest
  run            Build and run the Debug executable
  run-release    Build and run the Release executable
  clean          Remove generated build directories
  all            Build Debug, run tests, and build Release
  help           Show this message

Environment:
  CXX=<compiler>       C++ compiler to use (default: clang++)
  BUILD_JOBS=<count>   Parallel build jobs (default: nproc)

Examples:
  ./scripts/build.sh debug test
  CXX=g++ ./scripts/build.sh clean debug
  BUILD_JOBS=4 ./scripts/build.sh release
EOF
}

if (($# == 0)); then
    print_usage
    exit 0
fi

for command in "$@"; do
    case "$command" in
        debug)
            build_debug
            ;;
        release)
            build_release
            ;;
        test)
            run_tests
            ;;
        run)
            run_debug
            ;;
        run-release)
            run_release
            ;;
        clean)
            clean
            ;;
        all)
            build_debug
            run_tests
            build_release
            ;;
        help|-h|--help)
            print_usage
            ;;
        *)
            printf 'Unknown command: %s\n\n' "$command" >&2
            print_usage >&2
            exit 2
            ;;
    esac
done

