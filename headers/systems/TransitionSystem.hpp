#pragma once

#include "entt.hpp"

namespace TransitionSystem
{
	void transition(entt::registry& registry, const entt::entity entity, const entt::entity portal);
	void link_portals(entt::registry& registry, const entt::entity a, const entt::entity b);
	entt::entity get_destination(const entt::registry& registry, const entt::entity portal);
	void connect_next_level(entt::registry& registry, const entt::entity sink);
};
