#pragma once
#include <vector>
#include <ncurses.h>
#include <memory>
#include "MenuElt.hpp"

class Menu
{
	private:
		size_t starty, startx;
		size_t height, width;
		WINDOW *window;
		std::vector<std::unique_ptr<MenuElt>> elements;
		void (*loop_cb)(); // call every loop
		bool flag = true;

	public:
		Menu();
		Menu(std::vector<std::unique_ptr<MenuElt>> elements, void (*loop_cb)() = nullptr, const size_t start_idx = 0);
		Menu(const Menu& other) = delete;
		Menu operator=(const Menu& other) = delete;
		~Menu();

		std::any get_value(const std::string& str) const;
		void loop();
		bool get_flag() const { return flag; }
};
