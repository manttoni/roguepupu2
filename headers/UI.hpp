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
		std::map<std::string, Menu*> menus;
		//ColorPair color_pair;
	public:
		PANEL* get_panel() const { return panel; }
		int color_initialized(const short r, const short g, const short b) const;
		int color_pair_initialized(const Color& fg, const Color& bg) const;
		//ColorPair get_color_pair() const { return color_pair; }

	public:
		static UI& instance()
		{
			static UI inst;
			return inst;
		}

		void set_panel(PANEL* panel) { this->panel = panel; top_panel(panel); }
		//void set_color(const ColorPair color_pair) { this->color_pair = color_pair; }

		void print(const char ch);
		void print(const std::string& str);
		void println(const std::string& str);

		size_t get_curs_y() const;
		size_t get_curs_x() const;

		short get_next_color_id();
		short get_next_color_pair_id();

		short add_color(const short r, const short g, const short b);
		short add_color_pair(const short fg_id, const short bg_id);

		void enable(const chtype attr) { wattron(panel_window(panel), attr); }
		void disable(const chtype attr) { wattroff(panel_window(panel), attr); }
		void update() { update_panels(); doupdate(); }

		void add_menu(const std::string& name, Menu* menu) { menus[name] = menu; }

		void init()
		{
			initscr();
			std::signal(SIGSEGV, handle_signal);
			std::signal(SIGABRT, handle_signal);
			std::signal(SIGFPE, handle_signal);
			std::signal(SIGINT, handle_signal);
			noecho();
			curs_set(0);
			keypad(stdscr, TRUE);
			start_color();
			set_escdelay(25);
		}

		void end()
		{
			endwin();
			std::exit(0);
		}
};

namespace CaveView
{
	void draw_cave();
	void cave_generator();
	extern PANEL* cave_panel;
}
