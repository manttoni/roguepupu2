#pragma once

#include "ncurses/Window.hpp"
#include "game/Simulation.hpp"
#include "game/world/Position.hpp"
#include "ui/Theme.hpp"

class Renderer
{
	public:
		Renderer(Ncurses::Window& surface) : surface(surface) {}

		void render_player_perspective(const Game::Simulation& simulation);

	private:
		Ncurses::Window& surface;
		size_t frame = 0;

		struct Visual
		{
			char glyph;
			Ncurses::Color color;
		};
		Visual get_visual(
				const Game::Simulation& simulation,
				const Game::World::GlobalPosition& position);
		std::optional<Visual> get_entity_visual(
				const Game::Simulation& simulation,
				const Game::World::GlobalPosition& position);
		Visual get_cell_visual(const Game::World::Cell& cell);
};

