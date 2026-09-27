#pragma once

#include "ncurses/Window.hpp"
#include "game/Simulation.hpp"
#include "game/world/Position.hpp"
#include "ui/Theme.hpp"

class Renderer
{
	public:
		Renderer(Ncurses::Window& surface) : surface(surface) {}
		void render(const Game::Simulation& simulation, const Game::World::GlobalPosition& center);
		void render(const Game::Simulation& simulation); // deduce center (player position)
		/* This might not be here in the end, but could be. Printing messages on same surface is not necessarily bad.
		void render_messages(const std::vector<std::string>& messages)
		*/

	private:
		Ncurses::Window& surface;
		size_t frame = 0;
};

