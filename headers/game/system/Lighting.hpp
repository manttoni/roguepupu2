#pragma once

#include "game/Simulation.hpp"
#include "external/entt/fwd.hpp"

namespace Game::System::Lighting
{
	// Update lighting that comes from 'entity'
	void update(Simulation& simulation, entt::entity entity);
}

