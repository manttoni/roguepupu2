#include "domain/Damage.hpp"
#include "core/common.hpp"
#include "external/entt/entt.hpp"
#include "systems/combat/DamageSystem.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/state/AlignmentSystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "systems/state/InventorySystem.hpp"
#include "utils/ECS.hpp"

namespace CombatSystem
{
	int get_throwing_range_mod(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon)
	{
		int modifier = 0;
		const auto dex = StateSystem::get_attribute_modifier<Dexterity>(registry, attacker);
		const auto str = StateSystem::get_attribute_modifier<Strength>(registry, attacker);
		const auto finesse = registry.all_of<FinesseWeapon>(weapon);
		const auto ranged = registry.all_of<RangedWeapon>(weapon);

		if ((finesse && dex > str) || ranged)
			modifier = dex;
		else
			modifier = str;
		return modifier;
	}

	int get_damage_mod(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon)
	{
		int modifier = 0;
		const auto dex = StateSystem::get_attribute_modifier<Dexterity>(registry, attacker);
		const auto str = StateSystem::get_attribute_modifier<Strength>(registry, attacker);
		const auto finesse = registry.all_of<FinesseWeapon>(weapon);
		const auto ranged = registry.all_of<RangedWeapon>(weapon);

		if ((finesse && dex > str) || ranged)
			modifier = dex;
		else
			modifier = str;
		return modifier;
	}

	int get_attack_mod(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon)
	{
		int modifier = 0;
		const auto dex = StateSystem::get_attribute_modifier<Dexterity>(registry, attacker);
		const auto str = StateSystem::get_attribute_modifier<Strength>(registry, attacker);
		const auto finesse = registry.all_of<FinesseWeapon>(weapon);
		const auto ranged = registry.all_of<RangedWeapon>(weapon);

		if ((finesse && dex > str) || ranged)
			modifier = dex;
		else
			modifier = str;
		return modifier;
	}

	Damage::Roll get_damage_roll(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon)
	{
		int modifier = get_damage_mod(registry, attacker, weapon);
		Damage::Roll damage_roll = registry.get<Damage::Roll>(weapon);
		damage_roll.modifier += modifier;
		if (registry.all_of<VersatileWeapon>(weapon) && !EquipmentSystem::is_dual_wielding(registry, attacker))
			damage_roll.dice.sides += 2;

		return damage_roll;
	}

	int get_attack_roll(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon, const int advantage)
	{
		int modifier = get_attack_mod(registry, attacker, weapon);
		const auto roll = Dice(1, 20).roll(advantage);
		return roll + modifier;
	}

	int get_attack_advantage(const entt::registry& registry, const entt::entity attacker, const entt::entity defender, const entt::entity weapon)
	{
		const auto distance = ECS::distance(registry, attacker, defender);
		const auto melee_range = distance <= MELEE_RANGE;
		const auto melee_weapon = registry.all_of<MeleeWeapon>(weapon);
		const auto weapon_range = melee_weapon ? MELEE_RANGE : registry.get<AttackRange>(weapon).value;

		if (melee_weapon)
		{
			if (melee_range)
				return 0; // Normal
			else
				return -1; // Distadvantage for no good weapon
		}
		else
		{
			if (melee_range || distance > weapon_range)
				return -1; // Disadvantage for point blank or too far
			else
				return 0;
		}

		return 0;
	}

	/* If attack functions become too complex, split 'attack_with' into the different types
	 * */
	void attack_with(
			entt::registry& registry,
			const entt::entity attacker,
			const entt::entity defender,
			const entt::entity weapon)
	{
		const int advantage = get_attack_advantage(registry, attacker, defender, weapon);
		const int attack_roll = get_attack_roll(registry, attacker, weapon, advantage);
		const auto armor_class = StateSystem::get_armor_class(registry, defender);

		Event attack_event;
		attack_event.type = Event::Type::Attack;
		attack_event.weapon = weapon;
		attack_event.actor.entity = attacker;
		attack_event.target.entity = defender;
		ECS::queue_event(registry, attack_event);

		if (attack_roll < armor_class)
		{
			Event miss_event = attack_event;
			miss_event.type = Event::Type::AttackMiss;
			ECS::queue_event(registry, miss_event);
			return;
		}

		auto damage_roll = get_damage_roll(registry, attacker, weapon);
		damage_roll.roll();
		Event hit_event = attack_event;
		hit_event.type = Event::Type::AttackHit;
		ECS::queue_event(registry, hit_event);

		DamageSystem::take_damage(registry, defender, damage_roll);
	}

	void attack_with(entt::registry& registry, const entt::entity attacker, const entt::entity defender, const std::vector<entt::entity>& weapons)
	{
		// Some future plans for this
		// Attack more often with lighter weapons
		// Depending on character AP or something?
		// Mix with unarmed attacks in some cases
		for (const auto weapon : weapons)
			attack_with(registry, attacker, defender, weapon);
	}

	void melee_attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender)
	{
		const auto weapons = EquipmentSystem::get_equipped_items<MeleeWeapon>(registry, attacker);
		if (!weapons.empty())
			attack_with(registry, attacker, defender, weapons);
		else
		{
			const auto unarmed_weapons = registry.get<UnarmedWeapons>(attacker).weapons;
				//ECS::get_unarmed_melee_weapons(registry, attacker);
			attack_with(registry, attacker, defender, unarmed_weapons);
		}
	}

	void ranged_attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender)
	{
		const auto weapons = EquipmentSystem::get_equipped_items<RangedWeapon>(registry, attacker);
		if (!weapons.empty())
			attack_with(registry, attacker, defender, weapons);
		/*else
		{
			const auto unarmed_weapons = ECS::get_unarmed_ranged_weapons(registry, attacker); // idk some spit attacks
			attack_with(registry, attacker, defender, unarmed_weapons);
		}*/
	}

	void throwing_attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender)
	{
		const auto weapons = EquipmentSystem::get_equipped_items<ThrowingWeapon>(registry, attacker);
		(void) defender;
	}

	bool can_attack_with(
			const entt::registry& registry,
			const entt::entity attacker,
			const entt::entity defender,
			const entt::entity weapon)
	{
		if (!VisionSystem::has_vision(registry, attacker, defender))
			return false;

		const auto distance = ECS::distance(registry, attacker, defender);
		auto range = registry.get<AttackRange>(weapon).value;
		if (registry.all_of<MeleeWeapon>(weapon))
		{
			if (range >= distance)
				return true;
		}
		else if (registry.all_of<RangedWeapon>(weapon))
		{
			// Always return true, if target visible
			// Attacks within range don't get disadvantage
			return true;
		}
		else if (registry.all_of<ThrowingWeapon>(weapon))
		{
			// Attacks outside range are not possible
			range += get_throwing_range_mod(registry, attacker, weapon);
			if (range >= distance)
				return true;
		}
		return false;
	}

	/* Entity is in combat, if there are hostile entities within its vision,
	 * that can see entity.
	 * If 'entity' is hostile towards another entity it sees, that is not enough to return true.
	 * The other entity has to be hostile.
	 * "I am in combat if there is someone coming at me"
	 * */
	bool is_in_combat(const entt::registry& registry, const entt::entity entity)
	{
		for (const auto e1 : VisionSystem::get_visible_entities(registry, entity))
		{
			if (!registry.all_of<Alignment>(e1))
				continue; // cannot have bad/any opinions
			const auto visible = VisionSystem::get_visible_entities(registry, e1);
			auto it = std::find(visible.begin(), visible.end(), entity);
			if (it == visible.end())
				continue; // cannot see 'entity'
			if (!AlignmentSystem::is_hostile(registry, e1, entity))
				continue; // does not want to attack 'entity'

			// if 'entity' sees an entity, that also sees 'entity' and is hostile, then it is 'in combat'
			return true;
		}
		return false;
	}
};
