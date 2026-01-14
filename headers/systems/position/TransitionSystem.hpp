#pragma once

#include "entt.hpp"

class Cell;

namespace TransitionSystem
{
	void transition(entt::registry& registry, const entt::entity entity, const entt::entity portal);
	void link_portals(entt::registry& registry, const entt::entity a, const entt::entity b);
	entt::entity get_destination(const entt::registry& registry, const entt::entity portal);
	Cell* get_destination_cell(entt::registry& registry, const entt::entity portal);
};
