#pragma once

#include <cstdint>
#include "Event.hpp"
#include "external/entt/fwd.hpp"

namespace AISystem
{
	Intent get_npc_intent(entt::registry& registry, const entt::entity npc);
};
