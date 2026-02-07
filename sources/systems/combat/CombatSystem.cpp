
#include "external/entt/entt.hpp"
#include "systems/combat/AttackSystem.hpp"
#include "systems/combat/DamageSystem.hpp"
#include "domain/Attack.hpp"
#include "domain/Damage.hpp"

namespace CombatSystem
{
	void attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender, const Attack* attack, const entt::entity weapon)
	{
		Damage damage = AttackSystem::get_attack_damage(registry, attacker, *attack);
		(void) weapon;
		DamageSystem::take_damage(registry, defender, damage);
	}
};
