# Roguepupu 2

**A C++20 roguelike and simulation engine for exploring procedurally generated caves in the terminal.**

Roguepupu 2 (RP2) combines ASCII rendering, an Entity Component System, and JSON-defined content. Its world grows in chunks as the player explores, while game actions produce trees of events and consequences. The project is a place to build both a game and the systems behind it: world generation, movement, turns, entities, and content tools.

> **Status:** Early development. The current ECS rewrite is an exploration and engine prototype. You can generate a world, move through it, and edit entity definitions; it is not yet a complete roguelike with a full gameplay loop.

## What is implemented

- **Procedural caves:** Seed-based, layered Perlin noise generation across a growing world grid.
- **Chunk loading:** 50 × 50 cells per chunk; the player's chunk and eight neighbors are generated as needed.
- **Eight-direction movement:** Terrain and entity collision, movement points, and diagonal corner checks.
- **ECS entities:** EnTT components, tags, resources, and component dependencies, with definitions loaded from JSON.
- **Event-driven simulation:** Actions are dispatched through systems and recorded with their outcomes and consequences.
- **Turn infrastructure:** Actor scheduling and movement/action resource resets.
- **Terminal interface:** ncurses rendering, menus, dialogs, editable fields, and an entity definition editor.
- **Tests:** Google Test coverage for dice parsing, movement, and input handling.

## Quick start

The commands below target **Ubuntu 24.04**, including Ubuntu under **WSL2**. Use a terminal with 256-color support and a configurable color palette, such as Windows Terminal when running through WSL.

### 1. Install dependencies

```bash
sudo apt update
sudo apt install -y \
    build-essential clang cmake git \
    libncurses-dev nlohmann-json3-dev libgtest-dev
```

| Dependency | Purpose | Installation |
| --- | --- | --- |
| C++20 compiler | Compile the game | `clang` or `g++` from `build-essential` |
| CMake 3.25 or newer | Configure builds and tests | `cmake` |
| Make | Build with CMake's default Unix generator | Included in `build-essential` |
| ncurses development files | Terminal interface; links `ncursesw` and `panelw` | `libncurses-dev` |
| nlohmann/json | Read and write JSON content | `nlohmann-json3-dev` |
| Google Test | Build the test executable | `libgtest-dev`; optional when tests are disabled |
| Git | Clone the repository | `git` |
| Bash and GNU coreutils | Run the convenience build script | Normally installed on Ubuntu |
| EnTT | Entity Component System | Bundled in `headers/external/entt/` |
| siv::PerlinNoise | Procedural generation | Bundled in `headers/external/PerlinNoise.hpp` |

EnTT and PerlinNoise need no separate installation. Linux's math library is linked automatically. The build script uses `clang++` by default; GCC can be selected with `CXX=g++`.

### 2. Clone and play

```bash
git clone https://github.com/manttoni/roguepupu2.git
cd roguepupu2
./scripts/build.sh run-release
```

This configures a Release build, compiles the game, and starts it from the repository root.

To build first and launch separately:

```bash
./scripts/build.sh release
./build-release/bin/roguepupu2
```

**Run the executable from the repository root.** Runtime files are loaded through relative paths under `data/`; they are not copied beside the executable.

## How to play

1. Select **New Game** using the up/down arrows and Enter.
2. Enter a seed of 1–10 characters and press Enter.
3. Explore as `@`. The camera follows the player and nearby chunks load as you move.
4. Press Esc to return to the main menu. **Continue** resumes the current session.
5. Choose **Exit** and confirm to close the application.

Continue keeps the current game in memory only. There is no save/load system yet; exiting the application loses the session.

### Game controls

| Key | Action |
| --- | --- |
| Arrow keys | Move north, south, west, or east |
| Home | Move northwest |
| Page Up | Move northeast |
| End | Move southwest |
| Page Down | Move southeast |
| Space | End the current turn |
| Esc | Return to the main menu |
| Backslash (`\`) | Toggle player movement collision for debugging |

Numpad movement may work with Num Lock off if your terminal sends the corresponding navigation keys. Plain number keys are not movement bindings.

Diagonal movement requires both adjacent orthogonal cells to be passable. If movement points run out, press Space to begin the next turn and reset turn resources. If the starting position leaves you blocked by terrain, use the collision toggle to move to an open floor cell, then toggle it back.

### Map symbols

| Symbol | Meaning |
| --- | --- |
| `@` | Default player |
| `.` | Stone floor |
| `#` | Wall |
| `~` | Floor with water |

The supplied generator configuration currently disables the lake layer.

### Menu and editor controls

| Key | Action |
| --- | --- |
| Up / Down | Select a field or button |
| Enter | Activate a button, toggle a checkbox, or confirm a supported text field |
| Left / Right | Adjust a numeric value or cycle an enum value |
| Shift + Left / Right | Adjust numeric values by a larger step, where supported by the terminal |
| Backspace | Delete the last character in a text field |
| Esc | Cancel or return |

Mouse input is not enabled. The main menu's **Settings** and **Controls** entries are placeholders.

## Build and test

```bash
./scripts/build.sh debug    # Debug build with AddressSanitizer and tests
./scripts/build.sh test     # Build the test target and run it through CTest
./scripts/build.sh run      # Build and launch the Debug game
./scripts/build.sh all      # Build Debug, run tests, then build Release
```

Additional commands are `release`, `run-release`, `clean`, and `help`. Multiple commands can be supplied in order. `clean` removes both build directories and the generated compile database symlink.

Set the compiler and parallelism through environment variables:

```bash
BUILD_JOBS=4 ./scripts/build.sh release
CXX=g++ ./scripts/build.sh clean debug
```

Clean or use a fresh build directory when changing compilers. Debug builds also provide `compile_commands.json` at the repository root for editor tooling.

### Build directly with CMake

The helper script is optional. For a Release game without tests:

```bash
cmake -S . -B build-release \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DROGUEPUPU_BUILD_TESTS=OFF \
    -DROGUEPUPU_ENABLE_ASAN=OFF
cmake --build build-release --parallel 4
./build-release/bin/roguepupu2
```

| CMake option | Default | Purpose |
| --- | --- | --- |
| `ROGUEPUPU_BUILD_TESTS` | `ON` | Build Google Test tests |
| `ROGUEPUPU_ENABLE_ASAN` | `OFF` | Enable AddressSanitizer |
| `ROGUEPUPU_WARNINGS_AS_ERRORS` | `OFF` | Treat compiler warnings as errors |

For an existing Debug build, tests can also be run with `ctest --test-dir build-debug --output-on-failure`.

## Content and configuration

| Path | Purpose |
| --- | --- |
| `data/entity/definitions.json` | Entity templates, including `player_default` |
| `data/generator/conf.json` | Ordered world generation layers |
| `data/ui/theme.json` | Menu colors |
| `headers/game/components/` | Component definitions and dependencies |

### World generation

Layers are evaluated in file order at each global cell position. When a layer's noise value is **less than or equal to** its `threshold`, it replaces the cell's material and form and adds its `water_depth`.

Supported layer fields are `id`, `material`, `form`, `frequency`, `octaves`, `persistence`, `threshold`, `water_depth`, and `disabled`. Later matching layers can overwrite earlier terrain. A layer does not currently spawn entities through `entity_id`.

The active generator configuration is `data/generator/conf.json`; the root-level `conf.json` is not the default input for the current generator. Restart with a new game after changing generation settings.

### Entity editor

Open **Editor** from the main menu to create, search, and modify definitions. The basic workflow is **Load → Edit → Add → Write**:

- **Edit** changes the active definition; Confirm applies those edits and Cancel reverts them.
- **Add** places the active definition into the editor's in-memory collection.
- **Write** saves that collection to `data/entity/definitions.json`.
- **Read** reloads the collection from disk.

The editor is still incomplete, especially for arrays and tags. Restart the application after writing definitions so its entity database loads the changes.

## Architecture

RP2 separates terminal presentation from game state and rules:

| Area | Responsibility |
| --- | --- |
| `application/` | Main menu, player input, game loop, event history, and descriptions |
| `game/Simulation` | Owns the registry, world grid, generator, player, and scheduler |
| `game/systems/` | Validates and processes events, producing consequences |
| `game/world/` | Cells, chunks, generation, and typed coordinate conversions |
| `rendering/` and `ncurses/` | Map rendering and terminal resources |
| `ui/` and `editor/` | Reusable interface elements and entity definition tools |

A directional key creates a `Bump` event. A valid bump produces `Move`, which changes position and produces movement-point loss, leave-position, and enter-position consequences. The simulation processes these into an event tree with an outcome for each node. This provides a foundation for adding interactions without putting game rules into input or rendering code.

## Development direction

The intended setting combines post-apocalyptic fantasy with an underground world: caves, glowing mushrooms, ruins, and interacting creatures. Future work includes world population, richer interactions and combat, lighting and visibility, and multiple world levels. These are development goals rather than finished features.

## Troubleshooting

- **Missing JSON files:** Launch from the repository root and keep `data/` in place.
- **`clang++` not found:** Install `clang`, or select GCC with `CXX=g++`.
- **CMake version too old:** Check `cmake --version`; RP2 requires at least 3.25.
- **Missing `ncursesw`, `panelw`, or JSON headers:** Install the development packages listed above.
- **Google Test not found:** Install `libgtest-dev`, or disable tests for a game-only CMake build.
- **Build killed due to memory use:** Reduce parallelism, for example `BUILD_JOBS=2 ./scripts/build.sh debug`.
- **Colors look wrong:** Check the terminal's color and palette support. Runtime diagnostics are written to `logs/logs.log`.
- **Slow Esc response inside tmux:** Add `set -s escape-time 10` to your tmux configuration and reload it.

Build output is also appended to `logs/build.log` when using the helper script.

## Feedback

Bug reports and design discussion are welcome through [GitHub Issues](https://github.com/manttoni/roguepupu2/issues). For bugs, include the commit, build command, compiler version, terminal, seed, reproduction steps, and relevant log output.
