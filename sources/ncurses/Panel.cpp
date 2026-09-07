#include "ncurses/Panel.hpp"
#include "utils/Error.hpp"
#include "ncurses/Screen.hpp"

#include <ncurses.h>
#include <stdexcept>
#include <utility>

namespace Ncurses
{
	Panel::Panel(
		const int height,
		const int width,
		const int y,
		const int x)
		: window(height, width, y, x),
		  ptr(new_panel(window.get_ptr()))
	{
		if (!valid())
			Error::fatal("Failed to create ncurses panel");
		while (height > Ncurses::Screen::height() || width > Ncurses::Screen::width())
		{	// Use ctrl + [+/-] (in my terminal). This loop ends when terminal size is big enough.
			mvaddstr(0, 0, "Resize terminal with ctrl+[+/-]"); // TODO::add menu/panel scrolling
			refresh();
			getch();
		}
	}

	Panel::Panel() : Panel(LINES, COLS, 0, 0) {}

	Panel::~Panel()
	{
		if (ptr != nullptr)
			del_panel(ptr);
	}

	Panel::Panel(Panel&& other) noexcept
		: window(std::move(other.window)),
		  ptr(std::exchange(other.ptr, nullptr))
	{
	}

	Panel& Panel::operator=(Panel&& other) noexcept
	{
		if (this == &other)
			return *this;

		/*
		 * The current panel must be deleted before its Window is
		 * replaced and potentially destroyed.
		 */
		if (ptr != nullptr)
			del_panel(ptr);

		ptr = nullptr;
		window = std::move(other.window);
		ptr = std::exchange(other.ptr, nullptr);

		return *this;
	}

	PANEL* Panel::get_ptr() noexcept
	{
		return ptr;
	}

	const PANEL* Panel::get_ptr() const noexcept
	{
		return ptr;
	}

	Window& Panel::get_window() noexcept
	{
		return window;
	}

	const Window& Panel::get_window() const noexcept
	{
		return window;
	}

	void Panel::show()
	{
		show_panel(ptr);
	}

	void Panel::hide()
	{
		hide_panel(ptr);
	}

	bool Panel::hidden() const
	{
		return panel_hidden(ptr) != FALSE;
	}

	void Panel::bring_to_top()
	{
		top_panel(ptr);
	}

	void Panel::send_to_bottom()
	{
		bottom_panel(ptr);
	}

	void Panel::set_position(const int y, const int x)
	{
		move_panel(ptr, y, x);
	}

	Vec2<int> Panel::dimensions() const
	{
		return window.dimensions();
	}

	Vec2<int> Panel::position() const
	{
		return window.position();
	}
}
