#include <gtest/gtest.h>
#include <vector>

#include "components/Components.hpp"
#include "database/EntityFactory.hpp"
#include "external/entt/entt.hpp"
#include "helpers.hpp"
#include "systems/action/EventSystem.hpp"
#include "systems/combat/CombatSystem.hpp"
#include "systems/state/AlignmentSystem.hpp"

TEST_F(RegistryTest, FriendlyLosesOpinionOnAttack)
{
	const auto arena = TestHelpers::get_duel_arena(registry);
	const Alignment lawfulgood(Alignment::Type::LawfulGood);
	const auto& entities = arena.entities;
	registry.emplace_or_replace<Alignment>(entities[0], lawfulgood);
	registry.emplace_or_replace<Alignment>(entities[1], lawfulgood);
	ASSERT_TRUE(AlignmentSystem::is_friendly(registry, entities[0], entities[1]));

	// they have to stand next to each other to be able to hit with unarmed
	ASSERT_TRUE(ECS::distance(registry, entities[0], entities[1]) <= 1.5);

	// The opinion should be positive, more than 1
	const auto opinion = AlignmentSystem::get_opinion(registry, entities[1], entities[0]);
	ASSERT_GE(opinion, 1.0); // replace this magic number with some macro

	// By default test_creature has only 1 hp,
	// so give it more so it can have a worse opinion and not be dead instead,
	// in case it matters
	registry.emplace_or_replace<Health>(entities[1], 1000000); // It is over max_health, is allowed

	// 1 should lose opinion of 0, because of betrayal
	// (0 still has the same opinion of 1)
	CombatSystem::attack(registry, entities[0], entities[1]);

	// attack queued an event, event handler will call "lose_opinion()"
	EventSystem::resolve_events(registry);

	const auto opinion_after = AlignmentSystem::get_opinion(registry, entities[1], entities[0]);

	EXPECT_LT(opinion_after, opinion);
}
