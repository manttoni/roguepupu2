#include "external/entt/entt.hpp"
#include "components/Components.hpp"
#include "utils/ECS.hpp"

#define GATHERING_RANGE 1

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
		if (!registry.all_of<Position>(gatherer) || !registry.all_of<Position>(gatherable))
			return false;
		if (!registry.all_of<Gatherable>(gatherable))
			return false;
		if (!has_tool(registry, gatherer, gatherable))
			return false;
		if (ECS::distance(registry, gatherer, gatherable) > GATHERING_RANGE)
			return false;
		return true;
	}

	void gather(entt::registry& registry, const entt::entity gatherer, const entt::entity gatherable)
	{
		if (!can_gather(registry, gatherer, gatherable))
			return;

		const Gatherable& component = registry.get<Gatherable>(gatherable);
		for (size_t i = 0; i < component.amount; ++i)
		{	// remake this. Gathering can give rare loot by chance, multiple things etc..
			ECS::queue_event(registry, Event(
						{.entity = gatherer},
						{.type = Effect::Type::Gather, .entity_id = component.entity_id},
						{.entity = gatherable}
						));
		}

		registry.erase<Gatherable>(gatherable);
	}
};
