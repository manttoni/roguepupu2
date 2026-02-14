# Roguepupu 2

C++ roguelike cave exploration game and engine focused on ECS architecture, data-driven design, and terminal rendering.

## Features
- ncurses-based rendering and UI
- Entity Component System using EnTT
- Data-driven configuration with JSON
- Automated unit and integration tests (Google Test)
- Cave generation using Perlin noise

## Tech Stack
- C++
- ncurses
- EnTT
- nlohmann::json
- Google Test
- CMake

### Build
```bash
git clone https://github.com/manttoni/roguepupu2
cd roguepupu2
mkdir build && cd build
cmake ..
cd ..
cmake --build build
```

### Run
```bash
./build/roguepupu2
```

## Tests
```bash
./build/roguepupu2_tests
```
