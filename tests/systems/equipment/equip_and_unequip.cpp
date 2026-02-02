#include <gtest/gtest.h>
#include "registry_fixture.hpp"
#include "database/EntityFactory.hpp"
#include "systems/state/EquipmentSystem.hpp"

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
		EquipmentSystem::equip(registry, entity, equipment);
		EXPECT_TRUE(EquipmentSystem::is_equipped(registry, entity, equipment));
		EquipmentSystem::unequip(registry, entity, equipment);
		EXPECT_FALSE(EquipmentSystem::is_equipped(registry, entity, equipment));
	}
}
