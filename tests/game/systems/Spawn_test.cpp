#include <memory>
#include <string>
#include <utility>
#include <variant>

#include <gtest/gtest.h>

#include "databases/EntityDatabase.hpp"
#include "game/Simulation.hpp"
#include "game/component/Component.hpp"
#include "game/entity/Entity.hpp"

namespace
{
	namespace Component = Game::Component;
	namespace Event = Game::Event;
	namespace World = Game::World;
	using Position = Component::Value::Position;

	class SpawnTest : public ::testing::Test
	{
		protected:
			std::unique_ptr<Game::Simulation> simulation;
			const World::ChunkPosition test_chunk{6, -8};
			const World::GlobalPosition position = World::to_global(test_chunk, {2, 2});

			void SetUp() override
			{
				EntityDatabase database;
				auto loot = database.definitions.at("item_stone");
				loot["Name"] = "Spawn test stone";
				loot["GoldValue"] = 1;
				loot["Weight"] = 1.0;
				database.definitions["test_spawn_item"] = std::move(loot);
				database.definitions["test_spawn_table"] = {
					{"Tags", Json::array({"LootTable"})},
					{"EntityIDs", Json::array({"test_spawn_item"})},
					{"CountBudget", 1},
					{"GoldBudget", 1},
					{"WeightBudget", 1.0},
					{"AllowDuplicates", false}
				};

				simulation = std::make_unique<Game::Simulation>(
					"item-interactions-test", database, "player_default");
				ASSERT_TRUE(simulation->get_grid().add(test_chunk,
					World::Chunk{Game::Enum::Material::Stone, Game::Enum::Form::Floor}));
			}

			entt::registry& registry() { return simulation->get_registry(); }

			entt::entity create(const Game::Entity::Definition::ID& id)
			{
				return Game::Entity::create(registry(), simulation->get_entity_database(), id);
			}
	};

	TEST_F(SpawnTest, PlacesAnItemAndAppliesEnterPositionOnce)
	{
		const auto item = create("item_stone");
		ASSERT_TRUE(registry().valid(item));

		const auto node = simulation->simulate(Event::Spawn{
			.entity = item, .position = position, .search_radius = 0.0});

		ASSERT_EQ(node.outcome, Event::Outcome::Accepted);
		EXPECT_EQ(std::get<Event::Spawn>(node.event).entity, item);
		ASSERT_EQ(node.consequences.size(), 1u);
		const auto& entered = node.consequences.front();
		EXPECT_EQ(entered.outcome, Event::Outcome::Accepted);
		EXPECT_EQ(std::get<Event::EnterPosition>(entered.event).entity, item);
		EXPECT_EQ(registry().get<Position>(item).value, position);
	}

	TEST_F(SpawnTest, RejectsANullEntity)
	{
		const auto node = simulation->simulate(Event::Spawn{});

		EXPECT_EQ(node.outcome, Event::Outcome::Rejected);
		EXPECT_TRUE(node.consequences.empty());
	}

	TEST_F(SpawnTest, ZeroRadiusRejectsABlockedTile)
	{
		const auto blocker = registry().create();
		registry().emplace<Position>(blocker, position);
		registry().emplace<Component::Value::CollisionMovement>(blocker, true);
		const auto item = create("item_stone");

		const auto node = simulation->simulate(Event::Spawn{
			.entity = item, .position = position, .search_radius = 0.0});

		EXPECT_EQ(node.outcome, Event::Outcome::Rejected);
		EXPECT_TRUE(node.consequences.empty());
		EXPECT_FALSE(registry().valid(item));
		EXPECT_TRUE(registry().valid(blocker));
	}

	TEST_F(SpawnTest, PositiveRadiusUsesOnlyTilesWithinItsDistance)
	{
		auto& grid = simulation->get_grid();
		auto* chunk = grid.find_chunk(test_chunk);
		ASSERT_NE(chunk, nullptr);
		*chunk = World::Chunk{Game::Enum::Material::Stone, Game::Enum::Form::Wall};
		const auto floor = position + Vec2<int>{0, 2};
		grid.find_cell(floor)->form = Game::Enum::Form::Floor;

		const auto too_short = create("item_stone");
		const auto rejected = simulation->simulate(Event::Spawn{
			.entity = too_short, .position = position, .search_radius = 1.0});
		EXPECT_EQ(rejected.outcome, Event::Outcome::Rejected);
		EXPECT_FALSE(registry().valid(too_short));

		const auto item = create("item_stone");
		const auto accepted = simulation->simulate(Event::Spawn{
			.entity = item, .position = position, .search_radius = 2.0});
		ASSERT_EQ(accepted.outcome, Event::Outcome::Accepted);
		EXPECT_EQ(registry().get<Position>(item).value, floor);
	}

	TEST_F(SpawnTest, PositiveRadiusCanReachTheAdjacentLoadedChunk)
	{
		auto& grid = simulation->get_grid();
		auto* chunk = grid.find_chunk(test_chunk);
		ASSERT_NE(chunk, nullptr);
		*chunk = World::Chunk{Game::Enum::Material::Stone, Game::Enum::Form::Wall};
		ASSERT_TRUE(grid.add({test_chunk.y + 1, test_chunk.x},
			World::Chunk{Game::Enum::Material::Stone, Game::Enum::Form::Floor}));
		const auto start = World::to_global(test_chunk, {World::Chunk::height - 1, 2});
		const auto item = create("item_stone");

		const auto node = simulation->simulate(Event::Spawn{
			.entity = item, .position = start, .search_radius = 1.0});

		ASSERT_EQ(node.outcome, Event::Outcome::Accepted);
		EXPECT_EQ(registry().get<Position>(item).value, start + (Vec2<int>{1, 0}));
	}

	TEST_F(SpawnTest, OmittedRadiusSearchesTheTargetChunk)
	{
		auto& grid = simulation->get_grid();
		auto* chunk = grid.find_chunk(test_chunk);
		ASSERT_NE(chunk, nullptr);
		*chunk = World::Chunk{Game::Enum::Material::Stone, Game::Enum::Form::Wall};
		const auto floor = World::to_global(
			test_chunk, {World::Chunk::height - 1, World::Chunk::width - 1});
		grid.find_cell(floor)->form = Game::Enum::Form::Floor;
		const auto item = create("item_stone");

		const auto node = simulation->simulate(Event::Spawn{
			.entity = item, .position = position});

		ASSERT_EQ(node.outcome, Event::Outcome::Accepted);
		EXPECT_EQ(registry().get<Position>(item).value, floor);
	}

	TEST_F(SpawnTest, PlantsRequireSoil)
	{
		auto* cell = simulation->get_grid().find_cell(position);
		ASSERT_NE(cell, nullptr);
		const auto rejected_tree = create("plant_tree");
		const auto rejected = simulation->simulate(Event::Spawn{
			.entity = rejected_tree, .position = position, .search_radius = 0.0});
		EXPECT_EQ(rejected.outcome, Event::Outcome::Rejected);
		EXPECT_FALSE(registry().valid(rejected_tree));

		cell->material = Game::Enum::Material::Soil;
		const auto tree = create("plant_tree");
		const auto spawned = simulation->simulate(Event::Spawn{
			.entity = tree, .position = position, .search_radius = 0.0});
		ASSERT_EQ(spawned.outcome, Event::Outcome::Accepted);
		EXPECT_EQ(registry().get<Position>(tree).value, position);
	}

	TEST_F(SpawnTest, DeadActorsReceiveLootAndOnlyLivingActorsGetTurns)
	{
		const auto corpse = create("player_default");
		const auto npc = create("player_default");
		registry().remove<Component::Tag::Player>(corpse);
		registry().remove<Component::Tag::Player>(npc);
		registry().emplace<Component::Tag::Dead>(corpse);
		registry().get<Component::List::SpawnLoot>(corpse).values = {"test_spawn_table"};
		registry().get<Component::List::SpawnLoot>(npc).values.clear();

		const auto dead_spawn = simulation->simulate(Event::Spawn{
			.entity = corpse, .position = position, .search_radius = 0.0});
		ASSERT_EQ(dead_spawn.outcome, Event::Outcome::Accepted);
		ASSERT_EQ(dead_spawn.consequences.size(), 2u);
		EXPECT_EQ(dead_spawn.consequences[0].outcome, Event::Outcome::Accepted);
		EXPECT_EQ(dead_spawn.consequences[1].outcome, Event::Outcome::Accepted);
		const auto& receive = std::get<Event::ReceiveItem>(dead_spawn.consequences[1].event);
		EXPECT_EQ(receive.entity, corpse);

		const auto& items = registry().get<Component::List::Inventory>(corpse).values;
		ASSERT_EQ(items.size(), 1u);
		EXPECT_EQ(items.front(), receive.item);
		EXPECT_EQ(registry().get<Component::Value::Name>(receive.item).value, "Spawn test stone");
		EXPECT_EQ(registry().get<Component::Value::ContainedIn>(receive.item).value, corpse);
		EXPECT_FALSE(registry().all_of<Position>(receive.item));

		const auto living_spawn = simulation->simulate(Event::Spawn{
			.entity = npc, .position = position + Vec2<int>{0, 1}, .search_radius = 0.0});
		ASSERT_EQ(living_spawn.outcome, Event::Outcome::Accepted);

		auto& scheduler = simulation->get_scheduler();
		const auto first = scheduler.current_actor().entity;
		bool saw_npc = false;
		do
		{
			const auto turn = scheduler.current_actor();
			EXPECT_NE(turn.entity, corpse);
			if (turn.entity == npc)
			{
				saw_npc = true;
				EXPECT_EQ(turn.controller, Game::Turn::Controller::AI);
			}
			scheduler.next_turn();
		} while (scheduler.current_actor().entity != first);

		EXPECT_TRUE(saw_npc);
	}
}
