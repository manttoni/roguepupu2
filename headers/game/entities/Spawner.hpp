#pragma once

#include "databases/EntityDatabase.hpp"
#include "external/entt/entt.hpp"
#include "game/world/Grid.hpp"
#include "game/world/Position.hpp"
#include "game/entities/Entity.hpp"
#include <string>
#include <vector>

namespace Game::Entity
{
	/* Spawn entities or cells or chunks or grids
	 *
	 * Cells: Look at cell values and randomly spawn things that fit
	 * 	- Spawning entities close to terrain, like trees, bushes
	 *
	 * Chunks: Look at the whole Chunk at once and spawn groups of entities
	 * 	- Spawning npcs in a town, group of enemies in the dungeon
	 *
	 * Grid: Faction territories
	 * */

	class Spawner
	{
		public:
			Spawner(
					const EntityDatabase& entity_database,
					entt::registry& registry,
					const World::Grid& grid) // grid likely to change into some kind of wrapper
				: entity_database(entity_database),
				registry(registry),
				grid(grid)
		{
		}

			void spawn_on_cell(const World::GlobalPosition& position);
			void spawn_on_chunk(const World::ChunkPosition& position);
			void spawn_on_grid();

		private:
			const EntityDatabase& entity_database;
			entt::registry& registry;
			const World::Grid& grid;

			/* Requests can first be made, then reviewed as a whole
			 * */
			struct Request
			{
				World::GlobalPosition position;
				Definition::ID entity_id;
			};

			std::vector<Request> requests;

			void cell_requests(const World::GlobalPosition& position);
			void chunk_requests(const World::ChunkPosition& position);
			void grid_requests();
	};
}
