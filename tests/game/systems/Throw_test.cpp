#include "fixtures/ItemInteractionTest.hpp"

namespace
{
	namespace Event = Game::Event;
	namespace Value = Game::Component::Value;

	class ThrowTest : public ItemInteractionTest {};

	TEST_F(ThrowTest, ThrowsCarriedItemToPositionAndKeepsOtherItems)
	{
		const auto item = make_carried_item();
		const auto other = make_carried_item();
		const auto destination = center + Vec2<int>{0, 3};

		const auto result = simulation->simulate(Event::Throw{
			.entity = actor,
			.item = item,
			.target = destination
		});

		ASSERT_EQ(result.outcome, Event::Outcome::Accepted);
		expect_on_ground(item, destination);
		expect_carried(other);
		EXPECT_EQ(inventory().size(), 1u);
		EXPECT_EQ(registry().get<Value::Position>(actor).value, center);
	}

	TEST_F(ThrowTest, ThrowsAtEntityUsingItsCurrentPosition)
	{
		const auto item = make_carried_item();
		const auto target = make_ground_item(center + Vec2<int>{0, 2});
		const Event::Throw event{
			.entity = actor,
			.item = item,
			.target = target
		};
		const auto destination = center + Vec2<int>{0, 4};
		registry().replace<Value::Position>(target, destination);

		const auto result = simulation->simulate(event);

		ASSERT_EQ(result.outcome, Event::Outcome::Accepted);
		expect_on_ground(item, destination);
		expect_on_ground(target, destination);
		EXPECT_TRUE(inventory().empty());
	}

	TEST_F(ThrowTest, RejectsItemNotInInventoryWithoutChangingState)
	{
		const auto item = make_ground_item(center);
		const auto other = make_carried_item();
		const auto before = inventory();

		const auto result = simulation->simulate(Event::Throw{
			.entity = actor,
			.item = item,
			.target = center + Vec2<int>{0, 3}
		});

		expect_rejected(result);
		expect_on_ground(item, center);
		expect_carried(other);
		EXPECT_EQ(inventory(), before);
	}

	TEST_F(ThrowTest, RejectsNullEntityTargetWithoutLosingItem)
	{
		const auto item = make_carried_item();
		const auto before = inventory();

		const auto result = simulation->simulate(Event::Throw{
			.entity = actor,
			.item = item,
			.target = static_cast<entt::entity>(entt::null)
		});

		expect_rejected(result);
		expect_carried(item);
		EXPECT_EQ(inventory(), before);
	}

	TEST_F(ThrowTest, RejectsDestroyedEntityTargetWithoutLosingItem)
	{
		const auto item = make_carried_item();
		const auto before = inventory();
		const auto target = make_ground_item(center + Vec2<int>{0, 3});
		registry().destroy(target);
		// A newly created entity must not make the stale target valid again.
		make_ground_item(center + Vec2<int>{0, 4});
		ASSERT_FALSE(registry().valid(target));

		const auto result = simulation->simulate(Event::Throw{
			.entity = actor,
			.item = item,
			.target = target
		});

		expect_rejected(result);
		expect_carried(item);
		EXPECT_EQ(inventory(), before);
	}

	TEST_F(ThrowTest, RejectsEntityTargetWithoutPositionWithoutLosingItems)
	{
		const auto item = make_carried_item();
		const auto target = make_carried_item();
		const auto before = inventory();

		const auto result = simulation->simulate(Event::Throw{
			.entity = actor,
			.item = item,
			.target = target
		});

		expect_rejected(result);
		expect_carried(item);
		expect_carried(target);
		EXPECT_EQ(inventory(), before);
	}

	TEST_F(ThrowTest, RejectsEmptyTargetWithoutThrowingOrLosingItem)
	{
		const auto item = make_carried_item();
		const auto before = inventory();
		const Event::Throw event{.entity = actor, .item = item};

		Event::Node result;
		ASSERT_NO_THROW(result = simulation->simulate(event));

		expect_rejected(result);
		expect_carried(item);
		EXPECT_EQ(inventory(), before);
	}

	TEST_F(ThrowTest, RejectsThrowingSameItemAgainWithoutMovingIt)
	{
		const auto item = make_carried_item();
		const auto destination = center + Vec2<int>{0, 3};
		const auto first = simulation->simulate(Event::Throw{
			.entity = actor,
			.item = item,
			.target = destination
		});
		ASSERT_EQ(first.outcome, Event::Outcome::Accepted);
		expect_on_ground(item, destination);

		const auto second = simulation->simulate(Event::Throw{
			.entity = actor,
			.item = item,
			.target = center + Vec2<int>{0, 4}
		});

		expect_rejected(second);
		expect_on_ground(item, destination);
		EXPECT_TRUE(inventory().empty());
	}
}
