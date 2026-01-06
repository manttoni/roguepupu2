#pragma once
#include <curses.h>       // for chtype, doupdate, endwin, WINDOW, wattroff
#include <panel.h>        // for PANEL, panel_window, top_panel, update_panels
#include <cstdlib>        // for size_t, exit
#include <map>            // for map
#include <string>         // for string, allocator
#include <vector>         // for vector
#include "Color.hpp"      // for Color
#include "ColorPair.hpp"  // for ColorPair
#include "Menu.hpp"       // for Menu
#include "Utils.hpp"      // for middle, Coord
class Cave;
class Cell;
class ColorPair;
#define KEY_ESCAPE 27
#define KEY_LEFT_CLICK 420
#define KEY_RIGHT_CLICK 421

class UI
{
	public:
		static UI& instance()
		{
			static UI inst;
			return inst;
		}

	private:
		static void handle_signal(int sig)
		{
			(void) sig;
			endwin();
			std::exit(sig);
		}

	public:
		enum class Panel
		{
			Game,
			Status,
		};
	private:
		std::map<Panel, PANEL*> panels;
	public:
		void add_panel(const Panel p, PANEL* panel) { panels[p] = panel; }
		PANEL* get_panel(const Panel p) { return panels[p]; }
		std::map<Panel, PANEL*> get_panels() { return panels; }
		void init_panel(Panel id);
		void destroy_panel(Panel id);

	private:
		std::map<Color, short> initialized_colors;
		std::map<ColorPair, short> initialized_color_pairs;
	public:
		// these return -1 if not initialized
		// otherwise return the previously initialized color/pair
		short is_initialized_color(const Color& color) const;
		short is_initialized_color_pair(const ColorPair& color_pair) const;

		void set_initialized_color(const Color& color, const short color_id) {
			initialized_colors[color] = color_id;
		}
		void set_initialized_color_pair(const ColorPair& color_pair, const short color_pair_id) {
			initialized_color_pairs[color_pair] = color_pair_id;
		}
		void reset_colors();
		const std::map<Color, short>& get_initialized_colors() const { return initialized_colors; }
		const std::map<ColorPair, short>& get_initialized_color_pairs() const { return initialized_color_pairs; }

	private:
		Screen::Coord mouse_position;
	public:
		Screen::Coord get_mouse_position() const { return mouse_position; }
		void set_mouse_position(Screen::Coord& mouse_position) { this->mouse_position = mouse_position; }

	private:
		PANEL* current_panel;
		size_t loop_number;
	public:
		void set_current_panel(PANEL* current_panel, const bool make_top = false)
		{
			if (current_panel == nullptr)
				return;
			this->current_panel = current_panel;
			if (make_top == true)
				top_panel(current_panel);
		}
		void set_current_panel(Panel p, const bool make_top = false) {
			this->current_panel = panels[p];
			if (make_top == true)
				top_panel(panels[p]);
		}
		PANEL* get_current_panel() const { return current_panel; }
		WINDOW* get_current_window() const { return panel_window(current_panel); }

		void print_wstr(const std::wstring& wstr);
		void print_wstr(const size_t y, const size_t x, const std::wstring& wstr);
		void print_wide(const size_t y, const size_t x, wchar_t wc);
		void print_wide(wchar_t wc);
		void print_colors(const char* ptr);
		void print(const size_t y, const size_t x, const char ch);
		void print(const char ch);
		void print(const std::string& str);
		void print(const size_t y, const size_t x, const std::string& str);
		void println(const std::string& str = "");


		std::string dialog(const std::vector<std::string>& text, const std::vector<std::string>& options = {}, const Screen::Coord& position = Screen::middle(), const size_t initial_selection = 0);
		std::string dialog(const std::string& text, const std::vector<std::string>& options = {}, const Screen::Coord& position = Screen::middle(), const size_t initial_selection = 0);
		int input(int delay = -1); // wrapper for getch
		Vec2 get_direction(const int key);
		Cell* get_clicked_cell(Cave& cave);
		size_t get_curs_y() const;
		size_t get_curs_x() const;
		void enable_attr(const chtype attr) { wattron(panel_window(current_panel), attr); }
		void disable_attr(const chtype attr) { wattroff(panel_window(current_panel), attr); }
		void enable_color_pair(const ColorPair& color_pair);
		void disable_color_pair(const ColorPair& color_pair);
		void update() { update_panels(); doupdate(); }
		void init_panels();
		void destroy_panels();
		void init();
		void end();
		void resize_terminal();
};

