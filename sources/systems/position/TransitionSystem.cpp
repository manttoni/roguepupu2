#include "components/Components.hpp"
#include "domain/Position.hpp"
#include "domain/Cave.hpp"
#include "domain/World.hpp"
#include "external/entt/entt.hpp"
#include "generation/CaveGenerator.hpp"
#include "utils/ECS.hpp"
#include "utils/Log.hpp"

namespace TransitionSystem
{
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

	entt::entity get_destination(const entt::registry& registry, const entt::entity portal)
	{
		if (!registry.all_of<Transition>(portal))
			Log::error("Entity is not a portal");
		return registry.get<Transition>(portal).destination;
	}

	/* If destination is entt::null, then portal is not linked to anything
	 * In such a case, treat it as a passage between Caves and create a Cave to link to.
	 * */
	Position get_or_create_destination_position(entt::registry& registry, const entt::entity portal)
	{
		entt::entity destination = get_destination(registry, portal);
		if (destination == entt::null)
		{
			// add a new cave to world and get index in deque
			const size_t cave_idx = ECS::get_world(registry).new_cave();

			// Create generation settings (or use default)
			CaveGenerator::Data data(registry, ECS::get_cave(registry, cave_idx));

			// generate cave
			CaveGenerator::generate_cave(data);

			// Cave is now ready, link current cave to the new one via passage
			destination = ECS::get_unlinked_passage(registry, cave_idx);
			link_portals(registry, portal, destination);
		}
		return registry.get<Position>(destination);
	}
};
