#pragma once

#include "external/entt/fwd.hpp"
#include "domain/Position.hpp"

namespace LiquidSystem
{
	void simulate_liquids(entt::registry& registry, size_t cave_idx = Position::invalid_idx);
	double get_liquids_volume(const entt::registry& registry, const size_t cave_idx);
};
