#pragma once

#include <vector>

#include "ncurses/Panel.hpp"
#include "ui/Element.hpp"
#include "ncurses/Screen.hpp"
#include "utils/Vec2.hpp"
#include "ui/Theme.hpp"

namespace UI
{
	class Menu
	{
		private:
			Vec2<int> position;
			std::string title;
			std::vector<Element::Any> elements;
			Ncurses::Panel panel;
			int timeout = -1; // -1 is default and blocking
			void reset_panel();
			void print_elements(const size_t selected);
			Selection handle_input(const std::size_t selected, const Ncurses::Input::Event& event);
			UI::Theme theme = UI::load_theme();

		public:
			// Center the menu as close to position as possible.
			explicit Menu(const Vec2<int>& position = Ncurses::Screen::middle(), const std::string& title = "");

			~Menu() = default;

			Menu(const Menu&) = delete;
			Menu& operator=(const Menu&) = delete;

			Menu(Menu&&) noexcept = default;
			Menu& operator=(Menu&&) noexcept = default;

			void add(const Element::Any& element);
			void add(const std::vector<Element::Any>& elements);

			Selection get_selection(size_t default_selected = 0);

			void set_timeout(const int value)
			{
				timeout = value;
			}
	};
}
