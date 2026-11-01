# syntax=docker/dockerfile:1

# Build from the repository root:
#   docker build -t roguepupu2 .
# Run in a terminal (the game uses ncurses):
#   docker run --rm -it -e TERM="${TERM:-xterm-256color}" roguepupu2

FROM ubuntu:24.04 AS build

ARG BUILD_JOBS=2

RUN apt-get update \
	&& apt-get install -y --no-install-recommends \
		build-essential \
		cmake \
		libncurses-dev \
		nlohmann-json3-dev \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /src

# Copy the files needed to compile the game.
COPY CMakeLists.txt ./
COPY headers/ ./headers/
COPY sources/ ./sources/

RUN cmake -S . -B /build \
		-DCMAKE_BUILD_TYPE=Release \
		-DROGUEPUPU_BUILD_TESTS=OFF \
		-DROGUEPUPU_ENABLE_ASAN=OFF \
	&& cmake --build /build --parallel "${BUILD_JOBS}"

FROM ubuntu:24.04 AS runtime

RUN apt-get update \
	&& apt-get install -y --no-install-recommends \
		libncursesw6 \
		libstdc++6 \
		ncurses-term \
	&& rm -rf /var/lib/apt/lists/* \
	&& useradd --create-home --user-group game \
	&& mkdir -p /app/logs \
	&& chown game:game /app /app/logs

ENV LANG=C.UTF-8 \
	TERM=xterm-256color

WORKDIR /app

COPY --from=build /build/bin/roguepupu2 /usr/local/bin/roguepupu2
COPY --chown=game:game data/ ./data/

USER game

CMD ["/usr/local/bin/roguepupu2"]
