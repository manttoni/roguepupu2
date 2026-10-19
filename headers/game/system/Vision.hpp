#pragma once

#include "external/entt/entt.hpp"
#include "game/world/Grid.hpp"
#include "game/world/Position.hpp"

namespace Game::System::Vision
{
	bool los(
			const entt::registry& registry,
			const World::Grid& grid,
			World::GlobalPosition a,
			World::GlobalPosition b);
	void update_vision(
			entt::registry& registry,
			const World::Grid& grid,
			entt::entity entity);
}
