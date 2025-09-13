#pragma once
#include <string>
#include <ncurses.h>
#include <panel.h>

#define KEY_ESCAPE 27

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

namespace CaveView
{
	void draw_cave();
	void cave_generator();
	extern PANEL* cave_panel;
}
