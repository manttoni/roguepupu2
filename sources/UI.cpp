#include <ncursesw/ncurses.h>
#include <string>
#include <vector>
#include <memory>
#include <any>
#include <utility>
#include <climits>
#include <chrono>
#include "Utils.hpp"
#include "UI.hpp"
#include "Menu.hpp"
#include "MenuElt.hpp"
#include "MenuBtn.hpp"
#include "MenuNum.hpp"
#include "MenuTxt.hpp"
#include "Utils.hpp"
#include "CaveGenerator.hpp"
#include "Game.hpp"



void UI::print_wide(const size_t y, const size_t x, const wchar_t wc)
{
	assert(wcwidth(wc) == 1);
	wchar_t wc_str[2] = { wc, L'\0' };
	mvwaddwstr(panel_window(current_panel), y, x, wc_str);
}
void UI::print_wide(wchar_t wc)
{
	assert(wcwidth(wc) == 1);
	wchar_t wc_str[2] = { wc, L'\0' };
	waddwstr(panel_window(current_panel), wc_str);
}
void UI::print(const size_t y, const size_t x, const char ch)
{
	mvwaddch(panel_window(current_panel), y, x, ch);
}
void UI::print(const char ch)
{
	wprintw(panel_window(current_panel), "%c", ch);
}
void UI::print(const std::string& str)
{
	wprintw(panel_window(current_panel), "%s", str.c_str());
}
void UI::print(const size_t y, const size_t x, const std::string& str)
{
	mvwprintw(panel_window(current_panel), y, x, "%s\n", str.c_str());
}
void UI::println(const std::string& str)
{
	wprintw(panel_window(current_panel), "%s\n", str.c_str());
}
size_t UI::get_curs_y() const
{
	assert(current_panel != nullptr);
	int y, x;
	getyx(panel_window(current_panel), y, x);
	(void) x;
	return y;
}
size_t UI::get_curs_x() const
{
	assert(current_panel != nullptr);
	int y, x;
	getyx(panel_window(current_panel), y, x);
	(void) y;
	return x;
}
short UI::is_initialized_color(const Color& color) const
{
	for (const auto& [initialized_color, id] : initialized_colors)
		if (initialized_color == color)
			return id;
	return -1;
}
short UI::is_initialized_color_pair(const ColorPair& color_pair) const
{
	for (const auto& [initialized_color_pair, id] : initialized_color_pairs)
		if (initialized_color_pair == color_pair)
			return id;
	return -1;
}
void UI::enable_color_pair(const ColorPair& color_pair)
{
	short pair_id = is_initialized_color_pair(color_pair);
	if (pair_id == -1)
		pair_id = color_pair.init();
	wattron(UI::instance().get_current_window(), COLOR_PAIR(pair_id));
}
void UI::reset_colors()
{
	initialized_colors.clear();
	initialized_color_pairs.clear();
}

std::string UI::dialog(const std::string& text, const std::vector<std::string>& options)
{
	// Initialize elements for dialog box
	std::vector<std::unique_ptr<MenuElt>> elements;
	elements.push_back(std::make_unique<MenuTxt>(text));
	for (auto& option : options)
		elements.push_back(std::make_unique<MenuBtn>(option));

	// Create menu in the middle of screen
	auto menu = Menu(std::move(elements), Screen::middle());

	// If there are no options, it will just be printed as a message
	if (options.empty())
		menu.set_read_only(true);

	// Will return an option as a string it was constructed with
	return menu.loop();
}

// return input as int for ncurses
// if no delay defined, will wait forever for input
// otherwise will be non-blocking, but waiting for delay times ms
int UI::input(int delay)
{
	// -1 is default value for delay
	// getch() will be blocking by default
	if (delay == -1)
		return getch();

	// 0 is not enough time to get any input,
	// and -1 is only legit negative
	if (delay <= 0)
		return ERR;

	// ERR means no input within time limit
	int key = ERR;

	// Measure time
	auto start = std::chrono::steady_clock::now();
	auto now = start;

	// Make getch() non-blocking
	timeout(0);

	// This will loop for delay ms. It will break when it gets a valid input.
	while (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() < delay)
	{
		now = std::chrono::steady_clock::now();
		key = getch();

		// Check for mouse events
		MEVENT mouse_event;
		if (key == KEY_MOUSE && getmouse(&mouse_event) == OK)
		{
			if (mouse_event.bstate & BUTTON1_CLICKED)
				key = KEY_LEFT_CLICK;
			else if (mouse_event.bstate & BUTTON2_CLICKED)
				key = KEY_RIGHT_CLICK;
			else // mouse movement triggered the event
			{
				// save new mouse position
				mouse_position = {
					static_cast<size_t>(mouse_event.y),
					static_cast<size_t>(mouse_event.x)
				};
				key = ERR; // make it known that there was no input
				continue; // go back to beginning and wait for real input
			}
		}

		if (key == ERR) // no input
			continue;
		break; // got input
	}

	// make getch() blocking (default)
	timeout(-1);

	// Can press F1 anywhere to toggle debug
	if (key == KEY_F(1))
		UI::instance().toggle_show_debug();

	// update debug window
	if (show_debug == true)
	{
		auto& debug = menus.at("debug");
		debug.set_value("Keypress", key);
		debug.set_value("Colors", initialized_colors.size());
		debug.set_value("Color pairs", initialized_color_pairs.size());
		debug.set_value("Mouse y", mouse_position.y);
		debug.set_value("Mouse x", mouse_position.x);
		debug.loop();
	}

	return key;
}

void UI::init_menus()
{
	// Main menu
	std::vector<std::unique_ptr<MenuElt>> elements;
	elements.push_back(std::make_unique<MenuBtn>("New game", new_game));
	elements.push_back(std::make_unique<MenuBtn>("Quit", quit));
	menus["main"] = Menu(std::move(elements), Screen::middle());

	// test
	PANEL* main_panel = menus["main"].get_panel();
	assert(main_panel != nullptr);
	WINDOW* main_window = panel_window(main_panel);
	assert(main_window != nullptr);
	int height, width;
	getmaxyx(main_window, height, width);
	assert(menus["main"].get_width() == static_cast<size_t>(width));
	assert(menus["main"].get_height() == static_cast<size_t>(height));

	// Debug window
	std::vector<std::unique_ptr<MenuElt>> debug_elements;
	debug_elements.push_back(std::make_unique<MenuNum<size_t>>("Colors"));
	debug_elements.push_back(std::make_unique<MenuNum<size_t>>("Color pairs"));
	debug_elements.push_back(std::make_unique<MenuNum<int>>("Keypress"));
	debug_elements.push_back(std::make_unique<MenuNum<size_t>>("Mouse y"));
	debug_elements.push_back(std::make_unique<MenuNum<size_t>>("Mouse x"));
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
	loop_number = 0;
	setlocale(LC_ALL, "");
	initscr();
	start_color();
	init_menus();

	// signals should reset original terminal mode
	std::signal(SIGSEGV, handle_signal);
	std::signal(SIGABRT, handle_signal);
	std::signal(SIGFPE, handle_signal);
	std::signal(SIGINT, handle_signal);

	// keyboard input
	noecho();
	curs_set(0);
	keypad(stdscr, TRUE);
	set_escdelay(25);

	// mouse input
	mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);
	printf("\033[?1003h");	// mouse movement will trigger KEY_MOUSE events
	fflush(stdout);				// to know current cursor location

	Log::log("UI init finished");
}

void UI::end()
{
	// this will restore normal terminal mode
	endwin();

	// reset mouse mode
	printf("\033[?1003l");
	fflush(stdout);

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
		UI::instance().reset_colors();
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
		current_cave->reset_lights();
		const auto& cells = current_cave->get_cells();

		wmove(cave_window, 0, 0);
		for (size_t i = 0; i < cells.size(); ++i)
		{
			const Cell& cell = cells[i];
			const auto& color_pair = cell.get_color_pair();
			UI::instance().enable_color_pair(color_pair);
			UI::instance().print(cell.get_symbol());
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
