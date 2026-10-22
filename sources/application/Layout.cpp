#include "application/Layout.hpp"
#include "ncurses/Screen.hpp"

Layout::Layout()
{
	panels[Panel::Game] = Ncurses::Panel{};
	reset();
}

void Layout::reset()
{
	panels[Panel::Game]
		.resize(Ncurses::Screen::height(), Ncurses::Screen::width());
}
