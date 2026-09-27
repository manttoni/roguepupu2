#pragma once

#include "ncurses/Panel.hpp"

class Layout
{
	public:
		void reset();

		Ncurses::Panel& get_game_panel() { return game_panel; }

	private:
		Ncurses::Panel game_panel;
};
