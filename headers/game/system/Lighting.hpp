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
	void update_light(
		Simulation& simulation,
		entt::entity light,
		const World::GlobalPosition& position,
		Operation operation);

	void update(
		Simulation& simulation,
		entt::entity entity,
		Operation operation);

	void reset(Simulation& simulation);
}

