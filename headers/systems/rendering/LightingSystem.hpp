#pragma once

#include "external/entt/fwd.hpp"

class Cave;
class Cell;
namespace LightingSystem
{
	void apply_lights(entt::registry& registry, const size_t cave_idx);
	void reset_lights(entt::registry& registry, const size_t cave_idx);
	double get_illumination(const Cell& cell);
};
