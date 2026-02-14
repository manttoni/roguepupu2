#include <gtest/gtest.h>
#include "helpers.hpp"
#include "external/entt/entt.hpp"
#include "database/EntityFactory.hpp"
#include "systems/action/ActionSystem.hpp"
#include "systems/action/AISystem.hpp"



TEST_F(RegistryTest, EntityHasAggressiveIntent)
{
	auto arena = TestHelpers::get_duel_arena(registry);

	// They should have intent type attack
	const Intent ia = AISystem::get_npc_intent(registry, arena.entities[0]);
	const Intent ib = AISystem::get_npc_intent(registry, arena.entities[1]);

	EXPECT_TRUE(ia.type == Intent::Type::Attack) << static_cast<size_t>(ia.type);
	EXPECT_TRUE(ib.type == Intent::Type::Attack) << static_cast<size_t>(ib.type);
}

TEST_F(RegistryTest, EntitiesFistFight)
{
	auto arena = TestHelpers::get_duel_arena(registry);
	for (size_t i = 0; i < 10; ++i)
	{
		ActionSystem::act_round(registry, arena.cave_idx);
	}
	EXPECT_TRUE(registry.all_of<Dead>(arena.entities[0]) || registry.all_of<Dead>(arena.entities[1]));
}
