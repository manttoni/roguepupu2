#include <curses.h>     // for WINDOW, A_REVERSE, KEY_LEFT, KEY_RIGHT, box
#include <panel.h>      // for panel_window, del_panel, new_panel
#include <any>          // for any
#include <cassert>      // for assert
#include <memory>       // for unique_ptr, allocator
#include <string>       // for basic_string, operator+, operator==, char_traits
#include "UI/Menu.hpp"     // for Menu
#include "UI/MenuElt.hpp"  // for MenuElt
#include "UI/MenuTxt.hpp"  // for MenuTxt
#include "UI/UI.hpp"       // for UI, KEY_ESCAPE, KEY_LEFT_CLICK
#include "utils/Math.hpp"

Menu::Menu(	std::vector<std::unique_ptr<MenuElt>> elements_,
			const Vec2& start,
			void (*loop_cb)())
			: elements(std::move(elements_)), loop_cb(loop_cb), read_only(false)
{
	height = elements.size() + 2; // add border
	width = 4; // add border and space next to it
	for (size_t i = 0; i < elements.size(); ++i)
		width = std::max(static_cast<int>(elements[i]->get_size()) + 4, width);

	while (width > Screen::width() || height > Screen::height())
		UI::instance().dialog("Resize terminal");

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
	if (panel != nullptr)
	{
		WINDOW* window = panel_window(panel);

		// delete panel and window
		del_panel(panel);
		if (window != nullptr)
			delwin(window);
	}
}

Menu& Menu::operator=(Menu&& other) noexcept
{
	if (this == &other)
		return *this;

	panel = other.panel;
	other.panel = nullptr;

	elements = std::move(other.elements);
	height = other.height;
	width = other.width;
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
	Log::error("Not found");
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
	Log::error("MenuNum name not found: " + str);
}

int Menu::get_mouse_selection() const
{
	WINDOW* window = panel_window(panel);
	Vec2 start;
	getbegyx(window, start.y, start.x);
	Vec2 end = { start.y + height, start.x + width };
	Vec2 mouse = UI::instance().get_mouse_position();

	// Not inside menu, borders excluded
	if (mouse.x <= start.x || mouse.x >= end.x
		|| mouse.y <= start.y || mouse.y >= end.y)
		return -1;

	return mouse.y - start.y - 1; // -1 for border
}

// Print all MenuElts and if not read only, wait for a selection
// Can return a string if MenuBtn does not have a callback function
// This is useful for asking something from user
std::string Menu::loop()
{
	assert(panel != nullptr);
	WINDOW *window = panel_window(panel);
	size_t selected = initial_selected;
	int key = 0;
	while (key != KEY_ESCAPE)
	{
		while (!read_only && selected < elements.size() && !elements[selected]->is_selectable())
		{
			selected++;
			if (selected >= elements.size())
			{
				selected = 0;
				break;
			}
		}

		// call loop_callback
		if (loop_cb != nullptr)
			loop_cb();

		UI::instance().set_current_panel(panel, true);

		// print elements
		wmove(window, 1, 0); // because of box() start at y = 1
		for (size_t i = 0; i < elements.size(); ++i)
		{
			const bool highlight = !read_only && i == selected && elements[i]->is_selectable();
			if (highlight == true) UI::instance().enable_attr(A_REVERSE);
			if (elements[i]->get_text() == MenuTxt::HorizontalLine)
			{
				std::wstring line(width, L'─');
				UI::instance().print_wstr(line);
				continue;
			}
			UI::instance().print("  " + elements[i]->get_text() + "\n"); // spaces bcs of box()
			if (highlight == true) UI::instance().disable_attr(A_REVERSE);
		}

		box(window, 0, 0);
		UI::instance().update();

		if (read_only) break;

		// get key in non-blocking way
		key = UI::instance().input(100); // ms
		int mouse_select = get_mouse_selection();
		if (mouse_select >= 0 && mouse_select < static_cast<int>(elements.size()))
			selected = mouse_select;
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
			case KEY_LEFT_CLICK:
			case '\n':
				if (elements[selected]->get_type() == MenuElt::Type::BUTTON)
				{
					if (elements[selected]->get_func() == nullptr)
						return elements[selected]->get_text(); // this is probably asking multi choice like "yes" or "no"
					elements[selected]->callback();
				}
				break;
			default:
				break;
		}
	}
	return "";
}
