#include "utils/ECS.hpp"
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
		Event event = {.type = Event::Type::Attack};
		event.actor.entity = attacker;
		event.target.entity = defender;
		event.attack_id = attack->id;
		event.weapon = weapon;
		ECS::queue_event(registry, event);
		DamageSystem::take_damage(registry, defender, damage);
	}
};
