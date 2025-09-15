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

Menu::Menu() : panel(nullptr), start({0, 0}), loop_cb(nullptr), read_only(true)
{
}

Menu::Menu(	std::vector<std::unique_ptr<MenuElt>> elements_,
			const Screen::Coord& start,
			void (*loop_cb)())
			: elements(std::move(elements_)), start(start), loop_cb(loop_cb), read_only(false)
{
	// calculate menu size by amount of and longest element
	height = elements.size() + 2;
	width = 4;
	for (size_t i = 0; i < elements.size(); ++i)
		width = std::max(elements[i]->get_size() + 4, width);


	int y = static_cast<int>(start.y) - static_cast<int>(height) / 2;
	int x = static_cast<int>(start.x) - static_cast<int>(width) / 2;

	// clamp to screen bounds
	if (y < 0) y = 0;
	if (x < 0) x = 0;
	if (y + height > Screen::height()) y = Screen::height() - height;
	if (x + width > Screen::width()) x = Screen::width() - width;

	WINDOW* window = newwin(height, width, y, x);
	assert(window != nullptr);
	panel = new_panel(window);
	assert(panel != nullptr);
}

Menu::~Menu()
{
	/*
	if (panel != nullptr)
	{
		WINDOW* window = panel_window(panel);
		del_panel(panel);
		if (window)
			delwin(window);
	}
	*/
}

Menu& Menu::operator=(Menu&& other) noexcept
{
	if (this == &other)
		return *this;

	panel = other.panel;
	//other.panel = nullptr;

	elements = std::move(other.elements);
	height = other.height;
	width = other.width;
	start = other.start;
	loop_cb = other.loop_cb;
	read_only = other.read_only;

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

void Menu::set_value(const std::string& str, std::any value)
{
	for (size_t i = 0; i < elements.size(); ++i)
	{
		if (elements[i]->get_type() == MenuElt::Type::NUMBER
				&& elements[i]->MenuElt::get_text() == str)
		{
			elements[i]->set_value(value);
			return;
		}
	}
	throw std::runtime_error("MenuNum name not found: " + str);
}

void Menu::loop()
{
	assert(panel != nullptr);
	WINDOW *window = panel_window(panel);
	size_t selected = 0;
	int key = 0;
	while (key != KEY_ESCAPE)
	{
		// call loop_callback
		if (loop_cb != nullptr)
			loop_cb();

		UI::instance().set_panel(panel);
		top_panel(panel);

		// print elements
		wmove(window, 1, 0); // because of box() start at y = 1
		for (size_t i = 0; i < elements.size(); ++i)
		{
			if (!read_only && i == selected) UI::instance().enable(A_REVERSE);
			UI::instance().print("  " + elements[i]->get_text() + "\n"); // spaces bcs of box()
			if (!read_only && i == selected) UI::instance().disable(A_REVERSE);
		}

		box(window, 0, 0);
		UI::instance().update();
		if (read_only) break;
		key = UI::instance().input();
		switch (key)
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
					if (key == KEY_RIGHT) elements[selected]->increment();
					if (key == KEY_LEFT) elements[selected]->decrement();
				}
				break;
			case '\n':
				if (elements[selected]->get_type() == MenuElt::Type::BUTTON)
					elements[selected]->callback();
				break;
			default:
				break;
		}
	}
}
