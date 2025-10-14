#pragma once
#include <string>
#include <ncurses.h>
#include <panel.h>
#include <csignal>
#include <map>
#include <memory>
#include <any>
#include <utility>
#include "ColorPair.hpp"
#include "Color.hpp"
#include "Cell.hpp"
#include "Menu.hpp"

#define KEY_ESCAPE 27

class UI
{
	public:
		enum class Mode
		{
			CAVE_VIEW,
			GAME,
			MAIN,
		};
	private:
		Mode mode;
	public:
		void set_mode(const Mode mode) { this->mode = mode; }
		Mode get_mode() const { return mode; }

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
			GAME,
			LOG,
		};
	private:
		std::map<Panel, PANEL*> panels;
	public:
		void add_panel(const Panel p, PANEL* panel) { panels[p] = panel; }
		PANEL* get_panel(const Panel p) { return panels[p]; }

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

	private:
		bool show_debug = false;
	public:
		void set_show_debug(const bool value) { this->show_debug = value; }
		void toggle_show_debug() { show_debug = !show_debug; }


	private:
		PANEL* current_panel;
		std::map<std::string, Menu> menus;
		size_t ln;
	public:
		Menu& get_menu(const std::string& name) { return menus.at(name); }
		void set_current_panel(PANEL* current_panel) { this->current_panel = current_panel; }
		PANEL* get_current_panel() const { return current_panel; }
		WINDOW* get_current_window() const { return panel_window(current_panel); }
		size_t loop_number() const { return ln; }

		void print_wide(const size_t y, const size_t x, wchar_t wc);
		void print_wide(wchar_t wc);
		void print(const size_t y, const size_t x, const char ch);
		void print(const char ch);
		void print(const std::string& str);
		void println(const std::string& str);

		int input(int delay = -1); // wrapper for getch

		size_t get_curs_y() const;
		size_t get_curs_x() const;
		void enable_attr(const chtype attr) { wattron(panel_window(current_panel), attr); }
		void disable_attr(const chtype attr) { wattroff(panel_window(current_panel), attr); }
		void enable_color_pair(const ColorPair& color_pair);
		void update() { update_panels(); doupdate(); }
		void init_menus();
		void init();
		void end();
};

static inline void quit()
{
	UI::instance().end();
}

namespace CaveView
{
	void draw_cave();
	void cave_generator();
	extern PANEL* cave_panel;
	extern Cave* current_cave;
	extern Menu settings;
	void show_cell_info(const Cell& cell);
}
