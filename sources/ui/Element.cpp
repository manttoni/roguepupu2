#include <string>
#include "utils/Math.hpp"
#include "ui/Element.hpp"

namespace UI::Element
{
	std::string to_string(const Null&)
	{
		return "Null";
	}
	std::string to_string(const Text& element)
	{
		return element.text;
	}
	std::string to_string(const TextOut& element)
	{
		const auto* text = element.text;
		return element.label + " : " + (text ? *text : "nullptr");
	}
	std::string to_string(const TextIn& element)
	{
		const auto* text = element.text;
		return element.label + " : " + (text ? *text : "nullptr") + "[A_BLINK]_[RESET]";
	}

	std::string to_string(const Button& element)
	{
		return element.label;
	}
	std::string to_string(const Checkbox& element)
	{
		return element.label + " : " + (*(element.value) ? "[X]" : "[ ]");
	}

	std::string to_string(const Separator& element)
	{
		return std::string(element.size, element.ch);
	}

	Selection::State handle_input(Null&, const Ncurses::Input::Event&) { return Selection::State::Error; }
	Selection::State handle_input(Text&, const Ncurses::Input::Event&) { return Selection::State::Error; }
	Selection::State handle_input(TextOut&, const Ncurses::Input::Event&) { return Selection::State::Error; }
	Selection::State handle_input(TextIn& element, const Ncurses::Input::Event& event)
	{
		using Key = Ncurses::Input::Key;
		auto& text = *(element.text);
		if (event.key == Key::Backspace && text.size() > 0)
		{
			text.pop_back();
		}
		else if (Ncurses::Input::is_ascii(event.key) && text.size() < element.length.max)
		{
			text += event.ch;
		}
		else
			return Selection::State::Ignored;
		return Selection::State::Changed;
	}

	Selection::State handle_input(Button& element, const Ncurses::Input::Event& event)
	{
		using Key = Ncurses::Input::Key;
		using Role = Button::Role;

		if (event.key != Key::Enter)
			return Selection::State::Ignored;
		switch (element.role)
		{
			case Role::Normal:
				return Selection::State::Selected;
			case Role::Confirm:
				return Selection::State::Confirmed;
			case Role::Cancel:
				return Selection::State::Cancelled;
		}
	}
	Selection::State handle_input(Checkbox& element, const Ncurses::Input::Event& event)
	{
		using Key = Ncurses::Input::Key;

		if (event.key != Key::Enter)
			return Selection::State::Ignored;

		*(element.value) ^= 1;
		return Selection::State::Changed;
	}


	Selection::State handle_input(Separator&, const Ncurses::Input::Event&) { return Selection::State::Error; }

	std::string to_string(const Any& element)
	{
		return std::visit(
				[](const auto& value)
				{
				return to_string(value);
				},
				element
				);
	}
	std::string get_label(const Any& element)
	{
		return std::visit(
				[](const auto& value) -> const std::string&
				{
				return value.label;
				},
				element
				);
	}
}
