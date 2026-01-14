#pragma once

#include "entt.hpp"
#include "Event.hpp"

namespace AbilitySystem
{
	bool on_cooldown(const entt::registry& registry, const Ability& ability);
	void use_ability(entt::registry& registry, const entt::entity actor, const std::string& ability_id, const Target& target);
};
