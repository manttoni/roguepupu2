#pragma once

#include "external/entt/fwd.hpp"
#include "database/EntityFactory.hpp"

struct GameContext
{
	entt::registry& registry;
	EntityFactory& factory;
};
