#include "game/world/Grid.hpp"
#include "game/component/Component.hpp"
#include "external/entt/entt.hpp"
#include "utils/Log.hpp"
#include "game/world/Position.hpp"

namespace Game::World
{
	const Cell* Grid::find_cell(const GlobalPosition& global_position) const
	{
		const auto chunk_position = to_chunk(global_position);
		const auto it = chunks.find(chunk_position);

		if (it == chunks.end())
			return nullptr;

		const auto local_position = to_local(chunk_position, global_position);

		return &it->second.get_cell(local_position);
	}

	Cell* Grid::find_cell(const GlobalPosition& global_position)
	{
		const auto chunk_position = to_chunk(global_position);
		const auto it = chunks.find(chunk_position);

		if (it == chunks.end())
			return nullptr;

		const auto local_position = to_local(chunk_position, global_position);

		return &it->second.get_cell(local_position);
	}

	const Chunk* Grid::find_chunk(const ChunkPosition& chunk_position) const
	{
		const auto it = chunks.find(chunk_position);

		if (it == chunks.end())
			return nullptr;

		return &it->second;
	}

	Chunk* Grid::find_chunk(const ChunkPosition& position)
	{
		const auto it = chunks.find(position);

		if (it == chunks.end())
			return nullptr;

		return &it->second;
	}

	bool Grid::contains(const ChunkPosition& position) const
	{
		return chunks.contains(position);
	}

	bool Grid::add(const ChunkPosition& position, Chunk chunk)
	{
		if (contains(position))
			return false;
		chunks[position] = std::move(chunk);
		return true;
	}
}
