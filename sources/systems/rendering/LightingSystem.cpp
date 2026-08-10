#include <stddef.h>
#include <map>
#include <utility>
#include <vector>

#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/rendering/LightingSystem.hpp"
#include "domain/Color.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/Position.hpp"
#include "external/entt/entity/fwd.hpp"

namespace LightingSystem
{
	void clear_lights(entt::registry& registry)
	{
		auto& lightmap = registry.ctx().get<LightingSystem::Data>().lightmap;
		std::ranges::fill(lightmap, Color{});
	}
	void apply_lights(entt::registry& registry, const size_t cave_idx)
	{
		auto& cave = ECS::get_cave(registry, cave_idx);
		auto& data = registry.ctx().get<LightingSystem::Data>();
		if (data.cave_idx != cave_idx)
		{
			data.cave_idx = cave_idx;
			data.lightmap.resize(cave.get_size());
			clear_lights(registry);
		}
		auto& lightmap = data.lightmap;
		const auto light_entities = ECS::get_entities<Component::Tag::LightSource>(registry, cave_idx);
		for (const auto light_entity : light_entities)
		{
			const auto& position = registry.get<Position>(light_entity);
			const auto& entity_lights = registry.get<Component::List::Lights>(light_entity);
			for (const auto light : entity_lights.values)
			{
				const Color applied = registry.get<Color>(light) * registry.get<Component::Value::Intensity>(light).value;
				const auto illuminated_positions =
					VisionSystem::get_visible_positions(
							registry,
							position,
							registry.get<Component::Value::Radius>(light).value
							);
				for (const auto pos : illuminated_positions)
					lightmap[pos.cell_idx] += applied;
			}
		}
	}



	void reset_lights(entt::registry& registry, const size_t cave_idx)
	{
		clear_lights(registry);
		apply_lights(registry, cave_idx);
	}
};
