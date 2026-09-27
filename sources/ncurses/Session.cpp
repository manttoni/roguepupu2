#include "ncurses/Session.hpp"

#include <clocale>
#include <cstdio>
#include <ncurses.h>

#include "utils/Log.hpp"

namespace Ncurses
{
	Session::Session()
	{
		setlocale(LC_ALL, "");

		initscr();
		start_color();

		// Keyboard input
		noecho();
		curs_set(0);
		keypad(stdscr, TRUE);
		set_escdelay(25);

		// Mouse input
		/*
		   mousemask(
		   ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION,
		   nullptr);

		   std::printf("\033[?1003h");
		   std::fflush(stdout);
		   */

		Log::info() << "Ncurses initialized";
	}

	Session::~Session() noexcept
	{
		Log::info() << "Ncurses session ended";

		// Restore normal terminal mode.
		endwin();

		// Disable mouse movement reporting in case it was enabled.
		std::printf("\033[?1003l");
		std::fflush(stdout);
	}
}
