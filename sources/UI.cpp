#include <bits/chrono.h>  // for duration, duration_cast, operator-, steady_...
#include <curses.h>       // for wprintw, ERR, COLOR_PAIR, curs_set, endwin
#include <locale.h>       // for setlocale, LC_ALL
#include <panel.h>        // for panel_window, PANEL, new_panel
#include <stdio.h>        // for fflush, printf, stdout
#include <any>            // for any
#include <csignal>        // for signal, SIGABRT, SIGFPE, SIGINT, SIGSEGV
#include <cstdlib>        // for atoi, exit
#include <cstring>        // for strchr, strncmp
#include <string>         // for basic_string, string
#include "Color.hpp"      // for Color
#include "ColorPair.hpp"  // for ColorPair
#include "Game.hpp"       // for new_game
#include "Menu.hpp"       // for Menu
#include "MenuBtn.hpp"    // for MenuBtn
#include "MenuElt.hpp"    // for MenuElt
#include "MenuNum.hpp"    // for MenuNum
#include "MenuTxt.hpp"    // for MenuTxt
#include "UI.hpp"         // for UI, KEY_LEFT_CLICK, KEY_RIGHT_CLICK, quit
#include "Utils.hpp"      // for Coord, botleft, height, middle, topright
#include "entt.hpp"       // for unique_ptr, make_unique, vector, allocator

void UI::print_wide(const size_t y, const size_t x, const wchar_t wc)
{
	wchar_t wc_str[2] = { wc, L'\0' };
	mvwaddwstr(panel_window(current_panel), y, x, wc_str);
}
void UI::print_wide(wchar_t wc)
{
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
void UI::print_colors(const char* ptr)
{
	Color c;
	while (*ptr != '\0')
	{
		if (*ptr == '{') // color markup start
		{
			if (strncmp(ptr, "{reset}", 7) == 0)
			{
				//wattron(get_current_window(), COLOR_PAIR(0));
				disable_color_pair(ColorPair(c, Color{}));
				ptr = strchr(ptr, '}') + 1;
				continue;
			}
			const short r = static_cast<short>(atoi(ptr + 1));
			ptr = strchr(ptr, ',') + 1;
			const short g = static_cast<short>(atoi(ptr));
			ptr = strchr(ptr, ',') + 1;
			const short b = static_cast<short>(atoi(ptr));
			ptr = strchr(ptr, '}') + 1;
			c = Color(r, g, b);
			enable_color_pair(ColorPair(c, Color{}));
			continue;
		}
		print(*ptr);
		ptr++;
	}
}
void UI::print(const std::string& str)
{
	if (str.find('{') != std::string::npos)
		print_colors(str.c_str()); // Probably wants to print with colors
	else
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
	wattron(get_current_window(), COLOR_PAIR(pair_id));
}
void UI::disable_color_pair(const ColorPair& color_pair)
{
	short pair_id = is_initialized_color_pair(color_pair);
	if (pair_id == -1)
		pair_id = color_pair.init();
	wattroff(get_current_window(), COLOR_PAIR(pair_id));
}
void UI::reset_colors()
{
	initialized_colors.clear();
	initialized_color_pairs.clear();
}

std::string UI::dialog(const std::vector<std::string>& text, const std::vector<std::string>& options, const Screen::Coord& position, const size_t initial_selection)
{
	// Initialize elements for dialog box
	std::vector<std::unique_ptr<MenuElt>> elements;
	for (auto& t : text)
		elements.push_back(std::make_unique<MenuTxt>(t));
	if (!text.empty() && !options.empty())
		elements.push_back(std::make_unique<MenuTxt>(MenuTxt::HorizontalLine));
	for (auto& option : options)
		elements.push_back(std::make_unique<MenuBtn>(option));

	// Create menu in the middle of screen
	auto menu = Menu(std::move(elements), position);

	// If there are no options, it will just be printed as a message
	if (options.empty())
		menu.set_read_only(true);

	menu.set_selected(initial_selection);

	// Will return an option as a string it was constructed with
	return menu.loop();

}
std::string UI::dialog(const std::string& text, const std::vector<std::string>& options, const Screen::Coord& position, const size_t initial_selection)
{
	return dialog(std::vector<std::string>{text}, options, position, initial_selection);
}

// return input as int for ncurses
// if no delay defined, will wait forever for input
// otherwise will be non-blocking, but waiting for delay times ms
int UI::input(int delay)
{
	flushinp();

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
		set_current_panel(debug.get_panel(), true);
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


