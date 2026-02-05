#include "external/entt/entt.hpp"
#include "systems/combat/AttackSystem.hpp"
#include "systems/state/StateSystem.hpp"
#include "domain/Attack.hpp"

namespace AttackSystem
{
	Damage get_attack_damage(const entt::registry& registry, const entt::entity attacker, const Attack& attack)
	{
		const auto damage_attributes_sum = StateSystem::get_attributes_sum(registry, attacker, attack.damage_attributes);
		Damage calculated = attack.base_damage;
		calculated.amount *= static_cast<size_t>(1.0 + static_cast<double>(damage_attributes_sum) / 100.0);
		return calculated;
	}
	double get_attack_hit_chance(const entt::registry& registry, const entt::entity attacker, const Attack& attack)
	{
		const auto hit_chance_attributes_sum = StateSystem::get_attributes_sum(registry, attacker, attack.hit_chance_attributes);
		return 1.0 + static_cast<double>(hit_chance_attributes_sum) / 100.0;
	}
};
