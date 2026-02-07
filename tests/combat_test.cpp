#include <gtest/gtest.h>
#include "helpers.hpp"
#include "external/entt/entt.hpp"
#include "database/EntityFactory.hpp"
#include "systems/action/ActionSystem.hpp"
#include "systems/action/AISystem.hpp"

struct TestFightArena
{
	std::vector<entt::entity> creatures;
	size_t cave_idx;
};

TestFightArena get_duel_arena(entt::registry& registry)
{
	TestFightArena arena;
	arena.creatures = EntityFactory::instance().create_entities(registry, "test_creature", 2);
	arena.cave_idx = TestHelpers::get_cave_idx(registry, 10, TestHelpers::CaveType::Room);
	const auto& cave = ECS::get_cave(registry, arena.cave_idx);

	// Give them positions and opposing alignments and aggressive AI
	const auto mid_pos = cave.middle_position();
	registry.emplace<Position>(arena.creatures[0], mid_pos);
	const Alignment a(Alignment::Type::LawfulGood);
	registry.emplace_or_replace<Alignment>(arena.creatures[0], a);
	registry.emplace_or_replace<AI>(arena.creatures[0]);
	registry.get<AI>(arena.creatures[0]).aggressive = true;

	const auto left = Position(mid_pos.cell_idx - 1, arena.cave_idx);
	registry.emplace<Position>(arena.creatures[1], left);
	const Alignment b(Alignment::Type::ChaoticEvil);
	registry.emplace_or_replace<Alignment>(arena.creatures[1], b);
	registry.emplace_or_replace<AI>(arena.creatures[1]);
	registry.get<AI>(arena.creatures[1]).aggressive = true;

	return arena;
}

TEST_F(RegistryTest, EntityHasAggressiveIntent)
{
	auto arena = get_duel_arena(registry);

	// They should have intent type attack
	const Intent ia = AISystem::get_npc_intent(registry, arena.creatures[0]);
	const Intent ib = AISystem::get_npc_intent(registry, arena.creatures[1]);

	EXPECT_TRUE(ia.type == Intent::Type::Attack) << static_cast<size_t>(ia.type);
	EXPECT_TRUE(ia.attack != nullptr);
	EXPECT_TRUE(ib.type == Intent::Type::Attack) << static_cast<size_t>(ib.type);
	EXPECT_TRUE(ib.attack != nullptr);
}

TEST_F(RegistryTest, CreaturesFistFight)
{
	auto arena = get_duel_arena(registry);
	for (size_t i = 0; i < 10; ++i)
	{
		ActionSystem::act_round(registry, arena.cave_idx);
	}
	EXPECT_TRUE(registry.all_of<Dead>(arena.creatures[0]) || registry.all_of<Dead>(arena.creatures[1]));
}
