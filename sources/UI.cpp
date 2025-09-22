#include <ncurses.h>
#include <string>
#include <vector>
#include <memory>
#include <any>
#include <utility>
#include <climits>
#include "Utils.hpp"
#include "UI.hpp"
#include "Menu.hpp"
#include "MenuElt.hpp"
#include "MenuBtn.hpp"
#include "MenuNum.hpp"
#include "Utils.hpp"
#include "CaveGenerator.hpp"
#include "Game.hpp"

void UI::print(const char ch)
{
	wprintw(panel_window(current_panel), "%c", ch);
	Log::log("Printed char: " + ch);
}
void UI::print(const std::string& str)
{
	wprintw(panel_window(current_panel), "%s", str.c_str());
}
void UI::println(const std::string& str)
{
	wprintw(panel_window(current_panel), "%s\n", str.c_str());
}
/*size_t UI::get_curs_y() const
{
	assert(current_panel != nullptr);
	int y, x;
	getyx(panel_window(current_panel), y, x);
	(void) x;
	return y;
}*/
size_t UI::get_curs_x() const
{
	assert(current_panel != nullptr);
	int y, x;
	getyx(panel_window(current_panel), y, x);
	(void) y;
	return x;
}
short UI::get_next_color_id()
{
	static short id = 8;
	assert(COLORS != 0);
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
	assert(COLOR_PAIRS != 0);
	if (id == COLOR_PAIRS)
	{
		Log::log("Color pair ids all used");
		id = 1;
	}
	return id++;
}
short UI::color_initialized(const short r, const short g, const short b)
{
	for (const auto& [id, color] : initialized_colors)
	{
		if (color.get_r() == r && color.get_g() == g && color.get_b() == b)
			return id;
	}
	return -1;
}
short UI::color_pair_initialized(const Color& fg, const Color& bg)
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
short UI::add_color(short r, short g, short b)
{
	r = Math::clamp(r, static_cast<short>(0), static_cast<short>(1000));
	g = Math::clamp(g, static_cast<short>(0), static_cast<short>(1000));
	b = Math::clamp(b, static_cast<short>(0), static_cast<short>(1000));
	short color_id = color_initialized(r, g, b);
	if (color_id != -1)
		return color_id;
	color_id = instance().get_next_color_id();
	init_color(color_id, r, g, b);
	initialized_colors[color_id] = Color(color_id, r, g, b);
	menus["debug"].set_value("Colors", initialized_colors.size());
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
	pair_id = instance().get_next_color_pair_id();
	init_pair(pair_id, fg_id, bg_id);
	initialized_color_pairs[pair_id] = ColorPair(pair_id, fg, bg);
	menus["debug"].set_value("Color pairs", initialized_color_pairs.size());
	Log::log("Color pair initialized with id(" + std::to_string(pair_id) + "): " + std::to_string(fg_id) + ", " + std::to_string(bg_id));
	return pair_id;
}

// if read_only is false, loop() will be infinite if called here
int UI::input()
{
	Menu& debug = menus.at("debug");
	debug.loop();

	instance().update();
	flushinp();
	int key = getch();
	ln++; // increase main loop number
	debug.set_value("Keypress", key);

	MEVENT event;
	if (key == KEY_MOUSE && getmouse(&event) == OK)
	{
		debug.set_value("Mouse y", event.y);
		debug.set_value("Mouse x", event.x);

		// If playing, clicks will move and interact
		// ...

		// If viewing cave, print cell info on click
		if (UI::instance().get_mode() == UI::Mode::CAVE_VIEW &&
			CaveView::current_cave != nullptr &&
			event.bstate & BUTTON1_CLICKED)
		{
			assert(static_cast<size_t>(event.y) <= Screen::height());
			assert(static_cast<size_t>(event.x) <= Screen::width());

			size_t mouse_idx = event.y * Screen::width() + event.x;
			const Cave* cave = CaveView::current_cave;
			assert(mouse_idx < cave->get_size());
			const auto& cells = cave->get_cells();
			const Cell& selected_cell = cells[mouse_idx];
			CaveView::show_cell_info(selected_cell);
		}
	}
	return key;
}
void UI::init_colors()
{
	start_color();

	// Colors
	WHITE = add_color(1000, 1000, 1000);
	BLACK = add_color(0, 0, 0);
	BLUE = add_color(0, 0, 500);
	LIGHT_BLUE = add_color(0, 0, 50);
	MEDIUM_BLUE = add_color(0, 0, 500);
	ORANGE = add_color(1000, 666, 0);

	// Pairs
	GLOWING_FUNGUS = add_color_pair(MEDIUM_BLUE, BLACK);
	WOODY_FUNGUS = add_color_pair(ORANGE, BLACK);
	DEFAULT = add_color_pair(WHITE, BLACK);
}

void UI::init_menus()
{
	// Main menu
	std::vector<std::unique_ptr<MenuElt>> elements;
	elements.push_back(std::make_unique<MenuBtn>("Start game", start_game));
	elements.push_back(std::make_unique<MenuBtn>("CaveGenerator", CaveView::cave_generator));
	elements.push_back(std::make_unique<MenuBtn>("Quit", quit));
	menus["main"] = Menu(std::move(elements), Screen::middle());

	// Debug window
	std::vector<std::unique_ptr<MenuElt>> debug_elements;
	debug_elements.push_back(std::make_unique<MenuNum<size_t>>("Colors"));
	debug_elements.push_back(std::make_unique<MenuNum<size_t>>("Color pairs"));
	debug_elements.push_back(std::make_unique<MenuNum<int>>("Keypress"));
	debug_elements.push_back(std::make_unique<MenuNum<int>>("Mouse y"));
	debug_elements.push_back(std::make_unique<MenuNum<int>>("Mouse x"));
	debug_elements.push_back(std::make_unique<MenuNum<size_t>>("player_y"));
	debug_elements.push_back(std::make_unique<MenuNum<size_t>>("player_x"));
	menus["debug"] = Menu(std::move(debug_elements), Screen::botleft());
	menus["debug"].set_read_only(true);

	// Cell info
	std::vector<std::unique_ptr<MenuElt>> cell_info_elements;
	cell_info_elements.push_back(std::make_unique<MenuNum<size_t>>("Index"));
	cell_info_elements.push_back(std::make_unique<MenuNum<double>>("Density", std::pair<double, double>(0.0, 9.0)));
	cell_info_elements.push_back(std::make_unique<MenuNum<int>>("Color pair"));
	cell_info_elements.push_back(std::make_unique<MenuNum<size_t>>("Glow stacks"));
	cell_info_elements.push_back(std::make_unique<MenuNum<size_t>>("Entities"));
	menus["cell_info"] = Menu(std::move(cell_info_elements), Screen::topright());
	menus["cell_info"].set_read_only(true);
}
void UI::init()
{
	ln = 0;
	initscr();
	Log::log("initscr() width:" + std::to_string(Screen::width()) + " height:" + std::to_string(Screen::height()));
	init_menus();
	Log::log("Menus initialized");
	init_colors();
	Log::log("Colors initialized");
	std::signal(SIGSEGV, handle_signal);
	std::signal(SIGABRT, handle_signal);
	std::signal(SIGFPE, handle_signal);
	std::signal(SIGINT, handle_signal);
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	set_escdelay(25);
	mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);
	Log::log("UI init finished");
}

void UI::end()
{
	endwin();
	std::exit(0);
}



namespace CaveView
{
	Menu settings;
	PANEL* cave_panel = nullptr;
	Cave* current_cave = nullptr;

	void show_cell_info(const Cell& cell)
	{
		Menu& cell_info = UI::instance().get_menu("cell_info");
		cell_info.set_value("Index", cell.get_idx());
		cell_info.set_value("Density", cell.get_density());
		cell_info.set_value("Color pair", static_cast<int>(cell.get_color_pair_id()));
		size_t stacks = 0;
		for (const auto& [color_id, stack_amount] : cell.get_lights())
			stacks += stack_amount;
		cell_info.set_value("Glow stacks", stacks);
		cell_info.set_value("Entities", cell.get_entities().size());
		cell_info.loop();
	}

	void draw_cave()
	{
		WINDOW* cave_window = panel_window(cave_panel);
		static CaveGenerator cg;
		UI::instance().set_current_panel(cave_panel);

		auto current =
			std::make_tuple(
				std::any_cast<double>(settings.get_value("Frequency")),
				std::any_cast<int>(settings.get_value("Seed")),
				std::any_cast<int>(settings.get_value("Margin %")),
				std::any_cast<int>(settings.get_value("Octaves")),
				std::any_cast<double>(settings.get_value("A")),
				std::any_cast<double>(settings.get_value("B")),
				std::any_cast<double>(settings.get_value("C")),
				std::any_cast<double>(settings.get_value("fungus spawn chance")));
		static auto prev = std::make_tuple(-1.0, -1, -1, -1, -1.0, -1.0, -1.0, 0.0);

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
					std::get<6>(current),
					std::get<7>(current));
		}

		int level = std::any_cast<int>(settings.get_value("Level"));
		current_cave = &cg.get_cave(level);
		current_cave->reset_effects();
		const auto& cells = current_cave->get_cells();

		wmove(cave_window, 0, 0);
		for (size_t i = 0; i < cells.size(); ++i)
		{
			const Cell& cell = cells[i];
			UI::instance().enable_attr(COLOR_PAIR(cell.get_color_pair_id()));
			UI::instance().print(cell.get_char());
			UI::instance().disable_attr(COLOR_PAIR(cell.get_color_pair_id()));
		}
	}

	void cave_generator()
	{
		UI::instance().set_mode(UI::Mode::CAVE_VIEW);
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
				std::pair<int, int>{0, 0},
				0));
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
		elements.push_back(std::make_unique<MenuNum<double>>(
				"fungus spawn chance",
				std::pair<double, double>{0, 1},
				0.1, 0.01));
		settings = Menu(std::move(elements), {0, 0}, draw_cave);
		cave_panel = new_panel(newwin(Screen::height(), Screen::width(), 0, 0));
		top_panel(cave_panel);
		settings.loop();
		UI::instance().set_mode(UI::Mode::MAIN);
	}
} // CaveView
