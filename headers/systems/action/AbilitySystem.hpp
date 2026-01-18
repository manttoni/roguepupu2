#pragma once

#include <string>
#include "external/entt/fwd.hpp"

struct Ability;
struct Actor;
struct Target;

namespace AbilitySystem
{
	bool has_ability(const entt::registry& registry, const entt::entity entity, const std::string& ability_id);
	bool on_cooldown(const entt::registry& registry, const Ability& ability);
	void use_ability(entt::registry& registry, const Actor& actor, const std::string& ability_id, const Target& target);
};
