#pragma once

#include "game/Simulation.hpp"
#include "external/entt/fwd.hpp"
#include "game/events/Event.hpp"

namespace Game::AI
{
	Event::Any get_event(const Simulation& simulation, const entt::entity entity);
}
