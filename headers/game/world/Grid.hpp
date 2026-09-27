#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>

#include "game/world/Chunk.hpp"
#include "game/world/Position.hpp"

namespace Game::World
{
	struct ChunkPositionHash
	{
		std::size_t operator()(const ChunkPosition& position) const noexcept
		{
			std::size_t result = std::hash<std::int64_t>{}(position.y);

			result ^= std::hash<std::int64_t>{}(position.x)
				+ 0x9e3779b9
				+ (result << 6)
				+ (result >> 2);

			return result;
		}
	};

	using ChunkMap = std::unordered_map<ChunkPosition, Chunk, ChunkPositionHash>;

	class Grid
	{
		public:
			Grid(std::string seed) : seed(seed) {}

			Cell& get_cell(const GlobalPosition& position);

			/* return nullptr if cell hasn't been generated yet in places that cannot mutate World::Grid
			 * */
			const Cell* find_cell(const GlobalPosition& position) const;

		private:
			const std::string seed;
			ChunkMap generated_chunks;

			Chunk generate_chunk(const ChunkPosition& position);
			Chunk& get_chunk(const ChunkPosition& position);
	};
}
