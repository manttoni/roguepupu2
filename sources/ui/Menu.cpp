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

#include "ui/Menu.hpp"
#include "utils/Error.hpp"
#include "ncurses/Screen.hpp"
#include "ui/Menu.hpp"
#include "utils/Vec2.hpp"
#include "utils/Utils.hpp"
#include "utils/Math.hpp"
#include "utils/Parser.hpp"
#include "ui/Element.hpp"

namespace UI
{
	Menu::Menu(const Vec2<int>& position, const std::string& title) : position(position), title(title)
	{}

	void Menu::reset_panel()
	{
		const auto longest = std::max_element(
				elements.begin(),
				elements.end(),
				[](const auto& a, const auto& b)
				{
				return Element::to_string(a).size()
				< Element::to_string(b).size();
				}
				);

		const std::size_t longest_length =
			longest == elements.end()
			? 0
			: Element::to_string(*longest).size();

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
	}

	void Menu::add(const std::vector<Element::Any>& elements)
	{
		for (const auto& element : elements)
			add(element);
	}

	void Menu::print_elements(const size_t selected)
	{
		Ncurses::Window& surface = panel.get_window();
		surface.clear();
		surface.enable_color(theme.text);
		for (size_t i = 0; i < elements.size(); ++i)
		{
			if (selected == i) surface.enable_attribute(Ncurses::Attribute(A_REVERSE));
			std::visit( [&surface, i](const auto& e) { surface.write(i + 1, 2, Element::to_string(e)); }, elements[i] );
			if (selected == i) surface.disable_attribute(Ncurses::Attribute(A_REVERSE));
		}
		surface.disable_color(theme.text);
		surface.enable_color(theme.border);
		surface.draw_border();
		surface.disable_color(theme.border);
		surface.refresh();
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
		Log::debug()
			<< "Dispatching element index " << selected
			<< ", variant index " << elements[selected].index();
		const auto state = std::visit([&event](auto& element) {
				Log::debug()
				<< "Visitor reached for: "
				<< element.label;
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
		reset_panel();
		while (true)
		{
			print_elements(selected);
			const Ncurses::Input::Event event = Ncurses::Input::get_event(timeout);
			Log::debug()
				<< "Menu event: key=" << static_cast<int>(event.key)
				<< ", ch=" << static_cast<int>(
						static_cast<unsigned char>(event.ch)
						);
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
						if (selection.selected() || selection.confirmed() || selection.cancelled())
							return selection;
						else
							Log::debug() << "Handle input result was not selected, confirmed or cancelled";
					}
					break;
			}
		}
	}
}

