#include <ncurses.h>
#include <panel.h>
#include <memory>
#include <any>
#include <cassert>
#include "UI.hpp"
#include "Utils.hpp"
#include "Menu.hpp"
#include "MenuElt.hpp"
#include "MenuBtn.hpp"
#include "MenuNum.hpp"

Menu::Menu() : panel(nullptr), start({0, 0}), loop_cb(nullptr)
{
}

Menu::Menu(	std::vector<std::unique_ptr<MenuElt>> elements_,
			const Screen::Coord& start,
			void (*loop_cb)())
			: elements(std::move(elements_)), start(start), loop_cb(loop_cb)
{
	// calculate menu size by amount of and longest element
	height = elements.size() + 2;
	width = 4;
	for (size_t i = 0; i < elements.size(); ++i)
		width = std::max(elements[i]->get_size() + 4, width);

	// center
	int y = std::max(0, static_cast<int>(start.y) - static_cast<int>(height) / 2);
	int x = std::max(0, static_cast<int>(start.x) - static_cast<int>(width) / 2);

	panel = new_panel(newwin(height, width, y, x));
}

Menu::~Menu()
{
	if (panel != nullptr)
	{
		delwin(panel_window(panel));
		del_panel(panel);
	}
}

Menu& Menu::operator=(Menu&& other)
{
	if (this == &other)
		return *this;

	panel = other.panel;
	other.panel = nullptr;

	elements = std::move(other.elements);
	height = other.height;
	width = other.width;
	start = other.start;
	loop_cb = other.loop_cb;

	return *this;
}

std::any Menu::get_value(const std::string& str) const
{
	for (size_t i = 0; i < elements.size(); ++i)
	{
		if (elements[i]->get_type() == MenuElt::Type::NUMBER
			&& elements[i]->MenuElt::get_text() == str)
			return elements[i]->get_value();
	}
	throw std::runtime_error("Not found");
	return {};
}

void Menu::show()
{
	// make menu visible
	top_panel(panel);
	show_panel(panel);
	update_panels();
	doupdate();

	loop();

	// hide
	hide_panel(panel);
	update_panels();
	doupdate();
}

void Menu::loop()
{
	WINDOW *window = panel_window(panel);
	size_t selected = 0;
	int input = 0;
	while (input != KEY_ESCAPE)
	{
		// call loop_callback
		if (loop_cb != nullptr)
			loop_cb();

		// print elements
		wmove(window, 1, 0); // because of box()
		for (size_t i = 0; i < elements.size(); ++i)
		{
			if (i == selected) wattron(window, A_REVERSE);
			UI::print(window, "  " + elements[i]->get_text() + "\n"); // spaces bcs of box()
			if (i == selected) wattroff(window, A_REVERSE);
		}
		box(window, 0, 0);

		// print debug info
		wmove(window, 0, 2);
		int wy, wx;
		getmaxyx(window, wy, wx);
		UI::print(window, " " + std::to_string(wy) + " * " + std::to_string(wx) + " ");

		update_panels();
		doupdate();

		// process input
		flushinp();
		input = getch();
		switch (input)
		{
			case KEY_DOWN:
				Math::increment(selected, elements.size() - 1);
				break;
			case KEY_UP:
				Math::decrement(selected, size_t(0));
				break;
			case KEY_LEFT:
			case KEY_RIGHT:
				if (elements[selected]->get_type() == MenuElt::Type::NUMBER)
				{
					if (input == KEY_RIGHT) elements[selected]->increment();
					if (input == KEY_LEFT) elements[selected]->decrement();
				}
				break;
			case '\n':
				if (elements[selected]->get_type() == MenuElt::Type::BUTTON)
					elements[selected]->callback();
				break;
		}
	}
}
