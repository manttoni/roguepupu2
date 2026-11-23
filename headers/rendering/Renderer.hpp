#pragma once

#include <optional>

#include "game/Simulation.hpp"
#include "game/world/Position.hpp"
#include "ncurses/Pixel.hpp"

class Layout;
class Messages;

class Renderer
{
	public:
		Renderer(Layout& layout, const Messages& messages) :
			layout(layout), messages(messages) {}

		// Without a simulation, redraw the layout with an empty game panel.
		void redraw(
			const Game::Simulation* simulation = nullptr,
			const std::optional<Game::World::GlobalPosition> cursor =
			std::nullopt);

		void set_render_all(bool value) { render_all = value; }
		bool& get_render_all() { return render_all; }
		bool get_render_all() const { return render_all; }

	private:
		Layout& layout;
		const Messages& messages;
		size_t frame = 0;
		bool render_all = false;

		void render_game(
			const Game::Simulation& simulation,
			const std::optional<Game::World::GlobalPosition> cursor);
		void render_messages();
		void render_player_status();

		Ncurses::Color get_bgcolor(
			const Game::Simulation& simulation,
			const Game::World::GlobalPosition& position);
		Ncurses::Pixel get_pixel(
			const Game::Simulation& simulation,
			const Game::World::GlobalPosition& position);
		entt::entity get_entity(
			const Game::Simulation& simulation,
			const Game::World::GlobalPosition& position);
		Ncurses::Color get_cell_color(const Game::World::Cell& cell);
		char get_cell_glyph(const Game::World::Cell& cell);

};

