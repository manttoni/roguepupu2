#include <gtest/gtest.h>
#include "helpers.hpp"
#include "database/EntityFactory.hpp"

TEST_F(RegistryTest, WeaponsHaveAttacks)
{
	const auto weapons = EntityFactory::instance()
		.create_entities(
				registry,
				nlohmann::json{{"subcategory", "weapons"}}
				);
	for (const auto weapon : weapons)
	{
		ASSERT_TRUE(registry.all_of<Attacks>(weapon));
		EXPECT_FALSE(registry.get<Attacks>(weapon).attacks.empty());
	}
}
TEST_F(RegistryTest, CreaturesHaveUnarmedAttack)
{
	const auto creatures = EntityFactory::instance().
		create_entities(
				registry,
				nlohmann::json{{"category", "creatures"}}
				);
	for (const auto creature : creatures)
	{
		ASSERT_TRUE(registry.all_of<Attacks>(creature)) << registry.get<Name>(creature).name;
		EXPECT_FALSE(registry.get<Attacks>(creature).attacks.empty()) << registry.get<Name>(creature).name;
	}
}
