#include "databases/EntityDatabase.hpp"
#include "fixtures/WorldTest.hpp"
#include "game/Simulation.hpp"
#include "game/system/Movement.hpp"

#include <cmath>
#include <vector>

namespace
{
	namespace Movement = Game::System::Movement;
	using Game::World::GlobalPosition;

	class MovementTest : public WorldTest
	{
	protected:
		const std::vector<Vec2<int>> cardinal_directions{
			{-1,  0}, // Up
			{ 1,  0}, // Down
			{ 0, -1}, // Left
			{ 0,  1}, // Right
		};

		const std::vector<Vec2<int>> diagonal_directions{
			{-1, -1}, // Up-left
			{-1,  1}, // Up-right
			{ 1, -1}, // Down-left
			{ 1,  1}, // Down-right
		};

		void add_obstacle(const GlobalPosition position)
		{
			auto* cell = grid.find_cell(position);

			ASSERT_NE(cell, nullptr)
				<< "Missing cell at " << position;

			cell->form = Game::Enum::Form::Wall;
		}
	};

	TEST_F(MovementTest, CardinalPathIsClear)
	{
		for (const auto& direction : cardinal_directions)
		{
			SCOPED_TRACE(direction.to_string());

			const auto target = center + direction;
			EXPECT_TRUE(Movement::has_path(registry, grid, center, target));
		}
	}

	TEST_F(MovementTest, DiagonalPathIsClear)
	{
		for (const auto& direction : diagonal_directions)
		{
			SCOPED_TRACE(direction.to_string());

			const auto target = center + direction;
			EXPECT_TRUE(Movement::has_path(registry, grid, center, target));
		}
	}

	TEST_F(MovementTest, WallBlocksCardinalPath)
	{
		for (const auto& direction : cardinal_directions)
		{
			SCOPED_TRACE(direction.to_string());

			const auto target = center + direction;
			add_obstacle(target);

			EXPECT_FALSE(Movement::has_path(registry, grid, center, target));

			reset_grid();
		}
	}

	TEST_F(MovementTest, CardinalPathsStayClearBesideDiagonalWalls)
	{
		for (const auto& direction : diagonal_directions)
			add_obstacle(center + direction);

		for (const auto& direction : cardinal_directions)
		{
			SCOPED_TRACE(direction.to_string());

			const auto target = center + direction;
			EXPECT_TRUE(Movement::has_path(registry, grid, center, target));
		}
	}

	TEST_F(MovementTest, WallBlocksDiagonalDestination)
	{
		for (const auto& direction : diagonal_directions)
		{
			SCOPED_TRACE(direction.to_string());

			const auto target = center + direction;
			add_obstacle(target);

			EXPECT_FALSE(Movement::has_path(registry, grid, center, target));

			reset_grid();
		}
	}

	TEST_F(MovementTest, EitherCornerCanBlockDiagonalPath)
	{
		for (const auto& direction : diagonal_directions)
		{
			SCOPED_TRACE(direction.to_string());

			const auto target = center + direction;
			const GlobalPosition corner1{target.y, center.x};
			const GlobalPosition corner2{center.y, target.x};

			add_obstacle(corner1);
			EXPECT_FALSE(Movement::has_path(registry, grid, center, target));
			reset_grid();

			add_obstacle(corner2);
			EXPECT_FALSE(Movement::has_path(registry, grid, center, target));
			reset_grid();

			add_obstacle(corner1);
			add_obstacle(corner2);
			EXPECT_FALSE(Movement::has_path(registry, grid, center, target));
			reset_grid();
		}
	}

	TEST(MovementCostTest, CardinalAndDiagonalStepCosts)
	{
		Game::Simulation simulation{
			"item-interactions-test", EntityDatabase{}, "player_default"};
		const GlobalPosition origin{
			Game::World::Chunk::height / 2,
			Game::World::Chunk::width / 2};
		auto* chunk = simulation.get_grid().find_chunk(Game::World::to_chunk(origin));
		ASSERT_NE(chunk, nullptr);
		*chunk = Game::World::Chunk{
			Game::Enum::Material::Stone, Game::Enum::Form::Floor};

		const auto cardinal = origin + Vec2<int>{0, 1};
		const auto diagonal = origin + Vec2<int>{1, 1};
		EXPECT_DOUBLE_EQ(1.0, Movement::cost(simulation, origin, cardinal));
		EXPECT_DOUBLE_EQ(std::sqrt(2.0), Movement::cost(simulation, origin, diagonal));
	}
}

