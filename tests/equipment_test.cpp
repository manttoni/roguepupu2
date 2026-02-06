#include <gtest/gtest.h>
#include <nlohmann/detail/json_ref.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <map>
#include <string>
#include <vector>
#include <optional>

#include "helpers.hpp"
#include "database/EntityFactory.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "systems/state/InventorySystem.hpp"
#include "systems/items/LootSystem.hpp"
#include "systems/action/EventSystem.hpp"

TEST_F(RegistryTest, EquipAndUnequip)
{
	const nlohmann::json filter = {
		{"equipment", "any"}
	};
	const auto equipment_ids = EntityFactory::instance().filter_entity_ids(filter);
	for (const auto& id : equipment_ids)
	{
		const auto entity = EntityFactory::instance().create_entity(registry, "test_creature");
		const auto equipment = EntityFactory::instance().create_entity(registry, id);
		InventorySystem::add_item(registry, entity, equipment);
		EquipmentSystem::equip(registry, entity, equipment);
		EXPECT_TRUE(EquipmentSystem::is_equipped(registry, entity, equipment));
		EquipmentSystem::unequip(registry, entity, equipment);
		EXPECT_FALSE(EquipmentSystem::is_equipped(registry, entity, equipment));
	}
}

static std::string equipped_items_to_string(const entt::registry& registry, const entt::entity entity)
{
	if (!registry.all_of<EquipmentSlots>(entity))
		return "No equipment slots";
	std::string ret = "";
	const auto& loadouts = registry.get<EquipmentSlots>(entity).loadouts;
	ret += "Loadout 0:\n";
	if (loadouts[0].main_hand != entt::null)
		ret += "Main: " + registry.get<Name>(loadouts[0].main_hand).name + "\n";
	if (loadouts[0].off_hand != entt::null && loadouts[0].off_hand != loadouts[0].main_hand)
		ret += "Off: " + registry.get<Name>(loadouts[0].off_hand).name + "\n";
	ret += "Loadout 1:\n";
	if (loadouts[1].main_hand != entt::null)
		ret += "Main: " + registry.get<Name>(loadouts[1].main_hand).name + "\n";
	if (loadouts[1].off_hand != entt::null && loadouts[1].off_hand != loadouts[1].main_hand)
		ret += "Off: " + registry.get<Name>(loadouts[1].off_hand).name + "\n";
	return ret;
}

/* When entity receives items, they should equip them if they have free slots.
 * They should also equip items into their secondary loadout.
 * Does not affect player entity
 * */
TEST_F(RegistryTest, EquipItems)
{
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");
	registry.emplace_or_replace<EquipmentSlots>(creature);
	LootSystem::give_loot(registry, creature, "lt_all_weapons");
	ASSERT_FALSE(registry.get<Inventory>(creature).items.empty());
	EventSystem::resolve_events(registry);

	const auto& equipment_slots = registry.get<EquipmentSlots>(creature);
	EXPECT_FALSE(equipment_slots.loadouts[0].main_hand == entt::null) << equipped_items_to_string(registry, creature);
	EXPECT_FALSE(equipment_slots.loadouts[0].off_hand == entt::null) << equipped_items_to_string(registry, creature);
	EXPECT_FALSE(equipment_slots.loadouts[1].main_hand == entt::null) << equipped_items_to_string(registry, creature);
	EXPECT_FALSE(equipment_slots.loadouts[1].off_hand == entt::null) << equipped_items_to_string(registry, creature);
	EXPECT_FALSE(equipment_slots.equipped_items.at(Equipment::Slot::MainHand) == entt::null) << equipped_items_to_string(registry, creature);
	EXPECT_FALSE(equipment_slots.equipped_items.at(Equipment::Slot::OffHand) == entt::null) << equipped_items_to_string(registry, creature);
}

TEST_F(RegistryTest, ReceivingItemQueuesEvent)
{
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");
	registry.emplace_or_replace<Inventory>(creature);
	const auto item = EntityFactory::instance().create_entity(registry, "dagger");
	InventorySystem::add_item(registry, creature, item);
	const auto queue = registry.ctx().get<EventQueue>().queue;
	ASSERT_FALSE(queue.empty());
	EXPECT_EQ(queue.front().effect.type, Effect::Type::ReceiveItem);
}

TEST_F(RegistryTest, HasFreeSlotsWorksDualWield)
{
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");
	registry.emplace_or_replace<Inventory>(creature);
	registry.emplace_or_replace<EquipmentSlots>(creature);
	const auto dagger1 = EntityFactory::instance().create_entity(registry, "dagger");
	const auto dagger2 = EntityFactory::instance().create_entity(registry, "dagger");
	InventorySystem::add_item(registry, creature, dagger1);
	InventorySystem::add_item(registry, creature, dagger2);

	EXPECT_TRUE(EquipmentSystem::has_free_slots(registry, creature, registry.get<Equipment>(dagger1)));
	EquipmentSystem::equip(registry, creature, dagger1);
	EXPECT_TRUE(EquipmentSystem::has_free_slots(registry, creature, registry.get<Equipment>(dagger2)));
	EquipmentSystem::equip(registry, creature, dagger2);
	EXPECT_FALSE(EquipmentSystem::has_free_slots(registry, creature, registry.get<Equipment>(dagger1)));
}

TEST_F(RegistryTest, HasFreeSlotsWorksTwoHanded1)
{
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");
	registry.emplace_or_replace<Inventory>(creature);
	registry.emplace_or_replace<EquipmentSlots>(creature);
	const auto pickaxe = EntityFactory::instance().create_entity(registry, "pickaxe");
	const auto dagger = EntityFactory::instance().create_entity(registry, "dagger");
	InventorySystem::add_items(registry, creature, {pickaxe, dagger});

	EXPECT_TRUE(EquipmentSystem::has_free_slots(registry, creature, registry.get<Equipment>(pickaxe)));
	EquipmentSystem::equip(registry, creature, pickaxe);
	EXPECT_FALSE(EquipmentSystem::has_free_slots(registry, creature, registry.get<Equipment>(pickaxe)));
	EXPECT_FALSE(EquipmentSystem::has_free_slots(registry, creature, registry.get<Equipment>(dagger)));
}

TEST_F(RegistryTest, HasFreeSlotsWorksTwoHanded2)
{
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");
	registry.emplace_or_replace<Inventory>(creature);
	registry.emplace_or_replace<EquipmentSlots>(creature);
	const auto pickaxe = EntityFactory::instance().create_entity(registry, "pickaxe");
	const auto dagger = EntityFactory::instance().create_entity(registry, "dagger");
	InventorySystem::add_items(registry, creature, {pickaxe, dagger});

	EXPECT_TRUE(EquipmentSystem::has_free_slots(registry, creature, registry.get<Equipment>(pickaxe)));
	EquipmentSystem::equip(registry, creature, dagger);
	EXPECT_FALSE(EquipmentSystem::has_free_slots(registry, creature, registry.get<Equipment>(pickaxe)));
}

TEST_F(RegistryTest, WeaponsHaveSlots)
{
	const auto weapons = EntityFactory::instance()
		.create_entities(
				registry,
				nlohmann::json{{"subcategory", "weapons"}}
				);
	for (const auto weapon : weapons)
	{
		EXPECT_TRUE(registry.all_of<Equipment>(weapon));
		const auto& equipment = registry.get<Equipment>(weapon);
		EXPECT_TRUE(equipment.use_all || equipment.use_one) << registry.get<Name>(weapon).name;
	}
}

TEST_F(RegistryTest, DaggerHasUseOne)
{
	const auto dagger = EntityFactory::instance().create_entity(registry, "dagger");
	const auto& equipment = registry.get<Equipment>(dagger);
	const auto& use_one = equipment.use_one;
	ASSERT_TRUE(use_one.has_value());
	auto main_it = std::find(use_one->begin(), use_one->end(), Equipment::Slot::MainHand);
	auto off_it = std::find(use_one->begin(), use_one->end(), Equipment::Slot::OffHand);
	EXPECT_NE(main_it, use_one->end());
	EXPECT_NE(off_it, use_one->end());
}
