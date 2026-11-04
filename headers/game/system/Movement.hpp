#pragma once

#include "game/Simulation.hpp"
#include "external/entt/fwd.hpp"
#include "game/world/Position.hpp"

namespace Game::System::Movement
{
	double cost(
		const Game::Simulation& simulation,
		const World::GlobalPosition& from,
		const World::GlobalPosition& to);
	bool has_path(
		const entt::registry& registry,
		const World::Grid& grid,
		const World::GlobalPosition& from,
		const World::GlobalPosition& to);
}
