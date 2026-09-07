#include <stddef.h>
#include <vector>
#include <string>

#include "ui/Dialog.hpp"
#include "ui/Menu.hpp"
#include "ncurses/Screen.hpp"
#include "utils/Vec2.hpp"

namespace UI::Dialog
{
	Selection get_selection(
			const std::vector<std::string>& text,
			const std::vector<std::string>& buttons,
			const Vec2<int>& position,
			const size_t default_selected)
	{
		Menu dialog_box(position);
		for (const auto& label : text)
			dialog_box.add(UI::Element::Text{
					.text = label
					});

		if (!text.empty() && !buttons.empty())
			dialog_box.add(UI::Element::Separator{});

		for (const auto& label : buttons)
		{
			UI::Element::Button e{.label = label};
			if (label == "Back" || label == "Cancel" || label == "Quit")
				e.role = UI::Element::Button::Role::Cancel;
			if (label == "Confirm" || label == "OK" || label == "Ok")
				e.role = UI::Element::Button::Role::Confirm;
			dialog_box.add(e);
		}

		auto selection = dialog_box.get_selection(default_selected);
		selection.index -= text.size() + 1;
		return selection;
	}

	Selection get_selection(
			const std::string& text,
			const std::vector<std::string>& buttons,
			const Vec2<int>& position,
			const size_t default_selected
			)
	{
		return get_selection(std::vector<std::string>{text}, buttons, position, default_selected);
	}

	bool confirm(const std::string& message)
	{
		Menu m(Ncurses::Screen::middle());
		m.add(UI::Element::Text{.text = message});
		m.add(UI::Element::Separator{});
		m.add(UI::Element::confirm());
		m.add(UI::Element::cancel());
		return m.get_selection().confirmed();
	}

	void alert(const std::string& message)
	{
		get_selection(message, {"Ok"});
	}

	Selection get_input(const std::string& label, std::string* input)
	{
		Menu menu(Ncurses::Screen::middle());
		menu.add(UI::Element::TextIn{.label = label, .text = input});
		return menu.get_selection();
	}

	void message(const std::string& message, const Vec2<int>& position)
	{
		Menu msg_menu(position);
		msg_menu.add(UI::Element::Text{.text = message});
		msg_menu.get_selection();
	}

	void message(const std::vector<std::string>& messages, const Vec2<int>& position)
	{
		Menu msg_menu(position);
		for (const auto& message : messages)
			msg_menu.add(UI::Element::Text{.text = message});
		msg_menu.get_selection();
	}
};
