#include <ncurses.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <csignal>
#include <format>
#include <memory>
#include <any>
#include <utility>
#include "UI.hpp"
#include "CaveGenerator.hpp"
#include "Cave.hpp"
#include "Menu.hpp"
#include "MenuElt.hpp"
#include "MenuNum.hpp"
#include "MenuBtn.hpp"

namespace UI
{
	int curs_y(WINDOW *window = stdscr)
	{
		int y, x;
		getyx(window, y, x);
		return y;
	}
	void print(WINDOW* window, int i)
	{
		wprintw(window, "%d", i);
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
		move(curs_y(window) + 1, 0);
	}
	void print(WINDOW *window, const std::string& str)
	{
		wprintw(window, "%s", str.c_str());
	}

	void handle_signal(int sig)
	{
		(void) sig;
		end_ncurses();
		std::exit(sig);
	}

	void init_ncurses()
	{
		initscr();
		std::signal(SIGSEGV, handle_signal);
		std::signal(SIGABRT, handle_signal);
		std::signal(SIGFPE, handle_signal);
		std::signal(SIGINT, handle_signal);
		atexit(end_ncurses);
		noecho();
		curs_set(0);
		keypad(stdscr, TRUE);
		start_color();
		set_escdelay(25);
	}
	void end_ncurses()
	{
		endwin();
		std::exit(0);
	}
}

namespace CaveView
{
	Menu settings;
	PANEL* cave_panel;

	void draw_cave()
	{
		WINDOW* cave_window = panel_window(cave_panel);
		static CaveGenerator cg;

		auto current =
			std::make_tuple(
				std::any_cast<double>(settings.get_value("Frequency")),
				std::any_cast<int>(settings.get_value("Seed")),
				std::any_cast<int>(settings.get_value("Margin %")),
				std::any_cast<int>(settings.get_value("Octaves")));
		static auto prev = std::make_tuple(-1.0, -1, -1, -1);

		if (current != prev)
		{
			prev = current;
			cg = CaveGenerator(
					Screen::height(),
					Screen::width(),
					std::get<0>(current),
					std::get<1>(current),
					std::get<2>(current),
					std::get<3>(current));
		}
		int level = std::any_cast<int>(settings.get_value("Level"));
		Cave c = cg.get_cave(level);
		const std::vector<Cell> cells = c.get_cells();

		wmove(cave_window, 0, 0);
		for (size_t i = 0; i < cells.size(); ++i)
		{
			int density = static_cast<int>(std::round(cells[i].get_density()));
			wattron(cave_window, COLOR_PAIR(density));
			UI::print(cave_window, density);
			wattroff(cave_window, COLOR_PAIR(density));
		}
		update_panels();
		doupdate();
	}

	void cave_generator()
	{
		std::vector<std::unique_ptr<MenuElt>> elements;
		elements.push_back(std::make_unique<MenuNum<int>>(
				"Seed",
				std::pair<int, int>(10000, 99999),
				Random::randint(10000, 99999)));
		elements.push_back(std::make_unique<MenuNum<int>>(
				"Level",
				std::pair<int, int>{1, INT_MAX}, 1));
		elements.push_back(std::make_unique<MenuNum<double>>(
				"Frequency",
				std::pair<double, double>{0, 1},
				0.1,
				0.01));
		elements.push_back(std::make_unique<MenuNum<int>>(
				"Octaves",
				std::pair<int, int>{1, 16},
				8));
		elements.push_back(std::make_unique<MenuNum<int>>(
				"Margin %",
				std::pair<int, int>{0, 100},
				15));
		settings = Menu(std::move(elements), {0, 0}, draw_cave);
		cave_panel = new_panel(newwin(Screen::height(), Screen::width(), 0, 0));

		// Colors for densities
		for (size_t i = 0; i <= 9; ++i)
		{
			init_color(i + 10, i * 100, i * 100, i * 100);
			init_pair(i, i + 10, COLOR_BLACK);
		}

		// move cave_panel on top, that's where cave will be drawn. Settings menu will put itself on top
		top_panel(cave_panel);
		settings.show();
	}
} // CaveView
