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

#include "ncurses/Screen.hpp"
#include "ui/Element.hpp"
#include "ui/Format.hpp"
#include "ui/Menu.hpp"
#include "ui/Menu.hpp"
#include "utils/Log.hpp"
#include "utils/Math.hpp"
#include "utils/Math.hpp"
#include "utils/Parser.hpp"
#include "utils/Utils.hpp"
#include "utils/Vec2.hpp"

namespace UI
{
	Menu::Menu(const Vec2<int>& position, const std::string& title) : position(position), title(title)
	{}

	Menu::Menu(const std::string& title) : position(Ncurses::Screen::middle()), title(title)
	{}

	void Menu::reset_panel()
	{
		const auto formatted = Format::elements(elements);
		const auto longest = std::max_element(
				formatted.begin(),
				formatted.end(),
				[](const auto& a, const auto& b)
				{
					return a.size() < b.size();
				});
		const auto longest_length =
			longest == formatted.end() ? 0 : longest->size();

		const int height = 2 + elements.size();
		const int width  = 4 + std::max(longest_length, title.size() + 2);
		const int start_y = std::max(0, position.y - height / 2);
		const int start_x = std::max(0, position.x - width / 2);

		panel = Ncurses::Panel(height, width, start_y, start_x);
		panel.get_window().set_title(title);
		assert(panel.valid());
	}

	void Menu::add(const Element::Any& element)
	{
		elements.push_back(element);
		changed_ = true;
	}

	void Menu::add(const std::vector<Element::Any>& elements)
	{
		for (const auto& element : elements)
			add(element);
	}

	void Menu::print_elements(const size_t selected)
	{
		const auto lines = Format::elements(elements);

		Ncurses::Window& surface = panel.get_window();
		surface.clear();
		surface.enable_color(theme.text);

		for (size_t i = 0; i < lines.size(); ++i)
		{
			if (selected == i) surface.enable_attribute(Ncurses::Attribute(A_REVERSE));
			surface.write(i + 1, 2, lines[i]);
			if (selected == i) surface.disable_attribute(Ncurses::Attribute(A_REVERSE));
		}
		surface.disable_color(theme.text);
		surface.enable_color(theme.border);
		surface.draw_border();
		surface.disable_color(theme.border);
		surface.refresh(); // does this do anything in this context?
		update_panels();
		doupdate();
	}

	std::string Menu::get_label(const std::size_t index) const
	{
		if (index >= elements.size())
			return "";

		return std::visit(
				[](const auto& element) -> std::string
				{
					if constexpr (requires { element.label; })
						return element.label;
					else
						return "";
				},
				elements[index]
				);
	}

	Selection Menu::handle_input(const std::size_t selected, const Ncurses::Input::Event& event)
	{
		const auto state = std::visit([&event](auto& element) {
			return Element::handle_input(element, event);
		}, elements.at(selected));

		return Selection{
			.state = state,
			.index = selected,
			.label = get_label(selected)
		};
	}

	Selection Menu::get_selection(size_t selected)
	{
		selected = Math::clamp<size_t>(selected, 0, elements.size() - 1);
		changed_ = false;
		reset_panel();
		while (true)
		{
			print_elements(selected);
			const Ncurses::Input::Event event =
				Ncurses::Input::get_event(timeout);
			using Key = Ncurses::Input::Key;
			switch (event.key)
			{
				case Key::None: // getting input timed out -> no key was pressed
					return Selection{
						.state = Selection::State::TimedOut,
						.index = selected
					};
				case Key::Up:
					selected = (selected == 0) ? 0 : selected - 1;
					break;
				case Key::Down:
					selected = (selected == elements.size() - 1) ? selected : selected + 1;
					break;
				case Key::Escape:
					return Selection{
						.state = Selection::State::Cancelled
					};
				default:
					{
						const Selection selection = handle_input(selected, event);
						if (selection.state == Selection::State::Changed)
						{
							changed_ = true;
							Log::debug() << "\'" << selection.label << "\' changed";
						}
						if (selection.selected() || selection.confirmed() || selection.cancelled() || selection.state == Selection::State::MultiChoice)
							return selection;
					}
					break;
			}
		}
	}
}

