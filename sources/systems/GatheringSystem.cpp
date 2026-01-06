#include "systems/GatheringSystem.hpp"
#include "EntityFactory.hpp"
#include "Components.hpp"
#include "systems/InventorySystem.hpp"
#include "ECS.hpp"
#include "entt.hpp"

namespace GatheringSystem
{
	bool has_tool(const entt::registry& registry, const entt::entity gatherer, const entt::entity gatherable)
	{
		if (!registry.all_of<Inventory>(gatherer))
			return false;
		const auto tool_type = registry.get<Gatherable>(gatherable).tool_type;
		for (const auto item : registry.get<Inventory>(gatherer).inventory)
		{
			if (registry.all_of<Tool>(item) && registry.get<Tool>(item).type == tool_type)
				return true;
		}
		return false;
	}

	bool can_gather(const entt::registry& registry, const entt::entity gatherer, const entt::entity gatherable)
	{
		if (!registry.all_of<Gatherable>(gatherable))
			return false;
		if (!has_tool(registry, gatherer, gatherable))
			return false;
		if (ECS::distance(registry, gatherer, gatherable) > MELEE_RANGE)
			return false;
		return true;
	}

	void gather(entt::registry& registry, const entt::entity gatherer, const entt::entity gatherable)
	{
		if (!can_gather(registry, gatherer, gatherable))
			return;

		const Gatherable& component = registry.get<Gatherable>(gatherable);
		for (size_t i = 0; i < component.amount; ++i)
		{
			const auto e = EntityFactory::instance().create_entity(registry, component.entity_id);
			InventorySystem::add_item(registry, gatherer, e);
		}
		registry.ctx().get<EventQueue>().queue.push_back({
				.type = Event::Type::Gather,
				.actor = gatherer,
				.gatherable = gatherable
				});
		registry.erase<Gatherable>(gatherable);
	}
};
