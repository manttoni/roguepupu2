#include "UI/Dialog.hpp"
#include "UI/Menu.hpp"

namespace Dialog
{
	Menu::Element get_selection(
			const std::vector<std::string>& text,
			const std::vector<std::string>& buttons = {},
			const Vec2& position = Screen::middle(),
			const size_t default_selected = 0)
	{
		Menu dialog_box(position);
		for (const auto& label : text)
			dialog_box.add_element(Menu::Element(Menu::Type::Text, label));

		if (!text.empty() && !options.empty())
			dialog_box.add_element(Menu::Element(Menu::Type::Text, "--"));

		for (const auto& label : buttons)
			dialog_box.add_element(Menu::Element(Menu::Type::Button, label));

		return dialog_box.get_selection(default_selected);
	}
};
