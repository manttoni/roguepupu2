#include <gtest/gtest.h>

#include "databases/EntityDatabase.hpp"
#include "game/entity/Entity.hpp"

namespace
{
	TEST(EntityDefinitionTest, LootTableRequiresItsListBudgetsAndDuplicateFlag)
	{
		const Json table{
			{"Tags", Json::array({"LootTable"})},
			{"EntityIDs", Json::array()},
			{"CountBudget", 0},
			{"GoldBudget", 0},
			{"WeightBudget", 0.0},
			{"AllowDuplicates", false}
		};
		ASSERT_TRUE(Game::Entity::valid_definition(table));

		for (const auto* component :
			{"EntityIDs", "CountBudget", "GoldBudget", "WeightBudget", "AllowDuplicates"})
		{
			SCOPED_TRACE(component);
			auto incomplete = table;
			incomplete.erase(component);
			EXPECT_FALSE(Game::Entity::valid_definition(incomplete));
		}
	}

	TEST(EntityDefinitionTest, DamageRollRequiresBothTypeAndDice)
	{
		const Json damage{
			{"Tags", Json::array({"DamageRoll"})},
			{"DamageType", "Bludgeoning"},
			{"Dice", "1d4"}
		};
		ASSERT_TRUE(Game::Entity::valid_definition(damage));

		for (const auto* component : {"DamageType", "Dice"})
		{
			SCOPED_TRACE(component);
			auto incomplete = damage;
			incomplete.erase(component);
			EXPECT_FALSE(Game::Entity::valid_definition(incomplete));
		}
	}

	TEST(EntityDefinitionTest, CreatureRequiresTheNewComponentsAndAcceptsEmptySpawnLoot)
	{
		const EntityDatabase database;
		auto creature = database.definitions.at("player_default");
		creature["SpawnLoot"] = Json::array();
		ASSERT_TRUE(Game::Entity::valid_definition(creature));

		for (const auto* component : {"SpawnLoot", "ArmorClass", "HitPoints", "Size"})
		{
			SCOPED_TRACE(component);
			auto incomplete = creature;
			incomplete.erase(component);
			EXPECT_FALSE(Game::Entity::valid_definition(incomplete));
		}
	}

	TEST(EntityDefinitionTest, MissingDefinitionReturnsTheNullSentinel)
	{
		EntityDatabase database;
		database.definitions.erase("test_missing_definition");
		entt::registry registry;
		const auto existing = registry.create();

		const auto result = Game::Entity::create(
			registry, database, "test_missing_definition");

		EXPECT_TRUE(result == entt::null);
		EXPECT_FALSE(registry.valid(result));
		EXPECT_TRUE(registry.valid(existing));
	}
}
