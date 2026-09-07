#include "core/common.hpp"
#include "external/entt/entt.hpp"
#include "systems/Damage.hpp"
#include "systems/Vision.hpp"
#include "systems/Equipment.hpp"
#include "systems/Inventory.hpp"
#include "utils/ECS.hpp"



namespace System::Combat

{
	double get_attack_range(const entt::registry& registry, const entt::entity weapon)
	{
		double attack_range = registry.get<Component::Value::AttackRange>(weapon).value;
		if (ECS::weapon_has_property(registry, weapon, Domain::Enum::WeaponProperty::Reach))
			attack_range += 1.5;
		return attack_range;
	}

	bool can_attack_with(
			const entt::registry& registry,
			const entt::entity attacker,
			const entt::entity defender,
			const entt::entity weapon)
	{
		const auto distance = ECS::distance(registry, attacker, defender);
		const auto range = get_attack_range(registry, weapon);
		return range >= distance;
	}

	bool can_attack(const entt::registry& registry, const entt::entity attacker, const entt::entity defender)
	{
		// This will also include unarmed weapons, aka bodyparts like fists
		const auto weapons = System::Equipment::get_weapons(registry, attacker);
		for (const auto weapon : weapons)
		{
			if (can_attack_with(registry, attacker, defender, weapon))
				return true;
		}
		return false;
	}

	void attack_with(
			entt::registry& registry,
			const entt::entity attacker,
			const entt::entity defender,
			const entt::entity weapon)
	{
		ECS::queue_event(
				registry,
				Domain::Event::Attack{
				.attacker = attacker,
				.defender = defender,
				.weapon = weapon
				});

		const auto advantage = 0;
		const auto attack_modifier = System::State::get_attribute_modifier<Component::Value::Strength>(registry, attacker);
		const auto attack_roll = Domain::Dice(1, 20).roll(advantage) + attack_modifier;
		const auto armor_class = System::State::get_stat<Component::Value::ArmorClass>(registry, defender);

		const auto attack_missed = attack_roll < armor_class;
		if (attack_missed)
		{
			ECS::queue_event(
					registry,
					Domain::Event::AttackMiss{
					.attacker = attacker,
					.defender = defender,
					.weapon = weapon
					});
			return;
		}
		ECS::queue_event(
				registry,
				Domain::Event::AttackHit{
				.attacker = attacker,
				.defender = defender,
				.weapon = weapon
				});

		const auto damage_rolls = registry.get<Component::List::DamageRolls>(weapon);
		for (const auto damage_roll : damage_rolls)
		{
			const auto dice = registry.get<Domain::Dice>(damage_roll);
			const auto type = registry.get<Component::Value::DamageType>(damage_roll).value;
			// TODO: damage resistances
			System::Damage::take_damage(registry, defender, dice.roll(), type);
		}
	}

	/* Attacking is half automatic - weapons are chosen automatically
	 * Bonus action is used automatically for off hand attack, but can be toggled off (TODO)
	 * */
	void attack(entt::registry& registry, const entt::entity attacker, const entt::entity defender)
	{
		if (!can_attack(registry, attacker, defender))
		{
			Log::warning() << "Attacking but cannot really attack";
			return;
		}

		// Attack with main weapon consumes an action
		// Attack with secondary weapon comsumes a bonus action
		const auto weapons = System::Equipment::get_weapons(registry, attacker);
		assert(weapons.size() <= 2 && "Expecting to have max 2 weapons"); // There can be more later

		// weapons[0] is main weapon
		// weapons[1] is secondary weapon
		// monks can have two unarmed weapons or more?
		if (weapons.size() > 0)
		{
			const auto weapon = weapons[0];
			if (registry.get<Component::Resource::ActionPoints>(attacker).current > 0 && can_attack_with(registry, attacker, defender, weapon))
			{
				attack_with(registry, attacker, defender, weapon);
				registry.get<Component::Resource::ActionPoints>(attacker).current--;
			}
		}
		if (weapons.size() > 1)
		{
			const auto weapon = weapons[1];
			if (registry.get<Component::Resource::BonusActionPoints>(attacker).current > 0 && can_attack_with(registry, attacker, defender, weapon))
			{
				attack_with(registry, attacker, defender, weapon);
				registry.get<Component::Resource::BonusActionPoints>(attacker).current--;
			}
		}
	}
}
