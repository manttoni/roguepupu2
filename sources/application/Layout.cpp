#include "application/Layout.hpp"
#include "ncurses/Screen.hpp"

void Layout::reset()
{
	game_panel.resize(Ncurses::Screen::height(), Ncurses::Screen::width());
}
