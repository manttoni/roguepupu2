#include "fixtures/ItemInteractionTest.hpp"

namespace
{
	namespace Event = Game::Event;
	namespace Value = Game::Component::Value;

	class PickUpTest : public ItemInteractionTest {};

	TEST_F(PickUpTest, PicksUpGroundItemAndKeepsOtherItemsUnchanged)
	{
		const auto item = make_ground_item(center);
		const auto carried = make_carried_item();
		const auto other_position = center + Vec2<int>{0, 1};
		const auto other = make_ground_item(other_position);

		const auto result = simulation->simulate(Event::PickUp{
			.entity = actor,
			.item = item
		});

		ASSERT_EQ(result.outcome, Event::Outcome::Accepted);
		expect_carried(item);
		expect_carried(carried);
		expect_on_ground(other, other_position);
		EXPECT_EQ(inventory().size(), 2u);
		EXPECT_EQ(registry().get<Value::Position>(actor).value, center);
	}

	TEST_F(PickUpTest, RejectsActorWithoutInventoryAndLeavesItemOnGround)
	{
		const auto item = make_ground_item(center);
		registry().remove<Inventory>(actor);

		const auto result = simulation->simulate(Event::PickUp{
			.entity = actor,
			.item = item
		});

		expect_rejected(result);
		ASSERT_TRUE(registry().valid(item));
		ASSERT_TRUE(registry().all_of<Value::Position>(item));
		EXPECT_EQ(registry().get<Value::Position>(item).value, center);
		EXPECT_FALSE(registry().all_of<Inventory>(actor));
	}

	TEST_F(PickUpTest, RejectsPickingUpSameItemTwiceWithoutDuplicatingIt)
	{
		const auto item = make_ground_item(center);
		const Event::PickUp event{.entity = actor, .item = item};
		const auto first = simulation->simulate(event);
		ASSERT_EQ(first.outcome, Event::Outcome::Accepted);
		expect_carried(item);
		const auto before = inventory();

		const auto second = simulation->simulate(event);

		expect_rejected(second);
		expect_carried(item);
		EXPECT_EQ(inventory(), before);
	}

	TEST_F(PickUpTest, CanPickUpDropAndPickUpAgainWithoutDuplicatingItem)
	{
		const auto item = make_ground_item(center);
		const Event::PickUp pickup{.entity = actor, .item = item};
		const auto first = simulation->simulate(pickup);
		ASSERT_EQ(first.outcome, Event::Outcome::Accepted);
		expect_carried(item);

		const auto drop = simulation->simulate(Event::Drop{
			.entity = actor,
			.item = item
		});
		ASSERT_EQ(drop.outcome, Event::Outcome::Accepted);
		expect_on_ground(item, center);
		EXPECT_TRUE(inventory().empty());

		const auto second = simulation->simulate(pickup);

		ASSERT_EQ(second.outcome, Event::Outcome::Accepted);
		expect_carried(item);
		EXPECT_EQ(inventory().size(), 1u);
	}

	TEST_F(PickUpTest, CanPickUpThrownItemAtItsDestination)
	{
		const auto item = make_carried_item();
		const auto destination = center + Vec2<int>{0, 1};
		const auto thrown = simulation->simulate(Event::Throw{
			.entity = actor,
			.item = item,
			.target = destination
		});
		ASSERT_EQ(thrown.outcome, Event::Outcome::Accepted);
		expect_on_ground(item, destination);

		const auto pickup = simulation->simulate(Event::PickUp{
			.entity = actor,
			.item = item
		});

		ASSERT_EQ(pickup.outcome, Event::Outcome::Accepted);
		expect_carried(item);
		EXPECT_EQ(inventory().size(), 1u);
	}
}
