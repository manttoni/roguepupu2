#include <gtest/gtest.h>
#include "helpers.hpp"
#include "database/EntityFactory.hpp"
#include "systems/position/MovementSystem.hpp"
#include "utils/ECS.hpp"
#include "external/entt/entt.hpp"

/* Test all directions diagonal included
 * */
TEST_F(RegistryTest, BasicMove)
{
	const size_t cave_size = 5; // caves are squares
	const auto cave_idx = get_test_cave_idx(registry, cave_size, TestCaveType::Floor);
	const auto& cave = ECS::get_cave(registry, cave_idx);
	const auto mid_pos = cave.middle_position();
	const auto entity = EntityFactory::instance().create_entity(registry, "test_creature", mid_pos);
	const auto neighbors = cave.get_nearby_positions(mid_pos, 1.5);
	for (const auto neighbor : neighbors)
	{
		EXPECT_TRUE(registry.get<Position>(entity) == mid_pos);
		MovementSystem::move(registry, entity, neighbor);
		EXPECT_TRUE(registry.get<Position>(entity) == neighbor);
		MovementSystem::move(registry, entity, mid_pos);
	}
}
