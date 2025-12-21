#include "systems/TransitionSystem.hpp"
#include "entt.hpp"
#include "Components.hpp"

namespace TransitionSystem
{
	void transition(entt::registry& registry, const entt::entity entity, const entt::entity portal)
	{
		(void) registry; (void) entity; (void) portal;
	}

	bool is_portal(const entt::registry& registry, const entt::entity entity)
	{
		return registry.all_of<Portal>(entity);
	}
};
