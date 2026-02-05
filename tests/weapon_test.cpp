#include <gtest/gtest.h>
#include "helpers.hpp"
#include "database/EntityFactory.hpp"

TEST_F(RegistryTest, WeaponsHaveDamage)
{
	const auto weapons = EntityFactory::instance()
		.create_entities(
				registry,
				nlohmann::json{{"subcategory", "weapons"}}
				);
	for (const auto weapon : weapons)
		ASSERT_TRUE(registry.all_of<Damage>(weapon));
}
