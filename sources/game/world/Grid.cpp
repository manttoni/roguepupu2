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

		const auto local_position = to_local_position(position, chunk_position);

		return &it->second.get_cell(local_position);
	}

	Cell Grid::generate_cell(const GlobalPosition& position)
	{
		(void) position;
		return Cell(Game::Enum::Terrain::Ground);
	}

	Chunk Grid::generate_chunk(const ChunkPosition& chunk_position)
	{
		const int height = static_cast<int>(Chunk::height);
		const int width = static_cast<int>(Chunk::width);

		const GlobalPosition begin{
			chunk_position.y * height,
			chunk_position.x * width
		};

		const GlobalPosition end{
			begin.y + height,
			begin.x + width
		};

		Chunk::CellsArray cells;

		for (int y = begin.y; y < end.y; ++y)
		{
			for (int x = begin.x; x < end.x; ++x)
			{
				const GlobalPosition global_position{y, x};
				const LocalPosition local_position = to_local_position(global_position, chunk_position);
				cells[to_index(local_position)] = generate_cell(global_position);
			}
		}
		return Chunk{cells};
	}

	void Grid::generate_missing(const GlobalPosition& global_position)
	{
		const ChunkPosition chunk_position =
			to_chunk_position(global_position);

		if (generated_chunks.contains(chunk_position))
			return;

		generated_chunks.emplace(
				chunk_position,
				generate_chunk(chunk_position));
	}
}
