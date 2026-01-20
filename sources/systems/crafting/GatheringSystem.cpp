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

		// If gatherable has some effect that happens upon gathering apply it here
		// f.e. remove glow or change glyph or color etc...
		// Don't destroy here, eventsystem needs it to give loot
		// ...

		// This event will give the loot
		ECS::queue_event(registry, Event(
					{.entity = gatherer, .position = registry.get<Position>(gatherer)},
					{.type = Effect::Type::Gather},
					{.entity = gatherable, .position = registry.get<Position>(gatherable)}
					));
	}
};
