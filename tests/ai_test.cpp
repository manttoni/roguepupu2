#include <gtest/gtest.h>
#include "helpers.hpp"
#include "domain/Intent.hpp"
#include "systems/action/ActionSystem.hpp"
#include "systems/action/AISystem.hpp"
#include "external/entt/entt.hpp"
#include "database/EntityFactory.hpp"
#include "components/Components.hpp"

TEST_F(RegistryTest, NPCCanHaveIntent)
{
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature");
	registry.emplace_or_replace<AI>(creature);
	const auto intent = AISystem::get_npc_intent(registry, creature);

	// Expect at least some kind of intent
	EXPECT_NE(intent, Intent());
}

/* In an empty room with no other entities,
 * npc should just move randomly, if they have
 * idle_wander = true
 * */
TEST_F(RegistryTest, NPCIdleWander)
{
	const auto cave_idx = TestHelpers::get_cave_idx(registry, 10, TestHelpers::CaveType::Room);
	const auto& cave = ECS::get_cave(registry, cave_idx);
	const auto mid_pos = cave.middle_position();
	const auto creature = EntityFactory::instance().create_entity(registry, "test_creature", mid_pos);
	registry.emplace_or_replace<AI>(creature);
	registry.get<AI>(creature).idle_wander = true;
	const auto intent = AISystem::get_npc_intent(registry, creature);
	ASSERT_TRUE(intent.type == Intent::Type::Move);

	ActionSystem::resolve_intent(registry, intent);
	EXPECT_NE(mid_pos, registry.get<Position>(creature));
}
