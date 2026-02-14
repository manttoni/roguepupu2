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
	const auto creature1 = EntityFactory::instance().create_entity(registry, "test_creature");
	const auto creature2 = EntityFactory::instance().create_entity(registry, "test_creature");

	// With 0.0 tolerance these should get hostile
	registry.emplace_or_replace<Alignment>(creature1, Alignment(-1, -1));
	registry.emplace_or_replace<Alignment>(creature2, Alignment(1, 1));

	EXPECT_TRUE(AlignmentSystem::is_hostile(registry, creature1, creature2));
}


/* Alignment ranges:
 * Chaotic/Evil range is ]-inf, -0.5]
 * Neutral range is ]-0.5, 0.5[
 * Lawful/Good range is [0.5, inf[
 *
 * Try to create entities with integer values.
 * If they start shifting they will become outcasts,
 * but can be respected by other alignments
 *
 * Lawful Good character who does unlawful acts, will be neutral when their
 * chaos_law value drops from 1.0 to ~0.6. At that point they are still lawful good,
 * but very close to neutral good. With tolerance of ~0.1 they can still be accepted
 * by the "law" and have friendly status/opinion.
 *
 * So there is a tiny grey zone between alignments (~0.4 - ~0.6 for example),
 * where entities are not friends of any integer alignment with 0 tolerance.
 *
 * Stealing will shift towards chaotic, and if seen it will lower personal opinion.
 *
 * True neutral has no hostility with no modifiers (tolerance, charisma, personal opinions)
 *
 * Corner alignments have more enemies than side alignments
 *
 * Distances:
 * 0	1	2
 * 1	1.4	2.2
 * 2	2.2	2.8
 *
 * Opinion is calculated ~2.8 / 2 - distance,
 * (+ charisma + personal opinion + tolerance), ignored unless specified in these tests
 * Friendly treshold is >= 1.0, hostile is < 0.0, between is neutral territory.
 *
 * Same alignment opinion = 1.4, friendly
 * 	If they start shifting they will be neutral before completely converting
 *
 * one step horizontal/vertical shifted is 0.4, neutral
 * 	Same axis and other not opposite
 *
 * two step horizontal/vertical is -0.6, hostile
 * 	One same axis but other is opposite
 *
 * diagonal adjacent is 0.0, almost hostile, on a razor blade
 * 	No same axis but not opposite either
 *
 * knights move is -0.8, hostile
 * 	Opposite axis and other is not same
 *
 * two step diagonal is -1.4, hostile
 * 	Completely opposite
 *
 * Additional info, might change:
 * 	Attacking friendly or neutral will lower personal opinion by 1.
 * 		Friendly will warn once, neutral will attack back right away.
 * 	Charisma of 10 will add 1.0 opinion to others, which is too much, todo: change
 * 	Tolerance is just an adjustable offset of how accepting entities are of other entities and alignments
 * */

std::vector<entt::entity> get_entities_all_alignments_no_charisma(entt::registry& registry, const double tolerance)
{
	const auto entities = EntityFactory::instance().create_entities(registry, "test_creature", 9);
	for (size_t i = 0; i < 9; ++i)
	{
		const Alignment a(static_cast<Alignment::Type>(i), tolerance);
		registry.emplace_or_replace<Alignment>(entities[i], a);
		registry.emplace_or_replace<Charisma>(entities[i], 0);
	}
	return entities;
}

/* With 0 tolerance only same alignment is considered friendly.
 * This test should find exacly 9 cases where compared entities are friendly.
 * It should find 4 * 5 + 4 * 3 cases where they are hostile
 * Neutrals amount should he 81 - 9 - 20 - 12 = 40
 * */
TEST_F(RegistryTest, NoToleranceNoCharismaNoOpinions)
{
	const auto entities = get_entities_all_alignments_no_charisma(registry, 0.0);
	size_t friendlies = 0;
	size_t hostiles = 0;
	size_t neutrals = 0;
	for (const auto entity1 : entities)
	{
		const auto alignment1 = registry.get<Alignment>(entity1);
		for (const auto entity2 : entities)
		{
			const auto alignment2 = registry.get<Alignment>(entity2);
			if (AlignmentSystem::is_friendly(registry, entity1, entity2))
			{
				EXPECT_TRUE(alignment1.distance(alignment2) == 0);
				friendlies++;
			}
			else if (AlignmentSystem::is_hostile(registry, entity1, entity2))
			{
				EXPECT_GT(alignment1.distance(alignment2), hypot(2,2) / 2);
				hostiles++;
			}
			else
			{
				EXPECT_GE(alignment1.distance(alignment2), 0);
				EXPECT_LE(alignment1.distance(alignment2), hypot(2,2) / 2);
				neutrals++;
			}
		}
	}
	EXPECT_EQ(friendlies, 9);
	EXPECT_EQ(hostiles, 4 * 5 + 4 * 3);
	EXPECT_EQ(neutrals, 40);
}

/* With 1 tolerance adjacent alignments are considered friendly
 *
TEST_F(RegistryTest, OneTolerance)
{
}
*/
