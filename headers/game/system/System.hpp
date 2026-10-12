#pragma once

#include "game/event/Event.hpp"
#include "game/Simulation.hpp"

namespace Game::System
{
	Game::Event::Result dispatch(Simulation& simulation, const Game::Event::Any& event);
}

