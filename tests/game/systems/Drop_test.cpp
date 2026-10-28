#include "fixtures/ItemInteractionTest.hpp"

namespace
{
	namespace Event = Game::Event;
	namespace Value = Game::Component::Value;

	class DropTest : public ItemInteractionTest {};

	TEST_F(DropTest, DropsCarriedItemAtActorsPositionAndKeepsOtherItems)
	{
		const auto item = make_carried_item();
		const auto other = make_carried_item();

		const auto result = simulation->simulate(Event::Drop{
			.entity = actor,
			.item = item
		});

		ASSERT_EQ(result.outcome, Event::Outcome::Accepted);
		expect_on_ground(item, center);
		expect_carried(other);
		EXPECT_EQ(inventory().size(), 1u);
		EXPECT_EQ(registry().get<Value::Position>(actor).value, center);
	}

	TEST_F(DropTest, RejectsItemNotInInventoryWithoutChangingState)
	{
		const auto ground_position = center + Vec2<int>{0, 2};
		const auto item = make_ground_item(ground_position);
		const auto other = make_carried_item();
		const auto before = inventory();

		const auto result = simulation->simulate(Event::Drop{
			.entity = actor,
			.item = item
		});

		expect_rejected(result);
		expect_on_ground(item, ground_position);
		expect_carried(other);
		EXPECT_EQ(inventory(), before);
	}

	TEST_F(DropTest, RejectsItemOwnedByAnotherEntityWithoutTakingIt)
	{
		const auto other_owner = registry().create();
		const auto item = make_carried_item();
		registry().get<Inventory>(actor).values.clear();
		registry().emplace<Inventory>(
				other_owner, std::vector<entt::entity>{item});

		const auto result = simulation->simulate(Event::Drop{
			.entity = actor,
			.item = item
		});

		expect_rejected(result);
		EXPECT_TRUE(registry().valid(item));
		EXPECT_FALSE(registry().all_of<Value::Position>(item));
		EXPECT_TRUE(inventory().empty());
		EXPECT_EQ(registry().get<Inventory>(other_owner).values,
				std::vector<entt::entity>{item});
	}

	TEST_F(DropTest, RejectsDroppingSameItemAgainWithoutMovingIt)
	{
		const auto item = make_carried_item();
		const auto first = simulation->simulate(Event::Drop{
			.entity = actor,
			.item = item
		});
		ASSERT_EQ(first.outcome, Event::Outcome::Accepted);
		expect_on_ground(item, center);
		registry().replace<Value::Position>(actor, center + Vec2<int>{0, 1});

		const auto second = simulation->simulate(Event::Drop{
			.entity = actor,
			.item = item
		});

		expect_rejected(second);
		expect_on_ground(item, center);
		EXPECT_TRUE(inventory().empty());
	}
}
