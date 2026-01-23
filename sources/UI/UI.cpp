#include <bits/chrono.h>  // for duration, duration_cast, operator-, steady_...
#include <ncurses.h>       // for wprintw, ERR, COLOR_PAIR, curs_set, endwin
#include <locale.h>       // for setlocale, LC_ALL
#include <panel.h>        // for panel_window, PANEL, new_panel
#include <stdio.h>        // for fflush, printf, stdout
#include <any>            // for any
#include <csignal>        // for signal, SIGABRT, SIGFPE, SIGINT, SIGSEGV
#include <cstdlib>        // for atoi, exit
#include <cstring>        // for strchr, strncmp
#include <string>         // for basic_string, string
#include "utils/ECS.hpp"
#include "domain/Color.hpp"      // for Color
#include "domain/ColorPair.hpp"  // for ColorPair
#include "UI/Menu.hpp"       // for Menu
#include "UI/MenuBtn.hpp"    // for MenuBtn
#include "UI/MenuElt.hpp"    // for MenuElt
#include "UI/MenuTxt.hpp"    // for MenuTxt
#include "UI/UI.hpp"         // for UI, KEY_LEFT_CLICK, KEY_RIGHT_CLICK, quit
#include "utils/Vec2.hpp"
#include "infrastructure/GameState.hpp"

void UI::print_wstr(const size_t y, const size_t x, const std::wstring& wstr)
{
	mvwaddwstr(panel_window(current_panel), y, x, wstr.c_str());
}
void UI::print_wstr(const std::wstring& wstr)
{
	waddwstr(panel_window(current_panel), wstr.c_str());
}
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
		if (*ptr == '{') // color markup
		{
			if (strncmp(ptr, "{reset}", 7) == 0) // end
			{
				disable_color_pair(ColorPair(c, Color{}));
				ptr = strchr(ptr, '}') + 1;
				continue;
			} // start
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
	else if (current_panel != nullptr)
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

std::string UI::dialog(const std::vector<std::string>& text, const std::vector<std::string>& options, const Vec2& position, const size_t initial_selection)
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
std::string UI::dialog(const std::string& text, const std::vector<std::string>& options, const Vec2& position, const size_t initial_selection)
{
	return dialog(std::vector<std::string>{text}, options, position, initial_selection);
}

void UI::resize_terminal()
{	// Some screen elements naturally resize, some don't
	destroy_panel(Panel::Status);
	init_panel(Panel::Status);
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
					mouse_event.y,
					mouse_event.x
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

	if (key == KEY_RESIZE)
		resize_terminal();

	return key;
}

Vec2 UI::get_direction(const int key)
{
	switch (key)
	{
		case KEY_DOWN:	case '2':	return {1, 0};
		case KEY_END:	case '1':	return {1, -1};
		case KEY_HOME:	case '7':	return {-1, -1};
		case KEY_LEFT:	case '4':	return {0, -1};
		case KEY_NPAGE:	case '3':	return {1, 1};
		case KEY_PPAGE:	case '9':	return {-1, 1};
		case KEY_RIGHT:	case '6':	return {0, 1};
		case KEY_UP:	case '8':	return {-1, 0};
		default:		return {0, 0}; // no direction
	}
}

Vec2 UI::get_window_dimensions(const WINDOW* const window) const
{
	Vec2 dimensions;
	getmaxyx(window, dimensions.y, dimensions.x);
	return dimensions;
}

Vec2 UI::get_window_start(const WINDOW* const window) const
{
	Vec2 start;
	getbegyx(window, start.y, start.x);
	return start;
}

Position UI::get_clicked_position(const entt::registry& registry)
{
	PANEL* panel = get_panel(UI::Panel::Game);
	WINDOW* window = panel_window(panel);
	const auto player = registry.ctx().get<GameState>().player;
	const auto& player_position = registry.get<Position>(player);
	const auto& cave = ECS::get_cave(registry, player_position.cave_idx);

	const Vec2 window_dimensions = get_window_dimensions(window);
	const Vec2 window_start = get_window_start(window);
	const Vec2 mouse_position = get_mouse_position();
	const Vec2 mouse_relative = mouse_position - window_start;
	const Vec2 window_center = window_dimensions / 2;
	const Vec2 offset = mouse_relative - window_center;
	const Vec2 player_coords(player_position.cell_idx, cave.get_size());
	const Vec2 click_coords = player_coords + offset;
	if (click_coords.y < 0 ||
		click_coords.x < 0 ||
		click_coords.y >= static_cast<int>(cave.get_size()) ||
		click_coords.y >= static_cast<int>(cave.get_size()))
		return Position::invalid_position();
	const size_t click_idx = click_coords.to_idx(cave.get_size());
	return Position(click_idx, cave.get_idx());
}

void UI::init_panel(const Panel id)
{
	PANEL* panel = nullptr;
	switch (id)
	{
		case Panel::Game:
			panel = new_panel(newwin(Screen::height(), Screen::width(), 0, 0));
			break;
		case Panel::Status:
			panel = new_panel(newwin(7, 27, Screen::height() - 7, Screen::width() - 27));
			break;
	}
	panels[id] = panel;
}

void UI::init_panels()
{
	init_panel(Panel::Game);
	init_panel(Panel::Status);
}

void UI::destroy_panel(Panel id)
{
	PANEL* panel = panels[id];
	WINDOW* window = panel_window(panel);
	del_panel(panel);
	delwin(window);
}

void UI::destroy_panels()
{
	auto panels = UI::instance().get_panels();
	for (auto [id, panel] : panels)
		destroy_panel(id);
}

void UI::init()
{
	setlocale(LC_ALL, "");
	initscr();
	start_color();
	init_panels();

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
	Log::log("UI ended");
	destroy_panels();

	// this will restore normal terminal mode
	endwin();

	// reset mouse mode
	printf("\033[?1003l");
	fflush(stdout);

	std::exit(0);
}


