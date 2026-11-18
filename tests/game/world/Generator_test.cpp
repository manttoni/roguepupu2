#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "game/world/Generator.hpp"
#include "utils/Random.hpp"

namespace
{
	namespace World = Game::World;
	using Game::Enum::Form;
	using Game::Enum::Material;

	constexpr std::string_view flat_config = R"({
		"bases": ["flat"],
		"layers": {
			"flat": {
				"terrain": {"material": "Stone", "form": "Floor"},
				"entities": [
					{"id": "test_always", "chance": 1.0},
					{"id": "test_never", "chance": 0.0}
				]
			}
		}
	})";

	constexpr std::string_view chance_config = R"({
		"bases": ["flat"],
		"layers": {
			"flat": {
				"terrain": {"material": "Stone", "form": "Floor"},
				"entities": [{"id": "test_stone", "chance": 0.25}]
			}
		}
	})";

	bool same_requests(
		const std::vector<Game::Entity::SpawnRequest>& a,
		const std::vector<Game::Entity::SpawnRequest>& b)
	{
		return std::equal(a.begin(), a.end(), b.begin(), b.end(),
			[](const auto& first, const auto& second)
			{
				return first.id == second.id
					&& first.position == second.position
					&& first.search_radius == second.search_radius;
			});
	}

	class GeneratorTest : public ::testing::Test
	{
		protected:
			std::filesystem::path directory;

			void SetUp() override
			{
				const auto root = std::filesystem::temp_directory_path();
				const auto stamp = std::chrono::steady_clock::now()
					.time_since_epoch().count();
				for (int suffix = 0; ; ++suffix)
				{
					auto candidate = root / ("roguepupu2-generator-test-"
						+ std::to_string(stamp) + "-" + std::to_string(suffix));
					if (std::filesystem::create_directory(candidate))
					{
						directory = std::move(candidate);
						break;
					}
				}
			}

			void TearDown() override
			{
				std::error_code error;
				if (!directory.empty())
					std::filesystem::remove_all(directory, error);
			}

			std::filesystem::path write_config(std::string_view text) const
			{
				const auto path = directory / "generator.json";
				std::ofstream file;
				file.exceptions(std::ios::failbit | std::ios::badbit);
				file.open(path);
				file << text;
				file.close();
				return path;
			}
	};

	TEST_F(GeneratorTest, ChanceZeroAndOneAtNegativeChunkCoordinates)
	{
		World::Generator generator{"edges", write_config(flat_config)};
		const World::ChunkPosition position{-3, -2};
		const auto chunk = generator.generate_chunk(position);
		const auto& requests = generator.get_spawn_requests();

		ASSERT_EQ(requests.size(), chunk.get_cells().size());
		std::unordered_set<World::GlobalPosition> occupied;
		for (const auto& request : requests)
		{
			ASSERT_EQ(request.id, "test_always");
			ASSERT_EQ(World::to_chunk(request.position), position);
			occupied.insert(request.position);
		}
		EXPECT_EQ(occupied.size(), chunk.get_cells().size());
		EXPECT_TRUE(std::all_of(chunk.get_cells().begin(), chunk.get_cells().end(),
			[](const auto& cell)
			{
				return cell.material == Material::Stone && cell.form == Form::Floor;
			}));
	}

	TEST_F(GeneratorTest, SameSeedReplaysSpawnsRegardlessOfChunkOrderAndGlobalRng)
	{
		const auto path = write_config(chance_config);
		const World::ChunkPosition target{-2, 5};
		World::Generator first{"stable-world", path};
		first.generate_chunk(target);
		ASSERT_FALSE(first.get_spawn_requests().empty());

		World::Generator second{"stable-world", path};
		second.generate_chunk({4, -1});
		second.get_spawn_requests().clear();

		Random::seed(127);
		for (int i = 0; i < 16; ++i)
			(void) Random::integer(1, 20);

		second.generate_chunk(target);
		EXPECT_TRUE(same_requests(first.get_spawn_requests(), second.get_spawn_requests()));
	}

	TEST_F(GeneratorTest, WorldSeedAffectsSpawnPlacementOnTheSameTerrain)
	{
		const auto path = write_config(chance_config);
		World::Generator first{"stone-world", path};
		World::Generator second{"other-world", path};
		first.generate_chunk({1, -1});
		second.generate_chunk({1, -1});

		ASSERT_FALSE(first.get_spawn_requests().empty());
		ASSERT_FALSE(second.get_spawn_requests().empty());
		EXPECT_FALSE(same_requests(first.get_spawn_requests(), second.get_spawn_requests()));
	}

	TEST_F(GeneratorTest, OverlappingRegionsProvideTerrainForEveryCell)
	{
		const auto path = write_config(R"({
			"bases": ["world"],
			"layers": {
				"world": {"noise": {"frequency": 0.0, "regions": [
					{"id": "stone", "range": [0.0, 1.0]},
					{"id": "soil", "range": [0.0, 1.0]}
				]}},
				"stone": {"terrain": {"material": "Stone", "form": "Floor"}},
				"soil": {"terrain": {"material": "Soil", "form": "Wall"}}
			}
		})");
		World::Generator generator{"overlap", path};
		const auto chunk = generator.generate_chunk({0, 0});

		bool saw_stone = false;
		bool saw_soil = false;
		for (const auto& cell : chunk.get_cells())
		{
			ASSERT_NE(cell.material, Material::None);
			ASSERT_NE(cell.form, Form::None);
			saw_stone |= cell.material == Material::Stone;
			saw_soil |= cell.material == Material::Soil;
		}
		EXPECT_TRUE(saw_stone);
		EXPECT_TRUE(saw_soil);
	}

	TEST_F(GeneratorTest, ARegionOutsideTheNoiseRangeDoesNotSpawnItsEntities)
	{
		const auto path = write_config(R"({
			"bases": ["world"],
			"layers": {
				"world": {
					"terrain": {"material": "Stone", "form": "Floor"},
					"noise": {"frequency": 0.0, "regions": [
						{"id": "outside", "range": [0.0, 0.25]}
					]}
				},
				"outside": {"entities": [{"id": "test_outside", "chance": 1.0}]}
			}
		})");
		World::Generator generator{"outside", path};

		generator.generate_chunk({0, 0});

		EXPECT_TRUE(generator.get_spawn_requests().empty());
	}

	TEST(ChunkPositionsTest, CoversEveryGlobalPositionOfANegativeChunkOnce)
	{
		const World::ChunkPosition chunk{-1, -2};
		const auto positions = World::Chunk::get_positions(chunk);
		ASSERT_EQ(positions.size(), World::Chunk::height * World::Chunk::width);
		const std::unordered_set<World::GlobalPosition> unique(
			positions.begin(), positions.end());
		EXPECT_EQ(unique.size(), positions.size());
		EXPECT_TRUE(unique.contains(World::to_global(chunk, {0, 0})));
		EXPECT_TRUE(unique.contains(World::to_global(
			chunk, {World::Chunk::height - 1, World::Chunk::width - 1})));
		EXPECT_TRUE(std::all_of(positions.begin(), positions.end(), [&](const auto& position)
			{
				return World::to_chunk(position) == chunk;
			}));
	}
}
