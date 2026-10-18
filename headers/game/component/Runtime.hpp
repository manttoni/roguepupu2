#pragma once

#include <vector>
#include <unordered_set>
#include "game/world/Position.hpp"

/* This namespace describes components that are created during runtime.
 * Could get renamed soon?
 * */
namespace Game::Component::Runtime
{
	struct Vision
	{
		std::vector<World::GlobalPosition> visible;
		std::unordered_set<World::GlobalPosition> seen;
	};
}
