#include "game/entities/Spawner.hpp"

namespace Game::Entity
{
	void Spawner::spawn_on_cell(const World::GlobalPosition& position)
	{
		const auto* cell = grid.find_cell(position);
		if (cell == nullptr)
		{
			Log::error() << "Spawning on ungenerated cell: " << position;
			return;
		}

	}
	void Spawner::spawn_on_chunk(const World::ChunkPosition& position)
	{
		if (!grid.chunk_generated(position))
		{
			Log::error() << "Spawning on ungenerated chunk: " << position;
			return;
		}
	}
	void Spawner::spawn_on_grid()
	{
		if (grid.get_generated_chunks().empty())
		{
			Log::error() << "Spawning on empty grid";
			return;
		}

	}
	void Spawner::cell_requests(const World::GlobalPosition& position)
	{
	}
	void Spawner::chunk_requests(const World::ChunkPosition& position)
	{
	}
	void Spawner::grid_requests()
	{
	}
}
