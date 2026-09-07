#include <gtest/gtest.h>
#include <stddef.h>
#include <optional>
#include <vector>

#include "helpers.hpp"
#include "systems/Movement.hpp"
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
	const auto cave_idx = TestHelpers::get_cave_idx(registry, cave_size, TestHelpers::CaveType::Floor);
	const auto& cave = ECS::get_cave(registry, cave_idx);
	const auto mid_pos = cave.middle_position();
	const auto entity = registry.create();
	registry.emplace<Domain::Position>(entity, mid_pos);
	const auto neighbors = cave.get_nearby_positions(mid_pos, 1.5);
	for (const auto neighbor : neighbors)
	{
		EXPECT_TRUE(registry.get<Domain::Position>(entity) == mid_pos);
		EXPECT_TRUE(System::Movement::can_move(registry, mid_pos, neighbor));
		System::Movement::move(registry, entity, neighbor);
		EXPECT_TRUE(registry.get<Domain::Position>(entity) == neighbor);
		System::Movement::move(registry, entity, mid_pos);
	}
}

/* Walking diagonally is not possible if there is a corner to go around
 * */
TEST_F(RegistryTest, CornerMove)
{
	const size_t cave_size = 5;
	const auto cave_idx = TestHelpers::get_cave_idx(registry, cave_size, TestHelpers::CaveType::Floor);
	const auto& cave = ECS::get_cave(registry, cave_idx);
	const auto mid_pos = cave.middle_position();
	ECS::get_cell(registry, mid_pos).set_type(Domain::Cell::Type::Rock);

	const auto left_pos = Domain::Position(mid_pos.cell_idx - 1, cave_idx); // one to the left

	const auto north = Domain::Position(mid_pos.cell_idx - cave.get_size(), cave_idx); // one step north
	const auto south = Domain::Position(mid_pos.cell_idx + cave.get_size(), cave_idx); // one step south

	// moving to either one should not be possible, because has to go around rock

	EXPECT_FALSE(System::Movement::can_move(registry, left_pos, north));
	EXPECT_FALSE(System::Movement::can_move(registry, left_pos, south));
}

/* Moving is not possible if target cell has cell type rock
 * */
TEST_F(RegistryTest, RockMove)
{
	const size_t cave_size = 5;
	const auto cave_idx = TestHelpers::get_cave_idx(registry, cave_size, TestHelpers::CaveType::Rock);
	auto& cave = ECS::get_cave(registry, cave_idx);
	const auto mid_pos = cave.middle_position();
	ECS::get_cell(registry, mid_pos).set_type(Domain::Cell::Type::Floor);
	for (const auto neighbor : cave.get_nearby_positions(mid_pos, 1.5))
	{
		// All neighbors are solid rock, should not be able to move anywhere
		EXPECT_FALSE(System::Movement::can_move(registry, mid_pos, neighbor));
	}
}
