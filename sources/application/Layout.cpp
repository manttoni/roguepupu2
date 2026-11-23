#include "application/Layout.hpp"
#include "ncurses/Screen.hpp"

#include <algorithm>

Layout::Layout() : Layout(Settings{
	.cross_position = Ncurses::Screen::size() * 3 / 4
	})
{}

Layout::Layout(const Settings& settings) : settings(settings)
{
	panels.try_emplace(Panel::Game);
	panels.try_emplace(Panel::MessageLog);
	panels.try_emplace(Panel::PlayerStatus);
	panels.at(Panel::Game).get_window().set_borders({false, false, false, false});
	panels.at(Panel::MessageLog).get_window().set_borders({true, false, false, false});
	panels.at(Panel::PlayerStatus).get_window().set_borders({false, false, true, false});
	reset();
}

void Layout::reset()
{
	const auto screen_size = Ncurses::Screen::size();
	const auto place = [this](Panel id, int height, int width, int y, int x)
	{
		auto& panel = panels.at(id);
		panel.resize(height, width);
		panel.set_position(y, x);
	};

	// Keep a separator and at least one content cell in auxiliary panels.
	const bool show_log = screen_size.y >= 3;
	const bool show_status = screen_size.x >= 3;
	const int game_height = show_log ?
		std::clamp(settings.cross_position.y, 1, screen_size.y - 2) : screen_size.y;
	const int game_width = show_status ?
		std::clamp(settings.cross_position.x, 1, screen_size.x - 2) : screen_size.x;

	place(Panel::Game, game_height, game_width, 0, 0);

	auto& log = panels.at(Panel::MessageLog);
	if (show_log)
	{
		place(Panel::MessageLog, screen_size.y - game_height,
			game_width, game_height, 0);
		log.show();
	}
	else
		log.hide();

	auto& status = panels.at(Panel::PlayerStatus);
	if (show_status)
	{
		place(Panel::PlayerStatus, screen_size.y,
			screen_size.x - game_width, 0, game_width);
		status.show();
	}
	else
		status.hide();
}
