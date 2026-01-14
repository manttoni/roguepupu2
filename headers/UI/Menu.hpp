#pragma once
#include <vector>
#include <ncurses.h>
#include <panel.h>
#include <memory>
#include "MenuElt.hpp"
#include "utils/Vec2.hpp"

class Menu
{
	private:
		PANEL* panel;
		std::vector<std::unique_ptr<MenuElt>> elements;
		size_t height, width; // calculated automatically, no override atm
		void (*loop_cb)(); // call every loop
		bool read_only;
		int get_mouse_selection() const;
		size_t initial_selected = 0;

	public:
		void set_selected(const size_t initial_selection) { initial_selected = initial_selection; }
		void set_read_only(const bool value) { read_only = value; }
		bool get_read_only() const { return read_only; }
		PANEL* get_panel() const { return panel; }
		size_t get_height() const { return height; }
		size_t get_width() const { return width; }

	public:
		Menu();
		Menu(	std::vector<std::unique_ptr<MenuElt>> elements,
				const Vec2& start,
				void (*loop_cb)() = nullptr);
		Menu(const Menu& other);
		Menu& operator=(Menu&& other) noexcept;
		~Menu();

		std::string loop();

		// MenuNum, remove MenuNum class because might just be useless
		std::any get_value(const std::string& str) const;
		void set_value(const std::string& str, std::any value);
};
