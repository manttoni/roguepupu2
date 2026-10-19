#include <algorithm>
#include <array>
#include <cstddef>
#include <utility>
#include <vector>

#include "fixtures/WorldTest.hpp"
#include "game/component/Runtime.hpp"
#include "game/system/Vision.hpp"

namespace
{
	namespace Vision = Game::System::Vision;
	namespace Value = Game::Component::Value;
	using Position = Game::World::GlobalPosition;
	using VisionCache = Game::Component::Runtime::Vision;
	using Form = Game::Enum::Form;

	template <typename T>
	bool contains(const std::vector<T>& values, const T& value)
	{
		return std::find(values.begin(), values.end(), value) != values.end();
	}

	class VisionTest : public WorldTest
	{
		protected:
			void SetUp() override
			{
				WorldTest::SetUp();
				registry.emplace<Value::VisionRange>(actor, 10);
				registry.emplace<VisionCache>(actor);
			}

			void set_form(Position position, Form form)
			{
				auto* cell = grid.find_cell(position);
				ASSERT_NE(cell, nullptr) << "Missing cell at " << position;
				cell->form = form;
			}

			entt::entity make_entity(Position position, bool opaque = false)
			{
				const auto entity = registry.create();
				registry.emplace<Value::Position>(entity, position);
				registry.emplace<Value::CollisionVision>(entity, opaque);
				return entity;
			}

			void expect_los(Position from, Position to, bool expected) const
			{
				SCOPED_TRACE(from.to_string() + " -> " + to.to_string());
				EXPECT_EQ(Vision::los(registry, grid, from, to), expected);
				EXPECT_EQ(Vision::los(registry, grid, to, from), expected)
					<< "LOS must also give the same result in reverse";
			}

			void refresh()
			{
				Vision::update_vision(registry, grid, actor);
			}

			const VisionCache& cache() const
			{
				return registry.get<VisionCache>(actor);
			}
	};

	TEST_F(VisionTest, LosSamePositionIsVisibleEvenWhenOpaque)
	{
		set_form(center, Form::Wall);
		make_entity(center, true);

		EXPECT_TRUE(Vision::los(registry, grid, center, center));
	}

	TEST_F(VisionTest, LosClearPathsAreVisibleInAllOctants)
	{
		const std::vector<Vec2<int>> offsets{
			{-6, 0}, {6, 0}, {0, -6}, {0, 6},
			{-6, -6}, {-6, 6}, {6, -6}, {6, 6},
			{-2, -5}, {-2, 5}, {2, -5}, {2, 5},
			{-5, -2}, {-5, 2}, {5, -2}, {5, 2}
		};

		for (const auto& offset : offsets)
			expect_los(center, center + offset, true);
	}

	TEST_F(VisionTest, LosAllowsOpaqueTerrainAndEntitiesAtBothEndpoints)
	{
		const auto target = center + Vec2<int>{0, 6};
		set_form(center, Form::Wall);
		set_form(target, Form::Wall);
		registry.emplace<Value::CollisionVision>(actor, true);
		make_entity(target, true);

		expect_los(center, target, true);
	}

	TEST_F(VisionTest, LosIntermediateTerrainWallBlocks)
	{
		set_form(center + Vec2<int>{0, 3}, Form::Wall);

		expect_los(center, center + Vec2<int>{0, 6}, false);
	}

	TEST_F(VisionTest, LosIntermediateOpaqueEntityBlocks)
	{
		make_entity(center + Vec2<int>{0, 3}, true);

		expect_los(center, center + Vec2<int>{0, 6}, false);
	}

	TEST_F(VisionTest, LosTransparentEntityDoesNotBlockEvenIfItBlocksMovement)
	{
		const auto obstacle = make_entity(center + Vec2<int>{0, 3}, false);
		registry.emplace<Value::CollisionMovement>(obstacle, true);

		expect_los(center, center + Vec2<int>{0, 6}, true);
	}

	TEST_F(VisionTest, LosEntityWithoutCollisionVisionDoesNotBlock)
	{
		const auto entity = registry.create();
		registry.emplace<Value::Position>(entity, center + Vec2<int>{0, 3});

		expect_los(center, center + Vec2<int>{0, 6}, true);
	}

	TEST_F(VisionTest, LosIgnoresBlockersOutsideTheRay)
	{
		set_form(center + Vec2<int>{-1, 3}, Form::Wall);
		make_entity(center + Vec2<int>{1, 3}, true);

		expect_los(center, center + Vec2<int>{0, 6}, true);
	}

	TEST_F(VisionTest, LosCornerWallsBlockOnEitherSideAndWhenBothSidesAreBlocked)
	{
		const std::vector<Vec2<int>> directions{
			{-1, -1}, {-1, 1}, {1, -1}, {1, 1}
		};

		for (const auto& direction : directions)
		{
			const auto target = center + Vec2<int>{
				direction.y * 3, direction.x * 3};
			const auto side_x = center + Vec2<int>{0, direction.x};
			const auto side_y = center + Vec2<int>{direction.y, 0};

			set_form(side_x, Form::Wall);
			expect_los(center, target, false);

			set_form(side_x, Form::Floor);
			set_form(side_y, Form::Wall);
			expect_los(center, target, false);

			set_form(side_x, Form::Wall);
			expect_los(center, target, false);

			reset_grid();
		}
	}

	TEST_F(VisionTest, LosCornerEntitiesBlockOnEitherSideAndWhenBothSidesAreBlocked)
	{
		const std::vector<Vec2<int>> directions{
			{-1, -1}, {-1, 1}, {1, -1}, {1, 1}
		};

		for (const auto& direction : directions)
		{
			const auto target = center + Vec2<int>{
				direction.y * 3, direction.x * 3};
			const auto side_x = make_entity(
					center + Vec2<int>{0, direction.x}, true);
			const auto side_y = make_entity(
					center + Vec2<int>{direction.y, 0}, false);

			expect_los(center, target, false);

			registry.get<Value::CollisionVision>(side_x).value = false;
			registry.get<Value::CollisionVision>(side_y).value = true;
			expect_los(center, target, false);

			registry.get<Value::CollisionVision>(side_x).value = true;
			expect_los(center, target, false);

			registry.destroy(side_x);
			registry.destroy(side_y);
		}
	}

	TEST_F(VisionTest, LosChecksAllCrossedCellsForShallowAndSteepRays)
	{
		// These rays cross the side cell (sy, sx), which a thin line can miss.
		for (const int sy : {-1, 1})
		{
			for (const int sx : {-1, 1})
			{
				set_form(center + Vec2<int>{sy, sx}, Form::Wall);

				expect_los(center, center + Vec2<int>{2 * sy, 5 * sx}, false);
				expect_los(center, center + Vec2<int>{5 * sy, 2 * sx}, false);

				reset_grid();
			}
		}
	}

	TEST_F(VisionTest, LosRejectsUnloadedEndpoints)
	{
		const auto unloaded = center + Vec2<int>{
			Game::World::Chunk::height * 3, 0};
		ASSERT_EQ(grid.find_cell(unloaded), nullptr);

		expect_los(center, unloaded, false);
		EXPECT_FALSE(Vision::los(registry, grid, unloaded, unloaded));
	}

	TEST_F(VisionTest, LosRejectsUnloadedCellsBetweenLoadedEndpoints)
	{
		const Game::World::ChunkPosition far_chunk{2, 0};
		ASSERT_TRUE(grid.add(far_chunk, Game::World::Chunk{
				Game::Enum::Material::Stone, Form::Floor}));
		const auto target = Game::World::to_global(
				far_chunk, Game::World::LocalPosition{center.y, center.x});
		ASSERT_NE(grid.find_cell(target), nullptr);

		expect_los(center, target, false);
	}

	TEST_F(VisionTest, LosCrossesPositiveAndNegativeChunkBoundaries)
	{
		ASSERT_TRUE(grid.add({-1, 0}, Game::World::Chunk{
				Game::Enum::Material::Stone, Form::Floor}));
		const Position negative_start{-1, center.x};
		const Position negative_end{1, center.x};

		expect_los(negative_start, negative_end, true);
		set_form({0, center.x}, Form::Wall);
		expect_los(negative_start, negative_end, false);
		set_form({0, center.x}, Form::Floor);

		ASSERT_TRUE(grid.add({1, 0}, Game::World::Chunk{
				Game::Enum::Material::Stone, Form::Floor}));
		const int boundary = Game::World::Chunk::height;
		const Position positive_start{boundary - 1, center.x};
		const Position positive_end{boundary + 1, center.x};

		expect_los(positive_start, positive_end, true);
		set_form({boundary, center.x}, Form::Wall);
		expect_los(positive_start, positive_end, false);
	}

	TEST_F(VisionTest, UpdateCreatesMissingVisionCache)
	{
		registry.remove<VisionCache>(actor);
		ASSERT_FALSE(registry.all_of<VisionCache>(actor));

		refresh();

		ASSERT_TRUE(registry.all_of<VisionCache>(actor));
		EXPECT_TRUE(contains(cache().visible_positions, center));
	}

	TEST_F(VisionTest, UpdateIncludesCenterAndCircleBoundaryButExcludesOutside)
	{
		refresh();

		EXPECT_EQ(cache().visible_positions.size(), 317u);
		EXPECT_TRUE(contains(cache().visible_positions, center));
		EXPECT_TRUE(contains(
				cache().visible_positions, center + Vec2<int>{0, 10}));
		EXPECT_TRUE(contains(
				cache().visible_positions, center + Vec2<int>{6, 8}));
		EXPECT_FALSE(contains(
				cache().visible_positions, center + Vec2<int>{0, 11}));
		EXPECT_FALSE(contains(
				cache().visible_positions, center + Vec2<int>{7, 8}));
		EXPECT_FALSE(contains(
				cache().visible_positions, center + Vec2<int>{8, 8}));
		EXPECT_TRUE(cache().seen_positions.contains(center));
	}

	TEST_F(VisionTest, UpdateUsesVisionRangeComponentIncludingZeroRange)
	{
		// Regression: a hardcoded range of 10 ignores the entity's component.
		const std::array<std::pair<int, std::size_t>, 3> cases{{
			{0, 1}, {2, 13}, {5, 81}
		}};

		for (const auto& [range, expected_count] : cases)
		{
			SCOPED_TRACE(::testing::Message() << "VisionRange=" << range);
			registry.get<Value::VisionRange>(actor).value = range;

			refresh();

			EXPECT_EQ(cache().visible_positions.size(), expected_count);
			EXPECT_TRUE(contains(cache().visible_positions, center));
			EXPECT_TRUE(contains(
						cache().visible_positions, center + Vec2<int>{0, range}));
			EXPECT_FALSE(contains(
						cache().visible_positions, center + Vec2<int>{0, range + 1}));
		}
	}

	TEST_F(VisionTest, UpdateSeesOpaqueEntityButNotTheEntityBehindIt)
	{
		const auto blocker_position = center + Vec2<int>{0, 2};
		const auto hidden_position = center + Vec2<int>{0, 3};
		const auto blocker = make_entity(blocker_position, true);
		const auto hidden = make_entity(hidden_position);

		refresh();

		EXPECT_TRUE(contains(cache().visible_positions, blocker_position));
		EXPECT_TRUE(contains(cache().visible_entities, blocker));
		EXPECT_FALSE(contains(cache().visible_positions, hidden_position));
		EXPECT_FALSE(contains(cache().visible_entities, hidden));
		EXPECT_FALSE(cache().seen_positions.contains(hidden_position));
	}

	TEST_F(VisionTest, UpdateSeesWallButNotThePositionBehindIt)
	{
		const auto wall = center + Vec2<int>{0, 2};
		const auto behind = center + Vec2<int>{0, 3};
		set_form(wall, Form::Wall);

		refresh();

		EXPECT_TRUE(contains(cache().visible_positions, wall));
		EXPECT_FALSE(contains(cache().visible_positions, behind));
		EXPECT_TRUE(cache().seen_positions.contains(wall));
		EXPECT_FALSE(cache().seen_positions.contains(behind));
	}

	TEST_F(VisionTest, UpdateCollectsEveryEntityAtAVisiblePosition)
	{
		const auto position = center + Vec2<int>{0, 2};
		const auto first = make_entity(position);
		const auto second = make_entity(position);

		refresh();

		EXPECT_EQ(std::count(cache().visible_entities.begin(),
				cache().visible_entities.end(), first), 1);
		EXPECT_EQ(std::count(cache().visible_entities.begin(),
				cache().visible_entities.end(), second), 1);
	}

	TEST_F(VisionTest, UpdateExcludesEntitiesOutsideTheVisionRange)
	{
		const auto outside = make_entity(center + Vec2<int>{0, 11});

		refresh();

		EXPECT_FALSE(contains(cache().visible_entities, outside));
	}

	TEST_F(VisionTest, UpdateClearsOccludedVisibilityButPreservesExploredPositions)
	{
		const auto position = center + Vec2<int>{0, 3};
		const auto entity = make_entity(position);
		refresh();
		ASSERT_TRUE(contains(cache().visible_positions, position));
		ASSERT_TRUE(contains(cache().visible_entities, entity));

		set_form(center + Vec2<int>{0, 2}, Form::Wall);
		refresh();

		EXPECT_FALSE(contains(cache().visible_positions, position));
		EXPECT_FALSE(contains(cache().visible_entities, entity));
		EXPECT_TRUE(cache().seen_positions.contains(position));
	}

	TEST_F(VisionTest, UpdateAfterObserverMovementKeepsOldAndAddsNewExploredPositions)
	{
		const auto old_edge = center + Vec2<int>{0, -10};
		const auto new_edge = center + Vec2<int>{0, 20};
		refresh();
		ASSERT_TRUE(cache().seen_positions.contains(old_edge));
		ASSERT_FALSE(cache().seen_positions.contains(new_edge));

		const auto new_position = center + Vec2<int>{0, 12};
		registry.replace<Value::Position>(actor, new_position);
		refresh();

		EXPECT_TRUE(contains(cache().visible_positions, new_position));
		EXPECT_TRUE(contains(cache().visible_positions, new_edge));
		EXPECT_FALSE(contains(cache().visible_positions, old_edge));
		EXPECT_TRUE(cache().seen_positions.contains(old_edge));
		EXPECT_TRUE(cache().seen_positions.contains(new_edge));
	}

	TEST_F(VisionTest, UpdateAfterTargetMovementDropsEntitiesThatLeftVision)
	{
		const auto target = make_entity(center + Vec2<int>{0, 2});
		refresh();
		ASSERT_TRUE(contains(cache().visible_entities, target));

		registry.replace<Value::Position>(target, center + Vec2<int>{0, 11});
		refresh();

		EXPECT_FALSE(contains(cache().visible_entities, target));
	}

	TEST_F(VisionTest, UpdateAfterRemovingBlockerRevealsPreviouslyUnseenPositions)
	{
		const auto blocker = center + Vec2<int>{0, 2};
		const auto behind = center + Vec2<int>{0, 3};
		set_form(blocker, Form::Wall);
		refresh();
		ASSERT_FALSE(cache().seen_positions.contains(behind));

		set_form(blocker, Form::Floor);
		refresh();

		EXPECT_TRUE(contains(cache().visible_positions, behind));
		EXPECT_TRUE(cache().seen_positions.contains(behind));
	}

	TEST_F(VisionTest, UpdateRemovesDestroyedEntitiesFromCurrentVisibility)
	{
		const auto target = make_entity(center + Vec2<int>{0, 2});
		refresh();
		ASSERT_TRUE(contains(cache().visible_entities, target));

		registry.destroy(target);
		refresh();

		EXPECT_FALSE(contains(cache().visible_entities, target));
	}

	TEST_F(VisionTest, RepeatedUpdateDoesNotDuplicateCurrentVisibility)
	{
		make_entity(center + Vec2<int>{0, 2});
		refresh();
		const auto positions = cache().visible_positions;
		const auto entities = cache().visible_entities;
		const auto seen = cache().seen_positions;

		refresh();

		EXPECT_EQ(cache().visible_positions, positions);
		EXPECT_EQ(cache().visible_entities, entities);
		EXPECT_EQ(cache().seen_positions, seen);
	}

	TEST_F(VisionTest, UpdatingOneObserverDoesNotChangeAnotherObserversCache)
	{
		const auto other_position = center + Vec2<int>{0, 15};
		const auto other = make_entity(other_position);
		registry.emplace<Value::VisionRange>(other, 10);
		registry.emplace<VisionCache>(other);
		refresh();
		const auto first_positions = cache().visible_positions;
		const auto first_entities = cache().visible_entities;
		const auto first_seen = cache().seen_positions;

		Vision::update_vision(registry, grid, other);

		const auto& other_cache = registry.get<VisionCache>(other);
		EXPECT_TRUE(contains(other_cache.visible_positions, other_position));
		EXPECT_FALSE(contains(other_cache.visible_positions, center));
		EXPECT_FALSE(other_cache.seen_positions.contains(center));
		EXPECT_FALSE(contains(cache().visible_positions, other_position));
		EXPECT_EQ(cache().visible_positions, first_positions);
		EXPECT_EQ(cache().visible_entities, first_entities);
		EXPECT_EQ(cache().seen_positions, first_seen);
	}
}

