#include "game/world/Grid.hpp"
namespace
{
	// divisor must be positive
	int floor_div(int value, int divisor)
	{
		const int quotient = value / divisor;
		const int remainder = value % divisor;
		return quotient - (remainder < 0);
	}

	Game::World::ChunkPosition to_chunk_position(
			const Game::World::GlobalPosition& position)
	{
		const auto size = Game::World::Chunk::dimensions();

		return {
			floor_div(position.y, size.y),
				floor_div(position.x, size.x)
		};
	}

	Game::World::LocalPosition to_local_position(
			const Game::World::GlobalPosition& position,
			const Game::World::ChunkPosition& chunk)
	{
		const auto size = Game::World::Chunk::dimensions();

		return {
			position.y - chunk.y * size.y,
				position.x - chunk.x * size.x
		};
	}
}
namespace Game::World
{
	Cell& Grid::get_cell(const GlobalPosition& position)
	{
		const auto chunk_position = to_chunk_position(position);
		auto it = generated_chunks.find(chunk_position);

		if (it == generated_chunks.end())
		{
			it = generated_chunks
				.emplace(chunk_position, generate_chunk(chunk_position))
				.first;
		}

		const auto local_position =
			to_local_position(position, chunk_position);

		return it->second.get_cell(local_position);
	}

	const Cell* Grid::find_cell(const GlobalPosition& position) const
	{
		const auto chunk_position = to_chunk_position(position);
		const auto it = generated_chunks.find(chunk_position);

		if (it == generated_chunks.end())
			return nullptr;

		const auto local_position =
			to_local_position(position, chunk_position);

		return &it->second.get_cell(local_position);
	}
}
