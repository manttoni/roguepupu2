#pragma once

#include "entt.hpp"
#include "Event.hpp"

namespace AbilitySystem
{
	bool on_cooldown(const entt::registry& registry, const Ability& ability);
	void use_ability(entt::registry& registry, const entt::entity actor, Ability& ability, const Target& target);
};
