#pragma once

#include "external/entt/fwd.hpp"

namespace CombatSystem
{
	void attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender);
	bool is_in_combat(const entt::registry& registry, const entt::entity entity);
};
