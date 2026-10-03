#include <stddef.h>
#include "utils/Log.hpp"
#include <vector>
#include <string>

#include "ui/Dialog.hpp"
#include "ui/Menu.hpp"
#include "ncurses/Screen.hpp"
#include "utils/Vec2.hpp"

namespace UI::Dialog
{
	Selection get_selection(
			const std::string& title,
			const std::vector<std::string>& buttons,
			const Vec2<int>& position,
			const std::size_t default_selected)
	{
		Menu dialog_box(position, title);

		for (const auto& label : buttons)
		{
			UI::Element::Button button{.label = label};

			if (label == "Back" || label == "Cancel")
				button.role = UI::Element::Button::Role::Cancel;
			else if (
					label == "Confirm" ||
					label == "OK" ||
					label == "Ok" ||
					label == "Continue")
			{
				button.role = UI::Element::Button::Role::Confirm;
			}

			dialog_box.add(button);
		}

		UI::Selection selection;
		selection.index = default_selected;

		do
		{
			selection = dialog_box.get_selection(selection.index);
		}
		while (
				selection.state != Selection::State::Selected &&
				selection.state != Selection::State::Confirmed &&
				selection.state != Selection::State::Cancelled
			  );

		return selection;
	}

	bool confirm(const std::string& message)
	{
		Menu m(message);
		m.add(UI::Element::confirm());
		m.add(UI::Element::cancel());
		return m.get_selection().confirmed();
	}

	void alert(const std::string& message)
	{
		get_selection(message, {"Ok"});
	}

	void get_input(const std::string& label, std::string& input)
	{
		auto copy = input;

		Menu menu(label);
		UI::Element::TextIn text_in{.label = "", .text = &copy};
		text_in.confirm = true;
		menu.add(text_in);
		menu.set_timeout(-1);
		const auto s = menu.get_selection();
		if (s.cancelled())
			return;
		input = copy;
	}

	std::string get_input(const std::string& label)
	{
		std::string input;
		get_input(label, input);
		return input;
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
