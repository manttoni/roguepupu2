#pragma once

#include <ncurses.h>

namespace UI
{
	// print numbers
	void print(const int i);

	// print char*
	void println(const char* chptr);
	void print(const char* chptr);

	// print std::string
	void println(WINDOW *window = stdscr, const std::string& str = "");
	void print(WINDOW *window = stdscr, const std::string& str = "");

	void init_ncurses();
	void start_menu();
	void end_ncurses();
}
