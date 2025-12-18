#pragma once

#include "entt.hpp"  // for entity, registry, vector
struct Damage;  // lines 6-6

struct Damage;

namespace CombatSystem
{
	int get_attack_modifier(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon);
	int get_attack_roll(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon);
	std::vector<Damage> get_attack_damage(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon);
	void attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender);
};
