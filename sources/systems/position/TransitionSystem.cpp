#include <assert.h>
#include <stddef.h>
#include <string>

#include "components/Components.hpp"
#include "domain/Position.hpp"
#include "external/entt/entt.hpp"
#include "utils/ECS.hpp"
#include "external/entt/entity/fwd.hpp"
#include "systems/position/TransitionSystem.hpp"

namespace TransitionSystem
{
	void transition(entt::registry& registry, const entt::entity traveler, const entt::entity portal)
	{
		assert(registry.all_of<Transition>(portal));
		auto& comp = registry.get<Transition>(portal);
		if (comp.destination == entt::null)
		{	// This might be an unlinked next level portal
			if (registry.get<Name>(portal).name == "natural passage")
			{	// It is a passage to a cave
				const size_t cave_idx = ECS::generate_cave(registry);
				comp.destination = ECS::get_unlinked_passage(registry, cave_idx);
			}
			else return;
		}
		registry.emplace_or_replace<Position>(
				traveler,
				registry.get<Position>(comp.destination)
				);
	}

	void link_portals(entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		assert(registry.all_of<Transition>(a) && registry.all_of<Transition>(b));
		registry.replace<Transition>(a, b);
		registry.replace<Transition>(b, a);
	}

	void unlink_portals(entt::registry& registry, const entt::entity a, const entt::entity b)
	{
		assert(registry.all_of<Transition>(a) && registry.all_of<Transition>(b));
		registry.replace<Transition>(a, entt::null);
		registry.replace<Transition>(b, entt::null);
	}
};
