#pragma once

#include "external/entt/fwd.hpp"
#include "domain/Position.hpp"

namespace TransitionSystem
{
	void link_portals(entt::registry& registry, const entt::entity a, const entt::entity b);
	void unlink_portals(entt::registry& registry, const entt::entity a, const entt::entity b);
	entt::entity get_destination(const entt::registry& registry, const entt::entity portal);
	Position get_or_create_destination_position(entt::registry& registry, const entt::entity portal);
};
