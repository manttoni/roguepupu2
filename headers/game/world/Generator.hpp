#pragma once

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
		public:
			Generator(const std::string& seed, const std::filesystem::path& path = "data/generator/conf.json");

			Chunk generate_chunk(const ChunkPosition& position) const;

		private:
			struct Layer
			{
				Game::Enum::Terrain terrain;
				Random::Perlin::Generator perlin;
				double threshold = 0.5;

				Layer(
					Game::Enum::Terrain terrain,
					Random::Perlin::Generator perlin,
					double threshold = 0.5)
					: terrain(terrain),
					  perlin(std::move(perlin)),
					  threshold(threshold)
				{}
			};
			std::vector<Layer> layers;
			Game::Enum::Terrain default_terrain = Game::Enum::Terrain::Rock;

			Cell generate_cell(const GlobalPosition& position) const;
	};
}
