#include <variant>

#include <gtest/gtest.h>

#include "fixtures/ItemInteractionTest.hpp"

namespace
{
	namespace Component = Game::Component;
	namespace Event = Game::Event;
	using EntityPosition = Component::Value::Position;
	using MovementPoints = Component::Resource::MovementPoints;
	using CollisionMovement = Component::Value::CollisionMovement;

	class MoveEventTest : public ItemInteractionTest
	{
		protected:
			const Vec2<int> direction{0, 1};

			void set_destination_to_wall()
			{
				simulation->get_grid().find_cell(center + direction)->form =
					Game::Enum::Form::Wall;
			}
	};

	TEST_F(MoveEventTest, BumpThroughWallWithCollisionDisabledMovesAndSpendsPointsOnce)
	{
		set_destination_to_wall();
		registry().get<CollisionMovement>(actor).value = false;
		registry().get<MovementPoints>(actor).current = 2.0;

		const auto node = simulation->simulate(Event::Bump{
			.entity = actor, .direction = direction});

		ASSERT_EQ(node.outcome, Event::Outcome::Accepted);
		EXPECT_EQ(std::get<Event::Bump>(node.event).entity, actor);
		ASSERT_EQ(node.consequences.size(), 3u);
		EXPECT_TRUE(std::holds_alternative<Event::LoseMovementPoints>(
			node.consequences[0].event));
		EXPECT_TRUE(std::holds_alternative<Event::LeavePosition>(
			node.consequences[1].event));
		EXPECT_TRUE(std::holds_alternative<Event::EnterPosition>(
			node.consequences[2].event));
		for (const auto& consequence : node.consequences)
			EXPECT_EQ(consequence.outcome, Event::Outcome::Accepted);
		EXPECT_EQ(registry().get<EntityPosition>(actor).value, center + direction);
		EXPECT_DOUBLE_EQ(registry().get<MovementPoints>(actor).current, 1.0);
	}

	TEST_F(MoveEventTest, BumpAgainstWallWithCollisionEnabledHasNoConsequences)
	{
		set_destination_to_wall();
		registry().get<CollisionMovement>(actor).value = true;
		registry().get<MovementPoints>(actor).current = 2.0;

		const auto node = simulation->simulate(Event::Bump{
			.entity = actor, .direction = direction});

		expect_rejected(node);
		EXPECT_EQ(registry().get<EntityPosition>(actor).value, center);
		EXPECT_DOUBLE_EQ(registry().get<MovementPoints>(actor).current, 2.0);
	}

	TEST_F(MoveEventTest, DirectMoveReportsMovementBlockedAndPreservesState)
	{
		set_destination_to_wall();
		registry().get<CollisionMovement>(actor).value = true;
		const auto points = registry().get<MovementPoints>(actor).current;

		const auto node = simulation->simulate(Event::Move{
			.entity = actor, .from = center, .to = center + direction});

		EXPECT_EQ(node.outcome, Event::Outcome::MovementBlocked);
		EXPECT_TRUE(node.consequences.empty());
		EXPECT_EQ(registry().get<EntityPosition>(actor).value, center);
		EXPECT_DOUBLE_EQ(registry().get<MovementPoints>(actor).current, points);
	}

	TEST_F(MoveEventTest, BumpReportsInsufficientResourcesWithoutMovingOrSpendingPoints)
	{
		registry().get<CollisionMovement>(actor).value = false;
		registry().get<MovementPoints>(actor).current = 0.5;

		const auto node = simulation->simulate(Event::Bump{
			.entity = actor, .direction = direction});

		EXPECT_EQ(node.outcome, Event::Outcome::InsufficientResources);
		EXPECT_TRUE(node.consequences.empty());
		EXPECT_EQ(registry().get<EntityPosition>(actor).value, center);
		EXPECT_DOUBLE_EQ(registry().get<MovementPoints>(actor).current, 0.5);
	}
}
