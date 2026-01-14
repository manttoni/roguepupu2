#pragma once

#include <cstdint>
#include <string>
#include "external/entt/fwd.hpp"

struct Ability;
struct Target;

namespace AbilitySystem
{
	bool on_cooldown(const entt::registry& registry, const Ability& ability);
	void use_ability(entt::registry& registry, const entt::entity user, const std::string& ability_id, const Target& target);
};
