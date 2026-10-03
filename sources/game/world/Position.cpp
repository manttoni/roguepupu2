#include "game/world/Position.hpp"
#include "game/world/Chunk.hpp"

namespace Game::World
{
	namespace
	{
		// Requires a positive divisor.
		constexpr int floor_div(int value, int divisor)
		{
			return value / divisor - (value % divisor < 0);
		}

		constexpr int chunk_height = static_cast<int>(Chunk::height);
		constexpr int chunk_width = static_cast<int>(Chunk::width);

		static_assert(chunk_height > 0 && chunk_width > 0);
	}

	ChunkPosition to_chunk(const GlobalPosition& global)
	{
		return {
			floor_div(global.y, chunk_height),
				floor_div(global.x, chunk_width)
		};
	}

	GlobalPosition to_global(
			const ChunkPosition& chunk,
			const LocalPosition& local)
	{
		return {
			chunk.y * chunk_height + local.y,
			chunk.x * chunk_width + local.x
		};
	}

	LocalPosition to_local(
			const ChunkPosition& chunk,
			const GlobalPosition& global)
	{
		return {
			global.y - chunk.y * chunk_height,
			global.x - chunk.x * chunk_width
		};
	}


}
