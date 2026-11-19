#pragma once

#include "game/world/Position.hpp"

#include <cstdint>
#include <string_view>

namespace Random
{
	std::uint64_t hash_seed(std::string_view seed);

	// Returns a deterministic value in [0, 1).
	double at(
		std::uint64_t seed,
		const Game::World::GlobalPosition& position,
		std::uint64_t stream = 0);

	void seed(std::uint32_t value);
	int integer(int min, int max);
}
