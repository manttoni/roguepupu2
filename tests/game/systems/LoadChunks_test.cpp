#include <algorithm>
#include <memory>
#include <utility>
#include <variant>

#include <gtest/gtest.h>

#include "databases/EntityDatabase.hpp"
#include "game/Simulation.hpp"
#include "game/component/Component.hpp"
#include "ncurses/Screen.hpp"

namespace
{
	namespace Component = Game::Component;
	namespace Event = Game::Event;
	namespace World = Game::World;
	using Position = Component::Value::Position;

	class LoadChunksTest : public ::testing::Test
	{
		protected:
			std::unique_ptr<Game::Simulation> simulation;
			const World::ChunkPosition target{-5, 7};

			void SetUp() override
			{
				EntityDatabase database;
				auto marker = database.definitions.at("item_stone");
				marker["Name"] = "Chunk test marker";
				database.definitions["test_chunk_marker"] = std::move(marker);

				simulation = std::make_unique<Game::Simulation>(
					"item-interactions-test", database, "player_default");
				ASSERT_FALSE(simulation->get_grid().contains(target));
			}
	};

	TEST_F(LoadChunksTest, SkipsMissingDefinitionsAndProcessesTheRemainingRequests)
	{
		World::Generator preview{simulation->get_seed()};
		const auto expected = preview.generate_chunk(target);
		const auto& cells = expected.get_cells();
		const auto floor = std::find_if(cells.begin(), cells.end(), [](const auto& cell)
			{
				return cell.form == Game::Enum::Form::Floor;
			});
		ASSERT_NE(floor, cells.end());
		const auto index = static_cast<int>(floor - cells.begin());
		const auto position = World::to_global(target,
			{index / World::Chunk::width, index % World::Chunk::width});

		auto& requests = simulation->get_generator().get_spawn_requests();
		requests.push_back({.id = "test_missing_definition", .position = position});
		requests.push_back({.id = "test_chunk_marker", .position = position});

		const auto node = simulation->simulate(Event::LoadChunks{.position = target});

		ASSERT_EQ(node.outcome, Event::Outcome::Accepted);
		ASSERT_FALSE(node.consequences.empty());
		const auto& marker = node.consequences.front();
		ASSERT_EQ(marker.outcome, Event::Outcome::Accepted);
		const auto& event = std::get<Event::Spawn>(marker.event);
		const auto& registry = simulation->get_registry();
		EXPECT_EQ(registry.get<Component::Value::Name>(event.entity).value, "Chunk test marker");
		EXPECT_EQ(registry.get<Position>(event.entity).value, position);
		EXPECT_TRUE(simulation->get_grid().contains(target));
		EXPECT_TRUE(requests.empty());
	}

	TEST_F(LoadChunksTest, AlreadyLoadedAreaDoesNotGenerateAnotherBatch)
	{
		const auto first = simulation->simulate(Event::LoadChunks{.position = target});
		ASSERT_EQ(first.outcome, Event::Outcome::Accepted);
		ASSERT_TRUE(simulation->get_grid().contains(target));
		const auto chunk_count = simulation->get_grid().get_chunks().size();
		const auto positioned =
			simulation->get_registry().view<Position>().size();

		const auto second = simulation->simulate(Event::LoadChunks{.position = target});

		EXPECT_EQ(second.outcome, Event::Outcome::Accepted);
		EXPECT_TRUE(second.consequences.empty());
		EXPECT_EQ(simulation->get_grid().get_chunks().size(), chunk_count);
		EXPECT_EQ(simulation->get_registry().view<Position>().size(), positioned);
	}

	TEST_F(LoadChunksTest, GeneratedSpawnEventsCarryTheRequestedRadius)
	{
		const World::ChunkPosition fixture_chunk{9, -8};
		ASSERT_TRUE(simulation->get_grid().add(fixture_chunk,
			World::Chunk{Game::Enum::Material::Stone, Game::Enum::Form::Floor}));
		const auto position = World::to_global(fixture_chunk, {2, 2});

		auto& registry = simulation->get_registry();
		const auto blocker = registry.create();
		registry.emplace<Position>(blocker, position);
		registry.emplace<Component::Value::CollisionMovement>(blocker, true);

		auto& requests = simulation->get_generator().get_spawn_requests();
		requests.push_back({.id = "test_chunk_marker", .position = position});
		requests.push_back({
			.id = "test_chunk_marker", .position = position, .search_radius = 1.0});

		const auto node = simulation->simulate(Event::LoadChunks{.position = target});

		ASSERT_EQ(node.outcome, Event::Outcome::Accepted);
		ASSERT_GE(node.consequences.size(), 2u);
		const auto& first = node.consequences[0];
		const auto& second = node.consequences[1];
		const auto& exact = std::get<Event::Spawn>(first.event);
		const auto& near = std::get<Event::Spawn>(second.event);
		EXPECT_EQ(exact.search_radius, 0.0);
		EXPECT_EQ(near.search_radius, 1.0);
		EXPECT_EQ(first.outcome, Event::Outcome::Rejected);
		EXPECT_FALSE(registry.valid(exact.entity));
		ASSERT_EQ(second.outcome, Event::Outcome::Accepted);
		EXPECT_DOUBLE_EQ(World::distance(registry.get<Position>(near.entity).value, position), 1.0);
	}

	TEST_F(LoadChunksTest, EnteringPositionLoadsThePlayersNewChunk)
	{
		const auto player = simulation->get_player();
		const auto start = simulation->get_registry().get<Position>(player).value;
		const auto leave = simulation->simulate(Event::LeavePosition{
			.entity = player, .position = start});
		ASSERT_EQ(leave.outcome, Event::Outcome::Accepted);
		const auto destination = World::to_global(target, {2, 2});

		const auto enter = simulation->simulate(Event::EnterPosition{
			.entity = player, .position = destination});

		ASSERT_EQ(enter.outcome, Event::Outcome::Accepted);
		ASSERT_EQ(enter.consequences.size(), 1u);
		const auto& load = enter.consequences.front();
		EXPECT_EQ(load.outcome, Event::Outcome::Accepted);
		EXPECT_EQ(std::get<Event::LoadChunks>(load.event).position, target);
		EXPECT_TRUE(simulation->get_grid().contains(target));
		EXPECT_EQ(simulation->get_registry().get<Position>(player).value, destination);
	}

	TEST_F(LoadChunksTest, WiderViewportLoadsTheAdditionalColumns)
	{
#if NCURSES_REENTRANT
		GTEST_SKIP() << "Requires writable ncurses screen-dimension globals";
#else
		struct RestoreScreenSize
		{
			int lines = LINES;
			int columns = COLS;

			~RestoreScreenSize()
			{
				LINES = lines;
				COLS = columns;
			}
		} restore_screen_size;

		LINES = World::Chunk::height;
		COLS = 2 * World::Chunk::width - 1;
		const World::ChunkPosition further_column{target.y, target.x + 2};
		const auto first = simulation->simulate(Event::LoadChunks{.position = target});
		ASSERT_EQ(first.outcome, Event::Outcome::Accepted);
		EXPECT_FALSE(simulation->get_grid().contains(further_column));

		COLS = 2 * World::Chunk::width + 1;
		const auto wider = simulation->simulate(Event::LoadChunks{.position = target});

		EXPECT_EQ(wider.outcome, Event::Outcome::Accepted);
		EXPECT_TRUE(simulation->get_grid().contains(further_column));
#endif
	}
}
