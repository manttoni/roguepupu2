#pragma once

#include <ncurses.h>
#include "utils/Log.hpp"

namespace Ncurses
{
	inline void init()
	{
		setlocale(LC_ALL, "");
		initscr();
		start_color();

		// keyboard input
		noecho();
		curs_set(0);
		keypad(stdscr, TRUE);
		set_escdelay(25);

		// mouse input
		/*mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);
		  printf("\033[?1003h");	// mouse movement will trigger KEY_MOUSE events
		  fflush(stdout);				// to know current cursor location
		  */
		Log::info() << "Ncurses initialized";
	}
	void end()
	{
		Log::info() << "UI ended";

		// this will restore normal terminal mode
		endwin();

		// reset mouse mode
		printf("\033[?1003l");
		fflush(stdout);

		std::exit(0);
	}
}
