#pragma once

#include "entt.hpp"
#include "Event.hpp"

namespace AISystem
{
	Intent get_npc_intent(entt::registry& registry, const entt::entity npc);
};
