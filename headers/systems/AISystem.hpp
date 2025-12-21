#pragma once

#include "systems/ActionSystem.hpp"
#include "entt.hpp"

namespace AISystem
{
	ActionSystem::Intent get_npc_intent(const entt::registry& registry, const entt::entity npc);
};
