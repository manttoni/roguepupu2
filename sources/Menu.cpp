#include <ncurses.h>
#include <memory>
#include <any>
#include <cassert>
#include "UI.hpp"
#include "Utils.hpp"
#include "Menu.hpp"
#include "MenuElt.hpp"
#include "MenuBtn.hpp"
#include "MenuNum.hpp"

Menu::Menu() : window(nullptr), loop_cb(nullptr) {}

Menu::Menu(std::vector<std::unique_ptr<MenuElt>> elements_, void (*loop_cb)(), const size_t start_idx)
	: elements(std::move(elements_)), loop_cb(loop_cb)
{
	height = elements.size() + 2;
	width = 0;
	for (size_t i = 0; i < elements.size(); ++i)
		width = std::max(elements[i]->get_size(), width);
	width += 4;
	assert(COLS != 0);
	starty = start_idx / COLS;
	startx = start_idx % COLS;
	window = newwin(height, width, starty, startx);
}
Menu::~Menu()
{
	if (window != nullptr)
		delwin(window);
}

std::any Menu::get_value(const std::string& str) const
{
	for (size_t i = 0; i < elements.size(); ++i)
	{
		Log::log(elements[i]->get_type() + " " + elements[i]->MenuElt::get_text());
		if (elements[i]->get_type() == "number" && elements[i]->MenuElt::get_text() == str)
			return elements[i]->get_value();
	}
	throw std::runtime_error("Not found");
	return {};
}

void Menu::loop()
{
	size_t selected = 0;
	int input = 0;
	while (input != KEY_ESCAPE)
	{
		if (loop_cb != nullptr)
			loop_cb();
		flag = false;
		werase(window);
		wmove(window, 1, 0);
		for (size_t i = 0; i < elements.size(); ++i)
		{
			if (i == selected)
				wattron(window, A_REVERSE);
			UI::print(window, "  " + elements[i]->get_text() + "\n");
			if (i == selected)
				wattroff(window, A_REVERSE);
		}
		box(window, 0, 0);
		wrefresh(window);
		flushinp();
		input = getch();
		switch (input)
		{
			case KEY_DOWN:
				selected = selected == elements.size() - 1 ? selected : selected + 1;
				break;
			case KEY_UP:
				selected = selected == 0 ? 0 : selected - 1;
				break;
			case KEY_LEFT:
			case KEY_RIGHT:
				if (elements[selected]->get_type() == "number")
				{
					if (input == KEY_RIGHT) elements[selected]->increment();
					if (input == KEY_LEFT) elements[selected]->decrement();
					if (elements[selected]->MenuElt::get_text() != "Level")
						flag = true;
				}
				break;
			case '\n':
				if (elements[selected]->get_type() == "button")
					elements[selected]->callback();
				break;
		}
	}
}
