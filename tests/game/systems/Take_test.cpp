#include <variant>

#include <gtest/gtest.h>

#include "fixtures/ItemInteractionTest.hpp"

namespace
{
	namespace Component = Game::Component;
	namespace Event = Game::Event;
	using ContainedIn = Component::Value::ContainedIn;

	class TakeTest : public ItemInteractionTest
	{
		protected:
			entt::entity make_container()
			{
				const auto container = registry().create();
				registry().emplace<Inventory>(container);
				return container;
			}

			entt::entity make_unplaced_item()
			{
				return Game::Entity::create(
					registry(), simulation->get_entity_database(), "item_stone");
			}

			Event::Node receive(entt::entity holder, entt::entity item)
			{
				return simulation->simulate(Event::ReceiveItem{.entity = holder, .item = item});
			}
	};

	TEST_F(TakeTest, TransfersTheItemAndOwnershipThroughTheOrderedEventTree)
	{
		const auto container = make_container();
		const auto item = make_unplaced_item();
		const auto other = make_unplaced_item();
		ASSERT_EQ(receive(container, item).outcome, Event::Outcome::Accepted);
		ASSERT_EQ(receive(container, other).outcome, Event::Outcome::Accepted);
		ASSERT_EQ(registry().get<ContainedIn>(item).value, container);

		const auto node = simulation->simulate(Event::Take{.entity = actor, .item = item});

		ASSERT_EQ(node.outcome, Event::Outcome::Accepted);
		EXPECT_EQ(std::get<Event::Take>(node.event).item, item);
		ASSERT_EQ(node.consequences.size(), 2u);
		const auto& lost = std::get<Event::LoseItem>(node.consequences[0].event);
		const auto& received = std::get<Event::ReceiveItem>(node.consequences[1].event);
		EXPECT_EQ(node.consequences[0].outcome, Event::Outcome::Accepted);
		EXPECT_EQ(node.consequences[1].outcome, Event::Outcome::Accepted);
		EXPECT_EQ(lost.entity, container);
		EXPECT_EQ(lost.item, item);
		EXPECT_EQ(received.entity, actor);
		EXPECT_EQ(received.item, item);
		expect_carried(item);
		EXPECT_EQ(registry().get<ContainedIn>(item).value, actor);
		EXPECT_FALSE(registry().get<Inventory>(container).contains(item));
		EXPECT_TRUE(registry().get<Inventory>(container).contains(other));
		EXPECT_EQ(registry().get<ContainedIn>(other).value, container);
	}

	TEST_F(TakeTest, RejectsAGroundItemWithoutAnOwner)
	{
		const auto position = center + Vec2<int>{0, 1};
		const auto item = make_ground_item(position);

		const auto node = simulation->simulate(Event::Take{.entity = actor, .item = item});

		expect_rejected(node);
		expect_on_ground(item, position);
		EXPECT_FALSE(registry().all_of<ContainedIn>(item));
	}

	TEST_F(TakeTest, RejectsAnOwnershipLinkToAnInventoryMissingTheItem)
	{
		const auto container = make_container();
		const auto item = make_unplaced_item();
		registry().emplace<ContainedIn>(item, container);

		const auto node = simulation->simulate(Event::Take{.entity = actor, .item = item});

		expect_rejected(node);
		EXPECT_TRUE(inventory().empty());
		EXPECT_TRUE(registry().get<Inventory>(container).values.empty());
		EXPECT_EQ(registry().get<ContainedIn>(item).value, container);
	}

	TEST_F(TakeTest, RejectsAReceiverWithoutInventoryAndPreservesTheOwner)
	{
		const auto container = make_container();
		const auto item = make_unplaced_item();
		ASSERT_EQ(receive(container, item).outcome, Event::Outcome::Accepted);
		const auto receiver = registry().create();

		const auto node = simulation->simulate(Event::Take{.entity = receiver, .item = item});

		expect_rejected(node);
		EXPECT_TRUE(registry().get<Inventory>(container).contains(item));
		EXPECT_EQ(registry().get<ContainedIn>(item).value, container);
	}

	TEST_F(TakeTest, DroppingATakenItemClearsItsOwnershipLink)
	{
		const auto container = make_container();
		const auto item = make_unplaced_item();
		ASSERT_EQ(receive(container, item).outcome, Event::Outcome::Accepted);
		ASSERT_EQ(simulation->simulate(Event::Take{.entity = actor, .item = item}).outcome,
			Event::Outcome::Accepted);

		const auto node = simulation->simulate(Event::Drop{.entity = actor, .item = item});

		ASSERT_EQ(node.outcome, Event::Outcome::Accepted);
		expect_on_ground(item, center);
		EXPECT_FALSE(registry().all_of<ContainedIn>(item));
		EXPECT_TRUE(registry().get<Inventory>(container).values.empty());
	}
}
