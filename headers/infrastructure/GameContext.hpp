#pragma once

#include "external/entt/fwd.hpp"

struct GameContext
{
	entt::registry& registry;
	EntityFactory& factory;
};
