#include <panel.h>
#include <curses.h>
#include <string>
#include <cassert>
#include <optional>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <vector>

#include "utils/Error.hpp"
#include "utils/Screen.hpp"
#include "UI/Menu.hpp"
#include "UI/UI.hpp"
#include "utils/Vec2.hpp"
#include "utils/Utils.hpp"

Menu::Menu(const Vec2& position) : position(position), panel(nullptr), height(0), width(0) {}
Menu::~Menu()
{
	if (panel != nullptr)
	{
		WINDOW* window = panel_window(panel);
		del_panel(panel);
		if (window != nullptr)
			delwin(window);
	}
}

/* Need to know this because ncurses WINDOW needs a size,
 * so calculate biggest possible size this can take
 * */
size_t Menu::Element::get_size() const
{
	size_t size = 0;

	for (size_t i = 0; i < label.size(); ++i)
	{
		if (label[i] == '{' && Color::is_markup(label, i))
		{
			i = label.find('}', i);
			continue;
		}
		if (label[i] == '[' && Color::is_markup(label, i))
		{
			i = label.find(']', i);
			continue;
		}
		size++;
	}
	if (type == Type::TextField)
	{
		const std::string ascii = ": _";
		size += max_value + ascii.size();
	}
	if (type == Type::ValueSelector) // Value: < 10 >
	{
		const std::string ascii = ": <  >";
		const size_t number_size = std::max(
				std::to_string(min_value).size(),
				std::to_string(max_value).size()
				);
		size += ascii.size() + number_size;
	}
	if (type == Type::Checkbox)
	{
		const std::string ascii = ": [X]";
		size += ascii.size();
	}
	return size;
}

/* This returns a string that will be printed in the menu on the screen
 * */
std::string Menu::Element::get_text() const
{
	switch (type)
	{
		case Type::Button:
		case Type::Text:
			return label;
		case Type::TextField:
			if (auto strPtr = std::get_if<std::string*>(&value))
				return label + ": " + **strPtr + "_";
			else
				Error::fatal("Menu element (" + label + ") is invalid: value not string");
		case Type::ValueSelector:
			{
				if (auto intPtr = std::get_if<int*>(&value))
				{
					std::string text = label;
					if (**intPtr == min_value)
						text += ":   ";
					else
						text += ": < ";
					text += std::to_string(**intPtr);
					if (**intPtr == max_value)
						text += "  ";
					else
						text += " >";
					return text;
				}
				else
					Error::fatal("Menu element (" + label + ") is invalid: value not int");
			}
		case Type::Checkbox:
			if (auto boolPtr = std::get_if<bool*>(&value))
				return label + ": [" + (**boolPtr ? "X]" : " ]");
			else
				Error::fatal("Menu element (" + label + ") is invalid: value not bool");
		default:
			return "???";
	}
}

bool Menu::add_element(Element element)
{
	if (auto intPtr = std::get_if<int*>(&element.value))
	{
		**intPtr = std::max(element.min_value, **intPtr);
		**intPtr = std::min(element.max_value, **intPtr);
	}
	elements.push_back(element);
	return true;
}

/* Create/modify WINDOW that is inside PANEL.
 * Add some extra size because box() will need it (2, 1 for each border).
 * Also add extra size for some formatting
 * Menu knows where it should be located, so center it to that but clamp with screen size.
 * */
void Menu::set_panel()
{
	height = 2 + elements.size();
	auto it = std::max_element(elements.begin(), elements.end(),
			[](const auto& a, const auto& b)
			{
			return a.get_size() < b.get_size();
			});
	assert(it != elements.end());
	width = 4 + it->get_size();

	while (static_cast<int>(height) > Screen::height()
			|| static_cast<int>(width) > Screen::width())
	{	// Use ctrl + [+/-] (in my terminal). This loop ends when terminal size is big enough.
		// A better solution would be scrolling menus, maybe later
		mvaddstr(0, 0, "Resize terminal to fit menu. Default ctrl[+/-]");
		refresh();
		getch();
	}

	const Vec2 size(static_cast<int>(height), static_cast<int>(width));
	Vec2 start = position - size / 2;
	start = Vec2(
			std::min(Screen::height(), std::max(0, start.y)),
			std::min(Screen::width(), std::max(0, start.x))
			);
	if (!panel)
	{
		WINDOW* win = newwin(height, width, start.y, start.x);
		assert(win != nullptr);
		panel = new_panel(win);
		assert(panel != nullptr);
	}
	else
	{
		WINDOW* window = panel_window(panel);
		wresize(window, height, width);
		move_panel(panel, start.y, start.x);
		assert(window != nullptr);
		assert(panel != nullptr);
	}
	UI::instance().set_current_panel(panel, true);
}

/* By knowing how many unselectable elements there are,
 * can keep selected index always at a selectable element
 * */
size_t Menu::get_unselectable_count() const
{
	// unselectables are only Text
	size_t unselectable = 0;
	for (const auto& element : elements)
	{
		if (element.type == Element::Type::Text)
			unselectable++;
		assert(element.type != Element::Type::None);
	}
	return unselectable;
}

/* Mouse position is saved in UI::instance().
 * This function checks only vertical, because Menu doesn's have
 * horizontally aligned elements.
 * Use -1 to mean "mouse is not hovering on any of the selectable elements"
 * */
int Menu::get_mouse_selection() const
{
	WINDOW* window = panel_window(panel);
	Vec2 start, size;
	getbegyx(window, start.y, start.x);
	getmaxyx(window, size.y, size.x);
	const Vec2 end = start + size;
	const Vec2 mouse = UI::instance().get_mouse_position();

	// Not inside menu, borders excluded
	if (mouse.x <= start.x || mouse.x >= end.x
			|| mouse.y <= start.y || mouse.y >= end.y)
		return -1;

	int selection = mouse.y - start.y - 1; // -1 for border
	if (static_cast<size_t>(selection) < get_unselectable_count())
		return -1;
	assert(selection >= 0);
	return static_cast<size_t>(selection);
}

/* Print menu elements, and highlight the selected with ncurses A_REVERSE,
 * it will invert bg and fg colors.
 * */
void Menu::show_elements(const size_t selected) const
{
	wmove(panel_window(panel), 1, 0); // because of box() start at y = 1
	for (size_t i = 0; i < elements.size(); ++i)
	{
		const bool highlight = i == selected;
		if (highlight == true) UI::instance().enable_attr(A_REVERSE);
		if (elements[i].label == "--")
		{
			std::wstring line(width - 1, L'─');
			UI::instance().print_wstr(line + L'\n');
			continue;
		}
		UI::instance().print("  " + elements[i].get_text() + "\n");
		if (highlight == true) UI::instance().disable_attr(A_REVERSE);
	}
	box(panel_window(panel), 0, 0);
	UI::instance().update();
}

/* If user pressed up or down or hovers with mouse, return the index they want
 * */
size_t Menu::select_element(const size_t selected, const int key) const
{
	int mouse_selection = get_mouse_selection();
	if (mouse_selection != -1)
		return static_cast<size_t>(mouse_selection);

	switch (key)
	{
		case KEY_DOWN:
			return std::min(elements.size() - 1, selected + 1);
		case KEY_UP:
			if (selected == 0) return 0;
			return std::max(get_unselectable_count(), selected - 1);
	}
	return selected;
}

void Menu::change_value(Element& e, const int key)
{
	switch (key)
	{
		case KEY_LEFT:
			if (auto intPtr = std::get_if<int*>(&e.value))
				**intPtr = std::max(e.min_value, **intPtr - 1);
			return;
		case KEY_RIGHT:
			if (auto intPtr = std::get_if<int*>(&e.value))
				**intPtr = std::min(e.max_value, **intPtr + 1);
			return;
		default:
			return;
	}
}

void Menu::input_text(Element& e, const int key)
{
	if (auto strPtr = std::get_if<std::string*>(&e.value))
	{
		if (key == KEY_BACKSPACE && !(*strPtr)->empty())
			(*strPtr)->pop_back();
		else if (std::isalpha(static_cast<unsigned char>(key)) && (*strPtr)->size() < static_cast<size_t>(e.max_value))
			**strPtr += key;
	}
}

void Menu::set_bool(Element& e, const int key)
{
	if (key == '\n' || key == KEY_ENTER || key == KEY_LEFT_CLICK)
		if (auto boolPtr = std::get_if<bool*>(&e.value))
			**boolPtr ^= true;
}

bool Menu::selection_confirmed(const Element& e, const int key) const
{
	if (e.type != Element::Type::Button)
		return false;
	return key == '\n' || key == KEY_ENTER || key == KEY_LEFT_CLICK;
}

/* Needs either one or more Buttons or only unselectables. Will solve it if becomes problem.
 * Name is not good because can be used to simply display a message if has only unselectables.
 * */
Menu::Element Menu::get_selection(const size_t default_selected)
{
	int key = 0;
	size_t selected = default_selected + get_unselectable_count(); // selected will always be one of the selectable elements
	set_panel();
	while (key != KEY_ESCAPE)
	{
		show_elements(selected);
		if (get_unselectable_count() == elements.size())
			return Element{};
		key = UI::instance().input(100); // delay is pretty short because want hover highlight to update more often
		selected = select_element(selected, key); // changes selected from keyboard or mouse hover
		if (elements[selected].type == Element::Type::ValueSelector)
			change_value(elements[selected], key);
		if (elements[selected].type == Element::Type::TextField)
			input_text(elements[selected], key);
		if (elements[selected].type == Element::Type::Checkbox)
			set_bool(elements[selected], key);
		if (selection_confirmed(elements[selected], key)) // enter or left click on a Button
		{
			elements[selected].index = selected - get_unselectable_count();
			return elements[selected];
		}
	}
	return Element{};
}

