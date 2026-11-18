#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "application/EventDescriber.hpp"
#include "fixtures/ItemInteractionTest.hpp"

namespace
{
	namespace Event = Game::Event;

	class EventDescriberTest : public ItemInteractionTest
	{
		protected:
			entt::entity make_unplaced_item()
			{
				return Game::Entity::create(
					registry(), simulation->get_entity_database(), "item_stone");
			}
	};

	TEST_F(EventDescriberTest, PlayerReceiptProducesOneMessage)
	{
		const auto item = make_unplaced_item();
		const auto root = simulation->simulate(Event::ReceiveItem{
			.entity = actor, .item = item});
		ASSERT_EQ(root.outcome, Event::Outcome::Accepted);

		const auto messages = EventDescriber::describe(*simulation, root);

		ASSERT_EQ(messages.size(), 1u);
		EXPECT_NE(messages.front().find(" receives "), std::string::npos);
	}

	TEST_F(EventDescriberTest, OtherActorsReceiptProducesNoBlankOrVisibleMessage)
	{
		const auto item = make_unplaced_item();
		const auto other = registry().create();
		registry().emplace<Inventory>(other);
		const auto root = simulation->simulate(Event::ReceiveItem{
			.entity = other, .item = item});
		ASSERT_EQ(root.outcome, Event::Outcome::Accepted);
		EXPECT_TRUE(registry().get<Inventory>(other).contains(item));

		EXPECT_TRUE(EventDescriber::describe(*simulation, root).empty());
	}

	TEST_F(EventDescriberTest, TraversesPastASilentChildToDescribeAPlayerReceipt)
	{
		const auto other_item = make_unplaced_item();
		const auto player_item = make_unplaced_item();
		const auto other = registry().create();
		registry().emplace<Inventory>(other);
		auto other_receipt = simulation->simulate(Event::ReceiveItem{
			.entity = other, .item = other_item});
		auto player_receipt = simulation->simulate(Event::ReceiveItem{
			.entity = actor, .item = player_item});
		ASSERT_EQ(other_receipt.outcome, Event::Outcome::Accepted);
		ASSERT_EQ(player_receipt.outcome, Event::Outcome::Accepted);

		auto root = Event::Node::accepted();
		root.consequences.push_back(std::move(other_receipt));
		root.consequences.push_back(std::move(player_receipt));
		const auto messages = EventDescriber::describe(*simulation, root);

		ASSERT_EQ(messages.size(), 1u);
		EXPECT_NE(messages.front().find(" receives "), std::string::npos);
	}
}
