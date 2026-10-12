#pragma once

#include "game/Simulation.hpp"
#include "external/entt/fwd.hpp"
#include "game/event/Event.hpp"

namespace Game::AI
{
	Event::Any get_event(const Simulation& simulation, const entt::entity entity);
}
