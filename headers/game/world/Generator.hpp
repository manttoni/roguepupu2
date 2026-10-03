#pragma once

#include <nlohmann/json.hpp>
#include "game/world/Cell.hpp"
#include "game/world/Chunk.hpp"
#include "game/world/Position.hpp"
#include "game/Enum.hpp"
#include "utils/Perlin.hpp"
#include <vector>
#include <filesystem>

namespace Game::World
{
	class Generator
	{
		using Json = nlohmann::json;
		using Material = Game::Enum::Material;
		using Form = Game::Enum::Form;

		public:
			Generator(const std::string& seed, const std::filesystem::path& path = "data/generator/conf.json");

			Chunk generate_chunk(const ChunkPosition& position) const;

		private:
			struct Layer
			{
				Material material = Material::None;
				Form form = Form::None;
				double threshold = 0.0;
				double water_depth = 0.0;
				std::string id = "";

				Random::Perlin::Generator perlin;
			};
			std::vector<Layer> layers;
			std::string seed;
			void parse_layer(const Json& layers);
			void parse_layers(const Json& layer);

			Cell generate_cell(const GlobalPosition& position) const;
	};
}
