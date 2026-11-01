#include "game/system/Lighting.hpp"
#include "game/entity/Entity.hpp"
#include "game/system/Vision.hpp"
#include "external/entt/entt.hpp"
#include "game/Simulation.hpp"
#include "game/world/Position.hpp"
#include "game/component/Component.hpp"

namespace Game::System::Lighting
{
	void update_light(
		Simulation& simulation,
		const entt::entity light,
		const World::GlobalPosition& position,
		const Operation operation)
	{
		const auto& registry = simulation.get_registry();

		const auto radius =
			registry.get<Component::Value::Radius>(light).value;
		const auto& color =
			registry.get<Component::Value::Color>(light).value;
		const auto& color_id = color.id();

		const auto nearby_positions =
			World::nearby(position, radius);
		auto& lightmap =
			simulation.get_lightmap();
		const auto& grid =
			simulation.get_grid();

		for (const auto& np : nearby_positions)
		{
			if (!Vision::los(registry, grid, position, np))
				continue;

			if (operation == Operation::Add)
			{
				++lightmap[np][color_id];
				continue;
			}

			const auto position_it = lightmap.find(np);
			if (position_it == lightmap.end())
				continue;

			auto& colors = position_it->second;
			const auto color_it = colors.find(color_id);
			if (color_it == colors.end() || color_it->second == 0)
				continue;

			if (--color_it->second == 0)
				colors.erase(color_it);

			if (colors.empty())
				lightmap.erase(position_it);
		}
	}

	void update(
		Simulation& simulation,
		const entt::entity entity,
		const Operation operation)
	{
		auto& registry = simulation.get_registry();

		const auto* position =
			registry.try_get<Component::Value::Position>(entity);
		if (!position)
			return;
		if (!registry.all_of<Component::Tag::LightSource>(entity))
			return;

		if (const auto* lights =
			registry.try_get<Component::List::Lights>(entity);
			lights)
		{
			const auto& definitions =
				simulation.get_entity_database();
			for (const auto& light_id : lights->values)
			{
				const auto light_entity =
					Entity::create(registry, definitions, light_id);

				update_light(
					simulation,
					light_entity,
					position->value,
					operation);

				registry.destroy(light_entity);
			}
		}
	}

	void reset(Simulation& simulation)
	{
		const auto& registry = simulation.get_registry();
		auto& lightmap = simulation.get_lightmap();
		lightmap.clear();

		for (const auto& light_source :
			Entity::find_all(
				registry,
				Component::Tag::LightSource{}))
		{
			update(simulation, light_source, Operation::Add);
		}
	}
}
