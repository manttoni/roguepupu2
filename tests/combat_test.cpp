#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "helpers.hpp"
#include "database/EntityFactory.hpp"
#include "systems/combat/DamageSystem.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "domain/Damage.hpp"
#include "external/entt/entt.hpp"
#include "infrastructure/GameLogger.hpp"
#include "systems/action/EventSystem.hpp"
#include "components/Components.hpp"
#include "gtest/gtest.h"
#include "utils/ECS.hpp"

TEST_F(RegistryTest, EntityTakesDamage)
{
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");

	ASSERT_TRUE(creature != entt::null);
	ASSERT_TRUE(registry.all_of<Health>(creature));

	const Damage damage(Damage::Type::Piercing, 1);
	const auto health_before = registry.get<Health>(creature);
	DamageSystem::take_damage(registry, creature, damage);
	const auto health_after = registry.get<Health>(creature);
	EXPECT_NE(health_before, health_after);
}

TEST_F(RegistryTest, TakingDamagePrintsLog)
{
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");

	const Damage damage(Damage::Type::Piercing, 1);

	// DamageSystem will queue an event after removing hp
	DamageSystem::take_damage(registry, creature, damage);

	// EventSystem will log a message with the damage info
	EventSystem::resolve_events(registry);

	const auto logger = registry.ctx().get<GameLogger>();
	const auto last_message = logger.last(1).front();

	ASSERT_FALSE(last_message.empty());

	const std::string expected_message = ECS::get_colored_name(registry, creature) + " takes " + damage.to_string() + " damage";

	EXPECT_EQ(expected_message, last_message);
}

/* Creatures have a Damage component.
 * It is their unarmed damage.
 * (If talking about Damage component of a weapon,
 * creatures will use that instead, see next test)
 * */
TEST_F(RegistryTest, CreaturesHaveDamage)
{
	const auto creatures = EntityFactory::instance()
		.create_entities(
				registry,
				nlohmann::json{{"category", "creatures"}}
				);
	for (const auto creature : creatures)
		EXPECT_TRUE(registry.all_of<Damage>(creature)) << registry.get<Name>(creature).name;
}

TEST_F(RegistryTest, CreaturesUseWeaponDamage)
{
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");
	for (const auto& [slot, equipped_item] : registry.get<EquipmentSlots>(creature).equipped_items)
		EquipmentSystem::unequip(registry, creature, equipped_item);
	const auto unarmed_damage = ECS::get_main_weapon_damage(registry, creature);

	const auto weapon = EntityFactory::instance().create_entity(registry, "knife");
	EquipmentSystem::equip(registry, creature, weapon);
	const auto armed_damage = ECS::get_main_weapon_damage(registry, creature);

	// When creature equipped weapon, their damage changed
	EXPECT_NE(unarmed_damage, armed_damage);
}

/* Every single point increase in strength should increase attack damage
 * */
TEST_F(RegistryTest, StrengthIncreasesUnarmedDamage)
{
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");
	registry.emplace_or_replace<Strength>(creature, 1);
	for (const auto& [slot, equipped_item] : registry.get<EquipmentSlots>(creature).equipped_items)
		EquipmentSystem::unequip(registry, creature, equipped_item);
	auto& strength = registry.get<Strength>(creature).value;
	auto attack_damage_before = ECS::get_main_attack_damage(registry, creature);

	while (strength < 100)
	{
		strength++;
		const auto attack_damage_after = ECS::get_main_attack_damage(registry, creature);
		ASSERT_GT(attack_damage_after, attack_damage_before);
		attack_damage_before = attack_damage_after;
	}
}
