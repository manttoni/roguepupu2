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
	class Rasterizer
	{
		public:
			Rasterizer(const std::string& seed, const std::filesystem::path& path = "data/rasterizer/conf.json");

			Chunk rasterize_chunk(const ChunkPosition& position) const;

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

			Cell rasterize_cell(const GlobalPosition& position) const;
	};
}
