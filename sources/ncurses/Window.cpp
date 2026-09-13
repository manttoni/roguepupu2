#include "ncurses/Window.hpp"

#include <limits>
#include <stdexcept>
#include <utility>

#include "utils/Log.hpp"

namespace
{
	int ncurses_length(const std::size_t size)
	{
		const auto max = static_cast<std::size_t>(
			std::numeric_limits<int>::max());

		if (size > max)
			throw std::length_error("Text is too long for ncurses");

		return static_cast<int>(size);
	}
}

namespace Ncurses
{
	Window::Window(
		const std::size_t height,
		const std::size_t width,
		const std::size_t y,
		const std::size_t x)
		: ptr(newwin(
			static_cast<int>(height),
			static_cast<int>(width),
			static_cast<int>(y),
			static_cast<int>(x)))
	{
		if (ptr == nullptr)
			throw std::runtime_error("Failed to create ncurses window");

		keypad(ptr, true);
	}

	Window::~Window()
	{
		if (ptr != nullptr)
			delwin(ptr);
	}

	Window::Window(Window&& other) noexcept
		: ptr(std::exchange(other.ptr, nullptr))
	{
	}

	Window& Window::operator=(Window&& other) noexcept
	{
		if (this == &other)
			return *this;

		if (ptr != nullptr)
			delwin(ptr);

		ptr = std::exchange(other.ptr, nullptr);

		return *this;
	}

	WINDOW* Window::get_ptr() noexcept
	{
		return ptr;
	}

	const WINDOW* Window::get_ptr() const noexcept
	{
		return ptr;
	}

	void Window::put(
		const int y,
		const int x,
		const char character)
	{
		wmove(ptr, y, x);
		waddch(ptr, static_cast<unsigned char>(character));
	}

	void Window::put(const char character)
	{
		waddch(ptr, static_cast<unsigned char>(character));
	}

	void Window::write(
		const int y,
		const int x,
		const std::string& text)
	{
		wmove(ptr, y, x);
		waddnstr(ptr, text.data(), ncurses_length(text.size()));
	}

	void Window::write(const std::string& text)
	{
		waddnstr(ptr, text.data(), ncurses_length(text.size()));
	}

	void Window::clear()
	{
		werase(ptr);
	}

	void Window::draw_border()
	{
		box(ptr, 0, 0);
		write(0, 2, " " + title + " ");
	}

	void Window::refresh()
	{
		wrefresh(ptr);
	}

	void Window::stage_refresh()
	{
		wnoutrefresh(ptr);
	}

	void Window::enable_attribute(const chtype attribute)
	{
		wattron(ptr, attribute);
	}

	void Window::disable_attribute(const chtype attribute)
	{
		wattroff(ptr, attribute);
	}

	void Window::enable_color(const Color& color)
	{
		wattron(ptr, COLOR_PAIR(static_cast<int>(color.id())));
	}

	void Window::disable_color(const Color& color)
	{
		wattroff(ptr, COLOR_PAIR(static_cast<int>(color.id())));
	}

	Vec2<int> Window::dimensions() const
	{
		int height = 0;
		int width = 0;

		getmaxyx(ptr, height, width);

		return {height, width};
	}

	Vec2<int> Window::position() const
	{
		int y = 0;
		int x = 0;

		getbegyx(ptr, y, x);

		return {y, x};
	}

	int Window::cursor_y() const
	{
		int y = 0;
		int x = 0;

		getyx(ptr, y, x);
		(void)x;

		return y;
	}

	int Window::cursor_x() const
	{
		int y = 0;
		int x = 0;

		getyx(ptr, y, x);
		(void)y;

		return x;
	}
}
