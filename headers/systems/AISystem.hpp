#pragma once

#include "Intent.hpp"
#include "entt.hpp"

namespace AISystem
{
	Intent get_npc_intent(const entt::registry& registry, const entt::entity npc);
};
