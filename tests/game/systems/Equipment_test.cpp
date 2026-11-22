#include <algorithm>
#include <vector>

#include "fixtures/ItemInteractionTest.hpp"
#include "game/system/Equipment.hpp"

// Place in tests/game/systems/. The existing CMake test glob includes it.
// These tests exercise completed event trees and inspect the stored equipment
// lists as well as is_equipped(), so stale references cannot be hidden by a query.
namespace
{
	namespace Component = Game::Component;
	namespace Event = Game::Event;
	namespace Equipment = Game::System::Equipment;
	using EquippedItems = Component::List::EquippedItems;
	using ContainedIn = Component::Value::ContainedIn;

	class EquipmentTest : public ItemInteractionTest
	{
		protected:
			entt::entity item = entt::null;
			entt::entity spare = entt::null;
			entt::entity other_owner = entt::null;

			void SetUp() override
			{
				ASSERT_NO_FATAL_FAILURE(ItemInteractionTest::SetUp());
				ASSERT_TRUE(registry().all_of<EquippedItems>(actor));
				ASSERT_TRUE(registry().get<EquippedItems>(actor).values.empty());

				item = Game::Entity::create(
					registry(), simulation->get_entity_database(), "item_rusty_pan");
				spare = Game::Entity::create(
					registry(), simulation->get_entity_database(), "item_stone");
				ASSERT_TRUE(registry().valid(item));
				ASSERT_TRUE(registry().valid(spare));
				ASSERT_TRUE((registry().all_of<
					Component::Tag::Item, Component::Tag::Equipment>(item)));
				ASSERT_TRUE((registry().all_of<
					Component::Tag::Item, Component::Tag::Equipment>(spare)));

				// Use ReceiveItem instead of directly populating Inventory, so
				// every carried item starts with a matching ContainedIn link.
				ASSERT_EQ(simulation->simulate(Event::ReceiveItem{actor, item}).outcome,
					Event::Outcome::Accepted);
				ASSERT_EQ(simulation->simulate(Event::ReceiveItem{actor, spare}).outcome,
					Event::Outcome::Accepted);

				other_owner = registry().create();
				registry().emplace<Inventory>(other_owner);
				registry().emplace<EquippedItems>(other_owner);
			}

			Event::Node equip(entt::entity owner, entt::entity equipment)
			{
				return simulation->simulate(Event::Equip{owner, equipment});
			}

			void expect_owned(entt::entity owner, entt::entity equipment) const
			{
				ASSERT_TRUE(registry().valid(equipment));
				const auto& items = registry().get<Inventory>(owner).values;
				EXPECT_EQ(std::count(items.begin(), items.end(), equipment), 1);
				const auto* container = registry().try_get<ContainedIn>(equipment);
				ASSERT_NE(container, nullptr);
				EXPECT_EQ(container->value, owner);
				EXPECT_FALSE(registry().all_of<Component::Value::Position>(equipment));
			}

			void expect_equipped(
				entt::entity owner, entt::entity equipment, bool expected) const
			{
				const auto& items = registry().get<EquippedItems>(owner).values;
				EXPECT_EQ(std::count(items.begin(), items.end(), equipment),
					expected ? 1 : 0);
				EXPECT_EQ(Equipment::is_equipped(registry(), owner, equipment), expected);
			}

			void expect_released(entt::entity owner, entt::entity equipment) const
			{
				ASSERT_TRUE(registry().valid(equipment));
				EXPECT_FALSE(registry().get<Inventory>(owner).contains(equipment));
				EXPECT_FALSE(registry().all_of<ContainedIn>(equipment));
			}
	};

	TEST_F(EquipmentTest, EquipsCarriedEquipmentWithoutChangingInventoryOrOwnership)
	{
		const auto before = inventory();

		const auto result = equip(actor, item);

		ASSERT_EQ(result.outcome, Event::Outcome::Accepted);
		expect_equipped(actor, item, true);
		expect_equipped(actor, spare, false);
		expect_owned(actor, item);
		expect_owned(actor, spare);
		EXPECT_EQ(inventory(), before);
	}

	TEST_F(EquipmentTest, RejectsDuplicateEquipWithoutDuplicatingReferences)
	{
		ASSERT_EQ(equip(actor, item).outcome, Event::Outcome::Accepted);
		const auto before = registry().get<EquippedItems>(actor).values;

		const auto result = equip(actor, item);

		expect_rejected(result);
		EXPECT_EQ(registry().get<EquippedItems>(actor).values, before);
		expect_equipped(actor, item, true);
		expect_owned(actor, item);
	}

	TEST_F(EquipmentTest, UnequipsOnlyRequestedItemAndKeepsItInInventory)
	{
		ASSERT_EQ(equip(actor, item).outcome, Event::Outcome::Accepted);
		ASSERT_EQ(equip(actor, spare).outcome, Event::Outcome::Accepted);
		const auto before = inventory();

		const auto result = simulation->simulate(Event::Unequip{actor, item});

		ASSERT_EQ(result.outcome, Event::Outcome::Accepted);
		expect_equipped(actor, item, false);
		expect_equipped(actor, spare, true);
		expect_owned(actor, item);
		expect_owned(actor, spare);
		EXPECT_EQ(inventory(), before);
	}

	TEST_F(EquipmentTest, RejectsUnequippingAnItemThatIsNotEquipped)
	{
		ASSERT_EQ(equip(actor, spare).outcome, Event::Outcome::Accepted);

		const auto result = simulation->simulate(Event::Unequip{actor, item});

		expect_rejected(result);
		expect_equipped(actor, item, false);
		expect_equipped(actor, spare, true);
		expect_owned(actor, item);
	}

	TEST_F(EquipmentTest, AllowsReequippingAfterUnequip)
	{
		ASSERT_EQ(equip(actor, item).outcome, Event::Outcome::Accepted);
		ASSERT_EQ(simulation->simulate(Event::Unequip{actor, item}).outcome,
			Event::Outcome::Accepted);

		const auto result = equip(actor, item);

		ASSERT_EQ(result.outcome, Event::Outcome::Accepted);
		expect_equipped(actor, item, true);
		expect_owned(actor, item);
	}

	TEST_F(EquipmentTest, RejectedDropDoesNotUnequipAnotherOwnersItem)
	{
		ASSERT_EQ(simulation->simulate(Event::Take{other_owner, item}).outcome,
			Event::Outcome::Accepted);
		ASSERT_EQ(equip(other_owner, item).outcome, Event::Outcome::Accepted);

		const auto result = simulation->simulate(Event::Drop{actor, item});

		expect_rejected(result);
		expect_equipped(other_owner, item, true);
		expect_equipped(actor, item, false);
		expect_owned(other_owner, item);
		expect_owned(actor, spare);
	}

	TEST_F(EquipmentTest, RejectedThrowKeepsEquipmentAndOwnership)
	{
		ASSERT_EQ(equip(actor, item).outcome, Event::Outcome::Accepted);
		const auto before = inventory();

		// The default target is std::monostate, so this throw is invalid.
		const auto result = simulation->simulate(Event::Throw{
			.entity = actor,
			.item = item
			});

		expect_rejected(result);
		expect_equipped(actor, item, true);
		expect_owned(actor, item);
		EXPECT_EQ(inventory(), before);
	}

	TEST_F(EquipmentTest, LosingUnequippedItemKeepsOtherEquipment)
	{
		ASSERT_EQ(equip(actor, spare).outcome, Event::Outcome::Accepted);

		const auto result = simulation->simulate(Event::LoseItem{actor, item});

		ASSERT_EQ(result.outcome, Event::Outcome::Accepted);
		expect_released(actor, item);
		expect_equipped(actor, item, false);
		expect_equipped(actor, spare, true);
		expect_owned(actor, spare);
	}

	TEST_F(EquipmentTest, RejectsEquippingGroundItemWithoutChangingState)
	{
		ASSERT_EQ(simulation->simulate(Event::Drop{actor, item}).outcome,
			Event::Outcome::Accepted);
		const auto before = inventory();

		const auto result = equip(actor, item);

		expect_rejected(result);
		expect_equipped(actor, item, false);
		EXPECT_TRUE(registry().get<EquippedItems>(actor).values.empty());
		expect_on_ground(item, center);
		expect_released(actor, item);
		EXPECT_EQ(inventory(), before);
	}

	TEST_F(EquipmentTest, RejectsEquippingAnotherOwnersItem)
	{
		ASSERT_EQ(simulation->simulate(Event::Take{other_owner, item}).outcome,
			Event::Outcome::Accepted);
		ASSERT_EQ(equip(other_owner, item).outcome, Event::Outcome::Accepted);
		const auto before = inventory();

		const auto result = equip(actor, item);

		expect_rejected(result);
		expect_equipped(actor, item, false);
		expect_equipped(other_owner, item, true);
		expect_owned(other_owner, item);
		EXPECT_EQ(inventory(), before);
	}

	TEST_F(EquipmentTest, RejectsEquippingUnplacedItemWithoutOwner)
	{
		ASSERT_EQ(simulation->simulate(Event::LoseItem{actor, item}).outcome,
			Event::Outcome::Accepted);
		const auto before = inventory();

		const auto result = equip(actor, item);

		expect_rejected(result);
		expect_equipped(actor, item, false);
		expect_released(actor, item);
		EXPECT_FALSE(registry().all_of<Component::Value::Position>(item));
		EXPECT_EQ(inventory(), before);
	}

	TEST_F(EquipmentTest, LosingEquippedItemRemovesItsEquipmentReference)
	{
		ASSERT_EQ(equip(actor, item).outcome, Event::Outcome::Accepted);
		ASSERT_EQ(equip(actor, spare).outcome, Event::Outcome::Accepted);

		const auto result = simulation->simulate(Event::LoseItem{actor, item});

		ASSERT_EQ(result.outcome, Event::Outcome::Accepted);
		expect_released(actor, item);
		expect_equipped(actor, item, false);
		expect_equipped(actor, spare, true);
		expect_owned(actor, spare);
	}

	TEST_F(EquipmentTest, DroppingEquippedItemRemovesItsEquipmentReference)
	{
		ASSERT_EQ(equip(actor, item).outcome, Event::Outcome::Accepted);
		ASSERT_EQ(equip(actor, spare).outcome, Event::Outcome::Accepted);

		const auto result = simulation->simulate(Event::Drop{actor, item});

		ASSERT_EQ(result.outcome, Event::Outcome::Accepted);
		expect_on_ground(item, center);
		expect_released(actor, item);
		expect_equipped(actor, item, false);
		expect_equipped(actor, spare, true);
		expect_owned(actor, spare);
	}

	TEST_F(EquipmentTest, ThrowingEquippedItemRemovesItsEquipmentReference)
	{
		ASSERT_EQ(equip(actor, item).outcome, Event::Outcome::Accepted);
		ASSERT_EQ(equip(actor, spare).outcome, Event::Outcome::Accepted);
		const auto destination = center + Vec2<int>{0, 3};

		const auto result = simulation->simulate(Event::Throw{
			.entity = actor,
			.item = item,
			.target = destination
			});

		ASSERT_EQ(result.outcome, Event::Outcome::Accepted);
		expect_on_ground(item, destination);
		expect_released(actor, item);
		expect_equipped(actor, item, false);
		expect_equipped(actor, spare, true);
		expect_owned(actor, spare);
	}

	TEST_F(EquipmentTest, TransferredItemCanOnlyBeEquippedByItsNewOwner)
	{
		ASSERT_EQ(equip(actor, item).outcome, Event::Outcome::Accepted);
		ASSERT_EQ(equip(actor, spare).outcome, Event::Outcome::Accepted);

		const auto result = simulation->simulate(Event::Take{other_owner, item});

		ASSERT_EQ(result.outcome, Event::Outcome::Accepted);
		EXPECT_FALSE(registry().get<Inventory>(actor).contains(item));
		expect_owned(other_owner, item);
		expect_equipped(actor, item, false);
		expect_equipped(other_owner, item, false);
		expect_equipped(actor, spare, true);
		expect_owned(actor, spare);

		ASSERT_EQ(equip(other_owner, item).outcome, Event::Outcome::Accepted);
		expect_equipped(other_owner, item, true);
		expect_equipped(actor, item, false);
		expect_owned(other_owner, item);
	}

	TEST_F(EquipmentTest, PickingUpDroppedEquipmentDoesNotReequipIt)
	{
		ASSERT_EQ(equip(actor, item).outcome, Event::Outcome::Accepted);
		ASSERT_EQ(equip(actor, spare).outcome, Event::Outcome::Accepted);
		ASSERT_EQ(simulation->simulate(Event::Drop{actor, item}).outcome,
			Event::Outcome::Accepted);

		const auto result = simulation->simulate(Event::PickUp{actor, item});

		ASSERT_EQ(result.outcome, Event::Outcome::Accepted);
		expect_owned(actor, item);
		expect_equipped(actor, item, false);
		expect_equipped(actor, spare, true);

		ASSERT_EQ(equip(actor, item).outcome, Event::Outcome::Accepted);
		expect_equipped(actor, item, true);
	}
}
