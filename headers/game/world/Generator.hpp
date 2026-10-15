#pragma once

#include "game/world/Cell.hpp"
#include "game/world/Chunk.hpp"
#include "game/world/Position.hpp"
#include "game/entity/Definition.hpp"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Game::World
{
	struct SpawnRequest
	{
		Entity::Definition::ID id;
		GlobalPosition position;
	};

	class Generator
	{
		using Json = nlohmann::json;

		public:
			Generator(const std::string& seed, const std::filesystem::path& path = "data/generator/conf.json");

			Chunk generate_chunk(const ChunkPosition& position);
			std::vector<SpawnRequest>& get_spawn_requests() { return spawn_requests; }

		private:
			std::string seed;
			Json conf;

			void apply_layer(const std::string& layer_id, Cell& cell, const GlobalPosition& position);
			Cell generate_cell(const GlobalPosition& position);

			std::vector<SpawnRequest> spawn_requests;
	};
}
