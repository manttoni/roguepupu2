#pragma once

#include <vector>

#include "external/entt/fwd.hpp"
#include "domain/Action.hpp"

namespace AISystem
{
	std::vector<Action> get_actions(const entt::registry& registry, const entt::entity npc);
};
