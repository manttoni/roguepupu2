#include "game/world/Chunk.hpp"

namespace Game::World
{
	const Cell& Chunk::get_cell(const LocalPosition& position) const
	{
		return cells[to_index(position)];
	}

	Cell& Chunk::get_cell(const LocalPosition& position)
	{
		return cells[to_index(position)];
	}
}
