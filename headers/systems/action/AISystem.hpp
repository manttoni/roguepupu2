#pragma once

#include "external/entt/fwd.hpp"
#include "domain/Intent.hpp"

namespace AISystem
{
	Intent get_npc_intent(const entt::registry& registry, const entt::entity npc);
};
