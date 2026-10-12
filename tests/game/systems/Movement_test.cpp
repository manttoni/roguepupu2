#include "fixtures/WorldTest.hpp"
#include "game/system/Movement.hpp"

namespace
{
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

			void SetUp() override
			{
				WorldTest::SetUp();

				using namespace Game::Component;

				registry.emplace<Resource::MovementPoints>(actor, 10.0);
				registry.emplace<Value::CollisionMovement>(actor, true);
			}

			void add_obstacle(const Game::World::GlobalPosition position)
			{
				auto* cell = grid.find_cell(position);

				ASSERT_NE(cell, nullptr)
					<< "Missing cell at " << position;

				cell->form = Game::Enum::Form::Wall;
			}
	};

	TEST_F(MovementTest, CanMoveCardinalNoObstacle)
	{
		for (const auto& direction : cardinal_directions)
		{
			SCOPED_TRACE(direction.to_string());

			const auto target_position = center + direction;

			EXPECT_TRUE(
					Game::System::Movement::can_move(
						registry, grid, actor, target_position));
		}
	}

	TEST_F(MovementTest, CanMoveDiagonalNoObstacle)
	{
		for (const auto& direction : diagonal_directions)
		{
			SCOPED_TRACE(direction.to_string());

			const auto target_position = center + direction;

			EXPECT_TRUE(
					Game::System::Movement::can_move(
						registry, grid, actor, target_position));
		}
	}

	TEST_F(MovementTest, CannotMoveCardinalObstacle)
	{
		for (const auto& direction : cardinal_directions)
		{
			SCOPED_TRACE(direction.to_string());

			const auto target_position = center + direction;

			add_obstacle(target_position);

			EXPECT_FALSE(
					Game::System::Movement::can_move(
						registry, grid, actor, target_position));

			reset_grid();
		}
	}

	TEST_F(MovementTest, CanMoveCardinalPastObstacle)
	{
		for (const auto& direction : diagonal_directions)
			add_obstacle(center + direction);

		for (const auto& direction : cardinal_directions)
		{
			SCOPED_TRACE(direction.to_string());

			EXPECT_TRUE(
					Game::System::Movement::can_move(
						registry, grid, actor, center + direction));
		}
	}

	TEST_F(MovementTest, CannotMoveDiagonalObstacle)
	{
		for (const auto& direction : diagonal_directions)
		{
			SCOPED_TRACE(direction.to_string());

			const auto target_position = center + direction;

			add_obstacle(target_position);

			EXPECT_FALSE(
					Game::System::Movement::can_move(
						registry, grid, actor, target_position));

			reset_grid();
		}
	}

	TEST_F(MovementTest, CannotMoveDiagonalObstacleCorner)
	{
		for (const auto& direction : diagonal_directions)
		{
			SCOPED_TRACE(direction.to_string());

			const auto target_position = center + direction;
			const Game::World::GlobalPosition corner1{
				target_position.y,
					center.x};
			const Game::World::GlobalPosition corner2{
				center.y,
					target_position.x};

			add_obstacle(corner1);

			EXPECT_FALSE(
					Game::System::Movement::can_move(
						registry, grid, actor, target_position));

			reset_grid();

			add_obstacle(corner2);

			EXPECT_FALSE(
					Game::System::Movement::can_move(
						registry, grid, actor, target_position));

			reset_grid();

			add_obstacle(corner1);
			add_obstacle(corner2);
			EXPECT_FALSE(
					Game::System::Movement::can_move(
						registry, grid, actor, target_position));

			reset_grid();
		}
	}
}

