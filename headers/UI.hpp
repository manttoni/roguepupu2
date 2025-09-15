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
{	public:
		static UI& instance()
		{
			static UI inst;
			return inst;
		}
	public:
		short add_color(const short r, const short g, const short b);
		short add_color_pair(const short fg_id, const short bg_id);
		static inline short WHITE;
		static inline short BLACK;
		static inline short BLUE;
		static inline short LIGHT_BLUE;
		static inline short MEDIUM_BLUE;
		static inline short GLOWING_FUNGUS;
		static inline short DEFAULT;

	private:
		static void handle_signal(int sig)
		{
			(void) sig;
			endwin();
			std::exit(sig);
		}

	private:
		PANEL* panel;
		std::map<short, Color> initialized_colors;
		std::map<short, ColorPair> initialized_color_pairs;
		std::map<std::string, Menu> menus;
		size_t ln;
		//ColorPair color_pair;
	public:
		Menu& get_menu(const std::string& name) { return menus.at(name); }
		PANEL* get_panel() const { return panel; }
		size_t loop_number() const { return ln; }
		short color_initialized(const short r, const short g, const short b);
		short color_pair_initialized(const Color& fg, const Color& bg);
		Color get_color(const short color_id) const { return initialized_colors.at(color_id); }
		ColorPair get_color_pair(const short color_pair_id) { return initialized_color_pairs[color_pair_id]; }
		//ColorPair get_color_pair() const { return color_pair; }



		void set_panel(PANEL* panel) { this->panel = panel; }
		//void set_color(const ColorPair color_pair) { this->color_pair = color_pair; }

		void print(const char ch);
		void print(const std::string& str);
		void println(const std::string& str);

		int input(); // wrapper for getch

		size_t get_curs_y() const;
		size_t get_curs_x() const;

		short get_next_color_id();
		short get_next_color_pair_id();


		void enable(const chtype attr) { wattron(panel_window(panel), attr); }
		void disable(const chtype attr) { wattroff(panel_window(panel), attr); }
		void update() { update_panels(); doupdate(); }


		void init_colors();
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
