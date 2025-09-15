#pragma once
#include <vector>
#include <ncurses.h>
#include <panel.h>
#include <memory>
#include "Utils.hpp"
#include "MenuElt.hpp"

class Menu
{
	private:
		PANEL* panel;
		std::vector<std::unique_ptr<MenuElt>> elements;
		size_t height, width; // calculated automatically, no override atm
		Screen::Coord start;
		void (*loop_cb)(); // call every loop
		bool read_only;

	public:
		void set_read_only(const bool value) { read_only = value; }
		bool get_read_only() const { return read_only; }
		PANEL* get_panel() const { return panel; }


	public:
		Menu();
		Menu(	std::vector<std::unique_ptr<MenuElt>> elements,
				const Screen::Coord& start,
				void (*loop_cb)() = nullptr);
		Menu(const Menu& other);
		Menu& operator=(Menu&& other) noexcept;
		~Menu();

		void loop();

		// MenuNum
		std::any get_value(const std::string& str) const;
		void set_value(const std::string& str, std::any value);
};
