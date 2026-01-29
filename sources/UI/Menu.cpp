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
#include "utils/Error.hpp"

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

size_t Menu::Element::get_size() const
{
	size_t size = 0;

	for (size_t i = 0; i < label.size(); ++i)
	{	// this will be replaced by helper at some point
		if (label[i] == '{') i = label.find('}', i);
		else if (label[i] == '[') i = label.find(']', i);
		else size++;
	}
	if (type == Type::TextField)
	{
		assert(max_input);
		const std::string ascii = ": _";
		size += *max_input + ascii.size();
	}
	if (type == Type::ValueSelector) // Value: < 10 >
	{
		assert(min_value && max_value);
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

std::string Menu::Element::get_text() const
{
	switch (type)
	{
		case Type::Button:
		case Type::Text:
			return label;
		case Type::TextField:
			return label + ": " + *input + "_";
		case Type::ValueSelector:
			std::string text = label;
			if (*value == *min_value)
				text += ":   ";
			else
				text += ": < ";
			text += std::to_string(*value);
			if (*value == *max_value)
				text += " >";
			else
				text += "  ";
			return text;
		case Type::Checkbox:
			return label + ": [" + (*check ? "X]" : " ]");
		default:
			return "???";
	}
}

bool Menu::add_element(Element& element)
{
	if (element.type == Element::Type::None || element.label.empty())
		return false;
	switch (element.type)
	{
		case Element::Type::Button:
		case Element::Type::Text:
			elements.push_back(element);
			break;
		case Element::Type::TextField:
			if (!element.max_input)
				return false;
			if (!element.input)
				element.input.emplace("");
			elements.push_back(element);
			break;
		case Element::Type::ValueSelector:
			if (!element.min_value || !element.max_value)
				return false;
			if (!element.value)
				element.value = (*element.min_value + *element.max_value) / 2;
			elements.push_back(element);
			break;
		case Element::Type::Checkbox:
			if (!element.check)
				element.check = false;
			elements.push_back(element);
			break;
		default:
			return false;
	}
	return true;
}

void Menu::set_panel()
{
	height = 2 + elements.size();
	auto it = std::max_element(elements.begin(), elements.end(),
			[](const auto& a, const auto& b)
			{
			return a.get_size() < b.get_size();
			});
	assert(it != elements.end());
	width = 2 + it->get_size();

	while (height > Screen::height() || width > Screen::width())
	{	// Use ctrl + [+/-] (in my terminal). This loop ends when terminal size is big enough.
		// A better solution would be scrolling menus, maybe later
		mvaddstr(0, 0, "Resize terminal to fit menu");
		refresh();
		getch();
	}

	const Vec2 size(static_cast<int>(height), static_cast<int>(width));
	const Vec2 start = position - size / 2;
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
	if (selection < get_unselectable_count())
		return -1;
	return selection;
}

void Menu::show_elements(const size_t selected) const
{
	wmove(panel_window(panel), 1, 0); // because of box() start at y = 1
	for (size_t i = 0; i < elements.size(); ++i)
	{
		const bool highlight = i == selected;
		if (highlight == true) UI::instance().enable_attr(A_REVERSE);
		if (elements[i].label == "--")
		{
			std::wstring line(width, L'─');
			UI::instance().print_wstr(line);
			continue;
		}
		UI::instance().print("  " + elements[i].get_text() + "\n");
		if (highlight == true) UI::instance().disable_attr(A_REVERSE);
	}
	box(window, 0, 0);
	UI::instance().update();
}

size_t Menu::select_element(const size_t selected, const int key) const
{
	int mouse_selection = get_mouse_selection();
	if (mouse_selection != -1)
		return static_cast<size_t>(mouse_selection);

	switch (key)
	{
		case KEY_DOWN:
			return std::min(elements.size(), selected + 1);
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
			*e.value = std::max(*e.min_value, *e.value - 1);
			return;
		case KEY_RIGHT:
			*e.value = std::min(*e.max_value, *e.value + 1);
			return;
		default:
			return;
	}
}

void Menu::input_text(Element& e, const int key)
{
	if (key == KEY_BACKSPACE && !*e.input.empty())
		e.input->pop_back();
	else if (std::isalpha(static_cast<unsigned char>(ch)) && *e.input.size() < *e.max_input)
		*e.input += key;
}

bool Menu::selection_confirmed(const Element& e, const int key)
{
	if (e.type != Element::Type::Button)
		return false;
	return key == '\n' || key == KEY_ENTER || key == KEY_LEFT_CLICK;
}

/* Needs either one or more Buttons or only unselectables. Will solve it if becomes problem.
 * */
Element Menu::get_selection(const size_t default_selected)
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
		if (selection_confirmed(selected, key)) // enter or left click on a Button
			return elements[selected];
	}
	return Element{};
}

