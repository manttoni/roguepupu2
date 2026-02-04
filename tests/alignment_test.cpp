#include <gtest/gtest.h>
#include <memory>

#include "helpers.hpp"
#include "external/entt/entt.hpp"
#include "database/EntityFactory.hpp"
#include "components/Components.hpp"
#include "systems/state/AlignmentSystem.hpp"
#include "gtest/gtest.h"

TEST_F(RegistryTest, CreatureBecomesHostile)
{
	const auto dummy = EntityFactory::instance().create_entity(registry, "test_dummy");
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");

	// With 0.0 tolerance these should get hostile
	registry.emplace<Alignment>(dummy, Alignment(-1, -1));
	registry.emplace<Alignment>(creature, Alignment(1, 1));

	EXPECT_TRUE(AlignmentSystem::is_hostile(registry, creature, dummy));
}

/* With 0 tolerance all other base alignments are enemies.
 * */
TEST_F(RegistryTest, ZeroTolerance)
{
	for (int i = -1; i <= 1; ++i)
	{
		for (int j = -1; j <= 1; ++j)
		{
			const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");
			registry.emplace<Alignment>(creature, Alignment(i, j, 0));

			for (int k = -1; k <= 1; ++k)
			{
				for (int l = -1; l <= 1; ++l)
				{
					const auto other = EntityFactory::instance().create_entity(registry, "test_creature");
					registry.emplace<Alignment>(other, Alignment(k, l, 0));
					if (i == k && j == l)
					{
						EXPECT_FALSE(AlignmentSystem::is_hostile(registry, creature, other));
						EXPECT_FALSE(AlignmentSystem::is_hostile(registry, other, creature));
					}
					else
					{
						EXPECT_TRUE(AlignmentSystem::is_hostile(registry, creature, other));
						EXPECT_TRUE(AlignmentSystem::is_hostile(registry, other, creature));
					}
				}
			}

		}
	}

}

TEST_F(RegistryTest, OneTolerance)
{
	using Type = Alignment::Type;
	const auto lawful_good = EntityFactory::instance().create_entity(registry, "test_creature");
	registry.emplace<Alignment>(lawful_good, Alignment(Type::LawfulGood, 1));

	const auto chaotic_evil = EntityFactory::instance().create_entity(registry, "test_creature");
	registry.emplace<Alignment>(chaotic_evil, Alignment(Type::ChaoticEvil, 1));

	// With tolerance 1.0 they will be hostile, because their opinion is 1 - ~2.8 < -1
	EXPECT_TRUE(AlignmentSystem::is_hostile(registry, lawful_good, chaotic_evil));

	const auto lawful_neutral = EntityFactory::instance().create_entity(registry, "test_creature");
	registry.emplace<Alignment>(lawful_neutral, Alignment(Type::LawfulNeutral, 1));

	// These should not be hostile, because their distance is not more than tolerance
	// in fact they should also not be friendly, just neutral
	EXPECT_FALSE(AlignmentSystem::is_hostile(registry, lawful_good, lawful_neutral));
	EXPECT_FALSE(AlignmentSystem::is_friendly(registry, lawful_good, lawful_neutral));
}
