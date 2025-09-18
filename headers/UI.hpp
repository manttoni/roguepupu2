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
		static void handle_signal(int sig)
		{
			(void) sig;
			endwin();
			std::exit(sig);
		}


	/* PERMANENT COLORS */
	private:
		const std::map<std::string, std::array<short, 3>> color_rgb = {
			{"white",		{1000, 1000, 1000}},
			{"black",		{0, 0, 0}},
			{"light_blue",	{0, 0, 100}},
			{"orange",		{1000, 500, 0}},
			{"medium_blue",	{0, 0, 500}}
		}
		const std::map<std::string, std::array<std::string, 2>> color_pair_fgbg = {
			{"glowing_fungus",	{"medium_blue", "black"}},
			{"woody_fungus",	{"orange", "black"}},
			{"default",			{"black", "white"}}
		}
		std::map<std::string, Color> colors;
		std::map<std::string, ColorPairs> pairs;
	public:
		short get_color_id(const std::string& label) { return colors[label].get_id(); }
		short get_pair_id(const std::string& label) { return pairs[label].get_id(); }
		Color get_color(const std::string& label) { return colors[label]; }
		ColorPair get_pair(const std::string& label) { return pairs[label]; }

	/* TEMPORARY COLORS */
	private:
		// these hold initialized colors/pairs
		std::map<short, Color> temp_colors;
		std::map<short, ColorPair> temp_pairs;
		void clear_temp_colors();
	public:
		// get a usable color/pair id with rgb values/ids of initialized colors
		short get_temp_color_id(const short r, const short g, const short b);
		short get_temp_pair_id(const short fg_id, const short bg_id);

	/* ALL COLORS */
	private:
		short get_next_color_id();
		short get_next_color_pair_id();

	public:
		Color get_color(const short color_id) const;
		ColorPair get_pair(const short pair_id) const;

	// sketch

	/* INPUT */
	private:
		std::vector<InputWindow> input_windows;
	public:
		InputWindow& get_input_window(const std::string& label);
		void add_input_window(const InputWindow& input_window);
		void set_input_window(const std::string& label);
		int print();

	/* OUTPUT */
	private:
		std::vector<OutputWindow> output_windows;
	public:
		OutputWindow& get_output_window(const std::string& label);
		void add_output_window(const OutputWindow& output_window);
		void set_output_window(const std::string& label);

	//asd
	/* OUTPUT */
	private:
		PANEL* panel;
	public:
		void set_panel(PANEL* panel) { this->panel = panel; }
		PANEL* get_panel() const { return panel; }

		void print(const char ch);
		void print(const std::string& str);
		void println(const std::string& str);

		size_t get_curs_y() const;
		size_t get_curs_x() const;

		void enable_attr(const chtype attr) { wattron(panel_window(panel), attr); }
		void disable_attr(const chtype attr) { wattroff(panel_window(panel), attr); }

	/* INPUT */
	public:
		int input();


		std::map<std::string, Menu> menus;
		size_t ln;
	public:
		Menu& get_menu(const std::string& name) { return menus.at(name); }
		size_t loop_number() const { return ln; }



		void update() { update_panels(); doupdate(); }

	/* INIT */
	public:
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
