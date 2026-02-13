#pragma once
#include <ncurses.h>       // for chtype, doupdate, endwin, WINDOW, wattroff
#include <panel.h>        // for PANEL, panel_window, top_panel, update_panels
#include <cstdlib>        // for size_t, exit
#include <map>            // for map
#include <string>         // for string, allocator
#include <vector>         // for vector
#include "domain/Color.hpp"      // for Color
#include "domain/ColorPair.hpp"  // for ColorPair
#include "external/entt/entt.hpp"
#include "utils/Screen.hpp"
#include "utils/Log.hpp"
#include "domain/Position.hpp" // to return clicked position

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
			Log::log("Received signal: " + std::to_string(sig));
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
		Vec2 mouse_position;
	public:
		Vec2 get_mouse_position() const { return mouse_position; }
		void set_mouse_position(const Vec2& mouse_position) { this->mouse_position = mouse_position; }

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

	public:
		UI() = default;
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



		int input(int delay = -1); // wrapper for getch
		Vec2 get_direction(const int key);
		Position get_clicked_position(const entt::registry& registry);
		Position get_selected_position(entt::registry& registry);
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
		Vec2 get_window_dimensions(const WINDOW* const window) const;
		Vec2 get_window_start(const WINDOW* const window) const;
};

