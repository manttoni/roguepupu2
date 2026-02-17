#include <stddef.h>
#include <map>
#include <utility>
#include <vector>

#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/rendering/LightingSystem.hpp"
#include "components/Components.hpp"
#include "domain/Color.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/Position.hpp"
#include "external/entt/entity/fwd.hpp"

namespace LightingSystem
{
	void apply_lights(entt::registry& registry, const size_t cave_idx)
	{
		auto& cave = ECS::get_cave(registry, cave_idx);
		for (const auto entity : registry.view<Glow, Position, Color>())
		{
			const auto& [glow, position, color] = registry.get<Glow, Position, Color>(entity);
			if (position.cave_idx != cave_idx)
				continue;
			Color glow_color = color * glow.intensity;

			cave.get_cell(position).add_light(glow_color);
			for (const auto affected_pos : cave.get_nearby_positions(position, glow.radius))
			{
				if (!VisionSystem::has_line_of_sight(registry, affected_pos, position))
					continue;

				cave.get_cell(affected_pos).add_light(glow_color);
			}
		}
	}

	void clear_lights(entt::registry& registry, const size_t cave_idx)
	{
		for (const auto pos : ECS::get_cave(registry, cave_idx).get_positions())
			ECS::get_cell(registry, pos).clear_lights();
	}

	void reset_lights(entt::registry& registry, const size_t cave_idx)
	{
		clear_lights(registry, cave_idx);
		apply_lights(registry, cave_idx);
	}

	double get_illumination(const Cell& cell)
	{
		const auto& lights = cell.get_lights();
		double illumination = 0;
		for (const auto& [color, stacks] : lights)
		{
			illumination += color.get_illumination() * static_cast<double>(stacks);
		}
		return illumination;
	}
};
