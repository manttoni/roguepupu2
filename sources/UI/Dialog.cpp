#include <stddef.h>
#include <vector>
#include <string>

#include "UI/Dialog.hpp"
#include "UI/Menu.hpp"
#include "utils/Screen.hpp"

struct Vec2;

namespace Dialog
{
	using Element = Menu::Element;
	using Type = Element::Type;
	Element get_selection(
			const std::vector<std::string>& text,
			const std::vector<std::string>& buttons,
			const Vec2& position,
			const size_t default_selected)
	{
		Menu dialog_box(position);
		for (const auto& label : text)
			dialog_box.add_element(Element(Type::Text, label));

		if (!text.empty() && !buttons.empty())
			dialog_box.add_element(Element(Type::Text, "--"));

		for (const auto& label : buttons)
			dialog_box.add_element(Element(Type::Button, label));

		return dialog_box.get_selection(default_selected);
	}

	Menu::Element get_selection(
			const std::string& text,
			const std::vector<std::string>& buttons,
			const Vec2& position,
			const size_t default_selected
			)
	{
		return get_selection(std::vector<std::string>{text}, buttons, position, default_selected);
	}

	void show_message(const std::string& message)
	{
		Menu m(Screen::middle());
		m.add_element(Element(Type::Text, message));
		m.get_selection(); // badly named but it will not block if there are no options/buttons
	}
};
