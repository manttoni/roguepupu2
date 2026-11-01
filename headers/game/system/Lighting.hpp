#pragma once

#include "game/Simulation.hpp"
#include "external/entt/fwd.hpp"

namespace Game::System::Lighting
{
	enum class Operation
	{
		Add,
		Remove
	};
	void update(
		Simulation& simulation,
		entt::entity entity,
		Operation operation);

	void reset(Simulation& simulation);
}

