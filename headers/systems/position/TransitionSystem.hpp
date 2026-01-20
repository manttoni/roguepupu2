#pragma once

#include "external/entt/fwd.hpp"
#include "domain/Position.hpp"

namespace TransitionSystem
{
	void transition(entt::registry& registry, const entt::entity traveler, const entt::entity portal);
	void link_portals(entt::registry& registry, const entt::entity a, const entt::entity b);
	void unlink_portals(entt::registry& registry, const entt::entity a, const entt::entity b);
};
