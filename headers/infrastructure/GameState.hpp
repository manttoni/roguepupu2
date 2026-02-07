#pragma once

#include "external/entt/fwd.hpp"
#include "external/entt/entity/handle.hpp"

struct GameState
{
	entt::entity player = entt::null;
	bool game_running;
	size_t turn_number = 1;
};
