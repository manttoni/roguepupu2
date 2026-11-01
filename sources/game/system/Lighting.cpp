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
		const int multiplier)
	{
		const auto& registry = simulation.get_registry();

		const auto radius =
			registry.get<Component::Value::Radius>(light).value;
		const auto& color =
			registry.get<Component::Value::Color>(light).value;

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
			auto& light_color = lightmap[np];

			light_color.r = std::max(light_color.r, multiplier * color.r);
			light_color.g = std::max(light_color.g, multiplier * color.g);
			light_color.b = std::max(light_color.b, multiplier * color.b);
		}
	}

	void update(
		Simulation& simulation,
		const entt::entity entity,
		const Operation operation)
	{
		const auto multiplier = operation == Operation::Add ? 1 : -1;

		auto& registry = simulation.get_registry();

		if (!registry.all_of<Component::Value::Position>(entity))
			return;
		const auto& position =
			registry.get<Component::Value::Position>(entity).value;
		/* This would actually be a bug
		 * if (registry.all_of<Component::Tag::Light>(entity))
		{
			update_light(simulation, entity, position, multiplier);
			return;
		}*/
		if (!registry.all_of<Component::Tag::LightSource>(entity))
			return;
		/*if (const auto* equipped_items =
		  registry.try_get<Component::List::EquippedItems>(entity))
		  {
		  for (const auto& item : equipped_items->values)
		  update(simulation, item);
		  }*/
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
				update_light(simulation, light_entity, position, multiplier);
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
