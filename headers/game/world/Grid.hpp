#pragma once

#include <cstddef>
#include <cstdint>
#include <unordered_map>

#include "game/world/Settings.hpp"
#include "game/world/Chunk.hpp"
#include "game/world/Position.hpp"
#include "game/world/Generator.hpp"
#include "external/entt/fwd.hpp"

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
			Grid(std::string seed);

			Cell& get_cell(const GlobalPosition& position);
			const Cell* find_cell(const GlobalPosition& position) const;
			bool chunk_generated(const ChunkPosition& chunk_position) const;
			const ChunkMap& get_generated_chunks() const { return generated_chunks; }

			void generate_missing(const GlobalPosition& global_position);
			void generate_missing(const GlobalPosition& begin, const GlobalPosition& end);
			void preload_around(entt::registry& registry, const entt::entity entity);

		private:
			Generator generator;
			//Populator populator;

			ChunkMap generated_chunks;

			Settings settings;

			Cell generate_cell(const GlobalPosition& position);
			Chunk generate_chunk(const ChunkPosition& position);
			Chunk& get_chunk(const ChunkPosition& position);

			static constexpr std::size_t to_index(const LocalPosition& position)
			{
				return position.y * Chunk::width + position.x;
			}
	};
}

/* Grid > Chunk > Cell
 * */
