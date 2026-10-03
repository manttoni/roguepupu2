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
}
