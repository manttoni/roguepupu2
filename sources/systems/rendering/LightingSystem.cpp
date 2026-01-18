#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/rendering/LightingSystem.hpp"
#include "components/Components.hpp"
#include "domain/Color.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"

namespace LightingSystem
{
	void apply_lights(entt::registry& registry, const size_t cave_idx)
	{
		auto& cave = ECS::get_cave(registry, cave_idx);
		for (const auto entity : registry.view<Glow, Position, FGColor>())
		{
			const auto& [glow, position, color] = registry.get<Glow, Position, FGColor>(entity);
			if (position.cave_idx != cave_idx)
				continue;
			Color glow_color = color.color * glow.strength;

			for (const auto affected_pos : cave.get_nearby_positions(position, glow.radius))
			{
				if (!VisionSystem::has_line_of_sight(registry, affected_pos, position))
					continue;

				cave.get_cell(affected_pos).add_light(glow_color);
			}
		}
	}

	void reset_lights(entt::registry& registry, const size_t cave_idx)
	{
		auto& cave = ECS::get_cave(registry, cave_idx);
		cave.clear_lights();
		apply_lights(registry, cave_idx);
	}

	double get_illumination(const Cell& cell)
	{
		const auto& lights = cell.get_lights();
		size_t illumination = 0;
		for (const auto& [color, stacks] : lights)
		{
			const size_t intensity = color.get_r() + color.get_g() + color.get_b();
			illumination += intensity * stacks;
		}
		// All channels at maximum is 3000
		const double normalized = static_cast<double>(illumination) / 3000.0;
		return normalized;
	}
};
