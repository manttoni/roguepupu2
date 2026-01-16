#include "systems/TransitionSystem.hpp"
#include "World.hpp"
#include "Cave.hpp"
#include "ECS.hpp"
#include "entt.hpp"
#include "Components.hpp"
#include "Utils.hpp"

namespace TransitionSystem
{
	void transition(entt::registry& registry, const entt::entity entity, const entt::entity portal)
	{
		(void) registry; (void) entity; (void) portal;
	}

	void link_portals(entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		registry.emplace_or_replace<Transition>(a, b);
		registry.emplace_or_replace<Transition>(b, a);
	}

	entt::entity get_destination(const entt::registry& registry, const entt::entity portal)
	{
		if (!registry.all_of<Transition>(portal))
			return entt::null;
		return registry.get<Transition>(portal).destination;
	}

	Cell* get_destination_cell(entt::registry& registry, const entt::entity portal)
	{
		entt::entity destination = get_destination(registry, portal);
		if (destination == entt::null)
		{
			Cave& next_cave = ECS::get_active_cave(registry)->get_world()->get_cave();
			destination = ECS::get_source(registry, next_cave);
			link_portals(registry, portal, destination);
		}
		return ECS::get_cell(registry, destination);
	}
};
