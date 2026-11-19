#pragma once

#include "game/event/Event.hpp"
#include "game/Simulation.hpp"

namespace Game::System
{
	Game::Event::Node dispatch(Simulation& simulation, const Game::Event::Any& event);
}

