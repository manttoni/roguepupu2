#pragma once

#include "external/entt/entt.hpp"
#include "systems/state/EquipmentSystem.hpp"

struct Weapon;
namespace CombatSystem
{


	bool can_attack_with(const entt::registry& registry, const entt::entity attacker, const entt::entity defender, const entt::entity weapon);
	void melee_attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender);
	void ranged_attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender);
	void throwing_attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender);
	bool is_in_combat(const entt::registry& registry, const entt::entity entity);

	/* Use this to see if attacker can attack defender with a weapon that has T component.
	 * */
	template <typename T>
		bool can_attack(const entt::registry& registry, const entt::entity attacker, const entt::entity defender)
		{
			// Gets also unarmed weapons like fists
			const auto weapons = EquipmentSystem::get_all_equipped_weapons(registry, attacker);
			for (const auto w : weapons)
			{
				if (!registry.template all_of<T>(w))
					continue;

				if (can_attack_with(registry, attacker, defender, w))
					return true;
			}

			return false;
		}
	inline bool can_attack(const entt::registry& registry, const entt::entity attacker, const entt::entity defender)
	{
		return can_attack<Weapon>(registry, attacker, defender);
	}
};
