#pragma once

#include <nlohmann/json.hpp>
#include <variant>
#include <vector>
#include "utils/Range.hpp"
#include "ui/Selection.hpp"
#include "ncurses/Input.hpp"
#include "utils/Math.hpp"

namespace UI::Element
{
	using Json = nlohmann::json;

	struct Null
	{
		std::string label = "Null";
	};
	std::string to_string(const Null&);
	Selection::State handle_input(Null&, const Ncurses::Input::Event&);

	struct Text // uses whole width
	{
		std::string text;
	};
	std::string to_string(const Text& element);
	Selection::State handle_input(Text&, const Ncurses::Input::Event&);

	struct TextOut
	{
		std::string label;
		const std::string* text = nullptr;
	};
	std::string to_string(const TextOut& element);
	Selection::State handle_input(TextOut&, const Ncurses::Input::Event&);

	struct TextIn
	{
		std::string label;
		std::string* text = nullptr;
		Range<std::size_t> length{1, 10};
	};
	std::string to_string(const TextIn& element);
	Selection::State handle_input(TextIn& element, const Ncurses::Input::Event& event);

	template<typename T> struct ValueSelector
	{
		std::string label;
		T* value = nullptr;
		Range<T> range;
		T step{1};
	};
	template<typename T> std::string to_string(const ValueSelector<T>& element)
	{
		const auto left = *(element.value) == element.range.min ? "  " : "< ";
		const auto middle = std::to_string(*(element.value));
		const auto right = *(element.value) == element.range.max ? "  " : " >";
		return element.label + " : " + left + middle + right;
	}
	template<typename T>
		Selection::State handle_input(
				ValueSelector<T>& element,
				const Ncurses::Input::Event& event)
		{
			using Key = Ncurses::Input::Key;

			if (event.key != Key::Left && event.key != Key::Right)
				return Selection::State::Ignored;

			const T multiplier = event.shift ? T{10} : T{1};
			const T amount = element.step * multiplier;

			const T current = *element.value;
			T next = current;

			if (event.key == Key::Left)
			{
				if (current <= element.range.min ||
						amount >= current - element.range.min)
				{
					next = element.range.min;
				}
				else
				{
					next = current - amount;
				}
			}
			else
			{
				if (current >= element.range.max ||
						amount >= element.range.max - current)
				{
					next = element.range.max;
				}
				else
				{
					next = current + amount;
				}
			}

			if (current == next)
				return Selection::State::Ignored;

			*element.value = next;
			return Selection::State::Changed;
		}

	struct Button
	{
		enum class Role { Normal, Cancel, Confirm };
		std::string label;
		Role role = Role::Normal;
	};
	std::string to_string(const Button& element);
	Selection::State handle_input(Button& element, const Ncurses::Input::Event& event);

	struct Checkbox
	{
		std::string label;
		bool* value = nullptr;
	};
	std::string to_string(const Checkbox& element);
	Selection::State handle_input(Checkbox& element, const Ncurses::Input::Event& event);

	template<typename T> struct SingleChoice
	{
		std::string label;
		T* chosen = nullptr;
		std::vector<T> choices;
	};
	template<typename T> std::string to_string(const SingleChoice<T>& element)
	{
		return element.label + " : " + "[" + *(element.chosen) + "]";
	}
	template<typename T>
		Selection::State handle_input(
				SingleChoice<T>& element,
				const Ncurses::Input::Event& event)
		{
			using Key = Ncurses::Input::Key;

			if (element.chosen == nullptr || element.choices.empty())
				return Selection::State::Error;

			const auto it = std::find(
					element.choices.begin(),
					element.choices.end(),
					*element.chosen
					);

			if (it == element.choices.end())
				return Selection::State::Error;

			const std::size_t size = element.choices.size();
			const std::size_t index =
				std::distance(element.choices.begin(), it);

			std::size_t new_index;

			if (event.key == Key::Left)
				new_index = (index + size - 1) % size;
			else if (event.key == Key::Right)
				new_index = (index + 1) % size;
			else
				return Selection::State::Ignored;

			if (new_index == index)
				return Selection::State::Ignored;

			*element.chosen = element.choices[new_index];
			return Selection::State::Changed;
		}

	template<typename T> struct MultiChoice // choose any
	{
		std::string label;
		std::vector<T>* chosen = nullptr;
		std::vector<T> choices;
	};
	template<typename T> std::string to_string(const MultiChoice<T>& element)
	{
		return element.label + " : " + "[" + std::to_string(element.chosen->size()) + " chosen]";
	}
	template<typename T> Selection::State handle_input(MultiChoice<T>&, const Ncurses::Input::Event& event)
	{
		using Key = Ncurses::Input::Key;

		if (event.key != Key::Enter)
			return Selection::State::Ignored;

		return Selection::State::MultiChoice;
	}

	struct Separator
	{
		std::string label = ""; // Unused
		char ch = '-';
		size_t size = 3;
	};
	std::string to_string(const Separator& element);
	Selection::State handle_input(Separator&, const Ncurses::Input::Event&);

	using Any = std::variant<
		Null,
		Text,
		TextOut,
		TextIn,
		ValueSelector<int>,
		ValueSelector<double>,
		ValueSelector<Json::number_integer_t>,
		ValueSelector<Json::number_unsigned_t>,
		Button,
		Checkbox,
		SingleChoice<std::string>,
		MultiChoice<std::string>,
		Separator
			>;
	std::string to_string(const Any& element);
	std::string get_label(const Any& element);
	static inline Button confirm() { return Button{.label = "Confirm", .role = Button::Role::Confirm}; }
	static inline Button cancel() { return Button{.label = "Cancel", .role = Button::Role::Cancel}; }
}
