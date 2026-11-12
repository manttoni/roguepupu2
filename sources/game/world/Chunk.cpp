#include "game/world/Chunk.hpp"

namespace Game::World
{
	Chunk::Chunk(
		const Game::Enum::Material material,
		const Game::Enum::Form form)
	{
		for (auto& cell : cells)
		{
			cell.material = material;
			cell.form = form;
		}
	}

	const Cell& Chunk::get_cell(const LocalPosition& position) const
	{
		return cells[to_index(position)];
	}

	Cell& Chunk::get_cell(const LocalPosition& position)
	{
		return cells[to_index(position)];
	}

	std::vector<GlobalPosition> Chunk::get_positions(
		const ChunkPosition& chunk_position)
	{
		const auto dimensions = Chunk::dimensions();
		std::vector<GlobalPosition> positions;
		positions.reserve(dimensions.y * dimensions.x);

		for (int y = 0; y < dimensions.y; ++y)
		{
			for (int x = 0; x < dimensions.x; ++x)
				positions.push_back(to_global(chunk_position, LocalPosition{y, x}));
		}

		return positions;
	}
}
