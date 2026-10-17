#pragma once

#include "external/entt/fwd.hpp"
#include "databases/EntityDatabase.hpp"
#include "game/world/Grid.hpp"
#include "game/entity/Definition.hpp"
#include "game/entity/SpawnRequest.hpp"

namespace Game::Entity
{
	class Spawner
	{
		public:
			Spawner(entt::registry& registry,
					const EntityDatabase& database,
					const World::Grid& grid);

			entt::entity spawn(
					const Definition::ID& id,
					const World::GlobalPosition& position);
			entt::entity spawn(const Entity::SpawnRequest& request);

			bool can_spawn(
					const Definition::ID& id,
					const World::GlobalPosition& position
					) const;
			bool can_spawn(const Entity::SpawnRequest& request);

		private:
			entt::registry& registry;
			const EntityDatabase& database;
			const World::Grid& grid;
	};
}
