#include <ncurses.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <csignal>
#include <format>
#include "UI.hpp"
#include "CaveGenerator.hpp"
#include "Cave.hpp"

#define KEY_ESCAPE 27

namespace UI
{
	int curs_y(WINDOW *window = stdscr)
	{
		int y, x;
		getyx(window, y, x);
		return y;
	}
	void print(const int i)
	{
		wprintw(stdscr, "%d", i);
	}
	void println(const char* chptr)
	{
		wprintw(stdscr, "%s", chptr);
		waddch(stdscr, '\n');
	}
	void print(const char* chptr)
	{
		wprintw(stdscr, "%s", chptr);
	}
	void print(const std::string& str)
	{
		print(stdscr, str);
	}
	void println(const std::string& str)
	{
		print(stdscr, str);
		move(curs_y() + 1, 0);
	}
	void println(WINDOW *window, const std::string& str)
	{
		wprintw(window, "%s", str.c_str());
		waddch(window, '\n');
	}
	void print(WINDOW *window, const std::string& str)
	{
		wprintw(window, "%s", str.c_str());
	}

	void handle_signal(int sig)
	{
		(void) sig;
		end_ncurses();
	}

	void init_ncurses()
	{
		std::signal(SIGSEGV, handle_signal);
		std::signal(SIGABRT, handle_signal);
		std::signal(SIGFPE, handle_signal);
		std::signal(SIGINT, handle_signal);
		atexit(end_ncurses);
		initscr();
		noecho();
		curs_set(0);
		keypad(stdscr, TRUE);
		start_color();
	}

	void test_cave_generator()
	{
		size_t current_level = 1;
		double smoothness = 0.1;
		size_t margin_percent = 15;
		size_t seed = Random::randint(10000, 99999);
		CaveGenerator cg(LINES, COLS, smoothness, seed, margin_percent);
		refresh();
		int input = 0;
		while (input != KEY_ESCAPE)
		{
			Cave current_cave = cg.get_cave(current_level);
			current_cave.print_cave();
			move(0, 0);
			println(std::format("    UP/DOWN |      level: {:<6}", current_level));
			println(std::format(" LEFT/RIGHT | smoothness: {:0.3f} ", smoothness));
			println(std::format("NPAGE/PPAGE |   margin_%: {:<6}", margin_percent));
			refresh();
			input = getch();
			switch (input)
			{
				case KEY_UP:
					if (current_level > 1)
						current_level--;
					break;
				case KEY_DOWN:
					current_level++;
					break;
				case KEY_LEFT:
					smoothness -= 0.01;
					cg = CaveGenerator(LINES, COLS, smoothness, seed, margin_percent);
					break;
				case KEY_RIGHT:
					smoothness += 0.01;
					cg = CaveGenerator(LINES, COLS, smoothness, seed, margin_percent);
					break;
				case KEY_NPAGE:
					margin_percent--;
					cg = CaveGenerator(LINES, COLS, smoothness, seed, margin_percent);
					break;
				case KEY_PPAGE:
					margin_percent++;
					cg = CaveGenerator(LINES, COLS, smoothness, seed, margin_percent);
					break;
			}
			flushinp();
		}
	}

	void start_menu()
	{
		WINDOW *menu;
		int starty, startx, width = 30, height = 20;
		starty = (LINES - height) / 2;
		startx = (COLS - width) / 2;
		menu = newwin(height, width, starty, startx);
		refresh();

		size_t selected = 0;
		int input = 0;
		std::vector<std::string> choices = {"Test CaveGenerator", "Quit"};
		while (input != KEY_ESCAPE)
		{
			werase(menu);
			wmove(menu, 1, 0);
			for (size_t i = 0; i < choices.size(); ++i)
			{
				if (i == selected)
					wattron(menu, A_REVERSE);
				println(menu, "  " + choices[i]);
				if (i == selected)
					wattroff(menu, A_REVERSE);
			}
			box(menu, 0, 0);
			wrefresh(menu);
			input = getch();
			switch (input)
			{
				case KEY_DOWN:
					selected = selected == choices.size() - 1 ? 0 : 1;
					break;
				case KEY_UP:
					selected = selected == 0 ? 1 : 0;
					break;
				case '\n':
					switch (selected)
					{
						case 0:
							wclear(menu);
							test_cave_generator();
							break;
						case 1:
							return;
						default:
							break;
					}
				default:
					break;
			}
		}
	}

	void end_ncurses()
	{
		endwin();
	}
}
