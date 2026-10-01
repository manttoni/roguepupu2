#include "game/entities/Spawner.hpp"

namespace Game::Entity
{
	void Spawner::spawn_on_cell(const World::GlobalPosition& position)
	{
		if (
				const auto* cell = grid.find_cell(position);
				cell == nullptr)
		{
			Log::error() << "Spawning on ungenerated cell: " << position;
			return;
		}

	}
	void Spawner::spawn_on_chunk(const World::ChunkPosition& position)
	{
		if (
				const auto* chunk = grid.find_chunk(position);
				chunk == nullptr)
		{
			Log::error() << "Spawning on ungenerated chunk: " << position;
			return;
		}
	}
	void Spawner::spawn_on_grid()
	{
	}
	void Spawner::cell_requests(const World::GlobalPosition& position)
	{
		(void) position;
	}
	void Spawner::chunk_requests(const World::ChunkPosition& position)
	{
		(void) position;
	}
	void Spawner::grid_requests()
	{
	}
}
