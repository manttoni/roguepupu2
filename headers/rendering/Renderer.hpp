#pragma once

#include "ncurses/Window.hpp"
#include "game/Simulation.hpp"
#include "game/world/Position.hpp"
#include "ui/Theme.hpp"
#include "ncurses/Pixel.hpp"

class Renderer
{
	public:
		Renderer(Ncurses::Window& surface) : surface(surface) {}

		void render_player_perspective(
			const Game::Simulation& simulation,
			const std::optional<Game::World::GlobalPosition> cursor =
			std::nullopt);

		void write(const std::vector<std::string>& messages);
		void set_render_all(bool value) { render_all = value; }
		bool& get_render_all() { return render_all; }
		bool get_render_all() const { return render_all; }

	private:
		Ncurses::Window& surface;
		size_t frame = 0;
		bool render_all = false;

		Ncurses::Pixel get_pixel(
			const Game::Simulation& simulation,
			const Game::World::GlobalPosition& position);
		entt::entity get_entity(
			const Game::Simulation& simulation,
			const Game::World::GlobalPosition& position);
		Ncurses::Color get_cell_color(const Game::World::Cell& cell);
		char get_cell_glyph(const Game::World::Cell& cell);

};

