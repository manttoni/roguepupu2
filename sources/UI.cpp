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

void UI::print(const std::string& str)
{
	wprintw(panel_window(panel), "%s", str.c_str());
}
void UI::println(const std::string& str)
{
	wprintw(panel_window(panel), "%s\n", str.c_str());
}
size_t UI::get_curs_y() const
{
	int y, x;
	getyx(panel_window(panel), y, x);
	(void) x;
	return y;
}
size_t UI::get_curs_x() const
{
	int y, x;
	getyx(panel_window(panel), y, x);
	(void) y;
	return x;
}
short UI::get_next_color_id()
{
	static short id = 10;
	return ++id;
}

namespace CaveView
{
	Menu settings;
	PANEL* cave_panel = nullptr;

	void draw_cave()
	{
		WINDOW* cave_window = panel_window(cave_panel);
		static CaveGenerator cg;
		UI::instance().set_panel(cave_panel);

		auto current =
			std::make_tuple(
				std::any_cast<double>(settings.get_value("Frequency")),
				std::any_cast<int>(settings.get_value("Seed")),
				std::any_cast<int>(settings.get_value("Margin %")),
				std::any_cast<int>(settings.get_value("Octaves")),
				std::any_cast<double>(settings.get_value("A")),
				std::any_cast<double>(settings.get_value("B")),
				std::any_cast<double>(settings.get_value("C")));
		static auto prev = std::make_tuple(-1.0, -1, -1, -1, -1.0, -1.0, -1.0);

		if (current != prev)
		{
			prev = current;
			cg = CaveGenerator(
					Screen::height(),
					Screen::width(),
					std::get<0>(current),
					std::get<1>(current),
					std::get<2>(current),
					std::get<3>(current),
					std::get<4>(current),
					std::get<5>(current),
					std::get<6>(current));
		}
		int level = std::any_cast<int>(settings.get_value("Level"));
		Cave c = cg.get_cave(level);
		const auto& cells = c.get_cells();

		// print densities and tunnels
		wmove(cave_window, 0, 0);
		for (size_t i = 0; i < cells.size(); ++i)
		{
			int density = static_cast<int>(std::round(cells[i].get_density()));
			if (density > 0 && density <= 9)
			{
				UI::instance().enable(COLOR_PAIR(density));
				UI::instance().print(std::to_string(density));
				UI::instance().disable(COLOR_PAIR(density));
			}
			else
				UI::instance().print(" ");
		}
/*
		// print water things
		// start printing in blue
		init_pair(42, COLOR_BLUE, COLOR_BLACK);
		wattron(cave_window, COLOR_PAIR(42));

		// draw path of water
		auto path = c.find_path(c.get_source(), c.get_sink());
		assert(!path.empty());
		for (size_t i = 0; i < path.size(); ++i)
		{
			size_t idx = path[i];
			size_t idx_y = idx / c.get_width();
			size_t idx_x = idx % c.get_width();
			if (i == 0)
				mvwaddch(cave_window, idx_y, idx_x, 'v');
			else if (i == path.size() - 1)
				mvwaddch(cave_window, idx_y, idx_x, '^');
			else
				mvwaddch(cave_window, idx_y, idx_x, '~');
		}

		// stop printing in blue
		wattroff(cave_window, COLOR_PAIR(COLOR_BLUE));*/
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
		elements.push_back(std::make_unique<MenuNum<double>>(
				"A",
				std::pair<double, double>{0.1,10},
				2, 0.1));
		elements.push_back(std::make_unique<MenuNum<double>>(
				"B",
				std::pair<double, double>{0.1, 10},
				0.1, 0.1));
		elements.push_back(std::make_unique<MenuNum<double>>(
				"C",
				std::pair<double, double>{0.1, 10},
				0.1, 0.1));
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
