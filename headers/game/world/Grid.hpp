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
			const Cell* find_cell(const GlobalPosition& position) const;
			Cell* find_cell(const GlobalPosition& position);
			const Chunk* find_chunk(const ChunkPosition& position) const;
			Chunk* find_chunk(const ChunkPosition& position);
			bool contains(const ChunkPosition& position) const;
			bool add(const ChunkPosition& position, Chunk chunk);

		private:
			ChunkMap chunks;
			static constexpr std::size_t to_index(const LocalPosition& position)
			{
				return position.y * Chunk::width + position.x;
			}
	};
}

/* Grid > Chunk > Cell
 * */
