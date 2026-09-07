#pragma once

#include "external/entt/fwd.hpp"


                namespace System::Combat
            
{
	double get_attack_range(const entt::registry& registry, const entt::entity weapon);
	bool can_attack_with(const entt::registry& registry, const entt::entity attacker, const entt::entity defender, const entt::entity weapon);
	bool can_attack(const entt::registry& registry, const entt::entity attacker, const entt::entity defender);
	void attack_with(entt::registry& registry, const entt::entity attacker, const entt::entity defender, const entt::entity weapon);
	void attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender);
};
