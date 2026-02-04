#include <gtest/gtest.h>
#include <stddef.h>
#include <optional>
#include <vector>

#include "helpers.hpp"
#include "database/EntityFactory.hpp"
#include "systems/position/MovementSystem.hpp"
#include "utils/ECS.hpp"
#include "external/entt/entt.hpp"
#include "domain/Cave.hpp"
#include "domain/Cell.hpp"
#include "domain/Position.hpp"
#include "gtest/gtest.h"

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
		EXPECT_TRUE(MovementSystem::can_move(registry, mid_pos, neighbor));
		MovementSystem::move(registry, entity, neighbor);
		EXPECT_TRUE(registry.get<Position>(entity) == neighbor);
		MovementSystem::move(registry, entity, mid_pos);
	}
}

/* Walking diagonally is not possible if there is a corner to go around
 * */
TEST_F(RegistryTest, CornerMove)
{
	const size_t cave_size = 5;
	const auto cave_idx = get_test_cave_idx(registry, cave_size, TestCaveType::Floor);
	const auto& cave = ECS::get_cave(registry, cave_idx);
	const auto mid_pos = cave.middle_position();
	ECS::get_cell(registry, mid_pos).set_type(Cell::Type::Rock);

	const auto left_pos = Position(mid_pos.cell_idx - 1, cave_idx); // one to the left

	const auto north = Position(mid_pos.cell_idx - cave.get_size(), cave_idx); // one step north
	const auto south = Position(mid_pos.cell_idx + cave.get_size(), cave_idx); // one step south

	// moving to either one should not be possible, because has to go around rock

	EXPECT_FALSE(MovementSystem::can_move(registry, left_pos, north)) << dump_cave(registry, cave_idx, {left_pos, north});;
	EXPECT_FALSE(MovementSystem::can_move(registry, left_pos, south)) << dump_cave(registry, cave_idx, {left_pos, south});
}

/* Moving is not possible if target cell has cell type rock
 * */
TEST_F(RegistryTest, RockMove)
{
	const size_t cave_size = 5;
	const auto cave_idx = get_test_cave_idx(registry, cave_size, TestCaveType::Rock);
	auto& cave = ECS::get_cave(registry, cave_idx);
	const auto mid_pos = cave.middle_position();
	ECS::get_cell(registry, mid_pos).set_type(Cell::Type::Floor);
	for (const auto neighbor : cave.get_nearby_positions(mid_pos, 1.5))
	{
		// All neighbors are solid rock, should not be able to move anywhere
		EXPECT_FALSE(MovementSystem::can_move(registry, mid_pos, neighbor)) << dump_cave(registry, cave_idx, {mid_pos, neighbor});
	}
}
