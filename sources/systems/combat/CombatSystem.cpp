#include "domain/Damage.hpp"
#include "external/entt/entt.hpp"
#include "systems/combat/DamageSystem.hpp"
#include "systems/perception/VisionSystem.hpp"
#include "systems/state/AlignmentSystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "utils/ECS.hpp"

namespace CombatSystem
{
	Damage::Roll get_damage_roll(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon)
	{
		int modifier = 0;
		if (registry.any_of<FinesseWeapon, RangedWeapon>(weapon))
			modifier = StateSystem::get_stat<Dexterity>(registry, attacker);
		else
			modifier = StateSystem::get_stat<Strength>(registry, attacker);

		Damage::Roll weapon_damage_roll = registry.get<Damage::Roll>(weapon);
		if (registry.all_of<VersatileWeapon>(weapon) && !EquipmentSystem::is_dual_wielding(registry, attacker))
			weapon_damage_roll.dice.sides += 2;

		Damage::Roll damage_roll(weapon_damage_roll.type, weapon_damage_roll.dice, weapon_damage_roll.modifier + modifier);
		return damage_roll;
	}

	int get_attack_roll(const entt::registry& registry, const entt::entity attacker, const entt::entity weapon, const int advantage)
	{
		int modifier = 0;
		if (registry.any_of<FinesseWeapon, RangedWeapon>(weapon))
			modifier = StateSystem::get_stat<Dexterity>(registry, attacker);
		else
			modifier = StateSystem::get_stat<Strength>(registry, attacker);

		const auto roll = Dice(1, 20).roll(advantage);
		return roll + modifier;
	}

	int get_attack_advantage(const entt::registry& registry, const entt::entity attacker, const entt::entity defender, const entt::entity weapon)
	{
		(void) registry; (void) attacker; (void) defender; (void) weapon;
		return 0;
	}

	void attack_with(entt::registry& registry, const entt::entity attacker, const entt::entity defender, const entt::entity weapon)
	{
		const int advantage = get_attack_advantage(registry, attacker, defender, weapon);
		const int attack_roll = get_attack_roll(registry, attacker, weapon, advantage);
		const auto armor_class = StateSystem::get_armor_class(registry, defender);

		Event attack_event;
		attack_event.type = Event::Type::Attack;
		attack_event.weapon = weapon;
		attack_event.actor.entity = attacker;
		attack_event.target.entity = defender;
		attack_event.advantage = advantage;
		ECS::queue_event(registry, attack_event);

		if (attack_roll < armor_class)
		{
			Event miss_event = attack_event;
			miss_event.type = Event::Type::MissAttack;
			ECS::queue_event(registry, miss_event);
			return;
		}

		auto damage_roll = get_damage_roll(registry, attacker, weapon);
		Event hit_event = attack_event;
		hit_event.type = Event::Type::HitAttack;
		ECS::queue_event(registry, hit_event);

		DamageSystem::take_damage(registry, defender, damage_roll);
	}

	/* Attacker has to have something to attack defender regardless of distance.
	 * Have to check before.
	 * */
	void attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender)
	{
		const double distance = ECS::distance(registry, attacker, defender);
		assert(ECS::get_attack_range(registry, attacker).contains(distance));

		// Find out all weapons that can be used for attacking.
		// If none found, attack with unarmed since based
		// on assertion that is the only logical attack in that case.
		std::vector<entt::entity> used_weapons;

		//const auto& loadout = registry.get<EquipmentSlots>(attacker).get_active_loadout();
		//Don't use loadouts ever, they are just optional thing for QOL for player, not dev

		const auto& equipped = registry.get<EquipmentSlots>(attacker).equipped_items;
		const auto main_hand = equipped.at(EquipmentSlot::MainHand);
		const auto off_hand = equipped.at(EquipmentSlot::OffHand);
		if (main_hand != entt::null &&
				registry.get<AttackRange>(main_hand).range.contains(distance))
			used_weapons.push_back(main_hand);
		if (off_hand != entt::null && off_hand != main_hand &&
				registry.get<AttackRange>(off_hand).range.contains(distance))
			used_weapons.push_back(off_hand);

		if (used_weapons.empty())
		{
			attack_with(registry, attacker, defender, entt::null);
			return;
		}

		for (const auto weapon : used_weapons)
			attack_with(registry, attacker, defender, weapon);
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
