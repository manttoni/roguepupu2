#pragma once

#include <string>
#include <vector>

#include "game/Simulation.hpp"
#include "game/events/Event.hpp"

namespace EventDescriber
{
	std::vector<std::string> describe(const Game::Simulation& simulation, const Game::Event::Node& root);
}
