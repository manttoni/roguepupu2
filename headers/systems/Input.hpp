#pragma once

#include <vector>

#include "domain/Action.hpp"
#include "external/entt/fwd.hpp"
#include "domain/Position.hpp"


namespace System::Input
{
	std::vector<Domain::Action::Any> bump_actions(const entt::registry& registry, const Domain::Position& position);
	Domain::Position get_target(const entt::registry& registry);
	std::vector<Domain::Action::Any> get_actions(const entt::registry& registry);
}
