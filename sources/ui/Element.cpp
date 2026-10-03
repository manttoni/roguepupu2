#include <string>
#include "ncurses/Input.hpp"
#include "utils/Log.hpp"
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
		return
			element.label +
			(element.label.empty() ? "" : " : ") +
			(text ? *text : "nullptr") +
			(text->size() < element.length.max ? "_" + std::string(element.length.max - (text->size() + 1), ' ') : "");
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

		switch (event.key)
		{
			case Key::Backspace:
				if (text.size() > 0)
				{
					text.pop_back();
					return Selection::State::Changed;
				}
				return Selection::State::Ignored;
			case Key::Enter:
				if (element.confirm && text.size() >= element.length.min)
					return Selection::State::Confirmed;
				return Selection::State::Ignored;
			case Key::Space:
			case Key::Number:
			case Key::Alphabetic:
			case Key::Symbol:
				if (text.size() < element.length.max)
				{
					text += event.ch;
					return Selection::State::Changed;
				}
				return Selection::State::Ignored;
			default:
				return Selection::State::Ignored;
		}
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
				[](const auto& value) -> std::string
				{
				if constexpr (requires { value.label; })
				return value.label;
				else
				return "";
				},
				element
				);
	}
}
