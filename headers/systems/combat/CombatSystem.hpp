#pragma once

#include "external/entt/fwd.hpp"

namespace CombatSystem
{
	void attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender, const std::pair<entt::entity, const Attack*> weapon_attack);
};
