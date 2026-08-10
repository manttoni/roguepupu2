#pragma once

#include "domain/Action.hpp"
#include "external/entt/fwd.hpp"
#include "domain/Position.hpp"

namespace InputSystem
{
	std::vector<Action> bump_actions(const entt::registry& registry, const Position& position);
	Position get_target(const entt::registry& registry);
	std::vector<Action> get_actions(const entt::registry& registry);
}
