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
	Menu::Selection get_selection(
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
		{
			Element e(Type::Button, label);
			if (label == "Back" || label == "Cancel" || label == "Quit")
				e.is_cancel = true;
			if (label == "Confirm" || label == "OK" || label == "Ok")
				e.is_confirm = true;
			dialog_box.add_element(e);
		}

		return dialog_box.get_selection(default_selected);
	}

	Menu::Selection get_selection(
			const std::string& text,
			const std::vector<std::string>& buttons,
			const Vec2& position,
			const size_t default_selected
			)
	{
		return get_selection(std::vector<std::string>{text}, buttons, position, default_selected);
	}

	bool confirm(const std::string& message)
	{
		Menu m(Screen::middle());
		m.add_element(Element(Type::Text, message));
		m.add_element(Element(Type::Text, "--"));
		m.add_element(Element::confirm());
		m.add_element(Element::cancel());
		return m.get_selection().confirmed;
	}

	void alert(const std::string& message)
	{
		get_selection(message, {"Ok"});
	}
};
