#pragma once

#include <vector>

#include "external/entt/fwd.hpp"
#include "domain/Action.hpp"


namespace System::AI

{
	std::vector<Domain::Action::Any> get_actions(const entt::registry& registry, const entt::entity npc);
};
