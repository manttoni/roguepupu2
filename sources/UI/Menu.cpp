#include <panel.h>
#include <curses.h>
#include <string>
#include <cassert>
#include <optional>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <vector>
#include <variant>

#include "UI/Menu.hpp"
#include "utils/Error.hpp"
#include "utils/Screen.hpp"
#include "UI/Menu.hpp"
#include "UI/UI.hpp"
#include "utils/Vec2.hpp"
#include "utils/Utils.hpp"
#include "utils/Math.hpp"

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
	size_t size = 3 + 3; // 3 space on the left, 3 on right

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
		size += value_range.max + ascii.size();
	}
	if (type == Type::ValueSelector) // Value: < 10 >
	{
		const std::string ascii = ": <  >";
		const size_t number_size = std::max(
				std::to_string(value_range.min).size(),
				std::to_string(value_range.max).size()
				);
		size += ascii.size() + number_size;
	}
	if (type == Type::Checkbox)
	{
		const std::string ascii = ": [X]";
		size += ascii.size();
	}
	if (type == Type::MultiChoice)
	{
		const std::string ascii = ": <  >";
		assert(!choices.empty());
		auto it = std::max_element(choices.begin(), choices.end(),
				[](const auto& a, const auto& b)
				{
				return a.size() < b.size();
				});
		size += ascii.size() + it->size();
	}
	return size;
}

/* This returns a string that will be printed in the menu on the screen
 * The string consists of:
 * - label on left
 * - value string on right (if there is a value)
 * */
std::string Menu::Element::get_text(const size_t width) const
{
	std::string value_string = "";
	switch (type)
	{
		case Type::Button:
		case Type::Text:
			if (is_cancel || is_confirm)
				return "[A_BOLD]" + label + "[reset]";
			return label;
		case Type::TextField:
			if (auto p = std::get_if<std::string*>(&value))
				value_string = **p;
			else if (auto p = std::get_if<nlohmann::json*>(&value))
				value_string = (**p).get<std::string>();
			else Error::fatal("Menu element (" + label + ") is invalid: value not string or json");
			value_string += value_string.size() < static_cast<size_t>(value_range.max) ? "[A_BLINK]_[reset]" : "";
			return std::format("{:<{}}{:>{}}",
					label, width - value_range.max - 9,
					value_string, value_string.size() - 5);
		case Type::ValueSelector:
			{
				std::string number = "";
				bool is_min = false;
				bool is_max = false;
				if (auto p = std::get_if<int*>(&value))
				{
					is_min = **p == value_range.min;
					is_max = **p == value_range.max;
					number = std::to_string(**p);
				}
				else if (auto p = std::get_if<double*>(&value))
				{
					is_min = **p == value_range.min;
					is_max = **p == value_range.max;
					number = std::format("{:.{}f}", **p, Math::get_precision(delta));
				}
				else if (auto p = std::get_if<nlohmann::json*>(&value))
				{
					auto& j = **p;
					if (j.is_number_integer())
					{
						is_min = **p == value_range.min;
						is_max = **p == value_range.max;
						number = std::to_string(j.get<int>());
					}
					else if (j.is_number_float())
					{
						is_min = **p == value_range.min;
						is_max = **p == value_range.max;
						number = std::format("{:.{}f}", j.get<double>(), Math::get_precision(delta));
					}
				}
				if (number.empty())
					Error::fatal("ValueSelector does not have a numeric value");

				value_string += is_min ? "   " : " < ";
				value_string += number;
				value_string += is_max ? "   " : " >";
				return std::format("{:<{}}{:>{}}",
						label, width - value_string.size() - 9,
						value_string, value_string.size() - 5);
			}
		case Type::Checkbox:
			if (auto p = std::get_if<bool*>(&value))
				value_string = std::string("[") + (**p ? "X" : " ") + "]";
			else if (auto p = std::get_if<nlohmann::json*>(&value))
				value_string = std::string("[") + ((**p).get<bool>() ? "X" : " ") + "]";
			else
				Error::fatal("Menu element (" + label + ") is invalid: value not bool or json");
			return std::format("{:<{}}{:>{}}",
					label, width - value_string.size() - 13,
					value_string, value_string.size() - 3);
		case Type::MultiChoice:
			{
				std::string choice = "";
				if (auto p = get_if<std::string*>(&value))
					choice = **p;
				else if (auto p = get_if<nlohmann::json*>(&value))
					choice = (**p).get<std::string>();
				else Error::fatal("Invalid MultiChoice element, value not string");
				auto it = std::find(choices.begin(), choices.end(), choice);
				if (it == choices.end())
					Error::fatal("MultiChoice choice is not in choices");
				const size_t index = it - choices.begin();
				if (index == 0)
					value_string += "   ";
				else
					value_string += " < ";
				value_string += choice;
				if (index == choices.size() - 1)
					value_string += "  ";
				else
					value_string += " >";
				return std::format("{:<{}}{:>{}}",
						label, width - value_string.size() - 9,
						value_string, value_string.size() - 5);
			}
		default:
			return "???";
	}
}

void Menu::add_element(Element element)
{
	assert(element.type != Element::Type::None);
	if (auto intPtr = std::get_if<int*>(&element.value))
	{
		**intPtr = std::max(element.value_range.min, **intPtr);
		**intPtr = std::min(element.value_range.max, **intPtr);
	}
	if (auto dblPtr = std::get_if<double*>(&element.value))
	{
		**dblPtr = std::max(static_cast<double>(element.value_range.min), **dblPtr);
		**dblPtr = std::min(static_cast<double>(element.value_range.max), **dblPtr);
	}
	if (auto p = std::get_if<nlohmann::json*>(&element.value))
	{
		auto& j = **p;
		if (j.is_number_float())
		{
			double v = j.get<double>();
			**p = Math::clamp(v, static_cast<double>(element.value_range.min), static_cast<double>(element.value_range.max));
		}
		else if (j.is_number_integer())
		{
			int v = j.get<int>();
			**p = Math::clamp(v, element.value_range.min, element.value_range.max);
		}
	}
	elements.push_back(element);
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
	start.y = std::max(0, std::min(start.y, Screen::height() - static_cast<int>(height)));
	start.x = std::max(0, std::min(start.x, Screen::width() - static_cast<int>(width)));
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
		const bool highlighted = i == selected;
		if (highlighted == true) UI::instance().enable_attr(A_REVERSE);
		if (elements[i].label == "--")
		{
			std::wstring line(width - 1, L'─');
			UI::instance().print_wstr(line + L'\n');
			continue;
		}
		const std::string left = elements[i].highlight == '\0' ? "   " : std::string(" ") + elements[i].highlight + " ";
		const std::string line = "  " + left + elements[i].get_text(width);
		UI::instance().print(std::format("{:<{}}", line, width - 1) + "\n");
		if (highlighted == true) UI::instance().disable_attr(A_REVERSE);
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
	double change = 0;
	switch (key)
	{
		case KEY_LEFT:
			change = -e.delta;
			break;
		case KEY_RIGHT:
			change = e.delta;
			break;
		default:
			return;
	}

	if (auto p = std::get_if<int*>(&e.value))
		**p = Math::clamp(**p + static_cast<int>(change), e.value_range.min, e.value_range.max);
	else if (auto p = std::get_if<nlohmann::json*>(&e.value))
	{
		auto& j = **p;
		if (j.is_number_float())
		{
			double v = j.get<double>();
			j = Math::clamp(v + change, static_cast<double>(e.value_range.min), static_cast<double>(e.value_range.max));
		}
		else if (j.is_number_integer())
		{
			int v = j.get<int>();
			j = Math::clamp(v + static_cast<int>(change), e.value_range.min, e.value_range.max);
		}
	}
}

void Menu::input_text(Element& e, const int key)
{
	if (auto p = std::get_if<std::string*>(&e.value))
	{
		if (key == KEY_BACKSPACE && !(*p)->empty())
			(*p)->pop_back();
		else if (std::isprint(static_cast<unsigned char>(key)) && (*p)->size() < static_cast<size_t>(e.value_range.max))
			**p += key;
	}
	if (auto p = std::get_if<nlohmann::json*>(&e.value))
	{
		auto& j = **p;
		if (!j.is_string())
			Error::fatal("Json object in TextField is not string");
		std::string s = j.get<std::string>();
		if (key == KEY_BACKSPACE && !s.empty())
			s.pop_back();
		else if (std::isprint(static_cast<unsigned char>(key)) && s.size() < static_cast<size_t>(e.value_range.max))
			s += key;
		j = s;
	}
}

void Menu::set_bool(Element& e, const int key)
{
	if (key == '\n' || key == KEY_ENTER || key == KEY_LEFT_CLICK)
	{
		if (auto boolPtr = std::get_if<bool*>(&e.value))
			**boolPtr ^= true;
		else if (auto p = std::get_if<nlohmann::json*>(&e.value))
		{
			auto& j = **p;
			if (!j.is_boolean())
				Error::fatal("Json in Checkbox is not boolean");
			j = !j.get<bool>();
		}
		else
			Error::fatal("Checkbox value is not bool or json");
	}
}

void Menu::select_multi_choice(Element& e, const int key)
{
	const size_t max_idx = e.choices.size() - 1;
	std::string choice = "";
	if (auto p = std::get_if<std::string*>(&e.value))
		choice = **p;
	else if (auto p = std::get_if<nlohmann::json*>(&e.value))
		choice = (**p).get<std::string>();
	else Error::fatal("MultiChoice value not string or json");
	auto it = std::find(e.choices.begin(), e.choices.end(), choice);
	if (it == e.choices.end())
		Error::fatal("MultiChoice choice not in choices");
	size_t index = it - e.choices.begin();
	switch (key)
	{
		case KEY_LEFT:
			if (index > 0)
				index--;
			break;
		case KEY_RIGHT:
			if (index < max_idx)
				index++;
			break;
		default:
			break;
	}
	if (auto p = std::get_if<std::string*>(&e.value))
		**p = e.choices[index];
	else if (auto p = std::get_if<nlohmann::json*>(&e.value))
		**p = e.choices[index];
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
Menu::Selection Menu::get_selection(const size_t default_selected)
{
	int key = 0;
	size_t selected = default_selected + get_unselectable_count(); // selected will always be one of the selectable elements
	set_panel();
	while (key != KEY_ESCAPE)
	{
		show_elements(selected);
		if (get_unselectable_count() == elements.size())
			return Selection(); // This might or might not happen, probablyish not (always add some button)
		key = UI::instance().input(100); // delay is pretty short because want hover highlight to update more often
		selected = select_element(selected, key); // changes selected from keyboard or mouse hover
		if (elements[selected].type == Element::Type::ValueSelector)
			change_value(elements[selected], key);
		if (elements[selected].type == Element::Type::TextField)
			input_text(elements[selected], key);
		if (elements[selected].type == Element::Type::Checkbox)
			set_bool(elements[selected], key);
		if (elements[selected].type == Element::Type::MultiChoice)
			select_multi_choice(elements[selected], key);
		if (selection_confirmed(elements[selected], key)) // enter or left click on a Button
		{
			// index matters even with confirm and cancel
			const auto element = elements[selected];
			const auto index = selected - get_unselectable_count();
			Selection selection(index, element);
			if (element.is_cancel)
				selection.cancelled = true;
			if (element.is_confirm)
				selection.confirmed = true;
			return selection;
		}
		if (blocking == false) // combine this flag with outer loop to make nice interactive menu UI
		{
			Selection s(selected - get_unselectable_count());
			s.timed_out = true;
			return s;
		}
	}
	return Selection::cancel();
}

