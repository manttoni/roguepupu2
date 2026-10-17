#pragma once

#include "game/entity/Definition.hpp"
#include "game/world/Position.hpp"

namespace Game::Entity
{
	struct SpawnRequest
	{
		Definition::ID id;
		World::GlobalPosition position;
	};
}
