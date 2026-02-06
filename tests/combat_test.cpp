#include <gtest/gtest.h>
#include "helpers.hpp"
#include "external/entt/entt.hpp"
#include "database/EntityFactory.hpp"
#include "systems/action/AISystem.hpp"

TEST_F(RegistryTest, EntityHasAggressiveIntent)
{
	const auto creatures = EntityFactory::instance().create_entities(registry, "test_creature", 2);
	const auto cave_idx = TestHelpers::get_cave_idx(registry, 10, TestHelpers::CaveType::Room);
	const auto& cave = ECS::get_cave(registry, cave_idx);

	// Give them positions and opposing alignments and aggressive AI
	const auto mid_pos = cave.middle_position();
	registry.emplace<Position>(creatures[0], mid_pos);
	const Alignment a(Alignment::Type::LawfulGood);
	registry.emplace_or_replace<Alignment>(creatures[0], a);
	registry.emplace_or_replace<AI>(creatures[0]);
	registry.get<AI>(creatures[0]).aggressive = true;

	const auto left = Position(mid_pos.cell_idx - 1, cave_idx);
	registry.emplace<Position>(creatures[1], left);
	const Alignment b(Alignment::Type::ChaoticEvil);
	registry.emplace_or_replace<Alignment>(creatures[1], b);
	registry.emplace_or_replace<AI>(creatures[1]);
	registry.get<AI>(creatures[1]).aggressive = true;

	// They should have intent type attack
	const Intent ia = AISystem::get_npc_intent(registry, creatures[0]);
	const Intent ib = AISystem::get_npc_intent(registry, creatures[1]);

	EXPECT_TRUE(ia.type == Intent::Type::Attack) << static_cast<size_t>(ia.type);
	EXPECT_TRUE(ia.attack != nullptr);
	EXPECT_TRUE(ib.type == Intent::Type::Attack) << static_cast<size_t>(ib.type);
	EXPECT_TRUE(ib.attack != nullptr);
}
