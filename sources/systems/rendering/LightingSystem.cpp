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
	void apply_light(entt::registry& registry, const entt::entity light)
	{
		const auto& [color, radius] = registry.get<Color, Radius>(light);
		const auto& position = ECS::get_position(registry, light);
		auto& cave = ECS::get_cave(registry, position);
		const auto& lit_positions = cave.get_nearby_positions(registry, position, radius);
		for (const auto& lp : lit_positions)
			cave.get_cell(lp).add_light(color);
	}

	void apply_lights(entt::registry& registry, const size_t cave_idx)
	{
		const auto light_sources = get_entities<Lights>(registry, cave_idx);
		for (const auto ls : light_sources)
		{
			for (const auto light : registry.get<Lights>(ls).entities)
				apply_light(registry, light);
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
