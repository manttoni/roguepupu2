#include <gtest/gtest.h>
#include "helpers.hpp"
#include "domain/Cave.hpp"

TEST(DensityTest, ReduceDensity)
{
	World world;
	const auto idx = world.new_cave(10, Cell::Type::Rock);
	auto& cave = world.get_cave(idx);
	const auto middle = cave.middle_position();
	auto& cell = cave.get_cell(middle);
	const auto density_before = cell.get_density();
	cell.reduce_density(1);
	const auto density_after = cell.get_density();
	ASSERT_GT(density_before, density_after);
}
