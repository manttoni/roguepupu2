#pragma once

#include <cstddef>
#include <string_view>

#include <ncurses.h>

#include "utils/Vec2.hpp"
#include "ncurses/Color.hpp"

namespace Ncurses
{
	class Window
	{
		private:
			WINDOW* ptr = nullptr;
			std::string title = "";

		public:
			Window() = default;
			Window(
					std::size_t height,
					std::size_t width,
					std::size_t y,
					std::size_t x);

			~Window();

			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;

			Window(Window&& other) noexcept;
			Window& operator=(Window&& other) noexcept;

			WINDOW* get_ptr() noexcept;
			const WINDOW* get_ptr() const noexcept;

			std::string get_title() const { return title; }
			void set_title(const std::string& title) { this->title = title; }

			void put(int y, int x, char character);
			void put(char character);

			void write(const int y, const int x, const std::string& text);
			void write(const std::string& text);

			void clear();
			void draw_border();

			void refresh();
			void stage_refresh();

			void enable_attribute(const chtype attribute);
			void disable_attribute(const chtype attribute);

			void enable_color(const Color& color);
			void disable_color(const Color& color);

			Vec2<int> dimensions() const;
			Vec2<int> position() const;

			int cursor_y() const;
			int cursor_x() const;
	};
}
