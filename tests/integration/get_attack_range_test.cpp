#include <gtest/gtest.h>
#include "helpers.hpp"
#include "database/EntityFactory.hpp"
#include "utils/Range.hpp"
#include "utils/ECS.hpp"
#include "systems/state/EquipmentSystem.hpp"
#include "systems/state/InventorySystem.hpp"
#include "external/entt/entt.hpp"
#include "components/Components.hpp"

static void unequip_everything(entt::registry& registry, const entt::entity creature)
{
	if (registry.all_of<Inventory>(creature))
	{
		for (const auto item : registry.get<Inventory>(creature).items)
			EquipmentSystem::unequip(registry, creature, item);
	}
}

TEST_F(RegistryTest, GetAttackRangeWorksUnarmed)
{
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");
	const Range<double> r(0, 1.5);
	registry.emplace_or_replace<AttackRange>(creature, r);
	unequip_everything(registry, creature);

	EXPECT_EQ(ECS::get_attack_range(registry, creature), r);
}

TEST_F(RegistryTest, GetAttackRangeWorksWeapons)
{
	const auto creature = EntityFactory::instance()
		.create_entity(registry, "test_creature");
	const auto weapons = EntityFactory::instance()
		.create_entities(registry, nlohmann::json{{"subcategory", "weapons"}});
	for (const auto w : weapons)
	{
		InventorySystem::add_item(registry, creature, w);
		const auto& w_range = registry.get<AttackRange>(w).range;
		EXPECT_EQ(w_range, ECS::get_attack_range(registry, w)) << registry.get<Name>(w).name;
		EquipmentSystem::equip(registry, creature, w);
		const auto& c_range = ECS::get_attack_range(registry, creature);

		const Range<double> expected(std::min(w_range.min, c_range.min), std::max(w_range.max, c_range.max));
		EXPECT_EQ(c_range, expected) << registry.get<Name>(w).name;
		unequip_everything(registry, creature);
	}
}
