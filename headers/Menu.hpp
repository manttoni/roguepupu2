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

		void loop();

	public:
		Menu();
		Menu(	std::vector<std::unique_ptr<MenuElt>> elements,
				const Screen::Coord& start,
				void (*loop_cb)() = nullptr);
		Menu(const Menu& other);
		Menu& operator=(Menu&& other);
		~Menu();

		void show(); // push panel to top, start loop()
		std::any get_value(const std::string& str) const;
};
