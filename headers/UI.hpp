#pragma once

#include <ncurses.h>

namespace UI
{
	void print(const int i);
	void println(const char* chptr);
	void print(const char* chptr);
	void println(WINDOW *window = stdscr, const std::string& str = "");
	void print(WINDOW *window = stdscr, const std::string& str = "");
	void init_ncurses();
	void start_menu();
	void end_ncurses();
}
