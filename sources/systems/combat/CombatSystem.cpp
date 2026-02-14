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
		const auto& damage_spec = weapon == entt::null ?
			registry.get<Damage::Spec>(attacker) : // unarmed
			registry.get<Damage::Spec>(weapon);
		auto damage_roll = damage_spec.roll();
		auto& damage = damage_roll.result; // double

		// Mechanical weapons do their own thing, nothing you can do about it
		if (registry.all_of<MechanicalWeapon>(weapon))
			return damage_roll;

		// Finesse weapons can be optionally used with DEX
		const bool finesse = weapon == entt::null ?
			registry.all_of<FinesseWeapon>(attacker) :
			registry.all_of<FinesseWeapon>(weapon);

		// Versatile weapons get a bonus if not dual wielding
		const bool versatile = registry.all_of<VersatileWeapon>(weapon);

		// Dual wielding will get penalty
		const bool dual_wielding = EquipmentSystem::is_dual_wielding(registry, attacker);

		const auto attributes = ECS::get_attributes(registry, attacker);
		if (finesse)
		{
			damage *= 100 + std::max(attributes.strength, attributes.dexterity);
			damage /= 100;
		}
		else
		{
			damage *= 100 + attributes.strength;
			damage /= 100;
		}

		if (dual_wielding)
		{
			damage *= 0.8;
		}
		else if (versatile)
		{
			damage *= 1.2;
		}

		return damage_roll;
	}

	void attack_with(entt::registry& registry, const entt::entity attacker, const entt::entity defender, const entt::entity weapon)
	{
		// Ranged weapons use Perception and Dexterity for accuracy
		// Melee weapons use Agility and Dexterity or Sterngth for accuracy
		int accuracy = 0;
		const bool melee = weapon == entt::null || registry.all_of<MeleeWeapon>(weapon);
		const bool ranged = registry.all_of<RangedWeapon>(weapon);
		//const bool mechanical = registry.all_of<MechanicalWeapon>(weapon); // f.e. crossbow, pistol
		const bool finesse = weapon == entt::null ?
			registry.all_of<FinesseWeapon>(attacker) :
			registry.all_of<FinesseWeapon>(weapon);

		const auto attacker_attributes = ECS::get_attributes(registry, attacker);
		const auto defender_attributes = ECS::get_attributes(registry, defender);

		if (melee)
		{
			accuracy += attacker_attributes.agility;
			if (finesse)
				accuracy += std::max(attacker_attributes.dexterity, attacker_attributes.strength);
			else
				accuracy += attacker_attributes.strength;
		}
		else if (ranged)
			accuracy += attacker_attributes.perception + attacker_attributes.dexterity;
		else
			Error::fatal("Weapon is not melee nor ranged");

		// Evasion comes from Agility and Perception
		int evasion = defender_attributes.agility + defender_attributes.perception;

		// hit range [-200, 200]
		// Negative means miss, positive is hit
		int hit_quality = Random::rand<int>(0, accuracy) - Random::rand<int>(0, evasion);

		Event event;
		event.type = Event::Type::Attack;
		event.weapon = weapon;
		event.hit_quality = hit_quality; // logger can print some more info about attack with this data
		event.actor.entity = attacker;
		event.target.entity = defender;
		ECS::queue_event(registry, event);

		if (hit_quality >= 0)
		{
			auto damage_roll = get_damage_roll(registry, attacker, weapon);
			// if is critical hit, multiply...
			//
			// defender can resist damage with armor or spells
			// but they dont exist yet
			DamageSystem::take_damage(registry, defender, damage_roll);
			return;
		}
		else
		{
			// When attacker misses, defender gets a chance to counter attack
			// but one thing at a time
			return;
		}

		return;

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
		const auto main_hand = equipped.at(Equipment::Slot::MainHand);
		const auto off_hand = equipped.at(Equipment::Slot::OffHand);
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
