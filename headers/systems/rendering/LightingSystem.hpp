#pragma once

#include "external/entt/fwd.hpp"

class Cave, Cell;
namespace LightingSystem
{
	void clear_lights(Cave& cave);
	void apply_lights(const entt::registry& registry, const size_t cave_idx);
	void reset_lights(const entt::registry& registry, const size_t cave_idx);
	double get_illumination(const Cell& cell);
};
