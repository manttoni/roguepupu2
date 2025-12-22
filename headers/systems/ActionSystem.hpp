#pragma once

#include "entt.hpp"
#include "Intent.hpp"
class Cell;

namespace ActionSystem
{
	void player_turn(entt::registry& registry);
	void npc_turn(entt::registry& registry);
	bool can_act(const entt::registry& registry, const entt::entity actor);
	void use_action(entt::registry& registry, const entt::entity actor);
};
