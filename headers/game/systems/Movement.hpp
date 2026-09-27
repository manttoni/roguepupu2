#pragma once

#include "game/Simulation.hpp"
#include "external/entt/fwd.hpp"
#include "game/world/Position.hpp"

namespace Game::System::Movement
{
	bool can_move(const Simulation& simulation, const entt::entity entity, const World::GlobalPosition& to);
}
