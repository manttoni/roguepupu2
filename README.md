# Roguepupu 2

A terminal roguelike written in C++20 with ncurses and EnTT. It has procedural world generation, lighting and visibility, item interactions, and an in-game entity editor.

The game is in early development. Expect bugs and frequent changes.

Use a 256-color terminal that supports changing the color palette, including when running through Docker.

## Run with Docker

Requires Git and [Docker](https://docs.docker.com/engine/install/).

```bash
git clone https://github.com/manttoni/roguepupu2.git
cd roguepupu2
docker build -t roguepupu2 .
docker run --rm -it -e TERM="${TERM:-xterm-256color}" roguepupu2
```

Use `sudo docker` if your installation requires it. Editor changes made inside this disposable container are discarded when it exits.

## Build and run natively

On Ubuntu 24.04 or Ubuntu under WSL2:

```bash
sudo apt update
sudo apt install build-essential clang cmake git libncurses-dev nlohmann-json3-dev

git clone https://github.com/manttoni/roguepupu2.git
cd roguepupu2
./scripts/build.sh run-release
```

Requires a C++20 compiler and CMake 3.25 or newer. To launch an existing build, run `./build/release/bin/roguepupu2` from the repository root so the game can find `data/`.

For tests, install `libgtest-dev` and run `./scripts/build.sh test`.

## Basic controls

Choose **New Game** and enter a seed to start. Use the arrow keys and Enter in menus.

| Key | Action |
| --- | --- |
| Arrow keys | Move |
| Home / Page Up / End / Page Down | Move diagonally |
| Enter | Open the interaction menu |
| Space | End the turn |
| Esc | Cancel or return to the main menu |

There is no save/load system yet.

Report bugs through [GitHub Issues](https://github.com/manttoni/roguepupu2/issues). Logs are written to `logs/logs.log`.
