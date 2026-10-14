#pragma once

namespace Game::Entity
{
	class Spawner
	{
		public:
			Spawner(entt::registry& registry);
			entt::entity spawn(const Entity::Definition::ID& id, const GlobalPosition& position);
		private:
			entt::registry& registry;
	};
}
