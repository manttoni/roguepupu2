#pragma once

#include "external/entt/fwd.hpp"

namespace LightingSystem
{
	/* Only one of these is needed at a time,
	 * When player changes cave, reset it
	 * */
	struct Data
	{
		size_t cave_idx;
		std::vector<Color> lightmap;
	};
	void apply_lights(entt::registry& registry, const size_t cave_idx);
	void clear_lights(entt::registry& registry);
	void reset_lights(entt::registry& registry, const size_t cave_idx);
};
