#include <cstddef>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "databases/EntityDatabase.hpp"
#include "game/component/Component.hpp"
#include "game/entity/Entity.hpp"
#include "game/system/Loot.hpp"
#include "utils/Random.hpp"

namespace
{
	namespace Component = Game::Component;
	namespace Event = Game::Event;

	class LootTest : public ::testing::Test
	{
		protected:
			EntityDatabase database;
			entt::registry registry;
			const entt::entity recipient = registry.create();

			void SetUp() override
			{
				registry.emplace<Component::Value::Position>(
					recipient, Game::World::GlobalPosition{-8, 14});
			}

			void add_item(const std::string& id, std::size_t gold, double weight)
			{
				auto definition = database.definitions.at("item_stone");
				definition["Name"] = id;
				definition["GoldValue"] = gold;
				definition["Weight"] = weight;
				database.definitions[id] = std::move(definition);
			}

			void add_table(
				std::vector<std::string> items,
				std::size_t count,
				std::size_t gold,
				double weight,
				bool allow_duplicates)
			{
				database.definitions["test_loot_table"] = {
					{"Tags", Json::array({"LootTable"})},
					{"EntityIDs", std::move(items)},
					{"CountBudget", count},
					{"GoldBudget", gold},
					{"WeightBudget", weight},
					{"AllowDuplicates", allow_duplicates}
				};
			}

			Event::List generate(std::vector<std::string> tables = {"test_loot_table"})
			{
				const Component::List::SpawnLoot spawn_loot{std::move(tables)};
				return Game::System::Loot::generate(
					registry, database, recipient, &spawn_loot);
			}

			const std::string& item_name(const Event::Any& event) const
			{
				return registry.get<Component::Value::Name>(
					std::get<Event::ReceiveItem>(event).item).value;
			}
	};

	TEST_F(LootTest, AcceptsItemsExactlyAtBothBudgetLimits)
	{
		add_item("test_affordable", 2, 1.5);
		add_table({"test_affordable"}, 1, 2, 1.5, false);

		const auto events = generate();

		ASSERT_EQ(events.size(), 1u);
		const auto& receive = std::get<Event::ReceiveItem>(events.front());
		EXPECT_EQ(receive.entity, recipient);
		EXPECT_TRUE(registry.all_of<Component::Tag::Item>(receive.item));
		EXPECT_EQ(item_name(events.front()), "test_affordable");
		EXPECT_EQ(registry.view<Component::Tag::Item>().size(), 1u);
		EXPECT_TRUE(registry.view<Component::Tag::LootTable>().empty());
	}

	TEST_F(LootTest, CountBudgetCapsFreeAndWeightlessDuplicates)
	{
		add_item("test_free", 0, 0.0);
		add_table({"test_free"}, 2, 0, 0.0, true);

		const auto events = generate();

		ASSERT_EQ(events.size(), 2u);
		EXPECT_EQ(item_name(events[0]), "test_free");
		EXPECT_EQ(item_name(events[1]), "test_free");
		EXPECT_NE(std::get<Event::ReceiveItem>(events[0]).item,
			std::get<Event::ReceiveItem>(events[1]).item);
	}

	TEST_F(LootTest, GoldBudgetIsSpentAcrossRepeatedCopies)
	{
		add_item("test_gold_cost", 2, 1.0);
		add_table({"test_gold_cost"}, 6, 5, 100.0, true);

		EXPECT_EQ(generate().size(), 2u);
	}

	TEST_F(LootTest, WeightBudgetIsSpentAcrossRepeatedCopies)
	{
		add_item("test_weight_cost", 1, 1.5);
		add_table({"test_weight_cost"}, 6, 100, 3.0, true);

		EXPECT_EQ(generate().size(), 2u);
	}

	TEST_F(LootTest, SkipsItemsThatExceedEitherBudget)
	{
		add_item("test_expensive", 2, 0.0);
		add_item("test_heavy", 0, 2.0);
		add_item("test_affordable", 1, 1.0);
		add_table({"test_expensive", "test_heavy", "test_affordable"}, 3, 1, 1.0, false);

		const auto events = generate();

		ASSERT_EQ(events.size(), 1u);
		EXPECT_EQ(item_name(events.front()), "test_affordable");
		EXPECT_EQ(registry.view<Component::Tag::Item>().size(), 1u);
	}

	TEST_F(LootTest, DisablingDuplicatesAllowsOneCopyOfEachDistinctItem)
	{
		add_item("test_item_a", 1, 1.0);
		add_item("test_item_b", 1, 1.0);
		add_table({"test_item_a", "test_item_b"}, 4, 10, 10.0, false);

		const auto events = generate();

		ASSERT_EQ(events.size(), 2u);
		EXPECT_NE(item_name(events[0]), item_name(events[1]));
	}

	TEST_F(LootTest, SkipsMissingAndNonItemEntriesAndStillGeneratesValidLoot)
	{
		database.definitions["test_damage_definition"] = {
			{"Tags", Json::array({"DamageRoll"})},
			{"DamageType", "Bludgeoning"},
			{"Dice", "1d4"}
		};
		add_item("test_item", 1, 1.0);
		add_table({"test_unknown", "test_damage_definition", "test_item"}, 3, 3, 3.0, false);

		const auto events = generate();

		ASSERT_EQ(events.size(), 1u);
		EXPECT_EQ(item_name(events.front()), "test_item");
		EXPECT_TRUE(registry.view<Component::Tag::DamageRoll>().empty());
		EXPECT_TRUE(registry.view<Component::Tag::LootTable>().empty());
	}

	TEST_F(LootTest, SkipsMissingAndNonLootTableDefinitions)
	{
		add_item("test_item", 1, 1.0);
		add_table({"test_item"}, 1, 1, 1.0, false);

		const auto events = generate({"test_unknown_table", "test_item", "test_loot_table"});

		ASSERT_EQ(events.size(), 1u);
		EXPECT_EQ(item_name(events.front()), "test_item");
		EXPECT_EQ(registry.view<Component::Tag::Item>().size(), 1u);
	}

	TEST_F(LootTest, ReloadingTheTableAtTheSamePositionReplaysTheSelection)
	{
		add_item("test_item_a", 1, 1.0);
		add_item("test_item_b", 1, 1.0);
		add_table({"test_item_a", "test_item_b"}, 1, 1, 1.0, false);

		const auto first = generate();
		Random::seed(4242);
		(void) Random::integer(1, 20);
		const auto second = generate();

		ASSERT_EQ(first.size(), 1u);
		ASSERT_EQ(second.size(), 1u);
		EXPECT_EQ(item_name(first.front()), item_name(second.front()));
		EXPECT_NE(std::get<Event::ReceiveItem>(first.front()).item,
			std::get<Event::ReceiveItem>(second.front()).item);
	}

	TEST_F(LootTest, ZeroCountProducesNoItems)
	{
		add_item("test_item", 0, 0.0);
		add_table({"test_item"}, 0, 5, 5.0, true);

		EXPECT_TRUE(generate().empty());
		EXPECT_TRUE(registry.view<Component::Tag::Item>().empty());
	}

	TEST_F(LootTest, EmptyTableListAndInvalidRecipientProduceNoEvents)
	{
		const Component::List::SpawnLoot spawn_loot{{"test_loot_table"}};

		EXPECT_TRUE(generate({}).empty());
		EXPECT_TRUE(Game::System::Loot::generate(registry, database, recipient, nullptr).empty());
		EXPECT_TRUE(Game::System::Loot::generate(registry, database, entt::null, &spawn_loot).empty());
	}
}
