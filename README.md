# Roguepupu 2

C++ roguelike cave exploration game and engine focused on ECS architecture, data-driven design, and terminal rendering.

## Features
- ncurses-based rendering and UI
- Entity Component System using EnTT
- Data-driven configuration with JSON
- Automated unit and integration tests (Google Test)
- Infinite world generation using Perlin noise

## Tech Stack
- C++
- ncurses
- EnTT
- nlohmann::json
- Google Test
- CMake

### Play
```bash
./scripts/build.sh release
./build-release/bin/roguepupu2
```

### Test
```bash
./scripts/build.sh debug test
```
