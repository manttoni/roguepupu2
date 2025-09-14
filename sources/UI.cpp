#include <ncurses.h>
#include <string>
#include <vector>
#include <memory>
#include <any>
#include <utility>
#include "Utils.hpp"
#include "UI.hpp"
#include "Menu.hpp"
#include "MenuElt.hpp"
#include "MenuBtn.hpp"
#include "MenuNum.hpp"
#include "Utils.hpp"
#include "CaveGenerator.hpp"

void UI::print(const char ch)
{
	wprintw(panel_window(panel), "%c", ch);
}
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
	static short id = 8;
	if (id == COLORS)
	{
		Log::log("Color ids all used");
		id = 8;
	}
	return id++;
}
short UI::get_next_color_pair_id()
{
	static short id = 1;
	if (id == COLOR_PAIRS)
	{
		Log::log("Color pair ids all used");
		id = 1;
	}
	return id++;
}
int UI::color_initialized(const short r, const short g, const short b) const
{
	for (const auto& [id, color] : initialized_colors)
	{
		if (color.get_r() == r && color.get_g() == g && color.get_b() == b)
			return id;
	}
	return -1;
}
int UI::color_pair_initialized(const Color& fg, const Color& bg) const
{
	for (const auto& [id, color_pair] : initialized_color_pairs)
	{
		if (color_pair.get_fg() == fg && color_pair.get_bg() == bg)
			return id;
	}
	return -1;
}
// initializes color if not yet initialized
// returns id of the color
short UI::add_color(const short r, const short g, const short b)
{
	short color_id = color_initialized(r, g, b);
	if (color_id != -1)
		return color_id;
	color_id = get_next_color_id();
	init_color(color_id, r, g, b);
	initialized_colors[color_id] = Color(color_id, r, g, b);
	Log::log("Color initialized with id(" + std::to_string(color_id) + "): " + std::to_string(r) + " " + std::to_string(g) + " " + std::to_string(b));
	return color_id;
}
short UI::add_color_pair(const short fg_id, const short bg_id)
{
	// doesnt check if id is initialized
	const Color& fg = initialized_colors[fg_id];
	const Color& bg = initialized_colors[bg_id];
	short pair_id = color_pair_initialized(fg, bg);
	if (pair_id != -1)
		return pair_id;
	pair_id = get_next_color_pair_id();
	init_pair(pair_id, fg_id, bg_id);
	initialized_color_pairs[pair_id] = ColorPair(pair_id, fg, bg);
	Log::log("Color pair initialized with id(" + std::to_string(pair_id) + "): " + std::to_string(fg_id) + ", " + std::to_string(bg_id));
	return pair_id;
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

		wmove(cave_window, 0, 0);
		for (size_t i = 0; i < cells.size(); ++i)
		{
			const Cell& cell = cells[i];
			UI::instance().enable(COLOR_PAIR(cell.get_color_pair_id()));
			UI::instance().print(cell.get_char());
			UI::instance().disable(COLOR_PAIR(cell.get_color_pair_id()));
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
		settings.show();
	}
} // CaveView
