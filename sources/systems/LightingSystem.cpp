#include "systems/LightingSystem.hpp"

namespace LightingSystem
{
	void apply_lights(const entt::registry& registry, const size_t cave_idx)
	{
		auto& world = registry.ctx().get<World>();
		auto& cave = world.get_cave(cave_idx);
		auto& cells = cave.get_cells();
		for (const auto entity : cave.get_cache())
		{
			if (!registry.all_of<Glow, Position, Color>(entity))
				continue;

			const auto& [glow, position, color] = registry.get<Glow, Position, FGColor>(entity);
			Color glow_color = color.color * glow.strength;

			for (const auto affected_idx : cave.get_nearby_ids(*position.cell, glow.radius))
			{
				if (!VisionSystem::has_line_of_sight(cells[affected_idx], *position.cell))
					continue;

				cells[affected_idx].add_light(glow_color);
			}
		}
	}

	void reset_lights(const entt::registry& registry, const size_t cave_idx)
	{
		auto& world = registry.ctx().get<World>();
		auto& cave = world.get_cave(cave_idx);
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
