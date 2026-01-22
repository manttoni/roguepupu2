/* This tests ensures that
 * VisionSystem::has_line_of_sight(const entt::registry&, const Position&, const Position&)
 * works properly.
 *
 * It has to check that parameters are interchangeable, and some simple cases.
 * Due to more or less unrealistic circumstances sometimes there is no line of sight,
 * even if it looks like there should be, especially the walls.
 *
 * The function uses Bresenhams line drawing algorithm with some modifications
 * */

#include <gtest/gtest.h>
#include "external/entt/entt.hpp"
#include "registry_fixture.hpp"
#include "test_cave.hpp"
#include "domain/Position.hpp"
#include "domain/Cave.hpp"
#include "utils/ECS.hpp"
#include "systems/perception/VisionSystem.hpp"

/* VisionSystem::has_line_of_sight should not care about order of params
 * Check all pairs of positions
 * */
TEST_F(RegistryTest, LOSSymmetricalEmpty)
{
	const size_t CAVE_SIZE = 5;
	const auto cave_idx = get_test_cave_idx(registry, CAVE_SIZE);
	const auto& cave = ECS::get_cave(registry, cave_idx);
	const auto positions = cave.get_positions();
	for (size_t i = 0; i < positions.size(); ++i)
	{
		for (size_t j = i + 1; j < positions.size(); ++j)
		{
			const auto los_ij =
				VisionSystem::has_line_of_sight(registry, positions[i], positions[j]);
			const auto los_ji =
				VisionSystem::has_line_of_sight(registry, positions[j], positions[i]);
			EXPECT_EQ(los_ij, los_ji);
		}
	}

}
